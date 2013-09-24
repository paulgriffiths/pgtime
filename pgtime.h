/*!
 * \file        pgtime.h
 * \brief       Interface to miscellaneous time functions.
 * \author      Paul Griffiths
 * \copyright   Copyright 2013 Paul Griffiths. Distributed under the terms
 * of the GNU General Public License. <http://www.gnu.org/licenses/>
 */


#ifndef PG_PGTIME_H
#define PG_PGTIME_H

#include <time.h>
#include <stdbool.h>


/*  Function prototypes  */

#ifdef __cplusplus
extern "C" {
#endif

time_t get_day_diff(void);
time_t get_hour_diff(void);
time_t get_sec_diff(void);

bool validate_date(const struct tm *check_tm);
int tm_compare(const struct tm *first, const struct tm *second);
int tm_intraday_secs_diff(const struct tm *first, const struct tm *second);
bool is_leap_year(const int year);

struct tm *tm_increment_day(struct tm *changing_tm, const int quantity);
struct tm *tm_increment_hour(struct tm *changing_tm, const int quantity);
struct tm *tm_increment_minute(struct tm *changing_tm, const int quantity);
struct tm *tm_increment_second(struct tm *changing_tm, const int quantity);
struct tm *tm_decrement_day(struct tm *changing_tm, const int quantity);
struct tm *tm_decrement_hour(struct tm *changing_tm, const int quantity);
struct tm *tm_decrement_minute(struct tm *changing_tm, const int quantity);
struct tm *tm_decrement_second(struct tm *changing_tm, const int quantity);

bool check_utc_timestamp(const time_t check_time, int *secs_diff,
                         const struct tm *check_tm);
time_t get_utc_timestamp(const struct tm *utc_tm);
int get_utc_timestamp_sec_diff(const time_t check_time,
                               const struct tm *check_tm);

#ifdef __cplusplus
}
#endif


#endif          /*  PG_PGTIME_H  */
