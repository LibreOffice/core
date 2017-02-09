/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SAL_LOG_HXX
#define INCLUDED_SAL_LOG_HXX

#include <sal/config.h>

#include <cstdlib>
#include <sstream>
#include <string>

#include <sal/detail/log.h>
#include <sal/saldllapi.h>
#include <sal/types.h>

// Avoid the use of other sal code in this header as much as possible, so that
// this code can be called from other sal code without causing endless
// recursion.

/// @cond INTERNAL

extern "C" SAL_DLLPUBLIC void SAL_CALL sal_detail_log(
    enum sal_detail_LogLevel level, char const * area, char const * where,
    char const * message, sal_uInt32 backtraceDepth);

extern "C" SAL_DLLPUBLIC int SAL_CALL sal_detail_log_report(
    enum sal_detail_LogLevel level, char const * area);

namespace sal { namespace detail {

inline void log(
    sal_detail_LogLevel level, char const * area, char const * where,
    std::ostringstream const & stream, sal_uInt32 backtraceDepth)
{
    // An alternative would be to have sal_detail_log take a std::ostringstream
    // pointer (via a C void pointer); the advantage would be smaller client
    // code (the ".str().c_str()" part would move into the implementation of
    // sal_detail_log) and potential for proper support of embedded null
    // characters within the message, but the disadvantage would be dependence
    // on the C++ ABI; as a compromise, the ".str().c_str()" part has been moved
    // to this inline function so that it is potentially only emitted once per
    // dynamic library:
    sal_detail_log(level, area, where, stream.str().c_str(), backtraceDepth);
}

// Special handling of the common case where the message consists of just a
// string literal, to produce smaller call-site code:

struct StreamStart {};

struct StreamString {
    StreamString(char const * s): string(s) {}

    char const * string;

    typedef char Result;
};

struct StreamIgnore {
    typedef struct { char a[2]; } Result;
};

inline StreamString operator <<(
    SAL_UNUSED_PARAMETER StreamStart const &, char const * s)
{
    return StreamString(s);
}

template< typename T > inline StreamIgnore operator <<(
    SAL_UNUSED_PARAMETER StreamStart const &, SAL_UNUSED_PARAMETER T const &)
{
    std::abort();
#if defined _MSC_VER && _MSC_VER < 1700
    return StreamIgnore();
#endif
}

template< typename T > inline StreamIgnore operator <<(
    SAL_UNUSED_PARAMETER StreamString const &, SAL_UNUSED_PARAMETER T const &)
{
    std::abort();
#if defined _MSC_VER && _MSC_VER < 1700
    return StreamIgnore();
#endif
}

template< typename T > inline StreamIgnore operator <<(
    SAL_UNUSED_PARAMETER StreamIgnore const &, SAL_UNUSED_PARAMETER T const &)
{
    std::abort();
#if defined _MSC_VER && _MSC_VER < 1700
    return StreamIgnore();
#endif
}

template< typename T > typename T::Result getResult(T const &);

inline char const * unwrapStream(StreamString const & s) { return s.string; }

inline char const * unwrapStream(SAL_UNUSED_PARAMETER StreamIgnore const &) {
    std::abort();
#if defined _MSC_VER && _MSC_VER < 1700
    return 0;
#endif
}

} }

#define SAL_DETAIL_LOG_STREAM(condition, level, area, where, stream) \
    do { \
        if ((condition) && sal_detail_log_report(level, area)) { \
            if (sizeof ::sal::detail::getResult( \
                    ::sal::detail::StreamStart() << stream) == 1) \
            { \
                ::sal_detail_log( \
                    (level), (area), (where), \
                    ::sal::detail::unwrapStream( \
                        ::sal::detail::StreamStart() << stream), \
                    0); \
            } else { \
                ::std::ostringstream sal_detail_stream; \
                sal_detail_stream << stream; \
                ::sal::detail::log( \
                    (level), (area), (where), sal_detail_stream, 0); \
            } \
        } \
    } while (false)

/// @endcond

/** A simple macro to create a "file and line number" string.

    Potentially not only useful within the log framework (where it is used
    automatically), but also when creating exception messages.

    @attention For now, this functionality should only be used internally within
    LibreOffice. It may change again in a future version.

    @since LibreOffice 3.5
*/
#define SAL_WHERE SAL_DETAIL_WHERE

/** A facility for generating temporary string messages by piping items into a
    C++ std::ostringstream.

    This can be useful for example in a call to SAL_INFO when depending on some
    boolean condition data of incompatible types shall be streamed into the
    message, as in:

      SAL_INFO("foo", "object: " << (hasName ? obj->name : SAL_STREAM(obj)));

    @attention For now, this functionality should only be used internally within
    LibreOffice. It may change again in a future version.

    @since LibreOffice 3.5
*/
#ifdef _LIBCPP_VERSION
#define SAL_STREAM(stream) \
    (::std::ostringstream() << stream).str()
#else
#define SAL_STREAM(stream) \
    (dynamic_cast< ::std::ostringstream & >(::std::ostringstream() << stream).str())
#endif

/**
    @page sal_log Basic logging functionality.

    @short Macros for logging.

    SAL_INFO(char const * area, expr),
    SAL_INFO_IF(bool condition, char const * area, expr),
    SAL_WARN(char const * area, expr),
    SAL_WARN_IF(bool condition, char const * area, expr), and SAL_DEBUG(expr)
    produce an info, warning, or debug log entry with a message produced by
    piping items into a C++ std::ostringstream.  The given expr must be so that
    the full expression "stream << expr" is valid, where stream is a variable of
    type std::ostringstream.

      SAL_INFO("foo", "string " << s << " of length " << n)

    would be an example of such a call.

    The composed message should be in UTF-8 and it should contain no vertical
    formatting characters and no null characters

    For the _IF variants, log output is only generated if the given condition is
    true (in addition to the other conditions that have to be met).

    The SAL_DEBUG macro is for temporary debug statements that are used while
    working on code.  It is never meant to remain in the code.  It will always
    simply output the given expression in debug builds.

    For all the other macros, the given area argument must be non-null and must
    match the regular expression

    @verbatim
      <area> ::= <segment>("."<segment>)*
    @endverbatim

    with

    @verbatim
      <segment> ::= [0-9a-z]+
    @endverbatim

    For a list of areas used see @ref sal_log_areas "SAL debug areas". Whenever
    you use a new log area, add it to the file include/sal/log-areas.dox .

    Whether these macros generate any log output is controlled in a two-stage
    process.

    First, at compile time the macros SAL_LOG_INFO and SAL_LOG_WARN,
    respectively, control whether the INFO and WARN macros, respectively,
    expand to actual code (in case the macro is defined, to any value) or to
    no-ops (in case the macro is not defined).

    Second, at runtime the environment variable SAL_LOG further limits which
    macro calls actually generate log output.  The environment variable SAL_LOG
    must either be unset or must match the regular expression

    @verbatim
      <env> ::= <switch>*
    @endverbatim

    with

    @verbatim
      <switch> ::= <sense><item>
      <sense> ::= "+"|"-"
      <item> ::= <flag>|<level>("."<area>)?
      <flag> ::= "TIMESTAMP"|"RELATIVETIMER"
      <level> ::= "INFO"|"WARN"
    @endverbatim

    If the environment variable is unset, the setting "+WARN" is
    assumed instead (which results in all warnings being output but no
    infos).  If the given value does not match the regular expression,
    "+INFO+WARN" is used instead (which in turn results in everything
    being output).

    The "+TIMESTAMP" flag causes each output line (as selected by the level
    switch(es)) to be prefixed by a timestamp like 2016-08-18:14:04:43.

    The "+RELATIVETIMER" flag causes each output line (as selected by
    the level switch(es)) to be prefixed by a relative timestamp in
    seconds since the first output line like 1.312.

    If both +TIMESTAMP and +RELATIVETIMER are specified, they are
    output in that order.

    Specifying a flag with a negative sense has no effect. Specifying
    the same flag multiple times has no extra effect.

    A given macro call's level (INFO or WARN) and area is matched against the
    given switches as follows:  Only those switches for which the level matches
    the given level and for which the area is a prefix (including both empty and
    full prefixes) of the given area are considered.  Log output is generated if
    and only if among the longest such switches (if any), there is at least one
    that has a sense of "+".  (That is, if both +INFO.foo and -INFO.foo are
    present, +INFO.foo wins.)

    If no WARN selection is specified, but an INFO selection is, the
    INFO selection is used for WARN messages, too.

    For example, if SAL_LOG is "+INFO-INFO.foo+INFO.foo.bar", then calls like
    SAL_INFO("foo.bar", ...), SAL_INFO("foo.bar.baz", ...), or
    SAL_INFO("other", ...) generate output, while calls like
    SAL_INFO("foo", ...) or SAL_INFO("foo.barzzz", ...) do not.

    The generated log output consists of the optinal timestamp, the given level
    ("info" or "warn"), the given area, the process ID, the thread ID, the
    source file, and the source line number, each followed by a colon, followed
    by a space, the given message, and a newline.  The precise format of the log
    output is subject to change.  The log output is printed to stderr without
    further text encoding conversion.

    @see @ref sal_log_areas

    @attention For now, this functionality should only be used internally within
    LibreOffice. It may change again in a future version.

    @since LibreOffice 3.5
*/

/**
  Produce log entry from stream in the given log area.

  See @ref sal_log "basic logging functionality" for details.
*/
#define SAL_INFO(area, stream) \
    SAL_DETAIL_LOG_STREAM( \
        SAL_DETAIL_ENABLE_LOG_INFO, ::SAL_DETAIL_LOG_LEVEL_INFO, area, \
        SAL_WHERE, stream)

/**
  Produce log entry from stream in the given log area if condition is true.

  See @ref sal_log "basic logging functionality" for details.
*/
#define SAL_INFO_IF(condition, area, stream)  \
    SAL_DETAIL_LOG_STREAM( \
        SAL_DETAIL_ENABLE_LOG_INFO && (condition), \
        ::SAL_DETAIL_LOG_LEVEL_INFO, area, SAL_WHERE, stream)

/**
  Produce warning entry from stream in the given log area.

  See @ref sal_log "basic logging functionality" for details.
*/
#define SAL_WARN(area, stream) \
    SAL_DETAIL_LOG_STREAM( \
        SAL_DETAIL_ENABLE_LOG_WARN, ::SAL_DETAIL_LOG_LEVEL_WARN, area, \
        SAL_WHERE, stream)

/**
  Produce warning entry from stream in the given log area if condition is true.

  See @ref sal_log "basic logging functionality" for details.
*/
#define SAL_WARN_IF(condition, area, stream)   \
    SAL_DETAIL_LOG_STREAM( \
        SAL_DETAIL_ENABLE_LOG_WARN && (condition), \
        ::SAL_DETAIL_LOG_LEVEL_WARN, area, SAL_WHERE, stream)

/**
  Produce temporary debugging output from stream.  This macro is meant to be
  used only while working on code and should never exist in production code.

  See @ref sal_log "basic logging functionality" for details.
*/
#define SAL_DEBUG(stream) \
    SAL_DETAIL_LOG_STREAM( \
        SAL_LOG_TRUE, ::SAL_DETAIL_LOG_LEVEL_DEBUG, NULL, NULL, stream)

/**
  Produce temporary debugging output from stream along with a backtrace of the
  calling location.

  This macro is meant to be used only while working on code and should never
  exist in production code.

  @param backtraceDepth a sal_uInt32 value indicating the maximum backtrace
  depth; zero means no backtrace

  See @ref sal_log "basic logging functionality" for details.
*/
#define SAL_DEBUG_BACKTRACE(stream, backtraceDepth) \
    do { \
        if (sizeof ::sal::detail::getResult( \
                ::sal::detail::StreamStart() << stream) == 1) \
        { \
            ::sal_detail_log( \
                ::SAL_DETAIL_LOG_LEVEL_DEBUG, NULL, NULL, \
                ::sal::detail::unwrapStream( \
                    ::sal::detail::StreamStart() << stream), \
                backtraceDepth); \
        } else { \
            ::std::ostringstream sal_detail_stream; \
            sal_detail_stream << stream; \
            ::sal::detail::log( \
                ::SAL_DETAIL_LOG_LEVEL_DEBUG, NULL, NULL, sal_detail_stream, \
                backtraceDepth); \
        } \
    } while (false)



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
