/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include <stdlib.h>
#ifdef UNX
#include <stdio.h>
#endif
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
    // To be able to display the help nicely in a dialog box with proportional font,
    // we need to split it in chunks...
    //  ___HEAD___
    //  LEFT RIGHT
    //  LEFT RIGHT
    //  LEFT RIGHT
    //  __BOTTOM__
    //     [OK]

    const char *aCmdLineHelp_head =
        "%PRODUCTNAME %PRODUCTVERSION %PRODUCTEXTENSION %BUILDID\n"\
        "\n"\
        "Usage: %CMDNAME [options] [documents...]\n"\
        "\n"\
        "Options:\n";
    const char *aCmdLineHelp_left =
        "-minimized      \n"\
        "-invisible      \n"\
        "-norestore      \n"\
        "-quickstart     \n"\
        "-nologo         \n"\
        "-nolockcheck    \n"\
        "-nodefault      \n"\
        "-headless       \n"\
        "-conversionmode \n"\
        "-help/-h/-?     \n"\
        "-writer         \n"\
        "-calc           \n"\
        "-draw           \n"\
        "-impress        \n"\
        "-base           \n"\
        "-math           \n"\
        "-global         \n"\
        "-web            \n"\
        "-o              \n"\
        "-n              \n";
    const char *aCmdLineHelp_right =
        "Keep startup screen minimized.\n"\
        "No startup screen, no default document and no UI.\n"\
        "Suppress restart/restore after fatal errors.\n"\
        "Start the quickstart service (only available on Windows and OS/2).\n"\
        "Don't show startup screen.\n"\
        "Don't check for remote instances using the installation.\n"\
        "Don't start with an empty document.\n"\
        "Like invisible but no user interaction at all.\n"\
        "Enable further optimization for document conversion, incl. headless mode.\n"\
        "Show this message.\n"\
        "Create new text document.\n"\
        "Create new spreadsheet document.\n"\
        "Create new drawing.\n"\
        "Create new presentation.\n"\
        "Create new database.\n"\
        "Create new formula.\n"\
        "Create new global document.\n"\
        "Create new HTML document.\n"\
        "Open documents regardless whether they are templates or not.\n"\
        "Always open documents as new files (use as template).\n";
    const char *aCmdLineHelp_bottom =
        "-display <display>\n"\
        "      Specify X-Display to use in Unix/X11 versions.\n"
        "-p <documents...>\n"\
        "      Print the specified documents on the default printer.\n"\
        "-pt <printer> <documents...>\n"\
        "      Print the specified documents on the specified printer.\n"\
        "-view <documents...>\n"\
        "      Open the specified documents in viewer-(readonly-)mode.\n"\
        "-show <presentation>\n"\
        "      Open the specified presentation and start it immediately.\n"\
        "-accept=<accept-string>\n"\
        "      Specify an UNO connect-string to create an UNO acceptor through which\n"\
        "      other programs can connect to access the API.\n"\
        "-unaccept=<accept-string>\n"\
        "      Close an acceptor that was created with -accept=<accept-string>\n"\
        "      Use -unnaccept=all to close all open acceptors.\n"\
        "Remaining arguments will be treated as filenames or URLs of documents to open.\n";

    void ReplaceStringHookProc( UniString& rStr );

    void displayCmdlineHelp()
    {
        // If you put variables in other chunks don't forget to call the replace routines
        // for those chunks...
        String aHelpMessage_head(aCmdLineHelp_head, RTL_TEXTENCODING_ASCII_US);
        String aHelpMessage_left(aCmdLineHelp_left, RTL_TEXTENCODING_ASCII_US);
        String aHelpMessage_right(aCmdLineHelp_right, RTL_TEXTENCODING_ASCII_US);
        String aHelpMessage_bottom(aCmdLineHelp_bottom, RTL_TEXTENCODING_ASCII_US);
        ReplaceStringHookProc(aHelpMessage_head);
        ::rtl::OUString aDefault;
        String aVerId( ::utl::Bootstrap::getBuildIdData( aDefault ));
        aHelpMessage_head.SearchAndReplaceAscii( "%BUILDID", aVerId );
        aHelpMessage_head.SearchAndReplaceAscii( "%CMDNAME", String( "soffice", RTL_TEXTENCODING_ASCII_US) );
#ifdef UNX
        // On Unix use console for output
        fprintf(stderr, "%s\n", ByteString(aHelpMessage_head, RTL_TEXTENCODING_ASCII_US).GetBuffer());
        // Merge left and right column
        int n = aHelpMessage_left.GetTokenCount ('\n');
        ByteString bsLeft(aHelpMessage_left, RTL_TEXTENCODING_ASCII_US);
        ByteString bsRight(aHelpMessage_right, RTL_TEXTENCODING_ASCII_US);
        for ( int i = 0; i < n; i++ )
        {
            fprintf(stderr, "%s", bsLeft.GetToken(i, '\n').GetBuffer());
            fprintf(stderr, "%s\n", bsRight.GetToken(i, '\n').GetBuffer());
        }
        fprintf(stderr, "%s", ByteString(aHelpMessage_bottom, RTL_TEXTENCODING_ASCII_US).GetBuffer());
#else
        // All other get a dialog box
        CmdlineHelpDialog aDlg;
        aDlg.m_ftHead.SetText(aHelpMessage_head);
        aDlg.m_ftLeft.SetText(aHelpMessage_left);
        aDlg.m_ftRight.SetText(aHelpMessage_right);
        aDlg.m_ftBottom.SetText(aHelpMessage_bottom);
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
