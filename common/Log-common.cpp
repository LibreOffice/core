/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include "Log.hpp"

#include <common/ProcUtil.hpp>
#include <common/StaticLogHelper.hpp>
#include <common/Util.hpp>

#include <cstring>
#include <ctime>

namespace Log
{
    extern StaticHelper Static;

    namespace
    {

    /// Convert an unsigned number to ascii with 0 padding.
    template <int Width> void to_ascii_fixed(char* buf, std::size_t num)
    {
        buf[Width - 1] = '0' + num % 10; // Units.

        if (Width > 1)
        {
            num /= 10;
            buf[Width - 2] = '0' + num % 10; // Tens.
        }

        if (Width > 2)
        {
            num /= 10;
            buf[Width - 3] = '0' + num % 10; // Hundreds.
        }

        if (Width > 3)
        {
            num /= 10;
            buf[Width - 4] = '0' + num % 10; // Thousands.
        }

        if (Width > 4)
        {
            num /= 10;
            buf[Width - 5] = '0' + num % 10; // Ten-Thousands.
        }

        if (Width > 5)
        {
            num /= 10;
            buf[Width - 6] = '0' + num % 10; // Hundred-Thousands.
        }

        static_assert(Width >= 1 && Width <= 6, "Width is invalid.");
    }

    /// Copy a null-terminated string into another.
    /// Expects the destination to be large enough.
    /// Note: unlike strcpy, this returns the *new* out
    /// (destination) pointer, which saves a strlen call.
    char* strcopy(const char* in, char* out)
    {
        while (*in)
            *out++ = *in++;
        return out;
    }

    /// Convert unsigned long num to base-10 ascii in place.
    /// Returns the *end* position.
    char* to_ascii(char* buf, std::size_t num)
    {
        int i = 0;
        do
        {
            buf[i++] = '0' + num % 10;
            num /= 10;
        } while (num > 0);

        // Reverse.
        for (char *front = buf, *back = buf + i - 1; back > front; ++front, --back)
        {
            const char t = *front;
            *front = *back;
            *back = t;
        }

        return buf + i;
    }
    } // namespace

    Prefix::Prefix()
        : _last_tm({})
        , _last_time(0)
        , _year_pos(nullptr)
        , _level_pos(nullptr)
    {
        reset();
    }

    void Prefix::reset()
    {
        const std::chrono::time_point<std::chrono::system_clock> tp =
            std::chrono::system_clock::now();
        char* buffer = _buffer.data();

#if defined(IOS) || defined(__FreeBSD__) || defined(_WIN32)
        // Don't bother with identifying the process as there is just one process in the app anyway.

        // FIXME: Not sure why FreeBSD is here, too. Surely on FreeBSD COOL runs just like on Linux,
        // as a set of separate processes, so it would be useful to see from which process a log
        // line is?

        char* pos = buffer;

        // Don't bother with the thread identifier either. We output the thread name which is much
        // more useful anyway.
#else // !IOS && !__FreeBSD__  && !_WIN32
        // Note that snprintf is deemed signal-safe in most common implementations.
        char* pos = strcopy((Static.getInited() ? Static.getId().c_str() : "<shutdown>"), buffer);
        *pos++ = '-';

        // Thread ID.
        const auto osTid = ProcUtil::getThreadId();
        if (osTid > 99999)
        {
            if (osTid > 999999)
                pos = to_ascii(pos, osTid);
            else
            {
                to_ascii_fixed<6>(pos, osTid);
                pos += 6;
            }
        }
        else
        {
            to_ascii_fixed<5>(pos, osTid);
            pos += 5;
        }

        *pos++ = ' ';
#endif // !IOS && !__FreeBSD__

        auto t = std::chrono::system_clock::to_time_t(tp);
        std::tm& tm = _last_tm;
        Util::time_t_to_gmtime(t, tm);
        _last_time = t;

        // YYYY-MM-DD.
        _year_pos = pos;
        to_ascii_fixed<4>(pos, tm.tm_year + 1900);
        pos[4] = '-';
        pos += 5;
        to_ascii_fixed<2>(pos, tm.tm_mon + 1);
        pos[2] = '-';
        pos += 3;
        to_ascii_fixed<2>(pos, tm.tm_mday);
        pos[2] = ' ';
        pos += 3;

        // HH:MM:SS.uS
        to_ascii_fixed<2>(pos, tm.tm_hour);
        pos[2] = ':';
        pos += 3;
        to_ascii_fixed<2>(pos, tm.tm_min);
        pos[2] = ':';
        pos += 3;
        to_ascii_fixed<2>(pos, tm.tm_sec);
        pos[2] = '.';
        pos += 3;
        auto microseconds =
            std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch());
        auto fractional_seconds = microseconds.count() % 1000000;
        to_ascii_fixed<6>(pos, fractional_seconds);
        pos[6] = ' ';
        pos += 7;

        // Thread name and log level
        pos[0] = '[';
        pos[1] = ' ';
        pos += 2;
        pos = strcopy(ProcUtil::getThreadName(), pos);
        pos[0] = ' ';
        pos[1] = ']';
        pos[2] = ' ';
        pos += 3;
        _level_pos = pos;
        memcpy(pos, "LVL", 3); // Placeholder for the log-level.
        pos += 3;
        pos[0] = ' ';
        pos[1] = ' ';
        pos[2] = '\0';
        pos += 2;

        _prefix = std::string_view(buffer, pos - buffer);
    }

    std::string_view Prefix::update(std::string_view level,
                                    std::chrono::time_point<std::chrono::system_clock> tp)
    {
        std::time_t t = std::chrono::system_clock::to_time_t(tp);
        std::tm tm;
        Util::time_t_to_gmtime(t, tm);

        // Offsets, including delimiters.
        constexpr int mon_offset_from_year = 5;
        constexpr int day_offset_from_year = mon_offset_from_year + 3;
        constexpr int hr_offset_from_year = day_offset_from_year + 3;
        constexpr int min_offset_from_year = hr_offset_from_year + 3;
        constexpr int sec_offset_from_year = min_offset_from_year + 3;
        constexpr int ms_offset_from_year = sec_offset_from_year + 3;

        // Don't assume time is monotonic; System clocks aren't.
        const std::time_t delta = std::abs(t - _last_time);
        if (delta > 0)
        {
            // Seconds.
            to_ascii_fixed<2>(_year_pos + sec_offset_from_year, tm.tm_sec);

            // Minutes.
            if (tm.tm_min != _last_tm.tm_min || delta >= 60)
            {
                to_ascii_fixed<2>(_year_pos + min_offset_from_year, tm.tm_min);
                if (tm.tm_hour != _last_tm.tm_hour || delta >= 60 * 60)
                {
                    to_ascii_fixed<2>(_year_pos + hr_offset_from_year, tm.tm_hour);

                    // Day of Month.
                    if (tm.tm_mday != _last_tm.tm_mday || delta >= 24 * 60 * 60)
                    {
                        to_ascii_fixed<2>(_year_pos + day_offset_from_year, tm.tm_mday);
                        to_ascii_fixed<2>(_year_pos + mon_offset_from_year, tm.tm_mon + 1);
                        to_ascii_fixed<4>(_year_pos, tm.tm_year + 1900);
                    }
                }
            }
        }

        const auto microseconds =
            std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch());
        const auto fractional_seconds = microseconds.count() % 1000000;
        to_ascii_fixed<6>(_year_pos + ms_offset_from_year, fractional_seconds);

        assert(level.size() == 3 && "Expected 3-character log level");
        memcpy(_level_pos, level.data(), level.size());

        _last_time = t;
        _last_tm = tm;

        return _prefix;
    }

    std::string_view prefix(const std::string_view level,
                            std::chrono::time_point<std::chrono::system_clock> tp)
    {
        return Prefix::Instance.update(level, tp);
    }

    void reset() { Prefix::Instance.reset(); }

#ifdef BUILDING_TESTS
    char* prefixReference(const std::chrono::time_point<std::chrono::system_clock>& tp,
                          char* buffer, const std::string_view level)
    {
#if defined(IOS) || defined(__FreeBSD__) || defined(_WIN32)
        // Don't bother with identifying the process as there is just one process in the app anyway.

        // FIXME: Not sure why FreeBSD is here, too. Surely on FreeBSD COOL runs just like on Linux,
        // as a set of separate processes, so it would be useful to see from which process a log
        // line is?

        char* pos = buffer;

        // Don't bother with the thread identifier either. We output the thread name which is much
        // more useful anyway.
#else
        // Note that snprintf is deemed signal-safe in most common implementations.
        char* pos = strcopy((Static.getInited() ? Static.getId().c_str() : "<shutdown>"), buffer);
        *pos++ = '-';

        // Thread ID.
        const auto osTid = ProcUtil::getThreadId();
        if (osTid > 99999)
        {
            if (osTid > 999999)
                pos = to_ascii(pos, osTid);
            else
            {
                to_ascii_fixed<6>(pos, osTid);
                pos += 6;
            }
        }
        else
        {
            to_ascii_fixed<5>(pos, osTid);
            pos += 5;
        }

        *pos++ = ' ';
#endif

        auto t = std::chrono::system_clock::to_time_t(tp);
        std::tm tm;
        Util::time_t_to_gmtime(t, tm);

        // YYYY-MM-DD.
        to_ascii_fixed<4>(pos, tm.tm_year + 1900);
        pos[4] = '-';
        pos += 5;
        to_ascii_fixed<2>(pos, tm.tm_mon + 1);
        pos[2] = '-';
        pos += 3;
        to_ascii_fixed<2>(pos, tm.tm_mday);
        pos[2] = ' ';
        pos += 3;

        // HH:MM:SS.uS
        to_ascii_fixed<2>(pos, tm.tm_hour);
        pos[2] = ':';
        pos += 3;
        to_ascii_fixed<2>(pos, tm.tm_min);
        pos[2] = ':';
        pos += 3;
        to_ascii_fixed<2>(pos, tm.tm_sec);
        pos[2] = '.';
        pos += 3;
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch());
        auto fractional_seconds = microseconds.count() % 1000000;
        to_ascii_fixed<6>(pos, fractional_seconds);
        pos[6] = ' ';
        pos += 7;

        // Thread name and log level
        pos[0] = '[';
        pos[1] = ' ';
        pos += 2;
        pos = strcopy(ProcUtil::getThreadName(), pos);
        pos[0] = ' ';
        pos[1] = ']';
        pos[2] = ' ';
        pos += 3;
        memcpy(pos, level.data(), level.size());
        pos += 3;
        pos[0] = ' ';
        pos[1] = ' ';
        pos[2] = '\0';

        return buffer;
    }
#endif // BUILDING_TESTS

} // namespace Log

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
