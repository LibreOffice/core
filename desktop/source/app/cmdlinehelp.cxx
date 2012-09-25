/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <stdlib.h>
#ifdef UNX
#include <stdio.h>
#endif
#include <comphelper/string.hxx>
#include <sal/types.h>
#include <tools/string.hxx>
#include <vcl/msgbox.hxx>
#include <rtl/bootstrap.hxx>
#include <app.hxx>

#include "desktopresid.hxx"
#include "desktop.hrc"
#include "cmdlinehelp.hxx"

namespace desktop
{
    // to be able to display the help nicely in a dialog box with propotional font,
    // we need to split it in chunks...
    //  ___HEAD___
    //  LEFT RIGHT
    //  LEFT RIGHT
    //  LEFT RIGHT
    //  __BOTTOM__
    //     [OK]

    const char aCmdLineHelp_version[] =
        "%PRODUCTNAME %PRODUCTVERSION %PRODUCTEXTENSION\n"\
        "\n";
    const char aCmdLineHelp_head[] =
        "Usage: %CMDNAME [options] [documents...]\n"\
        "\n"\
        "Options:\n";
    const char aCmdLineHelp_left[] =
        "--minimized    \n"\
        "--invisible    \n"\
        "--norestore    \n"\
        "--quickstart   \n"\
        "--nologo       \n"\
        "--nolockcheck  \n"\
        "--nodefault    \n"\
        "--headless     \n"\
        "--help/-h/-?   \n"\
        "--version      \n"\
        "--writer       \n"\
        "--calc         \n"\
        "--draw         \n"\
        "--impress      \n"\
        "--base         \n"\
        "--math         \n"\
        "--global       \n"\
        "--web          \n"\
        "-o             \n"\
        "-n             \n";
    const char aCmdLineHelp_right[] =
        "keep startup bitmap minimized.\n"\
        "no startup screen, no default document and no UI.\n"\
        "suppress restart/restore after fatal errors.\n"\
        "starts the quickstart service\n"\
        "don't show startup screen.\n"\
        "don't check for remote instances using the installation\n"\
        "don't start with an empty document\n"\
        "like invisible but no userinteraction at all.\n"\
        "show this message and exit.\n"\
        "display the version information.\n"\
        "create new text document.\n"\
        "create new spreadsheet document.\n"\
        "create new drawing.\n"\
        "create new presentation.\n"\
        "create new database.\n"\
        "create new formula.\n"\
        "create new global document.\n"\
        "create new HTML document.\n"\
        "open documents regardless whether they are templates or not.\n"\
        "always open documents as new files (use as template).\n";
    const char aCmdLineHelp_bottom[] =
        "--display <display>\n"\
        "      Specify X-Display to use in Unix/X11 versions.\n"
        "-p <documents...>\n"\
        "      print the specified documents on the default printer.\n"\
        "--pt <printer> <documents...>\n"\
        "      print the specified documents on the specified printer.\n"\
        "--view <documents...>\n"\
        "      open the specified documents in viewer-(readonly-)mode.\n"\
        "--show <presentation>\n"\
        "      open the specified presentation and start it immediately\n"\
        "--accept=<accept-string>\n"\
        "      Specify an UNO connect-string to create an UNO acceptor through which\n"\
        "      other programs can connect to access the API\n"\
        "--unaccept=<accept-string>\n"\
        "      Close an acceptor that was created with --accept=<accept-string>\n"\
        "      Use --unnaccept=all to close all open acceptors\n"\
        "--infilter=<filter>\n"\
        "      Force an input filter type if possible\n"\
        "      Eg. --infilter=\"Calc Office Open XML\"\n"\
        "--convert-to output_file_extension[:output_filter_name] [--outdir output_dir] files\n"\
        "      Batch convert files.\n"\
        "      If --outdir is not specified then current working dir is used as output_dir.\n"\
        "      Eg. --convert-to pdf *.doc\n"\
        "          --convert-to pdf:writer_pdf_Export --outdir /home/user *.doc\n"\
        "--print-to-file [-printer-name printer_name] [--outdir output_dir] files\n"\
        "      Batch print files to file.\n"\
        "      If --outdir is not specified then current working dir is used as output_dir.\n"\
        "      Eg. --print-to-file *.doc\n"\
        "          --print-to-file --printer-name nasty_lowres_printer --outdir /home/user *.doc\n"\
        "\nRemaining arguments will be treated as filenames or URLs of documents to open.\n\n";

    rtl::OUString ReplaceStringHookProc(const rtl::OUString& rStr);

    void displayCmdlineHelp(OUString const & unknown)
    {
        // if you put variables in other chunks don't forget to call the replace routines
        // for those chunks...
        String aHelpMessage_version(aCmdLineHelp_version, RTL_TEXTENCODING_ASCII_US);
        String aHelpMessage_head(aCmdLineHelp_head, RTL_TEXTENCODING_ASCII_US);
        String aHelpMessage_left(aCmdLineHelp_left, RTL_TEXTENCODING_ASCII_US);
        String aHelpMessage_right(aCmdLineHelp_right, RTL_TEXTENCODING_ASCII_US);
        String aHelpMessage_bottom(aCmdLineHelp_bottom, RTL_TEXTENCODING_ASCII_US);
        aHelpMessage_version = ReplaceStringHookProc(aHelpMessage_version);
        aHelpMessage_head.SearchAndReplaceAscii( "%CMDNAME", String( "soffice", RTL_TEXTENCODING_ASCII_US) );
        if (!unknown.isEmpty())
        {
            aHelpMessage_head = "Unknown option: " + unknown + "\n\n"
                + aHelpMessage_head;
        }
#ifdef UNX
        // on unix use console for output
        fprintf(stdout, "%s%s",
                rtl::OUStringToOString(aHelpMessage_version, RTL_TEXTENCODING_ASCII_US).getStr(),
                rtl::OUStringToOString(aHelpMessage_head, RTL_TEXTENCODING_ASCII_US).getStr());
        // merge left and right column
        sal_Int32 n = comphelper::string::getTokenCount(aHelpMessage_left, '\n');
        rtl::OString bsLeft(rtl::OUStringToOString(aHelpMessage_left,
            RTL_TEXTENCODING_ASCII_US));
        rtl::OString bsRight(rtl::OUStringToOString(aHelpMessage_right,
            RTL_TEXTENCODING_ASCII_US));
        for ( sal_Int32 i = 0; i < n; ++i )
        {
            using comphelper::string::getToken;
            fprintf(stdout, "%s", getToken(bsLeft, i, '\n').getStr());
            fprintf(stdout, "%s\n", getToken(bsRight, i, '\n').getStr());
        }
        fprintf(stdout, "%s", rtl::OUStringToOString(aHelpMessage_bottom,
                    RTL_TEXTENCODING_ASCII_US).getStr());
#else
        // rest gets a dialog box
        CmdlineHelpDialog aDlg;
        String head = aHelpMessage_version;
        head.Append(aHelpMessage_head);
        aDlg.m_ftHead.SetText(head);
        aDlg.m_ftLeft.SetText(aHelpMessage_left);
        aDlg.m_ftRight.SetText(aHelpMessage_right);
        aDlg.m_ftBottom.SetText(aHelpMessage_bottom);
        aDlg.Execute();
#endif
    }

    void displayVersion()
    {
        rtl::OUString aVersionMsg(aCmdLineHelp_version);
        aVersionMsg = ReplaceStringHookProc(aVersionMsg);
#ifdef UNX
        fprintf(stdout, "%s", rtl::OUStringToOString(aVersionMsg, RTL_TEXTENCODING_ASCII_US).getStr());
#else
        // Just re-use the help dialog for now.
        CmdlineHelpDialog aDlg;
        aDlg.m_ftHead.SetText(aVersionMsg);
        aDlg.Execute();
#endif
    }

#ifndef UNX
    CmdlineHelpDialog::CmdlineHelpDialog (void)
    : ModalDialog( NULL, DesktopResId( DLG_CMDLINEHELP ) )
    , m_ftHead( this, DesktopResId( TXT_DLG_CMDLINEHELP_HEADER ) )
    , m_ftLeft( this, DesktopResId( TXT_DLG_CMDLINEHELP_LEFT ) )
    , m_ftRight( this, DesktopResId( TXT_DLG_CMDLINEHELP_RIGHT ) )
    , m_ftBottom( this, DesktopResId( TXT_DLG_CMDLINEHELP_BOTTOM ) )
    , m_btOk( this, DesktopResId( BTN_DLG_CMDLINEHELP_OK ) )
    {
        FreeResource();
    }
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
