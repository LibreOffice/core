/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DESKTOP_CRASHREPORT_HXX
#define INCLUDED_DESKTOP_CRASHREPORT_HXX

#include <desktop/dllapi.h>

#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>

#include <config_features.h>

#include <map>
#include <string>

namespace google_breakpad
{
class ExceptionHandler;
}

/**
 * Provides access to the crash reporter service.
 *
 * Valid keys are:
 *      * AdapterVendorId
 *      * AdapterDeviceId
 *
 */
class
#if HAVE_FEATURE_BREAKPAD
// MSVC 2013 has undefined symbols for inline functions if it's exported
CRASHREPORT_DLLPUBLIC
#endif
/*class*/ CrashReporter
{
public:
    static void AddKeyValue(const OUString& rKey, const OUString& rValue);

    static std::string getIniFileName();

    static void writeCommonInfo();

    static void storeExceptionHandler(google_breakpad::ExceptionHandler* pExceptionHandler);

    // when we create the ExceptionHandler we have no access to the user
    // profile yet, so update when we have access
    static void updateMinidumpLocation();

private:

    static osl::Mutex maMutex;

    static bool mbInit;

    static std::map<OUString, OUString> maKeyValues; // used to temporarily save entries before the old info has been uploaded

    static google_breakpad::ExceptionHandler* mpExceptionHandler;
};

// Add dummy methods for the non-breakpad case. That allows us to use
// the code without linking to the lib and without adding HAVE_FEATURE_BREAKPAD
// everywhere we want to log something to the crash report system.
#if HAVE_FEATURE_BREAKPAD
#else
inline void CrashReporter::AddKeyValue(SAL_UNUSED_PARAMETER const OUString& /*rKey*/, SAL_UNUSED_PARAMETER const OUString& /*rValue*/)
{
}
#endif


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
