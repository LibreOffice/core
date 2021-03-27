/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <algorithm>
#include <cassert>
#include <limits>
#include <type_traits>

#if defined(_MSC_VER)
#include <safeint.h>
#else
#ifndef __has_builtin
#   define __has_builtin(x) 0
#endif
#endif

namespace o3tl
{

template<typename T> inline
typename std::enable_if<std::is_signed<T>::value, T>::type saturating_add(
    T a, T b)
{
    if (b >= 0) {
        if (a <= std::numeric_limits<T>::max() - b) {
            return a + b;
        } else {
            return std::numeric_limits<T>::max();
        }
    } else {
        if (a >= std::numeric_limits<T>::min() - b) {
            return a + b;
        } else {
            return std::numeric_limits<T>::min();
        }
    }
}

template<typename T> inline
typename std::enable_if<std::is_unsigned<T>::value, T>::type saturating_add(
    T a, T b)
{
    if (a <= std::numeric_limits<T>::max() - b) {
        return a + b;
    } else {
        return std::numeric_limits<T>::max();
    }
}

template<typename T> inline
typename std::enable_if<std::is_signed<T>::value, T>::type saturating_sub(
    T a, T b)
{
    if (b >= 0) {
        if (a >= std::numeric_limits<T>::min() + b) {
            return a - b;
        } else {
            return std::numeric_limits<T>::min();
        }
    } else {
        if (a <= std::numeric_limits<T>::max() + b) {
            return a - b;
        } else {
            return std::numeric_limits<T>::max();
        }
    }
}

template<typename T> inline
typename std::enable_if<std::is_unsigned<T>::value, T>::type saturating_sub(
    T a, T b)
{
    if (a >= std::numeric_limits<T>::min() + b) {
        return a - b;
    } else {
        return std::numeric_limits<T>::min();
    }
}

template<typename T> inline
typename std::enable_if<std::is_signed<T>::value, T>::type saturating_toggle_sign(
    T a)
{
    if (a == std::numeric_limits<T>::min())
        return std::numeric_limits<T>::max();
    return a * -1;
}

#if defined(_MSC_VER)

template<typename T> inline bool checked_multiply(T a, T b, T& result)
{
    return !msl::utilities::SafeMultiply(a, b, result);
}

template<typename T> inline bool checked_add(T a, T b, T& result)
{
    return !msl::utilities::SafeAdd(a, b, result);
}

template<typename T> inline bool checked_sub(T a, T b, T& result)
{
    return !msl::utilities::SafeSubtract(a, b, result);
}

#elif (defined __GNUC__ && !defined __clang__) || (__has_builtin(__builtin_mul_overflow) && !(defined ANDROID && defined __clang__) && !(defined(__clang__) && (defined(__arm__) || defined(__i386__))))
// 32-bit clang fails with undefined reference to `__mulodi4'

template<typename T> inline bool checked_multiply(T a, T b, T& result)
{
    return __builtin_mul_overflow(a, b, &result);
}

template<typename T> inline bool checked_add(T a, T b, T& result)
{
    return __builtin_add_overflow(a, b, &result);
}

template<typename T> inline bool checked_sub(T a, T b, T& result)
{
    return __builtin_sub_overflow(a, b, &result);
}

#else

//https://www.securecoding.cert.org/confluence/display/c/INT32-C.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow
template<typename T> inline typename std::enable_if<std::is_signed<T>::value, bool>::type checked_multiply(T a, T b, T& result)
{
  if (a > 0) {  /* a is positive */
    if (b > 0) {  /* a and b are positive */
      if (a > (std::numeric_limits<T>::max() / b)) {
        return true; /* Handle error */
      }
    } else { /* a positive, b nonpositive */
      if (b < (std::numeric_limits<T>::min() / a)) {
        return true; /* Handle error */
      }
    } /* a positive, b nonpositive */
  } else { /* a is nonpositive */
    if (b > 0) { /* a is nonpositive, b is positive */
      if (a < (std::numeric_limits<T>::min() / b)) {
        return true; /* Handle error */
      }
    } else { /* a and b are nonpositive */
      if ( (a != 0) && (b < (std::numeric_limits<T>::max() / a))) {
        return true; /* Handle error */
      }
    } /* End if a and b are nonpositive */
  } /* End if a is nonpositive */

  result = a * b;

  return false;
}

//https://www.securecoding.cert.org/confluence/display/c/INT30-C.+Ensure+that+unsigned+integer+operations+do+not+wrap
template<typename T> inline typename std::enable_if<std::is_unsigned<T>::value, bool>::type checked_multiply(T a, T b, T& result)
{
    if (b && a > std::numeric_limits<T>::max() / b) {
        return true;/* Handle error */
    }

    result = a * b;

    return false;
}

//https://www.securecoding.cert.org/confluence/display/c/INT32-C.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow
template<typename T> inline typename std::enable_if<std::is_signed<T>::value, bool>::type checked_add(T a, T b, T& result)
{
    if (((b > 0) && (a > (std::numeric_limits<T>::max() - b))) ||
        ((b < 0) && (a < (std::numeric_limits<T>::min() - b)))) {
        return true;
    }

    result = a + b;

    return false;
}

//https://www.securecoding.cert.org/confluence/display/c/INT30-C.+Ensure+that+unsigned+integer+operations+do+not+wrap
template<typename T> inline typename std::enable_if<std::is_unsigned<T>::value, bool>::type checked_add(T a, T b, T& result)
{
    if (std::numeric_limits<T>::max() - a < b) {
        return true;/* Handle error */
    }

    result = a + b;

    return false;
}

//https://www.securecoding.cert.org/confluence/display/c/INT32-C.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow
template<typename T> inline typename std::enable_if<std::is_signed<T>::value, bool>::type checked_sub(T a, T b, T& result)
{
    if ((b > 0 && a < std::numeric_limits<T>::min() + b) ||
        (b < 0 && a > std::numeric_limits<T>::max() + b)) {
        return true;
    }

    result = a - b;

    return false;
}

//https://www.securecoding.cert.org/confluence/display/c/INT30-C.+Ensure+that+unsigned+integer+operations+do+not+wrap
template<typename T> inline typename std::enable_if<std::is_unsigned<T>::value, bool>::type checked_sub(T a, T b, T& result)
{
    if (a < b) {
        return true;
    }

    result = a - b;

    return false;
}

#endif

template<typename T> constexpr std::enable_if_t<std::is_signed_v<T>, std::make_unsigned_t<T>>
make_unsigned(T value)
{
    assert(value >= 0);
    return value;
}

// An implicit conversion from T2 to T1, useful in places where an explicit conversion from T2 to
// T1 is needed (e.g., in list initialization, if the implicit conversion would be narrowing) but
// tools like -fsanitize=implicit-conversion should still be able to detect truncation:
template<typename T1, typename T2> constexpr T1 narrowing(T2 value) { return value; }

// std::min wrapped to inform coverity that the result is now deemed sanitized
// coverity[ -taint_source ]
template<typename T> [[nodiscard]] inline T sanitizing_min(T a, T b)
{
    return std::min(a, b);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
