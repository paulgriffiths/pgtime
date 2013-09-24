/*!
 * \file        pgtime.c
 * \brief       Implementation of miscellaneous time functions.
 * \author      Paul Griffiths
 * \copyright   Copyright 2013 Paul Griffiths. Distributed under the terms
 * of the GNU General Public License. <http://www.gnu.org/licenses/>
 */


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>
#include "pgtime.h"


/*!
 * \brief           Checks whether a supplied date is valid.
 * \details         This function does not support leap seconds, and will
 * return false if `check_tm->tm_sec == 60`.
 * \param check_tm  A pointer to a struct tm containing the date to check.
 * \returns         true if the date if valid, false otherwise.
 */

bool
validate_date(const struct tm *check_tm) {
    static const int days_in_month[] = {31, 28, 31, 30, 31, 30,
                                        31, 31, 30, 31, 30, 31};

    if ( ( check_tm->tm_year == -1900 ) ||
         ( check_tm->tm_mon < 0 || check_tm->tm_mon > 11 ) ||
         ( check_tm->tm_mday < 1 ) ||
         ( check_tm->tm_mday > days_in_month[check_tm->tm_mon] &&
                !(check_tm->tm_mon == 1 &&
                  check_tm->tm_mday == 29 &&
                  is_leap_year(check_tm->tm_year + 1900)) ) ||
         ( check_tm->tm_hour < 0 || check_tm->tm_hour > 23 ) ||
         ( check_tm->tm_min < 0 || check_tm->tm_min > 59 ) ||
         ( check_tm->tm_sec < 0 || check_tm->tm_sec > 59 ) ) {
        return false;
    }
    return true;
}


/*!
 * \brief       Checks if a UTC timestamp is accurate.
 * \details     Checks if a UTC timestampe is accurate. A time_t timestamp
 * is computed from the supplied datetime, and compared to the
 * supplied time_t timestamp. The difference between the two, in seconds,
 * is stored in the supplied secs_diff argument. This function is needed
 * because the methodology used to calculate a timestamp by this library
 * can sometimes be inaccurate when leap seconds or other unpredictable
 * calendar changes occur. We therefore need a method to check if the
 * returned timestamp is accurate. Other functions provided in this
 * library call this function, so the user should not normally need to
 * call it.
 * \param check_time The time_t timestamp to check
 * \param secs_diff Modified to contain the difference, in seconds
 * \param check_tm  A pointer to a struct tm containing the date to check.
 * \returns     true if the supplied timestamp is accurate, false otherwise
 */

bool
check_utc_timestamp(const time_t check_time, int * secs_diff,
                    const struct tm *check_tm) {
    struct tm *ptm = gmtime(&check_time);
    if ( ptm == 0 ) {
        fprintf(stderr, "pgtime:%s:%d: couldn't get UTC time.\n",
                __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    struct tm compare_tm = *ptm;

    bool agrees = tm_compare(check_tm, &compare_tm) ? false : true; 
    if ( agrees ) {
        *secs_diff = 0;
    } else {
        *secs_diff = tm_intraday_secs_diff(check_tm, &compare_tm);
    }

    return agrees;
}


/*!
 * \brief       Returns a time_t interval representing one day.
 * \details     Returns a time_t interval representing one day. The C
 * standard does not define the units in which a time_t value is measured.
 * On POSIX-compliant systems it is measured in seconds, but we cannot
 * assume this for full portability.
 * \returns     A time_t interval representing one day.
 * \throws      bad_time if the current time cannot be obtained.
 */

/*
 *  The function works by setting up two struct tms a day apart and
 *  calculating the difference between the values that mktime() yields.
 *
 *  We've picked January 2 and January 3 as the dates to use, since
 *  we're likely clear of any DST or other weirdness on these dates.
 *  Since mktime() will modify the struct we pass to it if it represents
 *  a bad date, and since we reuse it, it should be good anyway.
 *
 *  The get_hour_diff() and get_sec_diff() functions work in a similar way.
 */

time_t
get_day_diff(void) {
    struct tm datum_day;
    datum_day.tm_sec = 0;
    datum_day.tm_min = 0;
    datum_day.tm_hour = 12;
    datum_day.tm_mday = 2;
    datum_day.tm_mon = 0;
    datum_day.tm_year = 30;
    datum_day.tm_isdst = -1;

    const time_t datum_time = mktime(&datum_day);
    if ( datum_time == -1 ) {
        fprintf(stderr, "pgtime:%s:%d: couldn't get calendar time.\n",
                __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    datum_day.tm_mday += 1;

    const time_t tomorrow_time = mktime(&datum_day);
    if ( tomorrow_time == -1 ) {
        fprintf(stderr, "pgtime:%s:%d: couldn't get calendar time.\n",
                __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    return (tomorrow_time - datum_time);
}


/*!
 * \brief       Returns a time_t interval representing one hour.
 * \details     Returns a time_t interval representing one hour. The C
 * standard does not define the units in which a time_t value is measured.
 * On POSIX-compliant systems it is measured in seconds, but we cannot
 * assume this for full portability.
 * \returns     A time_t interval representing one hour.
 * \throws      bad_time if the current time cannot be obtained.
 */

time_t
get_hour_diff(void) {
    struct tm datum_day;
    datum_day.tm_sec = 0;
    datum_day.tm_min = 0;
    datum_day.tm_hour = 12;
    datum_day.tm_mday = 2;
    datum_day.tm_mon = 0;
    datum_day.tm_year = 30;
    datum_day.tm_isdst = -1;

    const time_t datum_time = mktime(&datum_day);
    if ( datum_time == -1 ) {
        fprintf(stderr, "pgtime:%s:%d: couldn't get calendar time.\n",
                __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    datum_day.tm_hour += 1;

    const time_t nexthour_time = mktime(&datum_day);
    if ( nexthour_time == -1 ) {
        fprintf(stderr, "pgtime:%s:%d: couldn't get calendar time.\n",
                __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    return (nexthour_time - datum_time);
}


/*!
 * \brief       Returns a time_t interval representing one second.
 * \details     Returns a time_t interval representing one second. The C
 * standard does not define the units in which a time_t value is measured.
 * On POSIX-compliant systems it is measured in seconds, but we cannot
 * assume this for full portability.
 * \returns     A time_t interval representing one second.
 * \throws      bad_time if the current time cannot be obtained.
 */

time_t
get_sec_diff(void) {
    struct tm datum_day;
    datum_day.tm_sec = 0;
    datum_day.tm_min = 0;
    datum_day.tm_hour = 12;
    datum_day.tm_mday = 2;
    datum_day.tm_mon = 0;
    datum_day.tm_year = 30;
    datum_day.tm_isdst = -1;

    const time_t datum_time = mktime(&datum_day);
    if ( datum_time == -1 ) {
        fprintf(stderr, "pgtime:%s:%d: couldn't get calendar time.\n",
                __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    datum_day.tm_sec += 1;

    const time_t nextsec_time = mktime(&datum_day);
    if ( nextsec_time == -1 ) {
        fprintf(stderr, "pgtime:%s:%d: couldn't get calendar time.\n",
                __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    return (nextsec_time - datum_time);
}


/*!
 * \brief       Compares two struct tm structs.
 * \details     Compares two struct tm structs. Only the year, month, day,
 * hour, minute and second are compared. Any timezone or DST information
 * is ignored.
 * \param first The first struct tm struct.
 * \param second The second struct tm struct.
 * \returns     -1 if `first` is earlier than `second`, 1 if `first` is later
 * than `second`, and 0 if `first` is equal to `second`.
 */

int
tm_compare(const struct tm *first, const struct tm *second) {
    int compare_result;

    if ( first->tm_year != second->tm_year ) {
        compare_result = first->tm_year > second->tm_year ? 1 : -1;
    } else if ( first->tm_mon != second->tm_mon ) {
        compare_result = first->tm_mon > second->tm_mon ? 1 : -1;
    } else if ( first->tm_mday != second->tm_mday ) {
        compare_result = first->tm_mday > second->tm_mday ? 1 : -1;
    } else if ( first->tm_hour != second->tm_hour ) {
        compare_result = first->tm_hour > second->tm_hour ? 1 : -1;
    } else if ( first->tm_min != second->tm_min ) {
        compare_result = first->tm_min > second->tm_min ? 1 : -1;
    } else if ( first->tm_sec != second->tm_sec ) {
        compare_result = first->tm_sec > second->tm_sec ? 1 : -1;
    } else {
        compare_result = 0;
    }

    return compare_result;
}


/*!
 * \brief       Returns the difference between two struct tm structs.
 * \details     Returns the difference between two struct tm structs. The
 * structs are assumed to be within 24 hours of each other, and if they
 * are not, the returned result is computed as if they were. For instance,
 * comparing 10:00 on one day to 14:00 on the next day will yield a
 * difference equivalent to 4 hours, not to 28 hours.
 * \param first The first struct tm struct
 * \param second The second struct tm struct
 * \returns The difference, in seconds, between the two struct tm structs.
 * The difference is positive if `first` is earlier than `second`, and
 * negative if `second` is earlier than `first`.
 */

int
tm_intraday_secs_diff(const struct tm *first, const struct tm *second) {
    static const int secs_in_day = 86400;
    static const int secs_in_hour = 3600;
    static const int secs_in_min = 60;

    const int time_comp = tm_compare(first, second);
    int difference = 0;

    if ( time_comp == 0 ) {
        difference = 0;
    } else {
        difference = (second->tm_hour - first->tm_hour) * secs_in_hour;
        difference += (second->tm_min - first->tm_min) * secs_in_min;
        difference += (second->tm_sec - first->tm_sec);

        if ( time_comp == 1 && difference > 0 ) {
            difference -= secs_in_day;
        } else if ( time_comp == -1 && difference < 0 ) {
            difference += secs_in_day;
        }
    }
    
    return difference;
}


/*!
 * \brief           Checks if the supplied year is a leap year.
 * \details         Checks if the supplied year is a leap year.
 * \param year      A year
 * \returns         `true` if `year` is a leap year, `false` otherwise.
 */

bool
is_leap_year(const int year) {
    bool leap_year;
    if ( year % 4 == 0 &&
         (year % 100 != 0 ||
          year % 400 == 0) ) {
        leap_year = true;
    } else {
        leap_year = false;
    }
    return leap_year;
}


/*!
 * \brief               Adds one or more days to a struct tm, incrementing
 * the month and/or the year as necessary.
 * \param changing_tm   A pointer to the struct tm to increment. The
 * struct referred to by the pointer is modified by the function.
 * \param quantity      The number of days to add
 * \returns             A pointer to the same struct tm.
 */

struct tm*
tm_increment_day(struct tm *changing_tm, const int quantity) {
    enum months {january, february, march, april, may, june, july,
                 august, september, october, november, december};

    if ( quantity < 0 ) {
        tm_decrement_day(changing_tm, -quantity);
    } else {
        int num_days = quantity;

        while ( num_days-- ) {
            changing_tm->tm_mday += 1;
            switch ( changing_tm->tm_mon ) {
                case january:
                case march:
                case may:
                case july:
                case august:
                case october:
                    if ( changing_tm->tm_mday > 31 ) {
                        changing_tm->tm_mday = 1;
                        changing_tm->tm_mon += 1;
                    }
                    break;

                case december:
                    if ( changing_tm->tm_mday > 31 ) {
                        changing_tm->tm_mday = 1;
                        changing_tm->tm_mon = 0;
                        if ( changing_tm->tm_year == -1 ) {
                            changing_tm->tm_year = 1;
                        } else {
                            changing_tm->tm_year += 1;
                        }
                    }
                    break;

                case april:
                case june:
                case september:
                case november:
                    if ( changing_tm->tm_mday > 30 ) {
                        changing_tm->tm_mday = 1;
                        changing_tm->tm_mon +=1;
                    }
                    break;

                case 1:
                    if ( changing_tm->tm_mday > 29 ) {
                        changing_tm->tm_mday = 1;
                        changing_tm->tm_mon += 1;
                    } else if ( changing_tm->tm_mday > 28 &&
                                !is_leap_year(changing_tm->tm_year) ) {
                        changing_tm->tm_mday = 1;
                        changing_tm->tm_mon += 1;
                    }
                    break;

                default:
                    assert(false);
                    break;
            }
        }
    }

    return changing_tm;
}


/*!
 * \brief               Adds one or more hours to a struct tm, incrementing
 * the day, month and/or the year as necessary.
 * \param changing_tm   A pointer to the struct tm to increment. The
 * struct referred to by the pointer is modified by the function.
 * \param quantity      The number of hours to add
 * \returns             A pointer to the same struct tm.
 */

struct tm*
tm_increment_hour(struct tm *changing_tm, const int quantity) {
    static const int hours_in_day = 24;

    if ( quantity < 0 ) {
        tm_decrement_hour(changing_tm, -quantity);
    } else {
        int num_hours = quantity;

        if ( num_hours >= hours_in_day ||
             num_hours >= hours_in_day - changing_tm->tm_hour ) {
            int num_days = quantity / hours_in_day;
            num_hours -= num_days * hours_in_day;
            if ( num_hours >= hours_in_day - changing_tm->tm_hour ) {
                ++num_days;
                num_hours -= hours_in_day - changing_tm->tm_hour;
                changing_tm->tm_hour = num_hours;
            }
            tm_increment_day(changing_tm, num_days);
        }

        changing_tm->tm_hour += num_hours;
    }

    return changing_tm;
}


/*!
 * \brief               Adds one or more minutes to a struct tm, incrementing
 * the hour, day, month and/or the year as necessary.
 * \param changing_tm   A pointer to the struct tm to increment. The
 * struct referred to by the pointer is modified by the function.
 * \param quantity      The number of minutes to add
 * \returns             A pointer to the same struct tm.
 */

struct tm*
tm_increment_minute(struct tm *changing_tm, const int quantity) {
    static const int mins_in_hour = 60;

    if ( quantity < 0 ) {
        tm_decrement_minute(changing_tm, -quantity);
    } else {
        int num_mins = quantity;

        if ( num_mins >= mins_in_hour ||
             num_mins >= mins_in_hour - changing_tm->tm_min ) {
            int num_hours = quantity / mins_in_hour;
            num_mins -= num_hours * mins_in_hour;
            if ( num_mins >= mins_in_hour - changing_tm->tm_min ) {
                ++num_hours;
                changing_tm->tm_min += num_mins - mins_in_hour;
                num_mins = 0;
            }
            tm_increment_hour(changing_tm, num_hours);
        }

        changing_tm->tm_min += num_mins;
    }
    return changing_tm;
}


/*!
 * \brief               Adds one or more seconds to a struct tm, incrementing
 * the minute, hour, day, month and/or the year as necessary.
 * \param changing_tm   A pointer to the struct tm to increment. The
 * struct referred to by the pointer is modified by the function.
 * \param quantity      The number of seconds to add
 * \returns             A pointer to the same struct tm.
 */

struct tm*
tm_increment_second(struct tm *changing_tm, const int quantity) {
    static const int secs_in_min = 60;

    if ( quantity < 0 ) {
        tm_decrement_second(changing_tm, -quantity);
    } else {
        int num_secs = quantity;

        if ( num_secs >= secs_in_min ||
             num_secs >= secs_in_min - changing_tm->tm_sec ) {
            int num_mins = quantity / secs_in_min;
            num_secs -= num_mins * secs_in_min;
            if ( num_secs >= secs_in_min - changing_tm->tm_sec ) {
                ++num_mins;
                changing_tm->tm_sec += num_secs - secs_in_min;
                num_secs = 0;
            }
            tm_increment_minute(changing_tm, num_mins);
        }

        changing_tm->tm_sec += num_secs;
    }

    return changing_tm;
}


/*!
 * \brief               Deducts one or more days from a struct tm,
 * decrementing the month and/or the year as necessary.
 * \param changing_tm   A pointer to the struct tm to decrement. The
 * struct referred to by the pointer is modified by the function.
 * \param quantity      The number of days to deduct
 * \returns             A pointer to the same struct tm.
 */

struct tm*
tm_decrement_day(struct tm *changing_tm, const int quantity) {
    enum months {january, february, march, april, may, june, july,
                 august, september, october, november, december};

    if ( quantity < 0 ) {
        tm_increment_day(changing_tm, -quantity);
    } else {
        int num_days = quantity;

        while ( num_days-- ) {
            if ( changing_tm->tm_mday > 1 ) {
                changing_tm->tm_mday -= 1;
            } else {
                switch ( changing_tm->tm_mon ) {
                    case january:
                        changing_tm->tm_mday = 31;
                        changing_tm->tm_mon = 11;
                        if ( changing_tm->tm_year == 1 ) {
                            changing_tm->tm_year = -1;
                        } else {
                            changing_tm->tm_year -= 1;
                        }
                        break;

                    case february: 
                    case april:
                    case june:
                    case august:
                    case september:
                    case november:
                        changing_tm->tm_mday = 31;
                        changing_tm->tm_mon -= 1;
                        break;

                    case may:
                    case july:
                    case october:
                    case december:
                        changing_tm->tm_mday = 30;
                        changing_tm->tm_mon -= 1;
                        break;

                    case march:
                        if ( is_leap_year(changing_tm->tm_year) ) {
                            changing_tm->tm_mday = 29;
                        } else {
                            changing_tm->tm_mday = 28;
                        }
                        changing_tm->tm_mon -= 1;
                        break;

                    default:
                        assert(false);
                        break;
                }
            }
        }
    }

    return changing_tm;
}


/*!
 * \brief               Deducts one or more hours from a struct tm,
 * decrementing the day, month and/or the year as necessary.
 * \param changing_tm   A pointer to the struct tm to decrement. The
 * struct referred to by the pointer is modified by the function.
 * \param quantity      The number of hours to deduct
 * \returns             A pointer to the same struct tm.
 */

struct tm*
tm_decrement_hour(struct tm *changing_tm, const int quantity) {
    static const int hours_in_day = 24;

    if ( quantity < 0 ) {
        tm_increment_hour(changing_tm, -quantity);
    } else {
        int num_hours = quantity;

        if ( num_hours >= hours_in_day || num_hours > changing_tm->tm_hour ) {
            int num_days = quantity / hours_in_day;
            num_hours -= num_days * hours_in_day;
            if ( num_hours > changing_tm->tm_hour ) {
                ++num_days;
                num_hours -= changing_tm->tm_hour;
                changing_tm->tm_hour = hours_in_day;
            }
            tm_decrement_day(changing_tm, num_days);
        }

        changing_tm->tm_hour -= num_hours;
    }

    return changing_tm;
}


/*!
 * \brief               Deducts one or more minutes from a struct tm,
 * decrementing the hour, day, month and/or the year as necessary.
 * \param changing_tm   A pointer to the struct tm to decrement. The
 * struct referred to by the pointer is modified by the function.
 * \param quantity      The number of minutes to deduct
 * \returns             A pointer to the same struct tm.
 */

struct tm*
tm_decrement_minute(struct tm *changing_tm, const int quantity) {
    static const int mins_in_hour = 60;

    if ( quantity < 0 ) {
        tm_increment_minute(changing_tm, -quantity);
    } else {
        int num_mins = quantity;

        if ( num_mins >= mins_in_hour || num_mins > changing_tm->tm_min ) {
            int num_hours = quantity / mins_in_hour;
            num_mins -= num_hours * mins_in_hour;
            if ( num_mins > changing_tm->tm_min ) {
                ++num_hours;
                num_mins -= changing_tm->tm_min;
                changing_tm->tm_min = mins_in_hour;
            }
            tm_decrement_hour(changing_tm, num_hours);
        }

        changing_tm->tm_min -= num_mins;
    }
    return changing_tm;
}


/*!
 * \brief               Deducts one or more seconds from a struct tm,
 * decrementing the minute, hour, day, month and/or the year as necessary.
 * \param changing_tm   A pointer to the struct tm struct to decrement. The
 * struct referred to by the pointer is modified by the function.
 * \param quantity      The number of seconds to deduct
 * \returns             A pointer to the same struct tm.
 */

struct tm*
tm_decrement_second(struct tm *changing_tm, const int quantity) {
    static const int secs_in_min = 60;

    if ( quantity < 0 ) {
        tm_increment_second(changing_tm, -quantity);
    } else {
        int num_secs = quantity;

        if ( num_secs >= secs_in_min || num_secs > changing_tm->tm_sec ) {
            int num_mins = quantity / secs_in_min;
            num_secs -= num_mins * secs_in_min;
            if ( num_secs > changing_tm->tm_sec ) {
                ++num_mins;
                num_secs -= changing_tm->tm_sec;
                changing_tm->tm_sec = secs_in_min;
            }
            tm_decrement_minute(changing_tm, num_mins);
        }

        changing_tm->tm_sec -= num_secs;
    }

    return changing_tm;
}


/*!
 * \brief           Gets a time_t timestamp for a requested UTC time.
 * \param utc_tm    A pointer to a struct tm containing the UTC time.
 * \returns         A time_t timestamp for the requested UTC time.
 */

time_t
get_utc_timestamp(const struct tm *utc_tm) {

    //  Get a timestamp close to (i.e. within 24 hours of) the
    //  desired UTC time.

    struct tm copy_tm = *utc_tm;
    time_t utc_ts = mktime(&copy_tm);
    if ( utc_ts == -1 ) {
        fprintf(stderr, "pgtime:%s:%d: couldn't get calendar time.\n",
                __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    //  Compute the difference with the desired UTC time...

    int secs_diff = get_utc_timestamp_sec_diff(utc_ts, &copy_tm);

    //  ...and adjust the timestamp, if needed.

    if ( secs_diff ) {
        const time_t one_sec = get_sec_diff();
        utc_ts -= one_sec * secs_diff;

        secs_diff = get_utc_timestamp_sec_diff(utc_ts, &copy_tm);

        if ( secs_diff ) {

            //  We're pretty unlucky if we get here, but let's check
            //  for a leap second on either side, and give up if not.

            if ( get_utc_timestamp_sec_diff(utc_ts + one_sec, &copy_tm) == 0 ) {
               utc_ts += one_sec;
            } else if ( get_utc_timestamp_sec_diff(utc_ts - one_sec,
                                     &copy_tm) == 0 ) {
               utc_ts -= one_sec;
            } else { 
                fprintf(stderr, "pgtime:%s:%d: couldn't get calendar time.\n",
                        __FILE__, __LINE__);
                exit(EXIT_FAILURE);
            }
        }
    }

    return utc_ts;
}


/*!
 * \brief               Checks a time_t timestamp against a UTC time, and
 * returns the difference in seconds.
 * \details             This function only returns a good value
 * if the timestamp is less than 24 hours away from the desired time,
 * so the caller is responsible for making sure that it is. This function
 * may also return a bad value if a leap second or other unpredictable
 * calendar change falls between the desired UTC time and the provided
 * time stamp. The result should therefore always be checked with
 * check_utc_timestamp(), or by calling this function again.
 * \param check_time    The time_t timestamp to check
 * \param utc_tm        A pointer to a struct tm against which to check.
 * \returns             The difference, if any, represented in seconds.
 */

int
get_utc_timestamp_sec_diff(const time_t check_time, const struct tm *utc_tm) {

    //  Get a struct tm representing UTC time for the provided
    //  timestamp.

    struct tm* ptm = gmtime(&check_time);
    if ( ptm == 0 ) {
        fprintf(stderr, "pgtime:%s:%d: couldn't get UTC time.\n",
                __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    struct tm check_tm = *ptm;

    //  Compare the two and return the difference.

    return tm_intraday_secs_diff(utc_tm, &check_tm);
}
