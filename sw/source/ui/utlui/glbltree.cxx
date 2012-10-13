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

#include <vcl/msgbox.hxx>
#include <svl/stritem.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/docfile.hxx>
#include <vcl/help.hxx>
#include <sot/filelist.hxx>
#include <svl/eitem.hxx>
#include <svl/urlbmk.hxx>
#include <svtools/filter.hxx>
#include <sfx2/docinsert.hxx>
#include <sfx2/filedlghelper.hxx>

#include <sfx2/app.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <content.hxx>
#include <edglbldc.hxx>
#include <section.hxx>
#include <tox.hxx>
#include <cnttab.hxx>
#define NAVIPI_CXX
#include <navipi.hxx>
#include <navicont.hxx>
#include <edtwin.hxx>
#include <uitool.hxx>

#include <cmdid.h>
#include <helpid.h>
#include <navipi.hrc>
#include <utlui.hrc>
#include <comcore.hrc>
#include <globals.hrc>
#include "swabstdlg.hxx"

using namespace ::com::sun::star::uno;
using ::rtl::OUString;

// Kontextmenue fuer GlobalTree
#define CTX_INSERT_ANY_INDEX 10
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
#define CTX_EDIT_LINK       5

#define GLOBAL_UPDATE_TIMEOUT 2000

// Flags fuer PopupMenu-enable/disable
#define ENABLE_INSERT_IDX   0x0001
#define ENABLE_INSERT_FILE  0x0002
#define ENABLE_INSERT_TEXT  0x0004
#define ENABLE_EDIT         0x0008
#define ENABLE_DELETE       0x0010
#define ENABLE_UPDATE       0x0020
#define ENABLE_UPDATE_SEL   0x0040
#define ENABLE_EDIT_LINK    0x0080

// TabPos nach links schieben
#define  GLBL_TABPOS_SUB 5

const SfxObjectShell* SwGlobalTree::pShowShell = 0;
static const char* aHelpForMenu[] =
{
    0,                          //
    HID_GLBLTREE_UPDATE,        //CTX_UPDATE
    HID_GLBLTREE_INSERT,        //CTX_INSERT
    HID_GLBLTREE_EDIT,          //CTX_EDIT
    HID_GLBLTREE_DEL,           //CTX_DELETE
    HID_GLBLTREE_EDIT_LINK,     //CTX_EDIT_LINK
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

class SwGlobalFrameListener_Impl : public SfxListener
{
    sal_Bool bValid;
public:
    SwGlobalFrameListener_Impl(SfxViewFrame& rFrame) :
        bValid(sal_True)
        {
            StartListening(rFrame);
        }

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    sal_Bool                IsValid() const {return bValid;}
};

void    SwGlobalFrameListener_Impl::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    if( rHint.ISA(SfxSimpleHint) &&
            (((SfxSimpleHint&) rHint).GetId() == SFX_HINT_DYING))
        bValid = sal_False;
}

SwGlobalTree::SwGlobalTree(Window* pParent, const ResId& rResId) :

    SvTreeListBox(pParent, rResId),

    pActiveShell        ( NULL ),
    pEmphasisEntry      ( NULL ),
    pDDSource           ( NULL ),
    pSwGlblDocContents  ( NULL ),
    pDefParentWin       ( NULL ),
    pDocContent         ( NULL ),
    pDocInserter        ( NULL ),

    bIsInternalDrag         ( sal_False ),
    bLastEntryEmphasis      ( sal_False ),
    bIsImageListInitialized ( sal_False )

{
    SetDragDropMode(SV_DRAGDROP_APP_COPY  |
                    SV_DRAGDROP_CTRL_MOVE |
                    SV_DRAGDROP_ENABLE_TOP );

    aUpdateTimer.SetTimeout(GLOBAL_UPDATE_TIMEOUT);
    aUpdateTimer.SetTimeoutHdl(LINK(this, SwGlobalTree, Timeout));
    aUpdateTimer.Start();
    for(sal_uInt16 i = 0; i < GLOBAL_CONTEXT_COUNT; i++)
    {
        aContextStrings[i] = SW_RESSTR(i+ ST_GLOBAL_CONTEXT_FIRST);
    }
    SetHelpId(HID_NAVIGATOR_GLOB_TREELIST);
    SelectHdl();
    SetDoubleClickHdl(LINK(this, SwGlobalTree, DoubleClickHdl));
    EnableContextMenuHandling();
}

SwGlobalTree::~SwGlobalTree()
{
    delete pSwGlblDocContents;
    delete pDocInserter;
}

sal_Int8 SwGlobalTree::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;
    SvLBoxEntry* pLast = (SvLBoxEntry*)LastVisible();
    if(pEmphasisEntry)
    {
        ImplShowTargetEmphasis( Prev(pEmphasisEntry), sal_False );
        pEmphasisEntry = 0;
    }
    else if(bLastEntryEmphasis && pLast)
    {
        ImplShowTargetEmphasis( pLast, sal_False);
    }

    SvLBoxEntry* pDropEntry = bLastEntryEmphasis ? 0 : GetEntry(rEvt.maPosPixel);
    if( bIsInternalDrag )
    {
        SvLBoxEntry* pDummy = 0;
        sal_uLong nInsertionPos = LIST_APPEND;
        NotifyMoving( pDropEntry, pDDSource, pDummy, nInsertionPos );
    }
    else
    {
        TransferableDataHelper aData( rEvt.maDropEvent.Transferable );

        String sFileName;
        const SwGlblDocContent* pCnt = pDropEntry ?
                    (const SwGlblDocContent*)pDropEntry->GetUserData() :
                            0;
        if( aData.HasFormat( FORMAT_FILE_LIST ))
        {
            nRet = rEvt.mnAction;
            SwGlblDocContents* pTempContents = new SwGlblDocContents;
            int nAbsContPos = pDropEntry ?
                                (int) GetModel()->GetAbsPos(pDropEntry):
                                    - 1;
            sal_uInt16 nEntryCount = (sal_uInt16)GetEntryCount();

            // Daten holen
            FileList aFileList;
            aData.GetFileList( FORMAT_FILE_LIST, aFileList );
            for ( sal_uInt16 n = (sal_uInt16)aFileList.Count(); n--; )
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
                    if(nEntryCount < pTempContents->size())
                    {
                        nEntryCount++;
                        nAbsContPos++;
                        pCnt = (*pTempContents)[ nAbsContPos ];
                    }
                }
            }
            delete pTempContents;
        }
        else if( 0 != (sFileName =
                        SwNavigationPI::CreateDropFileName( aData )).Len())
        {
            INetURLObject aTemp(sFileName);
            GraphicDescriptor aDesc(aTemp);
            if( !aDesc.Detect() )   // keine Grafiken annehmen
            {
                nRet = rEvt.mnAction;
                InsertRegion(pCnt, &sFileName);
            }
        }
    }
    bLastEntryEmphasis = sal_False;
    return nRet;

}

sal_Int8 SwGlobalTree::AcceptDrop( const AcceptDropEvent& rEvt )
{
    sal_Int8 nRet = rEvt.mnAction;

    //initiate scrolling
    GetDropTarget( rEvt.maPosPixel );
    SvLBoxEntry* pLast = (SvLBoxEntry*)LastVisible();
    if( rEvt.mbLeaving )
    {
        if( pEmphasisEntry )
        {
            ImplShowTargetEmphasis( Prev(pEmphasisEntry), sal_False );
            pEmphasisEntry = 0;
        }
        else if(bLastEntryEmphasis && pLast)
        {
            ImplShowTargetEmphasis( pLast, sal_False);
        }
        bLastEntryEmphasis = sal_False;
    }
    else
    {
        SvLBoxEntry* pDropEntry = GetEntry( rEvt.maPosPixel );
        if(bIsInternalDrag)
        {
            if( pDDSource != pDropEntry )
                nRet = rEvt.mnAction;
        }
        else if( IsDropFormatSupported( FORMAT_FILE ) ||
                  IsDropFormatSupported( FORMAT_STRING ) ||
                  IsDropFormatSupported( FORMAT_FILE_LIST ) ||
                  IsDropFormatSupported( SOT_FORMATSTR_ID_SOLK ) ||
                   IsDropFormatSupported( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK )||
                   IsDropFormatSupported( SOT_FORMATSTR_ID_FILECONTENT ) ||
                   IsDropFormatSupported( SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR ) ||
                   IsDropFormatSupported( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR ) ||
                   IsDropFormatSupported( SOT_FORMATSTR_ID_FILENAME ))
                nRet = DND_ACTION_LINK;

        if(pEmphasisEntry && pEmphasisEntry != pDropEntry)
            ImplShowTargetEmphasis( Prev(pEmphasisEntry), sal_False );
        else if(pLast && bLastEntryEmphasis  && pDropEntry)
        {
            ImplShowTargetEmphasis( pLast, sal_False);
            bLastEntryEmphasis = sal_False;
        }

        if(pDropEntry)
            ImplShowTargetEmphasis( Prev(pDropEntry), DND_ACTION_NONE != nRet );
        else if(pLast)
        {
            ImplShowTargetEmphasis( pLast, DND_ACTION_NONE != nRet );
            bLastEntryEmphasis = sal_True;
        }
        pEmphasisEntry = pDropEntry;
    }
    return nRet;
}

PopupMenu* SwGlobalTree::CreateContextMenu()
{
    PopupMenu* pPop = 0;
    if(pActiveShell &&
        !pActiveShell->GetView().GetDocShell()->IsReadOnly())
    {
        sal_uInt16 nEnableFlags = GetEnableFlags();
        pPop = new PopupMenu;
        PopupMenu* pSubPop1 = new PopupMenu;
        PopupMenu* pSubPop2 = new PopupMenu;

        for (sal_uInt16 i = CTX_UPDATE_SEL; i <= CTX_UPDATE_ALL; i++)
        {
            pSubPop2->InsertItem( i, aContextStrings[ST_UPDATE_SEL - ST_GLOBAL_CONTEXT_FIRST - CTX_UPDATE_SEL+ i] );
            pSubPop2->SetHelpId(i, aHelpForMenu[i]);
        }
        pSubPop2->EnableItem(CTX_UPDATE_SEL, 0 != (nEnableFlags & ENABLE_UPDATE_SEL));

        pSubPop1->InsertItem(CTX_INSERT_ANY_INDEX, aContextStrings[ST_INDEX  - ST_GLOBAL_CONTEXT_FIRST]);
        pSubPop1->SetHelpId(CTX_INSERT_ANY_INDEX, aHelpForMenu[CTX_INSERT_ANY_INDEX]);
        pSubPop1->InsertItem(CTX_INSERT_FILE, aContextStrings[ST_FILE   - ST_GLOBAL_CONTEXT_FIRST]);
        pSubPop1->SetHelpId(CTX_INSERT_FILE, aHelpForMenu[CTX_INSERT_FILE]);
        pSubPop1->InsertItem(CTX_INSERT_NEW_FILE, aContextStrings[ST_NEW_FILE   - ST_GLOBAL_CONTEXT_FIRST]);
        pSubPop1->SetHelpId(CTX_INSERT_NEW_FILE, aHelpForMenu[CTX_INSERT_NEW_FILE]);
        pSubPop1->InsertItem(CTX_INSERT_TEXT, aContextStrings[ST_TEXT   - ST_GLOBAL_CONTEXT_FIRST]);
        pSubPop1->SetHelpId(CTX_INSERT_TEXT, aHelpForMenu[CTX_INSERT_TEXT]);


        pPop->InsertItem(CTX_UPDATE, aContextStrings[ST_UPDATE - ST_GLOBAL_CONTEXT_FIRST]);
        pPop->SetHelpId(CTX_UPDATE, aHelpForMenu[CTX_UPDATE]);
        pPop->InsertItem(CTX_EDIT, aContextStrings[ST_EDIT_CONTENT - ST_GLOBAL_CONTEXT_FIRST]);
        pPop->SetHelpId(CTX_EDIT, aHelpForMenu[CTX_EDIT]);
        if(nEnableFlags&ENABLE_EDIT_LINK)
        {
            pPop->InsertItem(CTX_EDIT_LINK, aContextStrings[ST_EDIT_LINK - ST_GLOBAL_CONTEXT_FIRST]);
            pPop->SetHelpId(CTX_EDIT_LINK, aHelpForMenu[CTX_EDIT_LINK]);
        }
        pPop->InsertItem(CTX_INSERT, aContextStrings[ST_INSERT - ST_GLOBAL_CONTEXT_FIRST]);
        pPop->SetHelpId(CTX_INSERT, aHelpForMenu[CTX_INSERT]);
        pPop->InsertSeparator() ;
        pPop->InsertItem(CTX_DELETE, aContextStrings[ST_DELETE - ST_GLOBAL_CONTEXT_FIRST]);
        pPop->SetHelpId(CTX_DELETE, aHelpForMenu[CTX_DELETE]);

        //evtl. disablen
        pSubPop1->EnableItem(CTX_INSERT_ANY_INDEX,  0 != (nEnableFlags & ENABLE_INSERT_IDX ));
        pSubPop1->EnableItem(CTX_INSERT_TEXT,       0 != (nEnableFlags & ENABLE_INSERT_TEXT));
        pSubPop1->EnableItem(CTX_INSERT_FILE,       0 != (nEnableFlags & ENABLE_INSERT_FILE));
        pSubPop1->EnableItem(CTX_INSERT_NEW_FILE,   0 != (nEnableFlags & ENABLE_INSERT_FILE));

        pPop->EnableItem(CTX_UPDATE,    0 != (nEnableFlags & ENABLE_UPDATE));
        pPop->EnableItem(CTX_INSERT,    0 != (nEnableFlags & ENABLE_INSERT_IDX));
        pPop->EnableItem(CTX_EDIT,      0 != (nEnableFlags & ENABLE_EDIT));
        pPop->EnableItem(CTX_DELETE,    0 != (nEnableFlags & ENABLE_DELETE));


        pPop->SetPopupMenu( CTX_INSERT, pSubPop1 );
        pPop->SetPopupMenu( CTX_UPDATE, pSubPop2 );
    }
    return pPop;
}

void SwGlobalTree::TbxMenuHdl(sal_uInt16 nTbxId, ToolBox* pBox)
{
    sal_uInt16 nEnableFlags = GetEnableFlags();
    if(FN_GLOBAL_OPEN == nTbxId)
    {
        PopupMenu *pMenu = new PopupMenu;
        for (sal_uInt16 i = CTX_INSERT_ANY_INDEX; i <= CTX_INSERT_TEXT; i++)
        {
            pMenu->InsertItem( i, aContextStrings[ST_INDEX  - ST_GLOBAL_CONTEXT_FIRST - CTX_INSERT_ANY_INDEX + i] );
            pMenu->SetHelpId(i, aHelpForMenu[i] );
        }
        pMenu->EnableItem(CTX_INSERT_ANY_INDEX, 0 != (nEnableFlags & ENABLE_INSERT_IDX ));
        pMenu->EnableItem(CTX_INSERT_TEXT,      0 != (nEnableFlags & ENABLE_INSERT_TEXT));
        pMenu->EnableItem(CTX_INSERT_FILE,      0 != (nEnableFlags & ENABLE_INSERT_FILE));
        pMenu->EnableItem(CTX_INSERT_NEW_FILE,  0 != (nEnableFlags & ENABLE_INSERT_FILE));
        pMenu->SetSelectHdl(LINK(this, SwGlobalTree, PopupHdl));
        pMenu->Execute( pBox, pBox->GetItemRect(nTbxId).BottomLeft());
        delete pMenu;
        pBox->EndSelection();
        pBox->Invalidate();
    }
    else if(FN_GLOBAL_UPDATE == nTbxId)
    {
        PopupMenu *pMenu = new PopupMenu;
        for (sal_uInt16 i = CTX_UPDATE_SEL; i <= CTX_UPDATE_ALL; i++)
        {
            pMenu->InsertItem( i, aContextStrings[ST_UPDATE_SEL - ST_GLOBAL_CONTEXT_FIRST - CTX_UPDATE_SEL+ i] );
            pMenu->SetHelpId(i, aHelpForMenu[i] );
        }
        pMenu->EnableItem(CTX_UPDATE_SEL, 0 != (nEnableFlags & ENABLE_UPDATE_SEL));
        pMenu->SetSelectHdl(LINK(this, SwGlobalTree, PopupHdl));
        pMenu->Execute( pBox, pBox->GetItemRect(nTbxId).BottomLeft());
        delete pMenu;
        pBox->EndSelection();
        pBox->Invalidate();
    }
}

sal_uInt16  SwGlobalTree::GetEnableFlags() const
{
    SvLBoxEntry* pEntry = FirstSelected();
    sal_uInt16 nSelCount = (sal_uInt16)GetSelectionCount();
    sal_uInt16 nEntryCount = (sal_uInt16)GetEntryCount();
    SvLBoxEntry* pPrevEntry = pEntry ? Prev(pEntry) : 0;

    sal_uInt16 nRet = 0;
    if(nSelCount == 1 || !nEntryCount)
        nRet |= ENABLE_INSERT_IDX|ENABLE_INSERT_FILE;
    if(nSelCount == 1)
    {
        nRet |= ENABLE_EDIT;
        if( ((SwGlblDocContent*)pEntry->GetUserData())->GetType() != GLBLDOC_UNKNOWN &&
                    (!pPrevEntry || ((SwGlblDocContent*)pPrevEntry->GetUserData())->GetType() != GLBLDOC_UNKNOWN))
            nRet |= ENABLE_INSERT_TEXT;
        if( GLBLDOC_SECTION == ((SwGlblDocContent*)pEntry->GetUserData())->GetType() )
            nRet |= ENABLE_EDIT_LINK;
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

void     SwGlobalTree::RequestHelp( const HelpEvent& rHEvt )
{
    sal_Bool bParent = sal_True;
    Update(sal_True);
    Display(sal_True);
    if( rHEvt.GetMode() & HELPMODE_QUICK )
    {
        Point aPos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
        SvLBoxEntry* pEntry = GetEntry( aPos );
        const SwGlblDocContent* pCont = pEntry ?
                            (const SwGlblDocContent*)pEntry->GetUserData() : 0;
        if( pCont &&  GLBLDOC_SECTION == pCont->GetType())
        {
            bParent = sal_False;
            SvLBoxTab* pTab;
            SvLBoxItem* pItem = GetItem( pEntry, aPos.X(), &pTab );
            if(pItem && SV_ITEM_ID_LBOXSTRING == pItem->IsA())
            {
                const SwSection* pSect = pCont->GetSection();
                String sEntry = pSect->GetLinkFileName().GetToken(0, sfx2::cTokenSeperator);
                if(!pSect->IsConnectFlag())
                    sEntry.Insert(aContextStrings[ST_BROKEN_LINK - ST_GLOBAL_CONTEXT_FIRST], 0 );
                Point aEntryPos = GetEntryPosition( pEntry );

                aEntryPos.X() = GetTabPos( pEntry, pTab );
                Size aSize( pItem->GetSize( this, pEntry ) );

                if((aEntryPos.X() + aSize.Width()) > GetSizePixel().Width())
                    aSize.Width() = GetSizePixel().Width() - aEntryPos.X();

                aEntryPos = OutputToScreenPixel(aEntryPos);
                Rectangle aItemRect( aEntryPos, aSize );
                if(Help::IsBalloonHelpEnabled())
                {
                    aEntryPos.X() += aSize.Width();
                    Help::ShowBalloon( this, aEntryPos, aItemRect, sEntry );
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

void     SwGlobalTree::SelectHdl()
{

    sal_uInt16 nSelCount = (sal_uInt16)GetSelectionCount();
    SvLBoxEntry* pSel = FirstSelected();
    sal_uInt16 nAbsPos = pSel ? (sal_uInt16)GetModel()->GetAbsPos(pSel) : 0;
    SwNavigationPI* pNavi = GetParentWindow();
    sal_Bool bReadonly = !pActiveShell ||
                pActiveShell->GetView().GetDocShell()->IsReadOnly();
    pNavi->aGlobalToolBox.EnableItem(FN_GLOBAL_EDIT,  nSelCount == 1 && !bReadonly);
    pNavi->aGlobalToolBox.EnableItem(FN_GLOBAL_OPEN,  nSelCount <= 1 && !bReadonly);
    pNavi->aGlobalToolBox.EnableItem(FN_GLOBAL_UPDATE,  GetEntryCount() > 0 && !bReadonly);
    pNavi->aGlobalToolBox.EnableItem(FN_ITEM_UP,
                    nSelCount == 1 && nAbsPos && !bReadonly);
    pNavi->aGlobalToolBox.EnableItem(FN_ITEM_DOWN,
                    nSelCount == 1 && nAbsPos < ((sal_uInt16)GetEntryCount()) - 1 && !bReadonly);

}

void     SwGlobalTree::DeselectHdl()
{
    SelectHdl();
}

DragDropMode SwGlobalTree::NotifyStartDrag( TransferDataContainer& ,
                                                SvLBoxEntry* pEntry )
{
    bIsInternalDrag = sal_True;
    pDDSource = pEntry;
    return SV_DRAGDROP_CTRL_MOVE;
}

long     SwGlobalTree::GetTabPos( SvLBoxEntry*, SvLBoxTab* pTab)
{
    return pTab->GetPos() - GLBL_TABPOS_SUB;
}

sal_Bool     SwGlobalTree::NotifyMoving(   SvLBoxEntry*  pTarget,
                                        SvLBoxEntry*  pSource,
                                        SvLBoxEntry*&,
                                        sal_uLong&
                                    )
{
    SvTreeList* _pModel = GetModel();
    sal_uInt16 nSource = (sal_uInt16) _pModel->GetAbsPos(pSource);
    sal_uInt16 nDest   = pTarget ? (sal_uInt16) _pModel->GetAbsPos(pTarget) : pSwGlblDocContents->size();

    if( pActiveShell->MoveGlobalDocContent(
            *pSwGlblDocContents, nSource, nSource + 1, nDest ) &&
            Update( sal_False ))
        Display();
    return sal_False;
}

sal_Bool     SwGlobalTree::NotifyCopying(  SvLBoxEntry*  /*pTarget*/,
                                        SvLBoxEntry*  /*pEntry*/,
                                        SvLBoxEntry*& /*rpNewParent*/,
                                        sal_uLong&        /*rNewChildPos*/
                                    )
{
    return sal_False;
}

sal_Bool SwGlobalTree::NotifyAcceptDrop( SvLBoxEntry* pEntry)
{
    return pEntry != 0;
}

void SwGlobalTree::StartDrag( sal_Int8 nAction, const Point& rPt )
{
    if( 1 == GetSelectionCount() )
        SvTreeListBox::StartDrag( nAction, rPt );
}

void SwGlobalTree::DragFinished( sal_Int8 nAction )
{
    SvTreeListBox::DragFinished( nAction );
    bIsInternalDrag = sal_False;
}

/***************************************************************************
    Beschreibung:   Wird ein Ctrl+DoubleClick in einen freien Bereich ausgefuehrt,
 *                  dann soll die Basisfunktion des Controls gerufen werden
***************************************************************************/
void  SwGlobalTree::MouseButtonDown( const MouseEvent& rMEvt )
{
    Point aPos( rMEvt.GetPosPixel());
    SvLBoxEntry* pEntry = GetEntry( aPos, sal_True );
    if( !pEntry && rMEvt.IsLeft() && rMEvt.IsMod1() && (rMEvt.GetClicks() % 2) == 0)
        Control::MouseButtonDown( rMEvt );
    else
        SvTreeListBox::MouseButtonDown( rMEvt );
}

void     SwGlobalTree::GetFocus()
{
    if(Update( sal_False ))
        Display();
    SvTreeListBox::GetFocus();
}

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

void SwGlobalTree::Clear()
{
    pEmphasisEntry = 0;
    SvTreeListBox::Clear();
}

void    SwGlobalTree::Display(sal_Bool bOnlyUpdateUserData)
{
    if(!bIsImageListInitialized)
    {
        aEntryImages = ImageList(SW_RES(IMG_NAVI_ENTRYBMP));
        bIsImageListInitialized = sal_True;
    }
    sal_uInt16 nCount = pSwGlblDocContents->size();
    if(bOnlyUpdateUserData && GetEntryCount() == pSwGlblDocContents->size())
    {
        SvLBoxEntry* pEntry = First();
        for( sal_uInt16 i = 0; i < nCount; i++)
        {
            SwGlblDocContent* pCont = (*pSwGlblDocContents)[i];
            pEntry->SetUserData(pCont);
            pEntry = Next(pEntry);
        }
    }
    else
    {
        SetUpdateMode( sal_False );
        SvLBoxEntry* pOldSelEntry = FirstSelected();
        String sEntryName;  // Name des Eintrags
        sal_uInt16 nSelPos = USHRT_MAX;
        if(pOldSelEntry)
        {
            sEntryName = GetEntryText(pOldSelEntry);
            nSelPos = (sal_uInt16)GetModel()->GetAbsPos(pOldSelEntry);
        }
        Clear();
        if(!pSwGlblDocContents)
            Update( sal_False );

        SvLBoxEntry* pSelEntry = 0;
        for( sal_uInt16 i = 0; i < nCount; i++)
        {
            SwGlblDocContent* pCont = (*pSwGlblDocContents)[i];
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
                    sEntry = pSect->GetSectionName();
                    aImage = aEntryImages.GetImage(SID_SW_START + CONTENT_TYPE_REGION);
                }
                break;
            }
            SvLBoxEntry* pEntry = InsertEntry(sEntry, aImage, aImage,
                        0, sal_False, LIST_APPEND, pCont);
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
        SetUpdateMode( sal_True );
    }
}

void SwGlobalTree::InsertRegion( const SwGlblDocContent* pCont, const String* pFileName )
{
    Sequence< OUString > aFileNames;
    if ( !pFileName )
    {
        pDefParentWin = Application::GetDefDialogParent();
        Application::SetDefDialogParent( this );
        delete pDocInserter;
        pDocInserter = new ::sfx2::DocumentInserter(
                rtl::OUString("swriter"), true );
        pDocInserter->StartExecuteModal( LINK( this, SwGlobalTree, DialogClosedHdl ) );
    }
    else if ( pFileName->Len() )
    {
        aFileNames.realloc(1);
        INetURLObject aFileName;
        aFileName.SetSmartURL( *pFileName );
        aFileNames.getArray()[0] = aFileName.GetMainURL( INetURLObject::NO_DECODE );
        InsertRegion( pCont, aFileNames );
    }
}

void    SwGlobalTree::EditContent(const SwGlblDocContent* pCont )
{
    sal_uInt16 nSlot = 0;
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
        if(Update( sal_False ))
            Display();
    }
}

IMPL_LINK( SwGlobalTree, PopupHdl, Menu* , pMenu)
{
    ExcecuteContextMenuAction( pMenu->GetCurItemId());
    return sal_True;
}

void    SwGlobalTree::ExcecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry )
{
    SvLBoxEntry* pEntry = FirstSelected();
    SwGlblDocContent* pCont = pEntry ? (SwGlblDocContent*)pEntry->GetUserData() : 0;
    // wird waehrend des Dialogs ein RequestHelp gerufen,
    // dann geht der Content verloren. Deshalb wird hier eine
    // Kopie angelegt, in der nur die DocPos richtig gesetzt ist.
    SwGlblDocContent* pContCopy = 0;
    if(pCont)
        pContCopy = new SwGlblDocContent(pCont->GetDocPos());
    SfxDispatcher& rDispatch = *pActiveShell->GetView().GetViewFrame()->GetDispatcher();
    sal_uInt16 nSlot = 0;
    bool bDeleteContentCopy = true;
    switch( nSelectedPopupEntry )
    {
        case CTX_UPDATE_SEL:
        {
            // zwei Durchlaeufe: zuerst die Bereiche, dann die Verzeichnisse
            // aktualisieren
            SvLBoxEntry* pSelEntry = FirstSelected();
            while( pSelEntry )
            {
                SwGlblDocContent* pContent = (SwGlblDocContent*)pSelEntry->GetUserData();
                if(GLBLDOC_SECTION == pContent->GetType() &&
                    pContent->GetSection()->IsConnected())
                {
                    ((SwSection*)pContent->GetSection())->UpdateNow();
                }

                pSelEntry = NextSelected(pSelEntry);
            }
            pSelEntry = FirstSelected();
            while( pSelEntry )
            {
                SwGlblDocContent* pContent = (SwGlblDocContent*)pSelEntry->GetUserData();
                if(GLBLDOC_TOXBASE == pContent->GetType())
                    pActiveShell->UpdateTableOf(*pContent->GetTOX());
                pSelEntry = NextSelected(pSelEntry);
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
            pActiveShell->GetLinkManager().UpdateAllLinks(sal_True);
            if(CTX_UPDATE_ALL == nSelectedPopupEntry)
                nSlot = FN_UPDATE_TOX;
            pCont = 0;
        }
        break;
        case CTX_EDIT:
        {
            OSL_ENSURE(pCont, "edit without entry ? " );
            EditContent(pCont);
        }
        break;
        case CTX_EDIT_LINK:
        {
            OSL_ENSURE(pCont, "edit without entry ? " );
            SfxStringItem aName(FN_EDIT_REGION,
                    pCont->GetSection()->GetSectionName());
            rDispatch.Execute(FN_EDIT_REGION, SFX_CALLMODE_ASYNCHRON, &aName, 0L);
        }
        break;
        case CTX_DELETE:
        {
            // sind mehrere Eintraege selektiert, dann muss nach jedem delete
            // das Array neu gefuellt werden. Damit man sich nichts merken muss,
            // beginnt das Loeschen am Ende
            SvLBoxEntry* pSelEntry = LastSelected();
            SwGlblDocContents* pTempContents  = 0;
            pActiveShell->StartAction();
            while(pSelEntry)
            {
                pActiveShell->DeleteGlobalDocContent(
                    pTempContents ? *pTempContents : *pSwGlblDocContents,
                                     (sal_uInt16)GetModel()->GetAbsPos(pSelEntry));
                pSelEntry = PrevSelected(pSelEntry);
                if(pSelEntry)
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

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "Dialogdiet fail!");
                AbstractMultiTOXTabDialog* pDlg = pFact->CreateMultiTOXTabDialog( DLG_MULTI_TOX,
                                                        this, aSet,
                                                        *pActiveShell,
                                                        0,
                                                        USHRT_MAX,
                                                        sal_True);
                OSL_ENSURE(pDlg, "Dialogdiet fail!");
                if(RET_OK == pDlg->Execute())
                {
                    SwTOXDescription&  rDesc = pDlg->GetTOXDescription(
                                                pDlg->GetCurrentTOXType());
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
            bDeleteContentCopy = false;
            pDocContent = pContCopy;
            InsertRegion( pContCopy );
            pCont = NULL;
        }
        break;
        case CTX_INSERT_NEW_FILE:
        {
            SfxViewFrame* pGlobFrm = pActiveShell->GetView().GetViewFrame();
            SwGlobalFrameListener_Impl aFrmListener(*pGlobFrm);

            sal_uLong nEntryPos = pEntry ? GetModel()->GetAbsPos(pEntry) : (sal_uLong)-1;
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
                String sNewFile(pMedium->GetURLObject().GetMainURL(INetURLObject::DECODE_TO_IURI));
                // Bereich mit dem Dok-Namen einfuegen
                // eigenes Dok in den Vordergrund

                if(aFrmListener.IsValid() && sNewFile.Len())
                {
                    pGlobFrm->ToTop();
                    // durch das Update sind die Eintraege invalid
                    if(nEntryPos != (sal_uLong)-1)
                    {
                        Update( sal_False );
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
                    return;
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
                pActiveShell->Up( sal_False, 1 );
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
    if(Update( sal_False ))
        Display();
    if ( bDeleteContentCopy )
        delete pContCopy;
    else
        bDeleteContentCopy = true;
}

IMPL_LINK_NOARG(SwGlobalTree, Timeout)
{
    if(!HasFocus() && Update( sal_False ))
        Display();
    return 0;
}

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

void    SwGlobalTree::ShowTree()
{
    aUpdateTimer.Start();
    SvTreeListBox::Show();
}

void    SwGlobalTree::HideTree()
{
    aUpdateTimer.Stop();
    SvTreeListBox::Hide();
}

void    SwGlobalTree::ExecCommand(sal_uInt16 nCmd)
{
    SvLBoxEntry* pEntry = FirstSelected();
    OSL_ENSURE(pEntry, "gleich knallt's");
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
            sal_Bool bMove = sal_False;
            sal_uInt16 nSource = (sal_uInt16)GetModel()->GetAbsPos(pEntry);
            sal_uInt16 nDest = nSource;
            switch(nCmd)
            {
                case FN_ITEM_DOWN:
                {
                    sal_uInt16 nEntryCount = (sal_uInt16)GetEntryCount();
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
                    Update( sal_False ))
                Display();
        }
    }
}

sal_Bool    SwGlobalTree::Update(sal_Bool bHard)
{
    SwView* pActView = GetParentWindow()->GetCreateView();
    sal_Bool bRet = sal_False;
    if(pActView && pActView->GetWrtShellPtr())
    {
        const SwWrtShell* pOldShell = pActiveShell;
        pActiveShell = pActView->GetWrtShellPtr();
        if(pActiveShell != pOldShell)
        {
            delete pSwGlblDocContents;
            pSwGlblDocContents = 0;
        }
        if(!pSwGlblDocContents)
        {
            pSwGlblDocContents = new SwGlblDocContents;
            bRet = sal_True;
            pActiveShell->GetGlobalDocContent(*pSwGlblDocContents);
        }
        else
        {
            sal_Bool bCopy = sal_False;
            SwGlblDocContents* pTempContents  = new SwGlblDocContents;
            pActiveShell->GetGlobalDocContent(*pTempContents);
            if(pTempContents->size() != pSwGlblDocContents->size() ||
                    pTempContents->size() != GetEntryCount())
            {
                bRet = sal_True;
                bCopy = sal_True;
            }
            else
            {
                for(sal_uInt16 i = 0; i < pTempContents->size() && !bCopy; i++)
                {
                    SwGlblDocContent* pLeft = (*pTempContents)[i];
                    SwGlblDocContent* pRight = (*pSwGlblDocContents)[i];
                    GlobalDocContentType eType = pLeft->GetType();
                    SvLBoxEntry* pEntry = GetEntry(i);
                    String sTemp = GetEntryText(pEntry);
                    if (
                         eType != pRight->GetType() ||
                         (
                           eType == GLBLDOC_SECTION &&
                           pLeft->GetSection()->GetSectionName() != sTemp
                         ) ||
                         (
                           eType == GLBLDOC_TOXBASE &&
                           pLeft->GetTOX()->GetTitle() != sTemp
                         )
                       )
                    {
                        bCopy = bRet = sal_True;
                    }
                }
            }
            if(bCopy || bHard)
            {
                pSwGlblDocContents->DeleteAndDestroyAll();
                pSwGlblDocContents->insert( *pTempContents );
                pTempContents->clear();

            }
            delete pTempContents;
        }

    }
    else
    {
        Clear();
        if(pSwGlblDocContents)
            pSwGlblDocContents->DeleteAndDestroyAll();
    }
    // hier muss noch eine Veraenderungspruefung rein!
    return bRet;
}

void SwGlobalTree::OpenDoc(const SwGlblDocContent* pCont)
{
    String sFileName(pCont->GetSection()->GetLinkFileName().GetToken(0,
            sfx2::cTokenSeperator));
    sal_Bool bFound = sal_False;
    const SfxObjectShell* pCurr = SfxObjectShell::GetFirst();
    while( !bFound && pCurr )
    {
        if(pCurr->GetMedium() &&
            String(pCurr->GetMedium()->GetURLObject().GetMainURL(INetURLObject::DECODE_TO_IURI)) == sFileName)
        {
            bFound = sal_True;
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
        SfxBoolItem aReadOnly(SID_DOC_READONLY, sal_False);
        SfxStringItem aTargetFrameName( SID_TARGETNAME, rtl::OUString("_blank") );
        SfxStringItem aReferer(SID_REFERER, pActiveShell->GetView().GetDocShell()->GetTitle());
        pActiveShell->GetView().GetViewFrame()->GetDispatcher()->
                Execute(SID_OPENDOC, SFX_CALLMODE_ASYNCHRON,
                            &aURL, &aReadOnly, &aReferer, &aTargetFrameName, 0L);
    }
}

IMPL_LINK_NOARG( SwGlobalTree, DoubleClickHdl)
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

IMPL_STATIC_LINK_NOINSTANCE(SwGlobalTree, ShowFrameHdl, SwGlobalTree*, EMPTYARG)
{
    if(SwGlobalTree::GetShowShell())
        SfxViewFrame::GetFirst(SwGlobalTree::GetShowShell())->ToTop();
    SwGlobalTree::SetShowShell(0);
    return 0;
}

void SwGlobalTree::InitEntry(SvLBoxEntry* pEntry,
        const XubString& rStr ,const Image& rImg1,const Image& rImg2,
        SvLBoxButtonKind eButtonKind)
{
    sal_uInt16 nColToHilite = 1; //0==Bitmap;1=="Spalte1";2=="Spalte2"
    SvTreeListBox::InitEntry( pEntry, rStr, rImg1, rImg2, eButtonKind );
    SvLBoxString* pCol = (SvLBoxString*)pEntry->GetItem( nColToHilite );
    SwLBoxString* pStr = new SwLBoxString( pEntry, 0, pCol->GetText() );
    pEntry->ReplaceItem( pStr, nColToHilite );
}

void SwLBoxString::Paint( const Point& rPos, SvTreeListBox& rDev, sal_uInt16 nFlags,
    SvLBoxEntry* pEntry )
{
    SwGlblDocContent* pCont = (SwGlblDocContent*)pEntry->GetUserData();
    if(pCont->GetType() == GLBLDOC_SECTION &&
        !(pCont->GetSection())->IsConnectFlag() )
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

void    SwGlobalTree::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        aEntryImages = ImageList(SW_RES(IMG_NAVI_ENTRYBMP));
        Update(sal_True);
    }
    SvTreeListBox::DataChanged( rDCEvt );
}

void SwGlobalTree::InsertRegion( const SwGlblDocContent* _pContent, const Sequence< OUString >& _rFiles )
{
    sal_Int32 nFiles = _rFiles.getLength();
    if ( nFiles )
    {
        sal_Bool bMove = sal_False;
        if ( !_pContent )
        {
            SvLBoxEntry* pLast = (SvLBoxEntry*)LastVisible();
            _pContent = (SwGlblDocContent*)pLast->GetUserData();
            bMove = sal_True;
        }
        String sFilePassword;
        sal_uInt16 nEntryCount = (sal_uInt16)GetEntryCount();
        const OUString* pFileNames = _rFiles.getConstArray();
        SwWrtShell& rSh = GetParentWindow()->GetCreateView()->GetWrtShell();
        rSh.StartAction();
        // after insertion of the first new content the 'pCont' parameter becomes invalid
        // find the index of the 'anchor' content to always use a current anchor content
        size_t nAnchorContent = pSwGlblDocContents->size() - 1;
        if ( !bMove )
        {
            for (size_t nContent = 0; nContent < pSwGlblDocContents->size();
                    ++nContent)
            {
                if( *_pContent == *(*pSwGlblDocContents)[ nContent ] )
                {
                    nAnchorContent = nContent;
                    break;
                }
            }
        }
        SwGlblDocContents aTempContents;
        for ( sal_Int32 nFile = 0; nFile < nFiles; ++nFile )
        {
            //update the global document content after each inserted document
            rSh.GetGlobalDocContent(aTempContents);
            SwGlblDocContent* pAnchorContent = 0;
            OSL_ENSURE(aTempContents.size() > (nAnchorContent + nFile), "invalid anchor content -> last insertion failed");
            if ( aTempContents.size() > (nAnchorContent + nFile) )
                pAnchorContent = aTempContents[nAnchorContent + nFile];
            else
                pAnchorContent = aTempContents.back();
            String sFileName(pFileNames[nFile]);
            INetURLObject aFileUrl;
            aFileUrl.SetSmartURL( sFileName );
            String sSectionName(String(aFileUrl.GetLastName(
                INetURLObject::DECODE_UNAMBIGUOUS)).GetToken(0,
                sfx2::cTokenSeperator));
            sal_uInt16 nSectCount = rSh.GetSectionFmtCount();
            String sTempSectionName(sSectionName);
            sal_uInt16 nAddNumber = 0;
            sal_uInt16 nCount = 0;
            // evtl : und Index anhaengen, wenn der Bereichsname schon vergeben ist
            while ( nCount < nSectCount )
            {
                const SwSectionFmt& rFmt = rSh.GetSectionFmt(nCount);
                if ((rFmt.GetSection()->GetSectionName() == sTempSectionName)
                    && rFmt.IsInNodesArr())
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

            if ( nAddNumber )
                sSectionName = sTempSectionName;

            SwSectionData aSectionData(CONTENT_SECTION, sSectionName);
            aSectionData.SetProtectFlag(true);
            aSectionData.SetHidden(false);

            aSectionData.SetLinkFileName(sFileName);
            aSectionData.SetType(FILE_LINK_SECTION);
            aSectionData.SetLinkFilePassword( sFilePassword );

            rSh.InsertGlobalDocContent( *pAnchorContent, aSectionData );
        }
        if ( bMove )
        {
            Update( sal_False );
            rSh.MoveGlobalDocContent(
                *pSwGlblDocContents, nEntryCount, nEntryCount + (sal_uInt16)nFiles, nEntryCount - (sal_uInt16)nFiles );
        }
        rSh.EndAction();
        Update( sal_False );
        Display();
    }
}

IMPL_LINK( SwGlobalTree, DialogClosedHdl, sfx2::FileDialogHelper*, _pFileDlg )
{
    Application::SetDefDialogParent( pDefParentWin );
    if ( ERRCODE_NONE == _pFileDlg->GetError() )
    {
        SfxMediumList* pMedList = pDocInserter->CreateMediumList();
        if ( pMedList )
        {
            Sequence< OUString >aFileNames( pMedList->size() );
            OUString* pFileNames = aFileNames.getArray();
            sal_Int32 nPos = 0;
            for ( size_t i = 0, n = pMedList->size(); i < n; ++i )
            {
                SfxMedium* pMed = pMedList->at( i );
                String sFileName = pMed->GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
                sFileName += sfx2::cTokenSeperator;
                sFileName += pMed->GetFilter()->GetFilterName();
                sFileName += sfx2::cTokenSeperator;
                pFileNames[nPos++] = sFileName;
            }
            delete pMedList;
            InsertRegion( pDocContent, aFileNames );
            DELETEZ( pDocContent );
        }
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
