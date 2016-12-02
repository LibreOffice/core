/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>
#include <config_features.h>
#include <config_version.h>
#include <config_folders.h>

#include <desktop/dllapi.h>

#include "app.hxx"
#include "cmdlineargs.hxx"
#include "cmdlinehelp.hxx"

#include <desktop/exithelper.h>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <sal/log.hxx>
#include <tools/extendapplicationenvironment.hxx>
#include <vcl/svmain.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XStorable2.hpp>
#include <comphelper/storagehelper.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <unotools/mediadescriptor.hxx>

#if HAVE_FEATURE_BREAKPAD
#include <fstream>
#include <desktop/crashreport.hxx>

#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID
#include <client/linux/handler/exception_handler.h>
#elif defined WNT
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-enum-value"
#endif
#include <client/windows/handler/exception_handler.h>
#if defined __clang__
#pragma clang diagnostic pop
#endif
#include <locale>
#include <codecvt>
#endif

#endif


#ifdef ANDROID
#  include <jni.h>
#  include <android/log.h>
#  include <salhelper/thread.hxx>

#  define LOGTAG "LibreOffice/sofficemain"
#  define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOGTAG, __VA_ARGS__))
#endif

#if HAVE_FEATURE_BREAKPAD

#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID
static bool dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor, void* /*context*/, bool succeeded)
{
    std::string ini_path = CrashReporter::getIniFileName();
    std::ofstream minidump_file(ini_path, std::ios_base::app);
    minidump_file << "DumpFile=" << descriptor.path() << "\n";;
    minidump_file.close();
    SAL_WARN("desktop", "minidump generated: " << descriptor.path());
    return succeeded;
}
#elif defined WNT
static bool dumpCallback(const wchar_t* path, const wchar_t* id,
                            void* /*context*/, EXCEPTION_POINTERS* /*exinfo*/,
                            MDRawAssertionInfo* /*assertion*/,
                            bool succeeded)
{
    std::string ini_path = CrashReporter::getIniFileName();
    std::ofstream minidump_file(ini_path, std::ios_base::app);
    // TODO: moggi: can we avoid this conversion
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv1;
    std::string aPath = conv1.to_bytes(std::wstring(path)) + conv1.to_bytes(std::wstring(id)) + ".dmp";
    minidump_file << "DumpFile=" << aPath << "\n";
    minidump_file << "GDIHandles=" << ::GetGuiResources (::GetCurrentProcess(), GR_GDIOBJECTS) << "\n";
    minidump_file.close();
    SAL_WARN("desktop", "minidump generated: " << aPath);
    return succeeded;
}
#endif

#endif
extern "C" int DESKTOP_DLLPUBLIC soffice_main()
{
#if HAVE_FEATURE_BREAKPAD

#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID
    google_breakpad::MinidumpDescriptor descriptor("/tmp");
    google_breakpad::ExceptionHandler eh(descriptor, nullptr, dumpCallback, nullptr, true, -1);

    CrashReporter::storeExceptionHandler(&eh);
#elif defined WNT
    google_breakpad::ExceptionHandler eh(L".", nullptr, dumpCallback, nullptr, google_breakpad::ExceptionHandler::HANDLER_ALL);

    CrashReporter::storeExceptionHandler(&eh);
#endif
#endif

#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID && !defined(LIBO_HEADLESS) && HAVE_FEATURE_OPENGL
    /* Run test for OpenGL support in own process to avoid crash with broken
     * OpenGL drivers. Start process as early as possible.
     */
    bool bSuccess = fire_glxtest_process();
    SAL_WARN_IF(!bSuccess, "desktop.opengl", "problems with glxtest");
#endif

#if defined ANDROID
    try {
        rtl::Bootstrap::setIniFilename("file:///assets/program/lofficerc");
#endif
    tools::extendApplicationEnvironment();

    desktop::Desktop aDesktop;
    // This string is used during initialization of the Gtk+ VCL module
    Application::SetAppName( "soffice" );
#ifdef UNX
    // handle --version and --help already here, otherwise they would be handled
    // after VCL initialization that might fail if $DISPLAY is not set
    const desktop::CommandLineArgs& rCmdLineArgs = desktop::Desktop::GetCommandLineArgs();
    const OUString& aUnknown( rCmdLineArgs.GetUnknown() );
    if ( !aUnknown.isEmpty() )
    {
        desktop::Desktop::InitApplicationServiceManager();
        desktop::displayCmdlineHelp( aUnknown );
        return EXIT_FAILURE;
    }
    if ( rCmdLineArgs.IsHelp() )
    {
        desktop::Desktop::InitApplicationServiceManager();
        desktop::displayCmdlineHelp( OUString() );
        return EXIT_SUCCESS;
    }
    if ( rCmdLineArgs.IsVersion() )
    {
        desktop::Desktop::InitApplicationServiceManager();
        desktop::displayVersion();
        return EXIT_SUCCESS;
    }
#endif
    return SVMain();
#if defined ANDROID
    } catch (const css::uno::Exception &e) {
        LOGI("Unhandled UNO exception: '%s'",
             OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
        throw; // to get exception type printed
    }
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
