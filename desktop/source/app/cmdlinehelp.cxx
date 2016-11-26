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


#include <stdlib.h>
#include <stdio.h>
#include <comphelper/string.hxx>
#include <sal/types.h>
#include <app.hxx>

#include "cmdlinehelp.hxx"

#ifdef _WIN32
#include "windows.h"
#include "io.h"
#include "Fcntl.h"
#endif

namespace desktop
{
    const char aCmdLineHelp_version[] =
        "%PRODUCTNAME %PRODUCTVERSION%PRODUCTEXTENSION\n"
        "Build ID: %BUILDID\n"
        "\n";
    const char aCmdLineHelp[] =
        "Usage: %CMDNAME [argument...]\n"
        "       argument - switches, switch parameters and document URIs (filenames).\n\n"
        "\x1b[1mGetting help and information\x1b[0m\n"
        "--help | -h | -?   Shows this help and quits.\n"
        "--helpwriter       Opens LbreOffice built-in Help or browser with Help on Writer.\n"
        "--helpcalc         Opens LbreOffice built-in Help or browser with Help on Calc.\n"
        "--helpdraw         Opens LbreOffice built-in Help or browser with Help on Draw.\n"
        "--helpimpress      Opens LbreOffice built-in Help or browser with Help on Impress.\n"
        "--helpbase         Opens LbreOffice built-in Help or browser with Help on Base.\n"
        "--helpbasic        Opens LbreOffice built-in Help or browser with Help on Basic scripting language.\n"
        "--helpmath         Opens LbreOffice built-in Help or browser with Help on Math.\n"
        "--version          Shows the version and quits.\n"
        "--nstemporarydirectory\n"
        "                   (MacOS X sandbox only) Returns path of the temporary directory for the current\n"
        "                   user and exits. Overrides all other arguments.\n\n"
        "\x1b[1mGeneral arguments\x1b[0m\n"
        "--quickstart[=no]  Activates[Deactivates] the Quickstarter service.\n"
        "--nolockcheck      Disables check for remote instances using this installation.\n"
        "--infilter={filter} Force an input filter type if possible.\n"
        "                   Eg. --infilter=\"Calc Office Open XML\"\n"
        "                       --infilter=\"Text (encoded):UTF8,LF,,,\"\n"
        "--pidfile={file}   Store soffice.bin pid to {file}.\n"
        "--display {display} Sets the DISPLAY environment variable on UNIX-like platforms to the value\n"
        "                   {display}. This parameter is only supported by the start script for\n"
        "                   the LibreOffice software on UNIX-like platforms.\n\n"
        "\x1b[1mUser/programmatic interface control\x1b[0m\n"
        "--nologo           Disables the splash screen at program start.\n"
        "--minimized        Starts minimized. The splash screen is not displayed.\n"
        "--nodefault        Starts without displaying anything except the splash screen (do not display\n"
        "                   initial window).\n"
        "--invisible        Starts in invisible mode. Neither the start-up logo nor the initial program\n"
        "                   window will be visible. LibreOffice can be controlled, and documents and dialogs\n"
        "                   can be controlled and opened via the API. Using this parameter, LibeOffice can\n"
        "                   only be ended using the taskmanager (Windows) or the kill command (UNIX-like\n"
        "                   systems). It cannot be used in conjunction with --quickstart.\n"
        "--headless         Starts in \"headless mode\" which allows using the application without GUI.\n"
        "                   This special mode can be used when the application is controlled by external\n"
        "                   clients via the API.\n"
        "--norestore        Disables restart and file recovery after a system crash.\n"
        "--splash-pipe      Uses com.sun.star.office.PipeSplashScreen instead of \n"
        "                   com.sun.star.office.SplashScreen.\n"
        "--accept={UNO-URL} Specifies an UNO-URL connect-string to create an UNO acceptor through which\n"
        "                   other programs can connect to access the API. UNO-URL is string the such kind\n"
        "                   uno:connection-type,params;protocol-name,params;ObjectName. For example:\n"
        "                   uno:socket,host=localhost,port=2002;urp;StarOffice.ServiceManager\n"
        "                   uno:pipe,name=mypipename;urp;MyObjectName\n"
        "--unaccept={UNO-URL} Closes an acceptor that was created with --accept={UNO string}.\n"
        "                   Use --unaccept=all to close all open acceptors.\n"
        "--language={lang}  Uses specified language, if language is not selected yet for UI. The lang is\n"
        "                   a tag of the language in IETF language tag.\n\n"
        "\x1b[1mDeveloper arguments\x1b[0m\n"
        "--terminate_after_init Developer option: exit after initialization complete (no documents loaded)\n"
        "--eventtesting     Developer option: exit after loading documents.\n"
        "--protector {arg1} {arg2} Used only in unit tests and suold have two arguments.\n\n"
        "\x1b[1mNew document creation arguments\x1b[0m\n"
        "The following arguments create an empty document of specified kind. Only one of them may be used in\n"
        "one command line. They have no effect if any file open arguments are specified.\n"
        "--writer           Creates an empty Writer document.\n"
        "--calc             Creates an empty Calc document.\n"
        "--draw             Creates an empty Draw document.\n"
        "--impress          Creates an empty Impress document.\n"
        "--base             Creates a new database.\n"
        "--global           Creates an empty Writer master (global) document.\n"
        "--math             Creates an empty Math document (formula).\n"
        "--web              Creates an empty HTML document.\n\n"
        "\x1b[1mFile open arguments\x1b[0m\n"
        "The following arguments define how following filenames are treated. New treatment begins after the\n"
        "argument, and ends at the next argument. The default treatment is to open documents for editing,\n"
        "and create new documents from document templates.\n"
        "-n                 Treat following filenames as templates for creation of new documents.\n"
        "-o                 Open following filenames for editing, regardless whether they are templates or not.\n"
        "--pt {Printername} Prints following filenames to the printer {Printername}, after which those files\n"
        "                   are closed. The splash screen does not appear. If used multiple times, only last\n"
        "                   {Printername} is effective for all documents of all --pt runs. Also, --printer-name\n"
        "                   argument of --print-to-file switch interferes with {Printername}.\n"
        "-p                 Prints following filenames to the default printer, after which those files are\n"
        "                   closed. The splash screen does not appear. If the file name contains spaces, then\n"
        "                   it must be enclosed in quotation marks.\n"
        "--view             Opens following filenames in viewer mode (read-only).\n"
        "--show             Opens following Impress documents and starts the presentation of each immediately.\n"
        "                   Files are closed after the presentation. Files that are not Impress documents are\n"
        "                   opened in default mode, regardless of previous mode.\n"
        "--convert-to output_file_extension[:output_filter_name] [--outdir output_dir]\n"
        "                   Batch convert files (implies --headless). If --outdir isn't specified, then\n"
        "                   current working directory is used as output_dir. If --convert-to is used more than\n"
        "                   once, only last value of output_file_extension[:output_filter_name] is effective.\n"
        "                   If --outdir is used more than once, only its last value is effective. For example:\n"
        "                   --convert-to pdf *.doc\n"
        "                   --convert-to pdf:writer_pdf_Export --outdir /home/user *.doc\n"
        "                   --convert-to \"html:XHTML Writer File:UTF8\" *.doc\n"
        "                   --convert-to \"txt:Text (encoded):UTF8\" *.doc\n"
        "--print-to-file [--printer-name printer_name] [--outdir output_dir]\n"
        "                   Batch print files to file. If --outdir is not specified, then current working\n"
        "                   directory is used as output_dir. If --printer-name or --outdir used multiple times,\n"
        "                   only last value of each is effective. Also, {Printername} of --pt switch interferes\n"
        "                   with --printer-name.\n"
        "--cat              Dump text content of the following files to console (implies --headless). Cannot be\n"
        "                   used with --convert-to.\n"
        "-env:<VAR>[=<VALUE>] Set a bootstrap variable. For example: to set a non-default user profile path:\n"
        "                   -env:UserInstallation=file:///tmp/test\n\n"
        "\x1b[1mIgnored switches\x1b[0m\n"
        "-psn               Ignored (MacOS X only)\n"
        "-Embedding         Ignored (COM+ related; Windows only)\n"
        "--nofirststartwizard Does nothing, accepted only for backward compatibility.\n\n";
#ifdef _WIN32
    namespace{
        class lcl_Console {
        public:
            explicit lcl_Console(short nBufHeight)
            {
                bFreeConsole = AllocConsole() != FALSE;
                CONSOLE_SCREEN_BUFFER_INFO cinfo;
                GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cinfo);
                cinfo.dwSize.Y = nBufHeight;
                SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), cinfo.dwSize);
                // stdin
                intptr_t stdHandle = reinterpret_cast<intptr_t>(GetStdHandle(STD_INPUT_HANDLE));
                int fileHandle = _open_osfhandle(stdHandle, _O_TEXT);
                FILE *fp = _fdopen(fileHandle, "r");
                *stdin = *fp;
                setvbuf(stdin, NULL, _IONBF, 0);
                // stdout
                stdHandle = reinterpret_cast<intptr_t>(GetStdHandle(STD_OUTPUT_HANDLE));
                fileHandle = _open_osfhandle(stdHandle, _O_TEXT);
                fp = _fdopen(fileHandle, "w");
                *stdout = *fp;
                setvbuf(stdout, NULL, _IONBF, 0);
                // stderr
                stdHandle = reinterpret_cast<intptr_t>(GetStdHandle(STD_ERROR_HANDLE));
                fileHandle = _open_osfhandle(stdHandle, _O_TEXT);
                fp = _fdopen(fileHandle, "w");
                *stderr = *fp;
                setvbuf(stderr, NULL, _IONBF, 0);
            }

            ~lcl_Console()
            {
                if (bFreeConsole)
                {
                    fprintf(stdout, "Press Enter to close this console...");
                    fgetc(stdin);
                    FreeConsole();
                }
            }
        private:
            bool bFreeConsole;
        };
    }
#endif

    void displayCmdlineHelp(OUString const & unknown)
    {
        OUString aHelpMessage_version = ReplaceStringHookProc(aCmdLineHelp_version);
        OUString aHelpMessage(OUString(aCmdLineHelp).replaceFirst("%CMDNAME", "soffice"));
        if (!unknown.isEmpty())
        {
            aHelpMessage = "Error in option: " + unknown + "\n\n"
                + aHelpMessage;
        }
#ifdef _WIN32
        sal_Int32 n = comphelper::string::getTokenCount(aHelpMessage, '\n');
        lcl_Console aConsole(short(n*2));
#endif
        fprintf(stdout, "%s%s",
                OUStringToOString(aHelpMessage_version, RTL_TEXTENCODING_ASCII_US).getStr(),
                OUStringToOString(aHelpMessage, RTL_TEXTENCODING_ASCII_US).getStr());
    }

    void displayVersion()
    {
        OUString aVersionMsg(aCmdLineHelp_version);
        aVersionMsg = ReplaceStringHookProc(aVersionMsg);
#ifdef _WIN32
        lcl_Console aConsole(short(10));
#endif
        fprintf(stdout, "%s", OUStringToOString(aVersionMsg, RTL_TEXTENCODING_ASCII_US).getStr());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
