/*************************************************************************
 *
 *  $RCSfile: cmdlinehelp.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 14:39:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
    // to be able to display the help nicely in a dialog box with propotional font,
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
        "-minimized    \n"\
        "-invisible    \n"\
        "-norestore    \n"\
        "-quickstart   \n"\
        "-nologo       \n"\
        "-nolockcheck  \n"\
        "-nodefault    \n"\
        "-headless     \n"\
        "-help/-h/-?   \n"\
        "-writer       \n"\
        "-calc         \n"\
        "-draw         \n"\
        "-impress      \n"\
        "-base         \n"\
        "-math         \n"\
        "-global       \n"\
        "-web          \n"\
        "-o            \n"\
        "-n            \n";
    const char *aCmdLineHelp_right =
        "keep startup bitmap minimized.\n"\
        "no startup screen, no default document and no UI.\n"\
        "suppress restart/restore after fatal errors.\n"\
        "starts the quickstart service\n"\
        "don't show startup screen.\n"\
        "don't check for remote instances using the installation\n"\
        "don't start with an empty document\n"\
        "like invisible but no userinteraction at all.\n"\
        "show this message and exit.\n"\
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
    const char *aCmdLineHelp_bottom =
        "-display <display>\n"\
        "      Specify X-Display to use in Unix/X11 versions.\n"
        "-p <documents...>\n"\
        "      print the specified documents on the default printer.\n"\
        "-pt <printer> <documents...>\n"\
        "      print the specified documents on the specified printer.\n"\
        "-view <documents...>\n"\
        "      open the specified documents in viewer-(readonly-)mode.\n"\
        "-show <sxi-document>\n"\
        "      open the specified presentation and start it immediately\n"\
        "-accept=<accept-string>\n"\
        "      Specify an UNO connect-string to create an UNO acceptor through which\n"\
        "      other programs can connect to access the API\n"\
        "-unaccept=<accept-sring>\n"\
        "      Close an acceptor that was created with -accept=<accept-string>\n"\
        "      Use -unnaccept=all to close all open acceptors\n"\
        "Remaining arguments will be treated as filenames or URLs of documents to open.\n";

    void ReplaceStringHookProc( UniString& rStr );

    void displayCmdlineHelp()
    {
        // if you put variables in other chunks don't forget to call the replace routines
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
        // on unix use console for output
        fprintf(stderr, "%s\n", ByteString(aHelpMessage_head,
                    RTL_TEXTENCODING_ASCII_US).GetBuffer());
        // merge left and right column
        int n = aHelpMessage_left.GetTokenCount ('\n');
        ByteString bsLeft(aHelpMessage_left, RTL_TEXTENCODING_ASCII_US);
        ByteString bsRight(aHelpMessage_right, RTL_TEXTENCODING_ASCII_US);
        for ( int i = 0; i < n; i++ )
        {
            fprintf(stderr, "%s", bsLeft.GetToken(i, '\n').GetBuffer());
            fprintf(stderr, "%s\n", bsRight.GetToken(i, '\n').GetBuffer());
        }
        fprintf(stderr, "%s", ByteString(aHelpMessage_bottom,
                    RTL_TEXTENCODING_ASCII_US).GetBuffer());
#else
        // rest gets a dialog box
        CmdlineHelpDialog aDlg;
        aDlg.m_ftHead.SetText(aHelpMessage_head);
        aDlg.m_ftLeft.SetText(aHelpMessage_left);
        aDlg.m_ftRight.SetText(aHelpMessage_right);
        aDlg.m_ftBottom.SetText(aHelpMessage_bottom);
        aDlg.Execute();
#endif
    }

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

}

