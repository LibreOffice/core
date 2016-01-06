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
#include <config_version.h>
#include <config_folders.h>

#include <desktop/dllapi.h>

#include "app.hxx"
#include "exithelper.h"
#include "cmdlineargs.hxx"
#include "cmdlinehelp.hxx"

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

OString getLibDir()
{
    OUString aOriginal = "$BRAND_BASE_DIR/" LIBO_LIBEXEC_FOLDER;
    rtl::Bootstrap::expandMacros(aOriginal);

    return rtl::OUStringToOString(aOriginal, RTL_TEXTENCODING_UTF8);
}

#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID
static bool dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor, void* /*context*/, bool succeeded)
{
    std::string ini_path = CrashReporter::getIniFileName();
    std::ofstream minidump_file(ini_path, std::ios_base::app);
    minidump_file << "DumpFile=" << descriptor.path() << "\n";;
    minidump_file.close();
    // send the minidump to the server (not yet implemented)
    SAL_WARN("crashreport", "minidump generated: " << descriptor.path());
    OString aCommand = getLibDir().copy(7) + "/minidump_upload " + ini_path.c_str();
    int retVal = std::system(aCommand.getStr());
    SAL_WARN_IF(retVal != 0, "crashreport", "Failed to upload minidump. Error Code: " << retVal);
    return succeeded;
}
#endif

#endif
extern "C" int DESKTOP_DLLPUBLIC soffice_main()
{
#if HAVE_FEATURE_BREAKPAD
    //limit the amount of code that needs to be executed before the crash reporting

    std::string ini_path = CrashReporter::getIniFileName();
    std::ofstream minidump_file(ini_path, std::ios_base::trunc);
    minidump_file << "ProductName=LibreOffice\n";
    minidump_file << "Version=" << LIBO_VERSION_DOTTED << "\n";
    minidump_file << "URL=" << "http://127.0.0.1:8000/submit" << "\n";
    minidump_file.close();

#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID
    google_breakpad::MinidumpDescriptor descriptor("/tmp");
    google_breakpad::ExceptionHandler eh(descriptor, NULL, dumpCallback, NULL, true, -1);
#else

#endif
#endif

#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID && !defined(LIBO_HEADLESS)
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
    OUString aUnknown( rCmdLineArgs.GetUnknown() );
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

#if defined(ANDROID) || defined(IOS)

extern "C" void PtylTestEncryptionAndExport(const char *pathname)
{
    OUString sUri(pathname, strlen(pathname), RTL_TEXTENCODING_UTF8);
    sUri = "file://" + sUri;

    css::uno::Reference<css::frame::XComponentLoader> loader(css::frame::Desktop::create(cppu::defaultBootstrap_InitialComponentContext()), css::uno::UNO_QUERY);
    css::uno::Reference<css::lang::XComponent> component;
    component.set(loader->loadComponentFromURL(sUri, "_default", 0, {}));

    utl::MediaDescriptor media;
    media[utl::MediaDescriptor::PROP_FILTERNAME()] <<= OUString("MS Word 2007 XML");
    OUString password("myPassword");
    css::uno::Sequence<css::beans::NamedValue> encryptionData { { "OOXPassword", css::uno::makeAny(password) } };
    media[utl::MediaDescriptor::PROP_ENCRYPTIONDATA()] <<= encryptionData;

    css::uno::Reference<css::frame::XModel> model(component, css::uno::UNO_QUERY);
    css::uno::Reference<css::frame::XStorable2> storable2(model, css::uno::UNO_QUERY);
    OUString saveAsUri(sUri + ".new.docx");
    SAL_INFO("desktop.app", "Trying to store as " << saveAsUri);
    OUString testPathName;
    osl::File::getSystemPathFromFileURL(saveAsUri+".txt", testPathName);
    storable2->storeToURL(saveAsUri, media.getAsConstPropertyValueList());
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
