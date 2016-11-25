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
#if _WIN32_WINNT < 0x0501
#define _WIN32_WINNT 0x0501
#endif
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
        "Usage: %CMDNAME [options] [documents...]\n"
        "\n"
        "Options:\n"
        "--minimized    keep startup bitmap minimized.\n"
        "--invisible    no startup screen, no default document and no UI.\n"
        "--norestore    suppress restart/restore after fatal errors.\n"
        "--quickstart   starts the quickstart service\n"
        "--safe-mode    starts the safe mode\n"
        "--nologo       don't show startup screen.\n"
        "--nolockcheck  don't check for remote instances using the installation\n"
        "--nodefault    don't start with an empty document\n"
        "--headless     like invisible but no user interaction at all.\n"
        "--help/-h/-?   show this message and exit.\n"
        "--version      display the version information.\n"
        "--writer       create new text document.\n"
        "--calc         create new spreadsheet document.\n"
        "--draw         create new drawing.\n"
        "--impress      create new presentation.\n"
        "--base         create new database.\n"
        "--math         create new formula.\n"
        "--global       create new global document.\n"
        "--web          create new HTML document.\n"
        "-o             open documents regardless whether they are templates or not.\n"
        "-n             always open documents as new files (use as template).\n"
        "--display <display>\n"
        "      Specify X-Display to use in Unix/X11 versions.\n"
        "-p <documents...>\n"
        "      print the specified documents on the default printer.\n"
        "--pt <printer> <documents...>\n"
        "      print the specified documents on the specified printer.\n"
        "--view <documents...>\n"
        "      open the specified documents in viewer-(readonly-)mode.\n"
        "--show <presentation>\n"
        "      open the specified presentation and start it immediately\n"
        "--language=<language_tag>\n"
        "      Override the UI language with the given locale\n"
        "      Eg. --language=fr\n"
        "--accept=<accept-string>\n"
        "      Specify an UNO connect-string to create an UNO acceptor through which\n"
        "      other programs can connect to access the API\n"
        "--unaccept=<accept-string>\n"
        "      Close an acceptor that was created with --accept=<accept-string>\n"
        "      Use --unnaccept=all to close all open acceptors\n"
        "--infilter=<filter>[:filter_options]\n"
        "      Force an input filter type if possible\n"
        "      Eg. --infilter=\"Calc Office Open XML\"\n"
        "          --infilter=\"Text (encoded):UTF8,LF,,,\"\n"
        "--convert-to output_file_extension[:output_filter_name[:output_filter_options]] [--outdir output_dir] files\n"
        "      Batch convert files (implies --headless).\n"
        "      If --outdir is not specified then current working dir is used as output_dir.\n"
        "      Eg. --convert-to pdf *.doc\n"
        "          --convert-to pdf:writer_pdf_Export --outdir /home/user *.doc\n"
        "          --convert-to \"html:XHTML Writer File:UTF8\" *.doc\n"
        "          --convert-to \"txt:Text (encoded):UTF8\" *.doc\n"
        "--print-to-file [-printer-name printer_name] [--outdir output_dir] files\n"
        "      Batch print files to file.\n"
        "      If --outdir is not specified then current working dir is used as output_dir.\n"
        "      Eg. --print-to-file *.doc\n"
        "          --print-to-file --printer-name nasty_lowres_printer --outdir /home/user *.doc\n"
        "--cat files\n"
        "      Dump text content of the files to console\n"
        "      Eg. --cat *.odt\n"
        "--pidfile=file\n"
        "      Store soffice.bin pid to file.\n"
        "-env:<VAR>[=<VALUE>]\n"
        "      Set a bootstrap variable.\n"
        "      Eg. -env:UserInstallation=file:///tmp/test to set a non-default user profile path.\n"
        "\nRemaining arguments will be treated as filenames or URLs of documents to open.\n\n";

#ifdef _WIN32
    namespace{
        class lcl_Console {
            enum eConsoleMode { unknown, attached, allocated };
        public:
            explicit lcl_Console(short nBufHeight)
                : mConsoleMode(unknown)
            {
                HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE),
                    hOut = GetStdHandle(STD_OUTPUT_HANDLE),
                    hErr = GetStdHandle(STD_ERROR_HANDLE);
                if (hOut == NULL) // application does not have associated standard handles
                {
                    STARTUPINFOA aStartupInfo{ sizeof(aStartupInfo) };
                    GetStartupInfoA(&aStartupInfo);
                    if ((aStartupInfo.dwFlags & STARTF_USESTDHANDLES) == STARTF_USESTDHANDLES)
                    {
                        // If standard handles had been passed to this process, use them
                        hIn = aStartupInfo.hStdInput;
                        hOut = aStartupInfo.hStdOutput;
                        hErr = aStartupInfo.hStdError;
                    }
                    else
                    {
                        // Try to attach parent console; on error try to create new.
                        // If this process already has its console, these will simply fail.
                        if (AttachConsole(ATTACH_PARENT_PROCESS) != FALSE)
                            mConsoleMode = attached;
                        else if (AllocConsole() != FALSE)
                            mConsoleMode = allocated;

                        hIn = GetStdHandle(STD_INPUT_HANDLE);
                        hOut = GetStdHandle(STD_OUTPUT_HANDLE);
                        hErr = GetStdHandle(STD_ERROR_HANDLE);

                        // Ensure that console buffer is enough to hold required data
                        CONSOLE_SCREEN_BUFFER_INFO cinfo;
                        GetConsoleScreenBufferInfo(hOut, &cinfo);
                        if (cinfo.dwSize.Y < nBufHeight)
                        {
                            cinfo.dwSize.Y = nBufHeight;
                            SetConsoleScreenBufferSize(hOut, cinfo.dwSize);
                        }
                    }
                }

                // stdin
                int fileHandle = _open_osfhandle(reinterpret_cast<intptr_t>(hIn), _O_TEXT);
                FILE *fp = _fdopen(fileHandle, "r");
                *stdin = *fp;
                setvbuf(stdin, NULL, _IONBF, 0);
                // stdout
                fileHandle = _open_osfhandle(reinterpret_cast<intptr_t>(hOut), _O_TEXT);
                fp = _fdopen(fileHandle, "w");
                *stdout = *fp;
                setvbuf(stdout, NULL, _IONBF, 0);
                // stderr
                fileHandle = _open_osfhandle(reinterpret_cast<intptr_t>(hErr), _O_TEXT);
                fp = _fdopen(fileHandle, "w");
                *stderr = *fp;
                setvbuf(stderr, NULL, _IONBF, 0);

                std::ios::sync_with_stdio(true);

                // In case we use parent's console, emit an empty string
                // to avoid output on a line with command prompt
                if (mConsoleMode == attached)
                    fprintf(stdout, "\n");
            }

            ~lcl_Console()
            {
                switch (mConsoleMode) {
                case unknown:
                    // Don't free the console
                    return;
                case attached:
                {
                    // Put Enter keypress to console input buffer to emit next command prompt after the command
                    INPUT_RECORD ir;
                    ir.EventType = KEY_EVENT;
                    KEY_EVENT_RECORD& ke = ir.Event.KeyEvent;
                    ke.bKeyDown = TRUE;
                    ke.wRepeatCount = 1;
                    ke.wVirtualKeyCode = VK_RETURN;
                    ke.wVirtualScanCode = MapVirtualKeyA(VK_RETURN, MAPVK_VK_TO_VSC);
                    ke.uChar.AsciiChar = '\r';
                    ke.dwControlKeyState = 0;
                    DWORD nEvents;
                    WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &ir, 1, &nEvents);
                    break;
                }
                case allocated:
                    fprintf(stdout, "Press Enter to continue...");
                    fgetc(stdin);
                    break;
                }

                FreeConsole();
            }
        private:
            eConsoleMode mConsoleMode;
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
