/*************************************************************************
 *
 *  $RCSfile: testapp.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mh $ $Date: 2002-11-18 11:18:30 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2002
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2002 by Sun Microsystems, Inc.
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVT_FILEDLG_HXX //autogen
#include <svtools/filedlg.hxx>
#endif

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef SVTOOLS_TESTTOOL_HXX //autogen
#include <svtools/testtool.hxx>
#endif
#ifndef _SVTOOLS_TTPROPS_HXX // handmade
#include <svtools/ttprops.hxx>
#endif

#include "servres.hrc"
#include "servres.hxx"
#include "testapp.hxx"


MainWindow::MainWindow(MyApp *pAppl)
: WorkWindow(NULL, WB_APP | WB_STDWORK)
, pApp(pAppl)
{}

IMPL_LINK(MainWindow,MenuSelectHdl,MenuBar*,aMenu)
{

    return pApp->GetDispatcher()->ExecuteFunction(aMenu->GetCurItemId());

}

void MainWindow::FileOpen()
{
    FileDialog Dlg(this,WB_OPEN );

    Dlg.AddFilter(CUniString("Alle Dateien"), CUniString("*.*"));
    Dlg.SetCurFilter (CUniString("*.*"));

    if (Dlg.Execute() == RET_OK)
    {

        EditFileWindow * aEditWin = new EditFileWindow(this,Dlg.GetPath());

    }
}


void MainWindow::TestGross()
{
    ModalDialogGROSSER_TEST_DLG Dlg(this,ResId(GROSSER_TEST_DLG));

    if (Dlg.Execute() == RET_OK)
    {
    }
}


BOOL MainWindow::Close()
{
    WorkWindow::Close();
    FileExit();
    return TRUE;
}

void MainWindow::FileExit()
{
/*  WriteSTBProfile();*/

//  if (pApp->CloseAll())
        pApp->Quit();
}


void MainWindow::Tree(GHEditWindow *aEditWin, Window *pBase, int Indent)
{
    String sIndent,aText;
    sIndent.Expand(5*Indent);

    aText = pBase->GetText();
    aText.SearchAndReplaceAllAscii("\n",CUniString("\\n"));

    aEditWin->AddText(String(sIndent).AppendAscii("Text: ").Append(aText).AppendAscii("\n"));
    aEditWin->AddText(String(sIndent).AppendAscii("Help: ").Append(String::CreateFromInt64(pBase->GetHelpId())).AppendAscii(":").Append(pBase->GetQuickHelpText()).AppendAscii(":").Append(pBase->GetHelpText()).AppendAscii("\n"));

    int i;
    for (i = 0 ; i < pBase->GetChildCount() ; i++)
    {
        Tree(aEditWin,pBase->GetChild(i),Indent+1);
    }
}

void MainWindow::WinTree()
{

    GHEditWindow * aEditWin = new GHEditWindow(this,CUniString("Window Tree"));
    Tree(aEditWin,this,0);

}

void MainWindow::SysDlg()
{

    switch (QueryBox(this,WB_YES_NO_CANCEL | WB_DEF_YES, CUniString("Soll noch ein Dialog geöffnet werden?")).Execute())
    {
        case RET_YES:
            while ( WarningBox(this,WB_OK_CANCEL | WB_DEF_OK,CUniString("Das ist jetzt aber die letzte Box")).Execute() == RET_OK );
            break;
        case RET_NO:
            break;
        case RET_CANCEL:InfoBox(this,CUniString("Schade")).Execute();
            break;
    }

/*

#define WB_OK                   ((WinBits)0x0010)
#define WB_OK_CANCEL            ((WinBits)0x0020)
#define WB_YES_NO               ((WinBits)0x0040)
#define WB_YES_NO_CANCEL        ((WinBits)0x0080)
#define WB_RETRY_CANCEL         ((WinBits)0x0100)

#define WB_DEF_OK               ((WinBits)0x0200)
#define WB_DEF_CANCEL           ((WinBits)0x0400)
#define WB_DEF_RETRY            ((WinBits)0x0800)
#define WB_DEF_YES              ((WinBits)0x1000)
#define WB_DEF_NO               ((WinBits)0x2000)

#define RET_OK               TRUE
#define RET_CANCEL           FALSE
#define RET_YES              2
#define RET_NO               3
#define RET_RETRY            4
*/
}

MyApp aApp;

MyApp::MyApp()
{
    pMainWin = NULL;
}

void MyApp::Property( ApplicationProperty& rProp )
{
    TTProperties* pTTProperties = PTR_CAST( TTProperties, &rProp );
    if ( pTTProperties )
    {
        pTTProperties->nPropertyVersion = TT_PROPERTIES_VERSION;
        switch ( pTTProperties->nActualPR )
        {
/*          case TT_PR_SLOTS:
            {
                pTTProperties->nSidOpenUrl = SID_OPENURL;
                pTTProperties->nSidFileName = SID_FILE_NAME;
                pTTProperties->nSidNewDocDirect = SID_NEWDOCDIRECT;
                pTTProperties->nSidCopy = SID_COPY;
                pTTProperties->nSidPaste = SID_PASTE;
                pTTProperties->nSidSourceView = SID_SOURCEVIEW;
                pTTProperties->nSidSelectAll = SID_SELECTALL;
                pTTProperties->nSidReferer = SID_REFERER;
                pTTProperties->nActualPR = 0;
            }
            break;*/
            case TT_PR_DISPATCHER:
            {
                PlugInDispatcher* pDispatcher = GetDispatcher();
                if ( !pDispatcher )
                    pTTProperties->nActualPR = TT_PR_ERR_NODISPATCHER;
                else
                {
                    pDispatcher->SetExecuteMode(EXECUTEMODE_DIALOGASYNCHRON);
                    if ( pDispatcher->ExecuteFunction(
                            pTTProperties->mnSID, pTTProperties->mppArgs, pTTProperties->mnMode )
                         == EXECUTE_NO )
                        pTTProperties->nActualPR = TT_PR_ERR_NOEXECUTE;
                    else
                        pTTProperties->nActualPR = 0;
                }
            }
            break;
/*          case TT_PR_IMG:
            {
                SvDataMemberObjectRef aDataObject = new SvDataMemberObject();
                SvData* pDataBmp = new SvData( FORMAT_BITMAP );
                pDataBmp->SetData( pTTProperties->mpBmp );
                aDataObject->Append( pDataBmp );
                aDataObject->CopyClipboard();
                pTTProperties->nActualPR = 0;
            }
            break;*/
            default:
            {
                pTTProperties->nPropertyVersion = 0;
            }
        }
        return;
    }
}


USHORT MyDispatcher::ExecuteFunction( USHORT nSID, SfxPoolItem** ppArgs, USHORT nMode)
{

    switch (nSID)
    {
        case IDM_FILE_EXIT:       pMainWin->FileExit();             break;
        case IDM_FILE_OPEN_TEST:  pMainWin->FileOpen();             break;
        case IDM_TEST_GROSS:      pMainWin->TestGross();            break;
        case IDM_TEST_WINTREE:    pMainWin->WinTree();              break;
        case IDM_SYS_DLG:         pMainWin->SysDlg();               break;
        default:
            {
                DBG_ERROR1("Dispatcher kennt Funktion nicht %s",ByteString::CreateFromInt64(nSID).GetBuffer());
                return EXECUTE_NO;
            }

    }
    return EXECUTE_YES;
}

PlugInDispatcher* MyApp::GetDispatcher()
{
    return pMyDispatcher;
}


void MyApp::Main()
{
    ResMgr *pRes = new ResMgr( CUniString("miniapp.res") );
    Resource::SetResManager( pRes );

    MainWindow  MainWin(this);
    pMainWin = &MainWin;

    MenuBar aMenu(ResId(MENU_CLIENT));
    MainWin.SetMenuBar( &aMenu );
    aMenu.SetSelectHdl(LINK(&MainWin, MainWindow, MenuSelectHdl));

    MyDispatcher MyDsp(pMainWin);
    pMyDispatcher = &MyDsp;

    MainWin.SetText(CUniString("Star Division Test Tool Client Window"));
    MainWin.Show();

    RemoteControl aRC;

    Execute();
}

