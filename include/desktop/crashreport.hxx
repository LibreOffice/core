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

#include <vector>
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
#if HAVE_FEATURE_BREAKPAD
    typedef enum {AddItem, Write, Create} tAddKeyHandling;
    static void AddKeyValue(const OUString& rKey, const OUString& rValue, tAddKeyHandling AddKeyHandling);

    static void StoreExceptionHandler(google_breakpad::ExceptionHandler* pExceptionHandler);

    static bool CrashReportInfoExists();

    static bool ReadSendConfig(std::string& response);

private:
    static osl::Mutex maMutex;
    static bool mbInit;
    typedef  struct _mpair
    {
        OUString first;
        OUString second;
        _mpair(const OUString& First, const OUString& Second)
        {
            first  = First;
            second = Second;
        };
    } mpair;

    typedef std::vector<mpair> vmaKeyValues;
    static vmaKeyValues maKeyValues; // used to temporarily save entries before the old info has been uploaded

    static google_breakpad::ExceptionHandler* mpExceptionHandler;

    static std::string getIniFileName();
    static void WriteCommonInfo();
    static void WriteToFile(std::ios_base::openmode Openmode);
    // when we create the ExceptionHandler we have no access to the user
    // profile yet, so update when we have access
    static void updateMinidumpLocation();

#else
    // Add dummy methods for the non-breakpad case. That allows us to use
    // // the code without linking to the lib and without adding HAVE_FEATURE_BREAKPAD
    // // everywhere we want to log something to the crash report system.
    typedef enum {AddItem, Write, InitWrite} tAddKeyHandling;
    template<typename T1, typename T2>
    static void AddKeyValue(SAL_UNUSED_PARAMETER const T1& /*rKey*/, SAL_UNUSED_PARAMETER const T2& /*rValue*/, SAL_UNUSED_PARAMETER tAddKeyHandling /*AddKeyHandling*/) {};
#endif
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
