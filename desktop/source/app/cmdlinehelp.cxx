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
        "%PRODUCTNAME %PRODUCTVERSION%PRODUCTEXTENSION %BUILDID\n"
        "\n";
    const char aCmdLineHelp[] =
        "Usage: %CMDNAME [argument...]\n"
        "       argument - switches, switch parameters and document URIs (filenames).\n\n"
        "Getting help and information:\n"
        "   --help | -h | -?    Shows this help and quits.\n"
        "   --helpwriter        Opens built-in or online Help on Writer.\n"
        "   --helpcalc          Opens built-in or online Help on Calc.\n"
        "   --helpdraw          Opens built-in or online Help on Draw.\n"
        "   --helpimpress       Opens built-in or online Help on Impress.\n"
        "   --helpbase          Opens built-in or online Help on Base.\n"
        "   --helpbasic         Opens built-in or online Help on Basic scripting\n"
        "                       language.\n"
        "   --helpmath          Opens built-in or online Help on Math.\n"
        "   --version           Shows the version and quits.\n"
        "   --nstemporarydirectory\n"
        "                       (MacOS X sandbox only) Returns path of the temporary\n"
        "                       directory for the current user and exits. Overrides\n"
        "                       all other arguments.\n\n"
        "General arguments:\n"
        "   --quickstart[=no]   Activates[Deactivates] the Quickstarter service.\n"
        "   --nolockcheck       Disables check for remote instances using one\n"
        "                       installation.\n"
        "   --infilter={filter} Force an input filter type if possible. For example:\n"
        "                       --infilter=\"Calc Office Open XML\"\n"
        "                       --infilter=\"Text (encoded):UTF8,LF,,,\"\n"
        "   --pidfile={file}    Store soffice.bin pid to {file}.\n"
        "   --display {display} Sets the DISPLAY environment variable on UNIX-like\n"
        "                       platforms to the value {display} (only supported by a\n"
        "                       start script).\n\n"
        "User/programmatic interface control:\n"
        "   --nologo            Disables the splash screen at program start.\n"
        "   --minimized         Starts minimized. The splash screen is not displayed.\n"
        "   --nodefault         Starts without displaying anything except the splash\n"
        "                       screen (do not display initial window).\n"
        "   --invisible         Starts in invisible mode. Neither the start-up logo nor\n"
        "                       the initial program window will be visible. Application\n"
        "                       can be controlled, and documents and dialogs can be\n"
        "                       controlled and opened via the API. Using the parameter,\n"
        "                       the process can only be ended using the taskmanager\n"
        "                       (Windows) or the kill command (UNIX-like systems). It\n"
        "                       cannot be used in conjunction with --quickstart.\n"
        "   --headless          Starts in \"headless mode\" which allows using the\n"
        "                       application without GUI. This special mode can be used\n"
        "                       when the application is controlled by external clients\n"
        "                       via the API.\n"
        "   --norestore         Disables restart and file recovery after a system crash.\n"
        "   --safe-mode         Starts in a safe mode, i.e. starts temporarily with a\n"
        "                       fresh user profile and helps to restore a broken\n"
        "                       configuration.\n"
        "   --accept={UNO-URL}  Specifies an UNO-URL connect-string to create an UNO\n"
        "                       acceptor through which other programs can connect to\n"
        "                       access the API. UNO-URL is string the such kind\n"
        "                   uno:connection-type,params;protocol-name,params;ObjectName.\n"
        "   --unaccept={UNO-URL} Closes an acceptor that was created with --accept. Use\n"
        "                       --unaccept=all to close all open acceptors.\n"
        "   --language={lang}   Uses specified language, if language is not selected\n"
        "                       yet for UI. The lang is a tag of the language in IETF\n"
        "                       language tag.\n\n"
        "Developer arguments:\n"
        "   --terminate_after_init\n"
        "                       Exit after initialization complete (no documents loaded).\n"
        "   --eventtesting      Exit after loading documents.\n\n"
        "New document creation arguments:\n"
        "The arguments create an empty document of specified kind. Only one of them may\n"
        "be used in one command line. If filenames are specified after an argument,\n"
        "then it tries to open those files in the specified component.\n"
        "   --writer            Creates an empty Writer document.\n"
        "   --calc              Creates an empty Calc document.\n"
        "   --draw              Creates an empty Draw document.\n"
        "   --impress           Creates an empty Impress document.\n"
        "   --base              Creates a new database.\n"
        "   --global            Creates an empty Writer master (global) document.\n"
        "   --math              Creates an empty Math document (formula).\n"
        "   --web               Creates an empty HTML document.\n\n"
        "File open arguments:\n"
        "The arguments define how following filenames are treated. New treatment begins\n"
        "after the argument and ends at the next argument. The default treatment is to\n"
        "open documents for editing, and create new documents from document templates.\n"
        "   -n                  Treats following files as templates for creation of new\n"
        "                       documents.\n"
        "   -o                  Opens following files for editing, regardless whether\n"
        "                       they are templates or not.\n"
        "   --pt {Printername}  Prints following files to the printer {Printername},\n"
        "                       after which those files are closed. The splash screen\n"
        "                       does not appear. If used multiple times, only last\n"
        "                       {Printername} is effective for all documents of all\n"
        "                       --pt runs. Also, --printer-name argument of\n"
        "                       --print-to-file switch interferes with {Printername}.\n"
        "   -p                  Prints following files to the default printer, after\n"
        "                       which those files are closed. The splash screen does\n"
        "                       not appear. If the file name contains spaces, then it\n"
        "                       must be enclosed in quotation marks.\n"
        "   --view              Opens following files in viewer mode (read-only).\n"
        "   --show              Opens and starts the following presentation documents \n"
        "                       of each immediately. Files are closed after the showing.\n"
        "                       Files other than Impress documents are opened in\n"
        "                       default mode , regardless of previous mode.\n"
        "   --convert-to OutputFileExtension[:OutputFilterName] [--outdir output_dir]\n"
        "                       Batch convert files (implies --headless). If --outdir\n"
        "                       isn't specified, then current working directory is used\n"
        "                       as output_dir. If --convert-to is used more than once,\n"
        "                       last value of OutputFileExtension[:OutputFilterName] is\n"
        "                       effective. If --outdir is used more than once, only its\n"
        "                       last value is effective. For example:\n"
        "                   --convert-to pdf *.doc\n"
        "                   --convert-to pdf:writer_pdf_Export --outdir /home/user *.doc\n"
        "                   --convert-to \"html:XHTML Writer File:UTF8\" *.doc\n"
        "                   --convert-to \"txt:Text (encoded):UTF8\" *.doc\n"
        "   --print-to-file [--printer-name printer_name] [--outdir output_dir]\n"
        "                       Batch print files to file. If --outdir is not specified,\n"
        "                       then current working directory is used as output_dir.\n"
        "                       If --printer-name or --outdir used multiple times, only\n"
        "                       last value of each is effective. Also, {Printername} of\n"
        "                       --pt switch interferes with --printer-name.\n"
        "   --cat               Dump text content of the following files to console\n"
        "                       (implies --headless). Cannot be used with --convert-to.\n"
        "   -env:<VAR>[=<VALUE>] Set a bootstrap variable. For example: to set\n"
        "                       a non-default user profile path:\n"
        "                       -env:UserInstallation=file:///tmp/test\n\n"
        "Ignored switches:\n"
        "   -psn                Ignored (MacOS X only).\n"
        "   -Embedding          Ignored (COM+ related; Windows only).\n"
        "   --nofirststartwizard Does nothing, accepted only for backward compatibility.\n"
        "   --protector {arg1} {arg2}\n"
        "                       Used only in unit tests and should have two arguments.\n\n";
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
