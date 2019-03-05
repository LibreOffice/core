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
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

namespace desktop
{
    const char aCmdLineHelp_version[] =
        "%PRODUCTNAME %PRODUCTVERSION%PRODUCTEXTENSION %BUILDID\n"
        "\n";
    const char aCmdLineHelp[] =
        "Usage: %CMDNAME [argument...]\n"
        "       argument - switches, switch parameters and document URIs (filenames).   \n\n"
        "Using without special arguments:                                               \n"
        "Opens the start center, if it is used without any arguments.                   \n"
        "   {file}              Tries to open the file (files) in the components        \n"
        "                       suitable for them.                                      \n"
        "   {file} {macro:///Library.Module.MacroName}                                  \n"
        "                       Opens the file and runs specified macros from           \n"
        "                       the file.                                               \n\n"
        "Getting help and information:                                                  \n"
        "   --help | -h | -?    Shows this help and quits.                              \n"
        "   --helpwriter        Opens built-in or online Help on Writer.                \n"
        "   --helpcalc          Opens built-in or online Help on Calc.                  \n"
        "   --helpdraw          Opens built-in or online Help on Draw.                  \n"
        "   --helpimpress       Opens built-in or online Help on Impress.               \n"
        "   --helpbase          Opens built-in or online Help on Base.                  \n"
        "   --helpbasic         Opens built-in or online Help on Basic scripting        \n"
        "                       language.                                               \n"
        "   --helpmath          Opens built-in or online Help on Math.                  \n"
        "   --version           Shows the version and quits.                            \n"
        "   --nstemporarydirectory                                                      \n"
        "                       (MacOS X sandbox only) Returns path of the temporary    \n"
        "                       directory for the current user and exits. Overrides     \n"
        "                       all other arguments.                                    \n\n"
        "General arguments:                                                             \n"
        "   --quickstart[=no]   Activates[Deactivates] the Quickstarter service.        \n"
        "   --nolockcheck       Disables check for remote instances using one           \n"
        "                       installation.                                           \n"
        "   --infilter={filter} Force an input filter type if possible. For example:    \n"
        "                       --infilter=\"Calc Office Open XML\"                     \n"
        "                       --infilter=\"Text (encoded):UTF8,LF,,,\"                \n"
        "   --pidfile={file}    Store soffice.bin pid to {file}.                        \n"
        "   --display {display} Sets the DISPLAY environment variable on UNIX-like      \n"
        "                       platforms to the value {display} (only supported by a   \n"
        "                       start script).                                          \n\n"
        "User/programmatic interface control:                                           \n"
        "   --nologo            Disables the splash screen at program start.            \n"
        "   --minimized         Starts minimized. The splash screen is not displayed.   \n"
        "   --nodefault         Starts without displaying anything except the splash    \n"
        "                       screen (do not display initial window).                 \n"
        "   --invisible         Starts in invisible mode. Neither the start-up logo nor \n"
        "                       the initial program window will be visible. Application \n"
        "                       can be controlled, and documents and dialogs can be     \n"
        "                       controlled and opened via the API. Using the parameter, \n"
        "                       the process can only be ended using the taskmanager     \n"
        "                       (Windows) or the kill command (UNIX-like systems). It   \n"
        "                       cannot be used in conjunction with --quickstart.        \n"
        "   --headless          Starts in \"headless mode\" which allows using the      \n"
        "                       application without GUI. This special mode can be used  \n"
        "                       when the application is controlled by external clients  \n"
        "                       via the API.                                            \n"
        "   --norestore         Disables restart and file recovery after a system crash.\n"
        "   --safe-mode         Starts in a safe mode, i.e. starts temporarily with a   \n"
        "                       fresh user profile and helps to restore a broken        \n"
        "                       configuration.                                          \n"
        "   --accept={UNO-URL}  Specifies an UNO-URL connect-string to create an UNO    \n"
        "                       acceptor through which other programs can connect to    \n"
        "                       access the API. Note that API access allows execution   \n"
        "                       of arbitrary commands.                                  \n"
        "                       The syntax of an UNO-URL connect-string is:             \n"
        "                   uno:connection-type,params;protocol-name,params;ObjectName  \n"
        "   --unaccept={UNO-URL} Closes an acceptor that was created with --accept. Use \n"
        "                       --unaccept=all to close all open acceptors.             \n"
        "   --language={lang}   Uses specified language, if language is not selected    \n"
        "                       yet for UI. The lang is a tag of the language in IETF   \n"
        "                       language tag.                                           \n\n"
        "Developer arguments:                                                           \n"
        "   --terminate_after_init                                                      \n"
        "                       Exit after initialization complete (no documents loaded)\n"
        "   --eventtesting      Exit after loading documents.                           \n\n"
        "New document creation arguments:                                               \n"
        "The arguments create an empty document of specified kind. Only one of them may \n"
        "be used in one command line. If filenames are specified after an argument,     \n"
        "then it tries to open those files in the specified component.                  \n"
        "   --writer            Creates an empty Writer document.                       \n"
        "   --calc              Creates an empty Calc document.                         \n"
        "   --draw              Creates an empty Draw document.                         \n"
        "   --impress           Creates an empty Impress document.                      \n"
        "   --base              Creates a new database.                                 \n"
        "   --global            Creates an empty Writer master (global) document.       \n"
        "   --math              Creates an empty Math document (formula).               \n"
        "   --web               Creates an empty HTML document.                         \n\n"
        "File open arguments:                                                           \n"
        "The arguments define how following filenames are treated. New treatment begins \n"
        "after the argument and ends at the next argument. The default treatment is to  \n"
        "open documents for editing, and create new documents from document templates.  \n"
        "   -n                  Treats following files as templates for creation of new \n"
        "                       documents.                                              \n"
        "   -o                  Opens following files for editing, regardless whether   \n"
        "                       they are templates or not.                              \n"
        "   --pt {Printername}  Prints following files to the printer {Printername},    \n"
        "                       after which those files are closed. The splash screen   \n"
        "                       does not appear. If used multiple times, only last      \n"
        "                       {Printername} is effective for all documents of all     \n"
        "                       --pt runs. Also, --printer-name argument of             \n"
        "                       --print-to-file switch interferes with {Printername}.   \n"
        "   -p                  Prints following files to the default printer, after    \n"
        "                       which those files are closed. The splash screen does    \n"
        "                       not appear. If the file name contains spaces, then it   \n"
        "                       must be enclosed in quotation marks.                    \n"
        "   --view              Opens following files in viewer mode (read-only).       \n"
        "   --show              Opens and starts the following presentation documents   \n"
        "                       of each immediately. Files are closed after the showing.\n"
        "                       Files other than Impress documents are opened in        \n"
        "                       default mode , regardless of previous mode.             \n"
        "   --convert-to OutputFileExtension[:OutputFilterName] \\                      \n"
        "     [--outdir output_dir] [--convert-images-to]                               \n"
        "                       Batch convert files (implies --headless). If --outdir   \n"
        "                       isn't specified, then current working directory is used \n"
        "                       as output_dir. If --convert-images-to is given, its     \n"
        "                       parameter is taken as the target filter format for *all*\n"
        "                       images written to the output format. If --convert-to is \n"
        "                       used more than once, the last value of                  \n"
        "                       OutputFileExtension[:OutputFilterName] is effective. If \n"
        "                       --outdir is used more than once, only its last value is \n"
        "                       effective. For example:                                 \n"
        "                   --convert-to pdf *.odt                                      \n"
        "                   --convert-to epub *.doc                                     \n"
        "                   --convert-to pdf:writer_pdf_Export --outdir /home/user *.doc\n"
        "                   --convert-to \"html:XHTML Writer File:UTF8\" \\             \n"
        "                                --convert-images-to \"jpg\" *.doc              \n"
        "                   --convert-to \"txt:Text (encoded):UTF8\" *.doc              \n"
        "   --print-to-file [--printer-name printer_name] [--outdir output_dir]         \n"
        "                       Batch print files to file. If --outdir is not specified,\n"
        "                       then current working directory is used as output_dir.   \n"
        "                       If --printer-name or --outdir used multiple times, only \n"
        "                       last value of each is effective. Also, {Printername} of \n"
        "                       --pt switch interferes with --printer-name.             \n"
        "   --cat               Dump text content of the following files to console     \n"
        "                       (implies --headless). Cannot be used with --convert-to. \n"
        "   --script-cat        Dump text content of any scripts embedded in the files  \n"
        "                       to console (implies --headless). Cannot be used with    \n"
        "                       --convert-to.                                           \n"
        "   -env:<VAR>[=<VALUE>] Set a bootstrap variable. For example: to set          \n"
        "                       a non-default user profile path:                        \n"
        "                       -env:UserInstallation=file:///tmp/test                  \n\n"
        "Ignored switches:                                                              \n"
        "   -psn                Ignored (MacOS X only).                                 \n"
        "   -Embedding          Ignored (COM+ related; Windows only).                   \n"
        "   --nofirststartwizard Does nothing, accepted only for backward compatibility.\n"
        "   --protector {arg1} {arg2}                                                   \n"
        "                       Used only in unit tests and should have two arguments.  \n\n";
#ifdef _WIN32
    namespace{
        // This class is only used to create a console when soffice.bin is run without own console
        // (like using soffice.exe launcher as opposed to soffice.com), and either --version or
        // --help command line options were specified, or an error in a command line option was
        // detected, which requires to output strings to user.
        class lcl_Console {
        public:
            explicit lcl_Console(short nBufHeight)
                : m_bOwnConsole(AllocConsole() != FALSE)
            {
                if (m_bOwnConsole)
                {
                    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

                    // Ensure that console buffer is enough to hold required data
                    CONSOLE_SCREEN_BUFFER_INFO cinfo;
                    GetConsoleScreenBufferInfo(hOut, &cinfo);
                    if (cinfo.dwSize.Y < nBufHeight)
                    {
                        cinfo.dwSize.Y = nBufHeight;
                        SetConsoleScreenBufferSize(hOut, cinfo.dwSize);
                    }

                    (void)freopen("CON", "r", stdin);
                    (void)freopen("CON", "w", stdout);
                    (void)freopen("CON", "w", stderr);

                    std::ios::sync_with_stdio(true);
                }
            }

            ~lcl_Console()
            {
                if (m_bOwnConsole)
                {
                    fflush(stdout);
                    fprintf(stdout, "Press Enter to continue...");
                    fgetc(stdin);
                    FreeConsole();
                }
            }
        private:
            bool m_bOwnConsole;
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
