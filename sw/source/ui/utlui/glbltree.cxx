/*************************************************************************
 *
 *  $RCSfile: glbltree.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-09-21 13:39:17 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop
#include <so3/iface.hxx>
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX //autogen
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _HELP_HXX //autogen
#include <vcl/help.hxx>
#endif
#ifndef _FILELIST_HXX //autogen
#include <so3/filelist.hxx>
#endif
#ifndef _DRAG_HXX //autogen
#include <vcl/drag.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _URLBMK_HXX //autogen
#include <svtools/urlbmk.hxx>
#endif
#ifndef _FILTER_HXX //autogen
#include <svtools/filter.hxx>
#endif


#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _CONTENT_HXX
#include <content.hxx>
#endif
#ifndef _EDGLBLDC_HXX
#include <edglbldc.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _CNTTAB_HXX
#include <cnttab.hxx>
#endif
#define NAVIPI_CXX
#ifndef _NAVIPI_HXX
#include <navipi.hxx>
#endif
#ifndef _NAVICONT_HXX
#include <navicont.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _NAVIPI_HRC
#include <navipi.hrc>
#endif
#ifndef _UTLUI_HRC
#include <utlui.hrc>
#endif
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif


// Kontextmenue fuer GlobalTree
#define CTX_INSERT_ANY_INDEX 10
//#define CTX_INSERT_CNTIDX   11
//#define CTX_INSERT_USRIDX   12
#define CTX_INSERT_FILE     11
#define CTX_INSERT_NEW_FILE 12
#define CTX_INSERT_TEXT     13

#define CTX_UPDATE_SEL      20
#define CTX_UPDATE_INDEX    21
#define CTX_UPDATE_LINK     22
#define CTX_UPDATE_ALL      23

#define CTX_UPDATE          1
#define CTX_INSERT          2
#define CTX_EDIT            3
#define CTX_DELETE          4

#define GLOBAL_UPDATE_TIMEOUT 2000

// Flags fuer PopupMenu-enable/disable
#define ENABLE_INSERT_IDX   0x0001
#define ENABLE_INSERT_FILE  0x0002
#define ENABLE_INSERT_TEXT  0x0004
#define ENABLE_EDIT         0x0008
#define ENABLE_DELETE       0x0010
#define ENABLE_UPDATE       0x0020
#define ENABLE_UPDATE_SEL   0x0040

// TabPos nach links schieben
#define  GLBL_TABPOS_SUB 5

const SfxObjectShell* SwGlobalTree::pShowShell = 0;
static const USHORT __FAR_DATA aHelpForMenu[] =
{
    0,                          //
    HID_GLBLTREE_UPDATE,        //CTX_UPDATE
    HID_GLBLTREE_INSERT,        //CTX_INSERT
    HID_GLBLTREE_EDIT,          //CTX_EDIT
    HID_GLBLTREE_DEL,           //CTX_DELETE
    0,                        //
    0,                        //
    0,                        //
    0,                        //
    0,                        //
    HID_GLBLTREE_INS_IDX,       //CTX_INSERT_ANY_INDEX
    HID_GLBLTREE_INS_FILE,      //CTX_INSERT_FILE
    HID_GLBLTREE_INS_NEW_FILE,  //CTX_INSERT_NEW_FILE
    HID_GLBLTREE_INS_TEXT,      //CTX_INSERT_TEXT
    0,                          //
    0,                          //
    0,                          //
    0,                          //
    0,                          //
    0,                          //
    HID_GLBLTREE_UPD_SEL,       //CTX_UPDATE_SEL
    HID_GLBLTREE_UPD_IDX,       //CTX_UPDATE_INDEX
    HID_GLBLTREE_UPD_LINK,      //CTX_UPDATE_LINK
    HID_GLBLTREEUPD_ALL         //CTX_UPDATE_ALL
};

/************************************************************************/
/*                                                                      */
/************************************************************************/
/* -----------------------------24.08.00 12:04--------------------------------

 ---------------------------------------------------------------------------*/
class SwGlobalFrameListener_Impl : public SfxListener
{
    BOOL bValid;
public:
    SwGlobalFrameListener_Impl(SfxViewFrame& rFrame) :
        bValid(TRUE)
        {
            StartListening(rFrame);
        }

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    BOOL                IsValid() const {return bValid;}
};
/* -----------------------------24.08.00 12:05--------------------------------

 ---------------------------------------------------------------------------*/
void    SwGlobalFrameListener_Impl::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if( rHint.ISA(SfxSimpleHint) &&
            (((SfxSimpleHint&) rHint).GetId() == SFX_HINT_DYING))
        bValid = FALSE;
}

/*-----------------12.06.97 09:38-------------------

--------------------------------------------------*/
SwGlobalTree::SwGlobalTree(Window* pParent, const ResId& rResId) :
    SvTreeListBox(pParent, rResId),
    pSwGlblDocContents(0),
    pEmphasisEntry(0),
    pDDSource(0),
    pActiveShell(0),
    bIsInternalDrag(FALSE),
    bLastEntryEmphasis(FALSE),
    bIsImageListInitialized(FALSE)
{
    SetDragDropMode(SV_DRAGDROP_APP_COPY  |
                    SV_DRAGDROP_CTRL_MOVE |
                    SV_DRAGDROP_ENABLE_TOP );

    aUpdateTimer.SetTimeout(GLOBAL_UPDATE_TIMEOUT);
    aUpdateTimer.SetTimeoutHdl(LINK(this, SwGlobalTree, Timeout));
    aUpdateTimer.Start();
    for(USHORT i = 0; i < GLOBAL_CONTEXT_COUNT; i++)
    {
        aContextStrings[i] = SW_RESSTR(i+ ST_GLOBAL_CONTEXT_FIRST);
    }
    SetHelpId(HID_NAVIGATOR_GLOB_TREELIST);
    SelectHdl();
    SetDoubleClickHdl(LINK(this, SwGlobalTree, DoubleClickHdl));
}

/*-----------------12.06.97 09:38-------------------

--------------------------------------------------*/
SwGlobalTree::~SwGlobalTree()
{
    delete pSwGlblDocContents;
}

/*-----------------12.06.97 09:38-------------------

--------------------------------------------------*/
BOOL     SwGlobalTree::Drop( const DropEvent& rEvt)
{
    BOOL bRet = FALSE;
    SvLBoxEntry* pLast = (SvLBoxEntry*)LastVisible();
    if(pEmphasisEntry)
    {
        ImplShowTargetEmphasis( Prev(pEmphasisEntry), FALSE );
        pEmphasisEntry = 0;
    }
    else if(bLastEntryEmphasis && pLast)
    {
        ImplShowTargetEmphasis( pLast, FALSE);
    }


    SvLBoxEntry* pDropEntry = bLastEntryEmphasis ? 0 : GetEntry(rEvt.GetPosPixel());
    if(bIsInternalDrag)
    {
        SvLBoxEntry* pDummy = 0;
        ULONG nInsertionPos = LIST_APPEND;
        NotifyMoving(pDropEntry, pDDSource, pDummy, nInsertionPos);
    }
    else
    {
        const USHORT nCount = DragServer::GetItemCount();
        for ( USHORT i = 0; i < nCount; ++i )
        {
            String sFileName;
            const SwGlblDocContent* pCnt = pDropEntry ?
                        (const SwGlblDocContent*)pDropEntry->GetUserData() :
                                0;
            if(DragServer::HasFormat(i, FORMAT_FILE_LIST))
            {
                bRet = TRUE;
                SwGlblDocContents* pTempContents = new SwGlblDocContents;
                int nAbsContPos = pDropEntry ?
                                    (int) GetModel()->GetAbsPos(pDropEntry):
                                        - 1;
                USHORT nEntryCount = (USHORT)GetEntryCount();

                // SvData basteln
                SvData aData( FORMAT_FILE_LIST );
                SvDataObjectRef xDataObj = SvDataObject::PasteDragServer( rEvt );
                xDataObj->GetData( &aData );

                // Daten holen
                FileList aFileList;
                FileList* pFileList = &aFileList;
                aData.GetData( (SvDataCopyStream**)&pFileList, pFileList->Type() );
                for ( USHORT n = (USHORT)aFileList.Count(); n--; )
                {
                    sFileName = aFileList.GetFile(n);
                    InsertRegion(pCnt, &sFileName);
                    // nach dem Einfuegen muss die Liste der Contents neu
                    // geholt werden, um nicht auf einem alten Content zu
                    // arbeiten
                    if(n)
                    {
                        pActiveShell->GetGlobalDocContent(*pTempContents);
                        // wenn das file erfolgreich eingefuegt wurde,
                        // dann muss auch der naechste Content geholt werden
                        if(nEntryCount < pTempContents->Count())
                        {
                            nEntryCount++;
                            nAbsContPos++;
                            pCnt = pTempContents->GetObject(nAbsContPos);
                        }
                    }
                }
                delete pTempContents;
                break;
           }
           else
           {
                String sFileName(SwNavigationPI::CreateDropFileName(i));
                if(sFileName.Len())
                {
                    InsertRegion(pCnt, &sFileName);
                    bRet = TRUE;
                }
            }
        }
    }
    bLastEntryEmphasis = FALSE;
    return bRet;

}
/*-----------------12.06.97 09:38-------------------

--------------------------------------------------*/
BOOL     SwGlobalTree::QueryDrop( DropEvent& rEvt)
{
    BOOL bRet = TRUE;
    //initiate scrolling
    GetDropTarget(rEvt.GetPosPixel());
    SvLBoxEntry* pLast = (SvLBoxEntry*)LastVisible();
    if(rEvt.IsLeaveWindow())
    {
        if(pEmphasisEntry)
        {
            ImplShowTargetEmphasis( Prev(pEmphasisEntry), FALSE );
            pEmphasisEntry = 0;
        }
        else if(bLastEntryEmphasis && pLast)
        {
            ImplShowTargetEmphasis( pLast, FALSE);
        }
        bLastEntryEmphasis = FALSE;
    }
    else
    {
        SvLBoxEntry* pDropEntry = GetEntry(rEvt.GetPosPixel());
        if(bIsInternalDrag)
        {
            bRet = pDDSource != pDropEntry;
        }
        else
        {
            if (DragServer::HasFormat(0, FORMAT_FILE))
            {
                String aFileName = DragServer::PasteFile(0);
                GraphicDescriptor aDesc(aFileName);
                if (aDesc.Detect()) // keine Grafiken annehmen
                    bRet = FALSE;
                else
                    bRet = TRUE;
            }
            else if ( DragServer::HasFormat(0, FORMAT_FILE_LIST) ||
                 INetBookmark::DragServerHasFormat( 0 ) )
            {
                 bRet = TRUE;
            }
            else
                bRet = FALSE;
            rEvt.SetAction(DROP_LINK);
        }
        if(pEmphasisEntry && pEmphasisEntry != pDropEntry)
            ImplShowTargetEmphasis( Prev(pEmphasisEntry), FALSE );
        else if(pLast && bLastEntryEmphasis  && pDropEntry)
        {
            ImplShowTargetEmphasis( pLast, FALSE);
            bLastEntryEmphasis = FALSE;
        }
        if(pDropEntry)
            ImplShowTargetEmphasis( Prev(pDropEntry), bRet );
        else if(pLast)
        {
            ImplShowTargetEmphasis( pLast, bRet );
            bLastEntryEmphasis = TRUE;
        }
        pEmphasisEntry = pDropEntry;
    }
    return bRet;
}
/*-----------------12.06.97 09:38-------------------

--------------------------------------------------*/
void     SwGlobalTree::Command( const CommandEvent& rCEvt )
{
    BOOL bParent = FALSE;
    switch( rCEvt.GetCommand() )
    {
        case COMMAND_CONTEXTMENU:
        {
            if(!pActiveShell ||
                !pActiveShell->GetView().GetDocShell()->IsReadOnly())
            {
                USHORT nEnableFlags = GetEnableFlags();
                PopupMenu aPop;
                PopupMenu aSubPop1;
                PopupMenu aSubPop2;

                for (USHORT i = CTX_UPDATE_SEL; i <= CTX_UPDATE_ALL; i++)
                {
                    aSubPop2.InsertItem( i, aContextStrings[ST_UPDATE_SEL - ST_GLOBAL_CONTEXT_FIRST - CTX_UPDATE_SEL+ i] );
                    aSubPop2.SetHelpId(i, aHelpForMenu[i]);
                }
                aSubPop2.EnableItem(CTX_UPDATE_SEL, nEnableFlags&ENABLE_UPDATE_SEL);
                aSubPop2.SetSelectHdl(LINK(this, SwGlobalTree, PopupHdl));

                aSubPop1.InsertItem(CTX_INSERT_ANY_INDEX, aContextStrings[ST_INDEX  - ST_GLOBAL_CONTEXT_FIRST]);
                aSubPop1.SetHelpId(CTX_INSERT_ANY_INDEX, aHelpForMenu[CTX_INSERT_ANY_INDEX]);
                aSubPop1.InsertItem(CTX_INSERT_FILE, aContextStrings[ST_FILE   - ST_GLOBAL_CONTEXT_FIRST]);
                aSubPop1.SetHelpId(CTX_INSERT_FILE, aHelpForMenu[CTX_INSERT_FILE]);
                aSubPop1.InsertItem(CTX_INSERT_NEW_FILE, aContextStrings[ST_NEW_FILE   - ST_GLOBAL_CONTEXT_FIRST]);
                aSubPop1.SetHelpId(CTX_INSERT_NEW_FILE, aHelpForMenu[CTX_INSERT_NEW_FILE]);
                aSubPop1.InsertItem(CTX_INSERT_TEXT, aContextStrings[ST_TEXT   - ST_GLOBAL_CONTEXT_FIRST]);
                aSubPop1.SetHelpId(CTX_INSERT_TEXT, aHelpForMenu[CTX_INSERT_TEXT]);


                aPop.InsertItem(CTX_UPDATE, aContextStrings[ST_UPDATE - ST_GLOBAL_CONTEXT_FIRST]);
                aPop.SetHelpId(CTX_UPDATE, aHelpForMenu[CTX_UPDATE]);
                aPop.InsertItem(CTX_EDIT, aContextStrings[ST_EDIT_CONTENT - ST_GLOBAL_CONTEXT_FIRST]);
                aPop.SetHelpId(CTX_EDIT, aHelpForMenu[CTX_EDIT]);
                aPop.InsertItem(CTX_INSERT, aContextStrings[ST_INSERT - ST_GLOBAL_CONTEXT_FIRST]);
                aPop.SetHelpId(CTX_INSERT, aHelpForMenu[CTX_INSERT]);
                aPop.InsertSeparator() ;
                aPop.InsertItem(CTX_DELETE, aContextStrings[ST_DELETE - ST_GLOBAL_CONTEXT_FIRST]);
                aPop.SetHelpId(CTX_DELETE, aHelpForMenu[CTX_DELETE]);

                //evtl. disablen
                aSubPop1.EnableItem(CTX_INSERT_ANY_INDEX,   nEnableFlags&ENABLE_INSERT_IDX );
                aSubPop1.EnableItem(CTX_INSERT_TEXT,    nEnableFlags&ENABLE_INSERT_TEXT);
                aSubPop1.EnableItem(CTX_INSERT_FILE,    nEnableFlags&ENABLE_INSERT_FILE);
                aSubPop1.EnableItem(CTX_INSERT_NEW_FILE, nEnableFlags&ENABLE_INSERT_FILE);

                aPop.EnableItem(CTX_UPDATE,             nEnableFlags&ENABLE_UPDATE);
                aPop.EnableItem(CTX_INSERT,             nEnableFlags&ENABLE_INSERT_IDX);
                aPop.EnableItem(CTX_EDIT,               nEnableFlags&ENABLE_EDIT);
                aPop.EnableItem(CTX_DELETE,             nEnableFlags&ENABLE_DELETE);


                aPop.SetPopupMenu( CTX_INSERT, &aSubPop1 );
                aPop.SetPopupMenu( CTX_UPDATE, &aSubPop2 );
                Link aLk = LINK(this, SwGlobalTree, PopupHdl );
                aPop.SetSelectHdl(aLk);
                aSubPop1.SetSelectHdl(aLk);
                aSubPop2.SetSelectHdl(aLk);

                aPop.Execute( this, rCEvt.GetMousePosPixel());
            }
        }
        break;
        default: bParent = TRUE;
    }
    if(bParent)
        SvTreeListBox::Command(rCEvt);
}
/*-----------------16.06.97 10:41-------------------

--------------------------------------------------*/
void SwGlobalTree::TbxMenuHdl(USHORT nTbxId, ToolBox* pBox)
{
    USHORT nEnableFlags = GetEnableFlags();
    if(FN_GLOBAL_OPEN == nTbxId)
    {
        PopupMenu *pMenu = new PopupMenu;
        for (USHORT i = CTX_INSERT_ANY_INDEX; i <= CTX_INSERT_TEXT; i++)
        {
            pMenu->InsertItem( i, aContextStrings[ST_INDEX  - ST_GLOBAL_CONTEXT_FIRST - CTX_INSERT_ANY_INDEX + i] );
            pMenu->SetHelpId(i, aHelpForMenu[i] );
        }
        pMenu->EnableItem(CTX_INSERT_ANY_INDEX, nEnableFlags&ENABLE_INSERT_IDX );
//      pMenu->EnableItem(CTX_INSERT_CNTIDX,    nEnableFlags&ENABLE_INSERT_IDX );
//      pMenu->EnableItem(CTX_INSERT_USRIDX,    nEnableFlags&ENABLE_INSERT_IDX );
        pMenu->EnableItem(CTX_INSERT_TEXT,      nEnableFlags&ENABLE_INSERT_TEXT);
        pMenu->EnableItem(CTX_INSERT_FILE,      nEnableFlags&ENABLE_INSERT_FILE);
        pMenu->EnableItem(CTX_INSERT_NEW_FILE,  nEnableFlags&ENABLE_INSERT_FILE);
        pMenu->SetSelectHdl(LINK(this, SwGlobalTree, PopupHdl));
        pMenu->Execute( pBox, pBox->GetItemRect(nTbxId).BottomLeft());
        delete pMenu;
        pBox->EndSelection();
        pBox->Invalidate();
    }
    else if(FN_GLOBAL_UPDATE == nTbxId)
    {
        PopupMenu *pMenu = new PopupMenu;
        for (USHORT i = CTX_UPDATE_SEL; i <= CTX_UPDATE_ALL; i++)
        {
            pMenu->InsertItem( i, aContextStrings[ST_UPDATE_SEL - ST_GLOBAL_CONTEXT_FIRST - CTX_UPDATE_SEL+ i] );
            pMenu->SetHelpId(i, aHelpForMenu[i] );
        }
        pMenu->EnableItem(CTX_UPDATE_SEL,   nEnableFlags&ENABLE_UPDATE_SEL);
        pMenu->SetSelectHdl(LINK(this, SwGlobalTree, PopupHdl));
        pMenu->Execute( pBox, pBox->GetItemRect(nTbxId).BottomLeft());
        delete pMenu;
        pBox->EndSelection();
        pBox->Invalidate();
    }
}
/*-----------------16.06.97 11:02-------------------

--------------------------------------------------*/
USHORT  SwGlobalTree::GetEnableFlags() const
{
    SvLBoxEntry* pEntry = FirstSelected();
    USHORT nSelCount = (USHORT)GetSelectionCount();
    USHORT nEntryCount = (USHORT)GetEntryCount();
    SvLBoxEntry* pPrevEntry = pEntry ? Prev(pEntry) : 0;

    USHORT nRet = 0;
    if(nSelCount == 1 || !nEntryCount)
        nRet |= ENABLE_INSERT_IDX|ENABLE_INSERT_FILE;
    if(nSelCount == 1)
    {
        nRet |= ENABLE_EDIT;
        if( ((SwGlblDocContent*)pEntry->GetUserData())->GetType() != GLBLDOC_UNKNOWN &&
                    (!pPrevEntry || ((SwGlblDocContent*)pPrevEntry->GetUserData())->GetType() != GLBLDOC_UNKNOWN))
            nRet |= ENABLE_INSERT_TEXT;
    }
    else if(!nEntryCount)
    {
        nRet |= ENABLE_INSERT_TEXT;
    }
    if(nEntryCount)
        nRet |= ENABLE_UPDATE|ENABLE_DELETE;
    if(nSelCount)
        nRet |= ENABLE_UPDATE_SEL;
    return nRet;
}

/*-----------------12.06.97 09:38-------------------

--------------------------------------------------*/
void     SwGlobalTree::RequestHelp( const HelpEvent& rHEvt )
{
    BOOL bParent = TRUE;
    Update(TRUE);
    Display(TRUE);
    if( rHEvt.GetMode() & HELPMODE_QUICK )
    {
        Point aPos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
        SvLBoxEntry* pEntry = GetEntry( aPos );
        const SwGlblDocContent* pCont = pEntry ?
                            (const SwGlblDocContent*)pEntry->GetUserData() : 0;
        if( pCont &&  GLBLDOC_SECTION == pCont->GetType())
        {
            bParent = FALSE;
            SvLBoxTab* pTab;
            SvLBoxItem* pItem = GetItem( pEntry, aPos.X(), &pTab );
            if(pItem && SV_ITEM_ID_LBOXSTRING == pItem->IsA())
            {
                const SwSection* pSect = pCont->GetSection();
                String sEntry = pSect->GetLinkFileName().GetToken(0, cTokenSeperator);
                if(!pSect->IsConnectFlag())
                    sEntry.Insert(aContextStrings[ST_BROKEN_LINK - ST_GLOBAL_CONTEXT_FIRST], 0 );
                Point aPos = GetEntryPos( pEntry );

                aPos.X() = GetTabPos( pEntry, pTab );
                Size aSize( pItem->GetSize( this, pEntry ) );

                if((aPos.X() + aSize.Width()) > GetSizePixel().Width())
                    aSize.Width() = GetSizePixel().Width() - aPos.X();

                aPos = OutputToScreenPixel(aPos);
                Rectangle aItemRect( aPos, aSize );
                if(Help::IsBalloonHelpEnabled())
                {
                    aPos.X() += aSize.Width();
                    Help::ShowBalloon( this, aPos, aItemRect, sEntry );
                }
                else
                    Help::ShowQuickHelp( this, aItemRect, sEntry,
                        QUICKHELP_LEFT|QUICKHELP_VCENTER );
            }
        }
    }

    if(bParent)
        SvTreeListBox::RequestHelp(rHEvt);
}
/*-----------------16.06.97 16:15-------------------

--------------------------------------------------*/
void     SwGlobalTree::SelectHdl()
{

    USHORT nSelCount = (USHORT)GetSelectionCount();
    SvLBoxEntry* pSel = FirstSelected();
    USHORT nAbsPos = pSel ? (USHORT)GetModel()->GetAbsPos(pSel) : 0;
    SwNavigationPI* pNavi = GetParentWindow();
    BOOL bReadonly = !pActiveShell ||
                pActiveShell->GetView().GetDocShell()->IsReadOnly();
    pNavi->aGlobalToolBox.EnableItem(FN_GLOBAL_EDIT,  nSelCount == 1 && !bReadonly);
    pNavi->aGlobalToolBox.EnableItem(FN_GLOBAL_OPEN,  nSelCount <= 1 && !bReadonly);
    pNavi->aGlobalToolBox.EnableItem(FN_GLOBAL_UPDATE,  GetEntryCount() > 0 && !bReadonly);
    pNavi->aGlobalToolBox.EnableItem(FN_ITEM_UP,
                    nSelCount == 1 && nAbsPos && !bReadonly);
    pNavi->aGlobalToolBox.EnableItem(FN_ITEM_DOWN,
                    nSelCount == 1 && nAbsPos < ((USHORT)GetEntryCount()) - 1 && !bReadonly);

}
/*-----------------16.06.97 16:15-------------------

--------------------------------------------------*/
void     SwGlobalTree::DeselectHdl()
{
    SelectHdl();
}

/*-----------------17.06.97 13:11-------------------

--------------------------------------------------*/
DragDropMode     SwGlobalTree::NotifyBeginDrag( SvLBoxEntry* pEntry)
{
    pDDSource = pEntry;
    return SV_DRAGDROP_CTRL_MOVE;
}

/*-----------------21.06.97 12:44-------------------

--------------------------------------------------*/
long     SwGlobalTree::GetTabPos( SvLBoxEntry*, SvLBoxTab* pTab)
{
    return pTab->GetPos() - GLBL_TABPOS_SUB;
}

/*-----------------12.06.97 09:38-------------------

--------------------------------------------------*/
BOOL     SwGlobalTree::NotifyMoving(   SvLBoxEntry*  pTarget,
                                        SvLBoxEntry*  pSource,
                                        SvLBoxEntry*&,
                                        ULONG&
                                    )
{
    SvTreeList* pModel = GetModel();
    USHORT nSource = (USHORT) pModel->GetAbsPos(pSource);
    USHORT nDest   = pTarget ? (USHORT) pModel->GetAbsPos(pTarget) : pSwGlblDocContents->Count();

    if( pActiveShell->MoveGlobalDocContent(
            *pSwGlblDocContents, nSource, nSource + 1, nDest ) &&
            Update())
        Display();
    return FALSE;
}
/*-----------------12.06.97 09:39-------------------

--------------------------------------------------*/
BOOL     SwGlobalTree::NotifyCopying(  SvLBoxEntry*  pTarget,
                                        SvLBoxEntry*  pEntry,
                                        SvLBoxEntry*& rpNewParent,
                                        ULONG&        rNewChildPos
                                    )
{
    return FALSE;
}
/*-----------------12.06.97 09:39-------------------

--------------------------------------------------*/
BOOL     SwGlobalTree::NotifyQueryDrop( SvLBoxEntry* pEntry)
{
    return pEntry != 0;
}
/*-----------------12.06.97 09:39-------------------

--------------------------------------------------*/
void     SwGlobalTree::BeginDrag( const Point& rPt)
{
    bIsInternalDrag = TRUE;
    if(GetSelectionCount() == 1)
        SvTreeListBox::BeginDrag(rPt);
}
/*-----------------12.06.97 09:39-------------------

--------------------------------------------------*/
void     SwGlobalTree::EndDrag()
{
    bIsInternalDrag = FALSE;
}

/***************************************************************************
    Beschreibung:   Wird ein Ctrl+DoubleClick in einen freien Bereich ausgefuehrt,
 *                  dann soll die Basisfunktion des Controls gerufen werden
***************************************************************************/
void  SwGlobalTree::MouseButtonDown( const MouseEvent& rMEvt )
{
    Point aPos( rMEvt.GetPosPixel());
    SvLBoxEntry* pEntry = GetEntry( aPos, TRUE );
    if( !pEntry && rMEvt.IsLeft() && rMEvt.IsMod1() && (rMEvt.GetClicks() % 2) == 0)
        Control::MouseButtonDown( rMEvt );
    else
        SvTreeListBox::MouseButtonDown( rMEvt );
}

/*-----------------12.06.97 13:08-------------------

--------------------------------------------------*/
void     SwGlobalTree::GetFocus()
{
    if(Update())
        Display();
    SvTreeListBox::GetFocus();
}

/*-----------------12.06.97 12:34-------------------

--------------------------------------------------*/
void     SwGlobalTree::KeyInput(const KeyEvent& rKEvt)
{
    const KeyCode aCode = rKEvt.GetKeyCode();
    if(aCode.GetCode() == KEY_RETURN)
    {
        switch(aCode.GetModifier())
        {
            case KEY_MOD2:
                // Boxen umschalten
                GetParentWindow()->ToggleTree();
            break;
        }
    }
    else
        SvTreeListBox::KeyInput(rKEvt);
}

/*-----------------17.06.97 11:59-------------------

--------------------------------------------------*/
void SwGlobalTree::Clear()
{
    pEmphasisEntry = 0;
    SvTreeListBox::Clear();
}
/*-----------------12.06.97 12:38-------------------

--------------------------------------------------*/
void    SwGlobalTree::Display(BOOL bOnlyUpdateUserData)
{
    if(!bIsImageListInitialized)
    {
        aEntryImages = ImageList(SW_RES(IMG_NAVI_ENTRYBMP));
        bIsImageListInitialized = TRUE;
    }
    USHORT nCount = pSwGlblDocContents->Count();
    if(bOnlyUpdateUserData && GetEntryCount() == pSwGlblDocContents->Count())
    {
        SvLBoxEntry* pEntry = First();
        for( USHORT i = 0; i < nCount; i++)
        {
            SwGlblDocContentPtr pCont = pSwGlblDocContents->GetObject(i);
            pEntry->SetUserData(pCont);
            pEntry = Next(pEntry);
        }
    }
    else
    {
        SetUpdateMode( FALSE );
        SvLBoxEntry* pOldSelEntry = FirstSelected();
        String sEntryName;  // Name des Eintrags
        USHORT nSelPos = USHRT_MAX;
        if(pOldSelEntry)
        {
            sEntryName = GetEntryText(pOldSelEntry);
            nSelPos = (USHORT)GetModel()->GetAbsPos(pOldSelEntry);
        }
        Clear();
        if(!pSwGlblDocContents)
            Update();

        SvLBoxEntry* pSelEntry = 0;
        for( USHORT i = 0; i < nCount; i++)
        {
            SwGlblDocContentPtr pCont = pSwGlblDocContents->GetObject(i);
            String sEntry;
            Image aImage;
            switch( pCont->GetType()  )
            {
                case GLBLDOC_UNKNOWN:
                {
                    sEntry = aContextStrings[ST_TEXT   - ST_GLOBAL_CONTEXT_FIRST];
                    aImage = aEntryImages.GetImage(SID_SW_START + GLOBAL_CONTENT_TEXT);
                }
                break;
                case GLBLDOC_TOXBASE:
                {
                    const SwTOXBase* pBase = pCont->GetTOX();
                    sEntry = pBase->GetTitle();
                    aImage = aEntryImages.GetImage(SID_SW_START + CONTENT_TYPE_INDEX);
                }
                break;
                case GLBLDOC_SECTION:
                {
                    const SwSection* pSect = pCont->GetSection();
                    sEntry = pSect->GetName();
                    aImage = aEntryImages.GetImage(SID_SW_START + CONTENT_TYPE_REGION);
                }
                break;
            }
            SvLBoxEntry* pEntry = InsertEntry(sEntry, aImage, aImage,
                        0, FALSE, LIST_APPEND, pCont);
            if(sEntry == sEntryName)
            {
                pSelEntry = pEntry;
            }
        }
        if(pSelEntry)
        {
            Select(pSelEntry);
        }
        else if(nSelPos != USHRT_MAX && nSelPos < nCount)
        {
            Select(GetEntry(nSelPos));
        }
        else if(nCount)
            Select(First());
        else
            SelectHdl();
        SetUpdateMode( TRUE );
    }
}

/*-----------------13.06.97 10:32-------------------

--------------------------------------------------*/
void SwGlobalTree::InsertRegion( const SwGlblDocContent* pCont,
                                const String* pFileName )
{
    String sFileName;
    SwView *pView = GetParentWindow()->GetCreateView();
    SwWrtShell &rSh = pView->GetWrtShell();
    String sFilePassword;
    if(!pFileName)
    {
        String sFilterName;
        if( GetFileFilterNameDlg( *this, sFileName, &sFilePassword, &sFilterName ))
        {
            sFileName += cTokenSeperator;
            sFileName += sFilterName;
            sFileName += cTokenSeperator;
        }
    }
    else if(pFileName->Len())
    {
        sFileName = URIHelper::SmartRelToAbs( *pFileName );
    }

    if(sFileName.Len())
    {
        INetURLObject aFileUrl(sFileName);
        String sSectionName(aFileUrl.GetLastName().GetToken(0, cTokenSeperator));
        USHORT nSectCount = rSh.GetSectionFmtCount();
        String sTempSectionName(sSectionName);
        USHORT nAddNumber = 0;
        USHORT nCount = 0;
        // evtl : und Index anhaengen, wenn der Bereichsname schon vergeben ist
        while(nCount < nSectCount)
        {
            const SwSectionFmt& rFmt = rSh.GetSectionFmt(nCount);
            if( rFmt.GetSection()->GetName() == sTempSectionName &&
                    rFmt.IsInNodesArr())
            {
                nCount = 0;
                nAddNumber++;
                sTempSectionName = sSectionName;
                sTempSectionName += ':';
                sTempSectionName += String::CreateFromInt32( nAddNumber );
            }
            else
                nCount++;
        }
        if(nAddNumber)
                sSectionName = sTempSectionName;

        SwSection   aSection(CONTENT_SECTION, sSectionName);
        aSection.SetProtect(TRUE);
        aSection.SetHidden(FALSE);

        aSection.SetLinkFileName(sFileName);
        aSection.SetType( FILE_LINK_SECTION);
        aSection.SetLinkFilePassWd( sFilePassword );

        if(pCont)
            rSh.InsertGlobalDocContent( *pCont, aSection );
        else
        {
            SvLBoxEntry* pLast = (SvLBoxEntry*)LastVisible();
            rSh.StartAction();
            rSh.InsertGlobalDocContent( *(SwGlblDocContent*)pLast->GetUserData(),
                                        aSection );
            Update();
            USHORT nEntryCount = (USHORT)GetEntryCount();
            if( rSh.MoveGlobalDocContent(
                *pSwGlblDocContents, nEntryCount, nEntryCount + 1, nEntryCount - 1) &&
                Update())
            rSh.EndAction();
            Display();

        }
    }
}

/*-----------------18.06.97 12:42-------------------

--------------------------------------------------*/
void    SwGlobalTree::EditContent(const SwGlblDocContent* pCont )
{
    USHORT nSlot = 0;
    switch( pCont->GetType() )
    {
        case GLBLDOC_UNKNOWN:
            pActiveShell->GetView().GetEditWin().GrabFocus();
        break;
        case GLBLDOC_TOXBASE:
        {
            const SwTOXBase* pBase = pCont->GetTOX();
            if(pBase)
                nSlot = FN_INSERT_MULTI_TOX;
        }
        break;
        case GLBLDOC_SECTION:
        {
            OpenDoc(pCont);

            nSlot = 0;
            pCont = 0;
        }
        break;
    }
    if(pCont)
        GotoContent(pCont);
    if(nSlot)
    {
        pActiveShell->GetView().GetViewFrame()->GetDispatcher()->Execute(nSlot);
        if(Update())
            Display();
    }
}

/*-----------------13.06.97 14:22-------------------

--------------------------------------------------*/
IMPL_LINK( SwGlobalTree, PopupHdl, Menu* , pMenu)
{
    USHORT nId = pMenu->GetCurItemId();
    SvLBoxEntry* pEntry = FirstSelected();
    SwGlblDocContent* pCont = pEntry ? (SwGlblDocContent*)pEntry->GetUserData() : 0;
    // wird waehrend des Dialogs ein RequestHelp gerufen,
    // dann geht der Content verloren. Deshalb wird hier eine
    // Kopie angelegt, in der nur die DocPos richtig gesetzt ist.
    SwGlblDocContent* pContCopy = 0;
    if(pCont)
        pContCopy = new SwGlblDocContent(pCont->GetDocPos());
    SfxDispatcher& rDispatch = *pActiveShell->GetView().GetViewFrame()->GetDispatcher();
    USHORT nSlot = 0;;
    switch( nId )
    {
        case CTX_UPDATE_SEL:
        {
            // zwei Durchlaeufe: zuerst die Bereiche, dann die Verzeichnisse
            // aktualisieren
            SvLBoxEntry* pEntry = FirstSelected();
            while( pEntry )
            {
                SwGlblDocContent* pCont = (SwGlblDocContent*)pEntry->GetUserData();
                if(GLBLDOC_SECTION == pCont->GetType() &&
                    pCont->GetSection()->IsConnected())
                {
                    ((SwSection*)pCont->GetSection())->UpdateNow();
                }

                pEntry = NextSelected(pEntry);
            }
            pEntry = FirstSelected();
            while( pEntry )
            {
                SwGlblDocContent* pCont = (SwGlblDocContent*)pEntry->GetUserData();
                if(GLBLDOC_TOXBASE == pCont->GetType())
                    pActiveShell->UpdateTableOf(*pCont->GetTOX());
                pEntry = NextSelected(pEntry);
            }

        }
        break;
        case CTX_UPDATE_INDEX:
        {
            nSlot = FN_UPDATE_TOX;
        }
        break;
        case CTX_UPDATE_LINK:
        case CTX_UPDATE_ALL:
        {
            pActiveShell->GetLinkManager().UpdateAllLinks(TRUE);
            if(CTX_UPDATE_ALL == nId)
                nSlot = FN_UPDATE_TOX;
            pCont = 0;
        }
        break;
        case CTX_EDIT:
        {
            DBG_ASSERT(pCont, "Edit ohne Entry ? " )
            EditContent(pCont);
        }
        break;
        case CTX_DELETE:
        {
            // sind mehrere Eintraege selektiert, dann muss nach jedem delete
            // das Array neu gefuellt werden. Damit man sich nichts merken muss,
            // beginnt das Loeschen am Ende
            SvLBoxEntry* pEntry = LastSelected();
            SwGlblDocContents* pTempContents  = 0;
            pActiveShell->StartAction();
            while(pEntry)
            {
                pActiveShell->DeleteGlobalDocContent(
                    pTempContents ? *pTempContents : *pSwGlblDocContents,
                                     (USHORT)GetModel()->GetAbsPos(pEntry));
                pEntry = PrevSelected(pEntry);
                if(pEntry)
                {
                    pTempContents = new SwGlblDocContents;
                    pActiveShell->GetGlobalDocContent(*pTempContents);
                }
            }
            delete pTempContents;
            pActiveShell->EndAction();
            pCont = 0;
        }
        break;
        case CTX_INSERT_ANY_INDEX:
//      case CTX_INSERT_CNTIDX:
//      case CTX_INSERT_USRIDX:
        {
            if(pContCopy)
            {
                SfxItemSet aSet(pActiveShell->GetView().GetPool(),
                            RES_COL, RES_COL,
                            RES_BACKGROUND, RES_BACKGROUND,
                            RES_FRM_SIZE, RES_FRM_SIZE,
                            SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE,
                            RES_LR_SPACE, RES_LR_SPACE,
                            FN_PARAM_TOX_TYPE, FN_PARAM_TOX_TYPE,
                            0);

                SwMultiTOXTabDialog* pDlg = new SwMultiTOXTabDialog(this, aSet,
                        *pActiveShell,
                        0,
                        USHRT_MAX,
                        TRUE);

                if(RET_OK == pDlg->Execute())
                {
                    SwTOXDescription&  rDesc = pDlg->GetTOXDescription(
                                                pDlg->GetCurrentTOXType());
                    SwForm* pForm = pDlg->GetForm(pDlg->GetCurrentTOXType());
                    SwTOXMgr aMgr(pActiveShell);
                    SwTOXBase* pToInsert = 0;
                    if(aMgr.UpdateOrInsertTOX(rDesc, &pToInsert, pDlg->GetOutputItemSet()))
                        pActiveShell->InsertGlobalDocContent( *pContCopy, *pToInsert );
                }
                pCont = 0;
                delete pDlg;
            }
        }
        break;
        case CTX_INSERT_FILE:
        {
            InsertRegion(pContCopy);
            pCont = 0;
        }
        break;
        case CTX_INSERT_NEW_FILE:
        {
            SfxViewFrame* pGlobFrm = pActiveShell->GetView().GetViewFrame();
            SwGlobalFrameListener_Impl aFrmListener(*pGlobFrm);

            USHORT nEntryPos = pEntry ? GetModel()->GetAbsPos(pEntry) : USHRT_MAX;
            // neues Dok anlegen
            SfxStringItem aFactory(SID_NEWDOCDIRECT,
                            SwDocShell::Factory().GetFilterContainer()->GetName());

             const SfxFrameItem* pItem = (SfxFrameItem*)
                            rDispatch.Execute(SID_NEWDOCDIRECT,
                                SFX_CALLMODE_SYNCHRON, &aFactory, 0L);

            // sichern unter
            SfxFrame* pFrm = pItem ? pItem->GetFrame() : 0;
            SfxViewFrame* pFrame = pFrm ? pFrm->GetCurrentViewFrame() : 0;
            if( pFrame )
            {
                const SfxBoolItem* pBool = (const SfxBoolItem*)
                        pFrame->GetDispatcher()->Execute(
                                SID_SAVEASDOC, SFX_CALLMODE_SYNCHRON );
                SfxObjectShell& rObj = *pFrame->GetObjectShell();
                const SfxMedium* pMedium = rObj.GetMedium();
                String sNewFile(pMedium->GetURLObject().GetMainURL());
                // Bereich mit dem Dok-Namen einfgen
                // eigenes Dok in den Vordergrund

                if(aFrmListener.IsValid() && sNewFile.Len())
                {
                    pGlobFrm->ToTop();
                    // durch das Update sind die Eintraege invalid
                    if(nEntryPos != USHRT_MAX)
                    {
                        Update();
                        Display();
                        Select(GetModel()->GetEntryAtAbsPos(nEntryPos));
                        pEntry = FirstSelected();
                        pCont = pEntry ? (SwGlblDocContent*)pEntry->GetUserData() : 0;
                    }
                    else
                    {
                        pEntry = 0;
                        pCont = 0;
                    }
                    if(pBool->GetValue())
                    {
                        InsertRegion(pCont, &sNewFile);
                        pFrame->ToTop();
                    }
                    else
                        pFrame->GetDispatcher()->Execute(SID_CLOSEWIN,
                                                SFX_CALLMODE_SYNCHRON);
                }
                else
                {
                    pFrame->ToTop();
                    return TRUE;
                }
            }
        }
        break;
        case CTX_INSERT_TEXT:
        {
            if(pCont)
                pActiveShell->InsertGlobalDocContent(*pCont);
            else
            {
                pActiveShell->SplitNode(); // leeres Dokument
                pActiveShell->Up();
            }
            pActiveShell->GetView().GetEditWin().GrabFocus();
        }
        break;
        case CTX_UPDATE:
            pCont = 0;
        break;
        default:;
        // hier passiert nichts
    }
    if(pCont)
        GotoContent(pCont);
    if(nSlot)
        rDispatch.Execute(nSlot);
    if(Update())
        Display();
    delete pContCopy;
    return TRUE;
}

/*-----------------16.06.97 07:57-------------------

--------------------------------------------------*/
IMPL_LINK( SwGlobalTree, Timeout, Timer*, EMPTYARG )
{
    if(!HasFocus()&&Update())
        Display();
    return 0;
}

/*-----------------13.06.97 16:56-------------------

--------------------------------------------------*/
void SwGlobalTree::GotoContent(const SwGlblDocContent* pCont)
{
    pActiveShell->EnterStdMode();

    switch( pCont->GetType()  )
    {
        case GLBLDOC_UNKNOWN:
            pActiveShell->GotoGlobalDocContent(*pCont);
        break;
        case GLBLDOC_TOXBASE:
        {
            String sName = pCont->GetTOX()->GetTOXName();
            if (!pActiveShell->GotoNextTOXBase(&sName))
                pActiveShell->GotoPrevTOXBase(&sName);
        }
        break;
        case GLBLDOC_SECTION:
        break;
    }

}
/*-----------------16.06.97 07:42-------------------

--------------------------------------------------*/
void    SwGlobalTree::Show()
{
    aUpdateTimer.Start();
    SvTreeListBox::Show();
}
/*-----------------16.06.97 07:42-------------------

--------------------------------------------------*/
void    SwGlobalTree::Hide()
{
    aUpdateTimer.Stop();
    SvTreeListBox::Hide();
}
/*-----------------18.06.97 10:02-------------------

--------------------------------------------------*/
void    SwGlobalTree::ExecCommand(USHORT nCmd)
{
    SvLBoxEntry* pEntry = FirstSelected();
    DBG_ASSERT(pEntry, "gleich knalltïs")
    if(FN_GLOBAL_EDIT == nCmd)
    {
        const SwGlblDocContent* pCont = (const SwGlblDocContent*)
                                                pEntry->GetUserData();
        EditContent(pCont);
    }
    else
    {
        if(GetSelectionCount() == 1)
        {
            BOOL bMove = FALSE;
            USHORT nSource = (USHORT)GetModel()->GetAbsPos(pEntry);
            USHORT nDest = nSource;
            switch(nCmd)
            {
                case FN_ITEM_DOWN:
                {
                    USHORT nEntryCount = (USHORT)GetEntryCount();
                    bMove = nEntryCount > nSource + 1;
                    nDest+= 2;
                }
                break;
                case FN_ITEM_UP:
                {
                    if(nSource)
                        bMove = 0 != nSource;
                    nDest--;
                }
                break;
            }
            if( bMove && pActiveShell->MoveGlobalDocContent(
                *pSwGlblDocContents, nSource, nSource + 1, nDest ) &&
                    Update())
                Display();
        }
    }
}

/*-----------------16.06.97 07:43-------------------

--------------------------------------------------*/
BOOL    SwGlobalTree::Update(BOOL bHard)
{
    SwView* pActView = GetParentWindow()->GetCreateView();
    BOOL bRet = FALSE;
    if(pActView)
    {
        const SwWrtShell* pOldShell = pActiveShell;
        pActiveShell = pActView->GetWrtShellPtr();
        if(pActiveShell != pOldShell)
            bRet = TRUE;
        if(!pSwGlblDocContents)
        {
            pSwGlblDocContents = new SwGlblDocContents;
            bRet = TRUE;
            pActiveShell->GetGlobalDocContent(*pSwGlblDocContents);
        }
        else
        {
            BOOL bCopy = FALSE;
            SwGlblDocContents* pTempContents  = new SwGlblDocContents;
            pActiveShell->GetGlobalDocContent(*pTempContents);
            if(pTempContents->Count() != pSwGlblDocContents->Count() ||
                    pTempContents->Count() != GetEntryCount())
            {
                bRet = TRUE;
                bCopy = TRUE;
            }
            else
            {
                for(USHORT i = 0; i < pTempContents->Count() && !bCopy; i++)
                {
                    SwGlblDocContent* pLeft = pTempContents->GetObject(i);
                    SwGlblDocContent* pRight = pSwGlblDocContents->GetObject(i);
                    GlobalDocContentType eType = pLeft->GetType();
                    SvLBoxEntry* pEntry = GetEntry(i);
                    String sTemp = GetEntryText(pEntry);
                    if(eType != pRight->GetType() ||
                        eType == GLBLDOC_SECTION &&
                            pLeft->GetSection()->GetName() != sTemp ||
                        eType == GLBLDOC_TOXBASE && pLeft->GetTOX()->GetTitle() != sTemp)
                            bCopy = bRet = TRUE;
                }
            }
            if(bCopy || bHard)
            {
                pSwGlblDocContents->DeleteAndDestroy(0, pSwGlblDocContents->Count());
                for(USHORT i = 0; i < pTempContents->Count(); i++)
                {
                    pSwGlblDocContents->Insert(pTempContents->GetObject(i));
                }
                for(i = pTempContents->Count(); i; i--)
                    pTempContents->Remove(i - 1);

            }
            delete pTempContents;
        }

    }
    else
    {
        Clear();
        if(pSwGlblDocContents)
            pSwGlblDocContents->DeleteAndDestroy(0, pSwGlblDocContents->Count());
    }
    // hier muss noch eine Veraenderungspruefung rein!
    return bRet;
}

/*-----------------25.06.97 16:20-------------------

--------------------------------------------------*/
void SwGlobalTree::OpenDoc(const SwGlblDocContent* pCont)
{
    String sFileName(pCont->GetSection()->GetLinkFileName().GetToken(0, cTokenSeperator));
    BOOL bFound = FALSE;
    const SfxObjectShell* pCurr = SfxObjectShell::GetFirst();
    while( !bFound && pCurr )
    {
        if(pCurr->GetMedium() &&
            pCurr->GetMedium()->GetURLObject().GetMainURL() == sFileName)
        {
            bFound = TRUE;
            SwGlobalTree::SetShowShell(pCurr);
            Application::PostUserEvent( STATIC_LINK(
                        this, SwGlobalTree, ShowFrameHdl ) );
            pCurr = 0;
        }
        else
            pCurr = SfxObjectShell::GetNext(*pCurr);
    }
    if(!bFound)
    {
        SfxStringItem aURL(SID_FILE_NAME,
            sFileName);
        SfxBoolItem aReadOnly(SID_DOC_READONLY, FALSE);
        SfxStringItem aReferer(SID_REFERER, pActiveShell->GetView().GetDocShell()->GetTitle());
        pActiveShell->GetView().GetViewFrame()->GetDispatcher()->
                Execute(SID_OPENDOC, SFX_CALLMODE_ASYNCHRON,
                            &aURL, &aReadOnly, &aReferer, 0);
    }
}

/*-----------------25.06.97 16:08-------------------

--------------------------------------------------*/
IMPL_LINK(  SwGlobalTree, DoubleClickHdl, SwGlobalTree *, EMPTYARG )
{
    SvLBoxEntry* pEntry = GetCurEntry();
    SwGlblDocContent* pCont = (SwGlblDocContent*)pEntry->GetUserData();
    if(pCont->GetType() == GLBLDOC_SECTION)
        OpenDoc(pCont);
    else
    {
        GotoContent(pCont);
        pActiveShell->GetView().GetEditWin().GrabFocus();
    }
    return 0;
}

/*-----------------07.10.97 08:09-------------------

--------------------------------------------------*/
IMPL_STATIC_LINK(SwGlobalTree, ShowFrameHdl, SwGlobalTree*, EMPTYARG)
{
    if(SwGlobalTree::GetShowShell())
        SfxViewFrame::GetFirst(SwGlobalTree::GetShowShell())->ToTop();
    SwGlobalTree::SetShowShell(0);
    return 0;
}
/* -----------------04.11.98 10:43-------------------
 *
 * --------------------------------------------------*/
void SwGlobalTree::InitEntry(SvLBoxEntry* pEntry,
        const XubString& rStr ,const Image& rImg1,const Image& rImg2)
{
    USHORT nColToHilite = 1; //0==Bitmap;1=="Spalte1";2=="Spalte2"
    SvTreeListBox::InitEntry( pEntry, rStr, rImg1, rImg2 );
    SvLBoxString* pCol = (SvLBoxString*)pEntry->GetItem( nColToHilite );
    SwLBoxString* pStr = new SwLBoxString( pEntry, 0, pCol->GetText() );
    pEntry->ReplaceItem( pStr, nColToHilite );
}
/* -----------------04.11.98 10:39-------------------
 *
 * --------------------------------------------------*/

void SwLBoxString::Paint( const Point& rPos, SvLBox& rDev, USHORT nFlags,
    SvLBoxEntry* pEntry )
{
    SwGlblDocContent* pCont = (SwGlblDocContent*)pEntry->GetUserData();
    const SwSection* pSect;
    if(pCont->GetType() == GLBLDOC_SECTION &&
        !(pSect = pCont->GetSection())->IsConnectFlag() )
    {
        Font aOldFont( rDev.GetFont());
        Font aFont(aOldFont);
        Color aCol( COL_LIGHTRED );
        aFont.SetColor( aCol );
        rDev.SetFont( aFont );
        rDev.DrawText( rPos, GetText() );
        rDev.SetFont( aOldFont );
    }
    else
        SvLBoxString::Paint( rPos, rDev, nFlags, pEntry);
}

