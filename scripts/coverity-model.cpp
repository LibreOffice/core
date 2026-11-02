/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* If this is modified it must be manually uploaded to coverity scan
 * at "Modeling File" in the Analysis Settings tab of
 * https://scan.coverity.com/projects/collaboraonline
 */

namespace std {
    class string {};
}

namespace CppUnit {
    class Message;
    class SourceLine;

    struct Asserter {
        static void failIf(bool shouldFail, const Message&, const SourceLine&) {
            if (shouldFail) __coverity_panic__();
        }

        static void failIf(bool shouldFail, std::string, const SourceLine&) {
            if (shouldFail) __coverity_panic__();
        }
    };
}

/* TYPELIB_DANGER_GET and TYPELIB_DANGER_RELEASE are a matched pair of inline functions in
 * typelib/typedescription.h. Together they hand the caller a borrowed pointer to a type
 * description and then give it back. Neither call changes who owns the reference the caller
 * already holds.
 *
 * Coverity derives its own model from the bodies and gets the ownership wrong. It reads the
 * release inside TYPELIB_DANGER_GET as a free of the caller's reference, then reports every
 * later read of the description as a use after free. The description is alive on that path,
 * because the caller still holds the reference it came from.
 *
 * Model the pair as ownership-neutral. The assignment keeps the aliasing that the real code
 * has, since for every type class that reaches this point the description is the reference.
 */
extern "C" {
    struct _typelib_TypeDescription;
    struct _typelib_TypeDescriptionReference;
    typedef struct _typelib_TypeDescription typelib_TypeDescription;
    typedef struct _typelib_TypeDescriptionReference typelib_TypeDescriptionReference;

    void TYPELIB_DANGER_GET(typelib_TypeDescription** ppMacroTypeDescr,
                            typelib_TypeDescriptionReference* pMacroTypeRef) {
        *ppMacroTypeDescr = reinterpret_cast<typelib_TypeDescription*>(pMacroTypeRef);
    }

    void TYPELIB_DANGER_RELEASE(typelib_TypeDescription*) {}
}

/* The sal log functions read the timezone, backtrace and locale caches of sal. Every static
 * object whose constructor or destructor can reach a log call is then reported as using another
 * file's global. sal is its own library, which every other library loads first, so its caches
 * outlive those objects. Model the log functions as doing nothing.
 */
extern "C" {
    enum sal_detail_LogLevel { SAL_DETAIL_LOG_LEVEL_INFO };

    void sal_detail_log(sal_detail_LogLevel, char const*, char const*, char const*, unsigned int) {}
    void sal_detail_logFormat(sal_detail_LogLevel, char const*, char const*, char const*, ...) {}
}

/* rtl_math_approxEqual returns false early when either argument is exactly zero. The derived model
 * keeps that test, so after a false result the second argument may be zero, even when the code
 * has already checked it against zero. Model the result as unknown, with no test of either
 * argument.
 */
extern "C" {
    bool coverity_unknown_bool();

    bool rtl_math_approxEqual(double, double) { return coverity_unknown_bool(); }
}

namespace __gnu_cxx {
    /* libstdc++ throws from these when a pthread mutex call fails. Terminating is the acceptable
     * response, so model them as paths that do not return.
     */
    void __throw_concurrence_lock_error() { __coverity_panic__(); }
    void __throw_concurrence_unlock_error() { __coverity_panic__(); }
    void __throw_concurrence_broadcast_error() { __coverity_panic__(); }
    void __throw_concurrence_wait_error() { __coverity_panic__(); }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
