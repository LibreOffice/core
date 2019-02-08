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
#include <vcl/graphicfilter.hxx>
#include <vcl/settings.hxx>

#include <vcl/treelistentry.hxx>
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
#include <navipi.hxx>
#include <navicont.hxx>
#include <edtwin.hxx>
#include <uitool.hxx>

#include <cmdid.h>
#include <helpids.h>
#include <strings.hrc>
#include <globals.hrc>
#include <bitmaps.hlst>
#include <swabstdlg.hxx>
#include <memory>

using namespace ::com::sun::star::uno;

// Context menu for GlobalTree
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

// TabPos: push to left
#define  GLBL_TABPOS_SUB 5

const SfxObjectShell* SwGlobalTree::pShowShell = nullptr;
static const char* aHelpForMenu[] =
{
    nullptr,
    HID_GLBLTREE_UPDATE,        //CTX_UPDATE
    HID_GLBLTREE_INSERT,        //CTX_INSERT
    HID_GLBLTREE_EDIT,          //CTX_EDIT
    HID_GLBLTREE_DEL,           //CTX_DELETE
    HID_GLBLTREE_EDIT_LINK,     //CTX_EDIT_LINK
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    HID_GLBLTREE_INS_IDX,       //CTX_INSERT_ANY_INDEX
    HID_GLBLTREE_INS_FILE,      //CTX_INSERT_FILE
    HID_GLBLTREE_INS_NEW_FILE,  //CTX_INSERT_NEW_FILE
    HID_GLBLTREE_INS_TEXT,      //CTX_INSERT_TEXT
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    HID_GLBLTREE_UPD_SEL,       //CTX_UPDATE_SEL
    HID_GLBLTREE_UPD_IDX,       //CTX_UPDATE_INDEX
    HID_GLBLTREE_UPD_LINK,      //CTX_UPDATE_LINK
    HID_GLBLTREEUPD_ALL         //CTX_UPDATE_ALL
};

class SwGlobalFrameListener_Impl : public SfxListener
{
    bool bValid;
public:
    explicit SwGlobalFrameListener_Impl(SfxViewFrame& rFrame)
        : bValid(true)
    {
        StartListening(rFrame);
    }

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    bool                IsValid() const {return bValid;}
};

void SwGlobalFrameListener_Impl::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    if( rHint.GetId() == SfxHintId::Dying)
        bValid = false;
}

enum GLOBAL_CONTEXT_IDX
{
    IDX_STR_UPDATE = 0,
    IDX_STR_EDIT_CONTENT = 1,
    IDX_STR_EDIT_INSERT = 2,
    IDX_STR_INDEX = 3,
    IDX_STR_FILE = 4,
    IDX_STR_NEW_FILE = 5,
    IDX_STR_INSERT_TEXT = 6,
    IDX_STR_DELETE = 7,
    IDX_STR_UPDATE_SEL = 8,
    IDX_STR_UPDATE_INDEX = 9,
    IDX_STR_UPDATE_LINK = 10,
    IDX_STR_UPDATE_ALL = 11,
    IDX_STR_BROKEN_LINK = 12,
    IDX_STR_EDIT_LINK = 13
};

static const char* GLOBAL_CONTEXT_ARY[] =
{
    STR_UPDATE,
    STR_EDIT_CONTENT,
    STR_EDIT_INSERT,
    STR_INDEX,
    STR_FILE,
    STR_NEW_FILE,
    STR_INSERT_TEXT,
    STR_DELETE,
    STR_UPDATE_SEL,
    STR_UPDATE_INDEX,
    STR_UPDATE_LINK,
    STR_UPDATE_ALL,
    STR_BROKEN_LINK,
    STR_EDIT_LINK
};

SwGlobalTree::SwGlobalTree(vcl::Window* pParent, SwNavigationPI* pDialog)
    : SvTreeListBox(pParent)
    , m_xDialog(pDialog)
    , m_pActiveShell(nullptr)
    , m_pEmphasisEntry(nullptr)
    , m_pDDSource(nullptr)
    , m_bIsInternalDrag(false)
    , m_bLastEntryEmphasis(false)
{
    SetDragDropMode(DragDropMode::APP_COPY  |
                    DragDropMode::CTRL_MOVE |
                    DragDropMode::ENABLE_TOP );

    m_aUpdateTimer.SetTimeout(GLOBAL_UPDATE_TIMEOUT);
    m_aUpdateTimer.SetInvokeHandler(LINK(this, SwGlobalTree, Timeout));
    m_aUpdateTimer.Start();
    for (sal_uInt16 i = 0; i < GLOBAL_CONTEXT_COUNT; i++)
    {
        m_aContextStrings[i] = SwResId(GLOBAL_CONTEXT_ARY[i]);
    }
    SetHelpId(HID_NAVIGATOR_GLOB_TREELIST);
    SelectHdl();
    SetDoubleClickHdl(LINK(this, SwGlobalTree, DoubleClickHdl));
    EnableContextMenuHandling();
}

SwGlobalTree::~SwGlobalTree()
{
    disposeOnce();
}

void SwGlobalTree::dispose()
{
    m_pSwGlblDocContents.reset();
    m_pDocInserter.reset();
    m_aUpdateTimer.Stop();
    m_xDialog.clear();
    SvTreeListBox::dispose();
}

Size SwGlobalTree::GetOptimalSize() const
{
    return LogicToPixel(Size(110, 112), MapMode(MapUnit::MapAppFont));
}

sal_Int8 SwGlobalTree::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;
    SvTreeListEntry* pLast = LastVisible();
    if(m_pEmphasisEntry)
    {
        ImplShowTargetEmphasis( Prev(m_pEmphasisEntry), false );
        m_pEmphasisEntry = nullptr;
    }
    else if(m_bLastEntryEmphasis && pLast)
    {
        ImplShowTargetEmphasis( pLast, false);
    }

    SvTreeListEntry* pDropEntry = m_bLastEntryEmphasis ? nullptr : GetEntry(rEvt.maPosPixel);
    if( m_bIsInternalDrag )
    {
        SvTreeListEntry* pDummy = nullptr;
        sal_uLong nInsertionPos = TREELIST_APPEND;
        NotifyMoving( pDropEntry, m_pDDSource, pDummy, nInsertionPos );
    }
    else
    {
        TransferableDataHelper aData( rEvt.maDropEvent.Transferable );

        OUString sFileName;
        const SwGlblDocContent* pCnt = pDropEntry ?
                    static_cast<const SwGlblDocContent*>(pDropEntry->GetUserData()) :
                            nullptr;
        if( aData.HasFormat( SotClipboardFormatId::FILE_LIST ))
        {
            nRet = rEvt.mnAction;
            std::unique_ptr<SwGlblDocContents> pTempContents(new SwGlblDocContents);
            int nAbsContPos = pDropEntry ?
                                static_cast<int>(GetModel()->GetAbsPos(pDropEntry)):
                                    - 1;
            sal_uLong nEntryCount = GetEntryCount();

            // Get data
            FileList aFileList;
            aData.GetFileList( SotClipboardFormatId::FILE_LIST, aFileList );
            for ( size_t n = aFileList.Count(); n--; )
            {
                sFileName = aFileList.GetFile(n);
                InsertRegion(pCnt, &sFileName);
                // The list of contents must be newly fetched after inserting,
                // to not work on an old content.
                if(n)
                {
                    m_pActiveShell->GetGlobalDocContent(*pTempContents);
                    // If the file was successfully inserted,
                    // then the next content must also be fetched.
                    if(nEntryCount < pTempContents->size())
                    {
                        nEntryCount++;
                        nAbsContPos++;
                        pCnt = (*pTempContents)[ nAbsContPos ].get();
                    }
                }
            }
        }
        else if( !(sFileName =
                        SwNavigationPI::CreateDropFileName( aData )).isEmpty())
        {
            INetURLObject aTemp(sFileName);
            GraphicDescriptor aDesc(aTemp);
            if( !aDesc.Detect() )   // accept no graphics
            {
                nRet = rEvt.mnAction;
                InsertRegion(pCnt, &sFileName);
            }
        }
    }
    m_bLastEntryEmphasis = false;
    return nRet;

}

sal_Int8 SwGlobalTree::AcceptDrop( const AcceptDropEvent& rEvt )
{
    sal_Int8 nRet = rEvt.mnAction;

    //initiate scrolling
    GetDropTarget( rEvt.maPosPixel );
    SvTreeListEntry* pLast = LastVisible();
    if( rEvt.mbLeaving )
    {
        if( m_pEmphasisEntry )
        {
            ImplShowTargetEmphasis( Prev(m_pEmphasisEntry), false );
            m_pEmphasisEntry = nullptr;
        }
        else if(m_bLastEntryEmphasis && pLast)
        {
            ImplShowTargetEmphasis( pLast, false);
        }
        m_bLastEntryEmphasis = false;
    }
    else
    {
        SvTreeListEntry* pDropEntry = GetEntry( rEvt.maPosPixel );
        if(m_bIsInternalDrag)
        {
            if( m_pDDSource != pDropEntry )
                nRet = rEvt.mnAction;
        }
        else if( IsDropFormatSupported( SotClipboardFormatId::SIMPLE_FILE ) ||
                  IsDropFormatSupported( SotClipboardFormatId::STRING ) ||
                  IsDropFormatSupported( SotClipboardFormatId::FILE_LIST ) ||
                  IsDropFormatSupported( SotClipboardFormatId::SOLK ) ||
                   IsDropFormatSupported( SotClipboardFormatId::NETSCAPE_BOOKMARK )||
                   IsDropFormatSupported( SotClipboardFormatId::FILECONTENT ) ||
                   IsDropFormatSupported( SotClipboardFormatId::FILEGRPDESCRIPTOR ) ||
                   IsDropFormatSupported( SotClipboardFormatId::UNIFORMRESOURCELOCATOR ) ||
                   IsDropFormatSupported( SotClipboardFormatId::FILENAME ))
                nRet = DND_ACTION_LINK;

        if(m_pEmphasisEntry && m_pEmphasisEntry != pDropEntry)
            ImplShowTargetEmphasis( Prev(m_pEmphasisEntry), false );
        else if(pLast && m_bLastEntryEmphasis  && pDropEntry)
        {
            ImplShowTargetEmphasis( pLast, false);
            m_bLastEntryEmphasis = false;
        }

        if(pDropEntry)
            ImplShowTargetEmphasis( Prev(pDropEntry), DND_ACTION_NONE != nRet );
        else if(pLast)
        {
            ImplShowTargetEmphasis( pLast, DND_ACTION_NONE != nRet );
            m_bLastEntryEmphasis = true;
        }
        m_pEmphasisEntry = pDropEntry;
    }
    return nRet;
}

VclPtr<PopupMenu> SwGlobalTree::CreateContextMenu()
{
    VclPtr<PopupMenu> pPop;
    if(m_pActiveShell &&
        !m_pActiveShell->GetView().GetDocShell()->IsReadOnly())
    {
        const MenuEnableFlags nEnableFlags = GetEnableFlags();
        pPop = VclPtr<PopupMenu>::Create();
        VclPtrInstance<PopupMenu> pSubPop1;
        VclPtrInstance<PopupMenu> pSubPop2;

        for (sal_uInt16 i = CTX_UPDATE_SEL; i <= CTX_UPDATE_ALL; i++)
        {
            pSubPop2->InsertItem( i, m_aContextStrings[IDX_STR_UPDATE_SEL + i - CTX_UPDATE_SEL] );
            pSubPop2->SetHelpId(i, aHelpForMenu[i]);
        }
        pSubPop2->EnableItem(CTX_UPDATE_SEL, bool(nEnableFlags & MenuEnableFlags::UpdateSel));

        pSubPop1->InsertItem(CTX_INSERT_ANY_INDEX, m_aContextStrings[IDX_STR_INDEX]);
        pSubPop1->SetHelpId(CTX_INSERT_ANY_INDEX, aHelpForMenu[CTX_INSERT_ANY_INDEX]);
        pSubPop1->InsertItem(CTX_INSERT_FILE, m_aContextStrings[IDX_STR_FILE]);
        pSubPop1->SetHelpId(CTX_INSERT_FILE, aHelpForMenu[CTX_INSERT_FILE]);
        pSubPop1->InsertItem(CTX_INSERT_NEW_FILE, m_aContextStrings[IDX_STR_NEW_FILE]);
        pSubPop1->SetHelpId(CTX_INSERT_NEW_FILE, aHelpForMenu[CTX_INSERT_NEW_FILE]);
        pSubPop1->InsertItem(CTX_INSERT_TEXT, m_aContextStrings[IDX_STR_INSERT_TEXT]);
        pSubPop1->SetHelpId(CTX_INSERT_TEXT, aHelpForMenu[CTX_INSERT_TEXT]);

        pPop->InsertItem(CTX_UPDATE, m_aContextStrings[IDX_STR_UPDATE]);
        pPop->SetHelpId(CTX_UPDATE, aHelpForMenu[CTX_UPDATE]);
        pPop->InsertItem(CTX_EDIT, m_aContextStrings[IDX_STR_EDIT_CONTENT]);
        pPop->SetHelpId(CTX_EDIT, aHelpForMenu[CTX_EDIT]);
        if(nEnableFlags&MenuEnableFlags::EditLink)
        {
            pPop->InsertItem(CTX_EDIT_LINK, m_aContextStrings[IDX_STR_EDIT_LINK]);
            pPop->SetHelpId(CTX_EDIT_LINK, aHelpForMenu[CTX_EDIT_LINK]);
        }
        pPop->InsertItem(CTX_INSERT, m_aContextStrings[IDX_STR_EDIT_INSERT]);
        pPop->SetHelpId(CTX_INSERT, aHelpForMenu[CTX_INSERT]);
        pPop->InsertSeparator() ;
        pPop->InsertItem(CTX_DELETE, m_aContextStrings[IDX_STR_DELETE]);
        pPop->SetHelpId(CTX_DELETE, aHelpForMenu[CTX_DELETE]);

        //disabling if applicable
        pSubPop1->EnableItem(CTX_INSERT_ANY_INDEX,  bool(nEnableFlags & MenuEnableFlags::InsertIdx ));
        pSubPop1->EnableItem(CTX_INSERT_TEXT,       bool(nEnableFlags & MenuEnableFlags::InsertText));
        pSubPop1->EnableItem(CTX_INSERT_FILE,       bool(nEnableFlags & MenuEnableFlags::InsertFile));
        pSubPop1->EnableItem(CTX_INSERT_NEW_FILE,   bool(nEnableFlags & MenuEnableFlags::InsertFile));

        pPop->EnableItem(CTX_UPDATE,    bool(nEnableFlags & MenuEnableFlags::Update));
        pPop->EnableItem(CTX_INSERT,    bool(nEnableFlags & MenuEnableFlags::InsertIdx));
        pPop->EnableItem(CTX_EDIT,      bool(nEnableFlags & MenuEnableFlags::Edit));
        pPop->EnableItem(CTX_DELETE,    bool(nEnableFlags & MenuEnableFlags::Delete));

        pPop->SetPopupMenu( CTX_INSERT, pSubPop1 );
        pPop->SetPopupMenu( CTX_UPDATE, pSubPop2 );
    }
    return pPop;
}

void SwGlobalTree::TbxMenuHdl(sal_uInt16 nTbxId, ToolBox* pBox)
{
    const MenuEnableFlags nEnableFlags = GetEnableFlags();
    const OUString sCommand(pBox->GetItemCommand(nTbxId));
    if (sCommand == "insert")
    {
        ScopedVclPtrInstance<PopupMenu> pMenu;
        for (sal_uInt16 i = CTX_INSERT_ANY_INDEX; i <= CTX_INSERT_TEXT; ++i)
        {
            pMenu->InsertItem( i, m_aContextStrings[IDX_STR_INDEX + i - CTX_INSERT_ANY_INDEX] );
            pMenu->SetHelpId(i, aHelpForMenu[i] );
        }
        pMenu->EnableItem(CTX_INSERT_ANY_INDEX, bool(nEnableFlags & MenuEnableFlags::InsertIdx ));
        pMenu->EnableItem(CTX_INSERT_TEXT,      bool(nEnableFlags & MenuEnableFlags::InsertText));
        pMenu->EnableItem(CTX_INSERT_FILE,      bool(nEnableFlags & MenuEnableFlags::InsertFile));
        pMenu->EnableItem(CTX_INSERT_NEW_FILE,  bool(nEnableFlags & MenuEnableFlags::InsertFile));
        pMenu->SetSelectHdl(LINK(this, SwGlobalTree, PopupHdl));
        pMenu->Execute(pBox, pBox->GetItemRect(nTbxId));
        pMenu.disposeAndClear();
        pBox->EndSelection();
        pBox->Invalidate();
    }
    else if (sCommand == "update")
    {
        ScopedVclPtrInstance<PopupMenu> pMenu;
        for (sal_uInt16 i = CTX_UPDATE_SEL; i <= CTX_UPDATE_ALL; i++)
        {
            pMenu->InsertItem( i, m_aContextStrings[IDX_STR_UPDATE_SEL + i - CTX_UPDATE_SEL] );
            pMenu->SetHelpId(i, aHelpForMenu[i] );
        }
        pMenu->EnableItem(CTX_UPDATE_SEL, bool(nEnableFlags & MenuEnableFlags::UpdateSel));
        pMenu->SetSelectHdl(LINK(this, SwGlobalTree, PopupHdl));
        pMenu->Execute(pBox, pBox->GetItemRect(nTbxId));
        pMenu.disposeAndClear();
        pBox->EndSelection();
        pBox->Invalidate();
    }
}

MenuEnableFlags  SwGlobalTree::GetEnableFlags() const
{
    SvTreeListEntry* pEntry = FirstSelected();
    sal_uLong nSelCount = GetSelectionCount();
    sal_uLong nEntryCount = GetEntryCount();
    SvTreeListEntry* pPrevEntry = pEntry ? Prev(pEntry) : nullptr;

    MenuEnableFlags nRet = MenuEnableFlags::NONE;
    if(nSelCount == 1 || !nEntryCount)
        nRet |= MenuEnableFlags::InsertIdx|MenuEnableFlags::InsertFile;
    if(nSelCount == 1)
    {
        nRet |= MenuEnableFlags::Edit;
        if (pEntry && static_cast<SwGlblDocContent*>(pEntry->GetUserData())->GetType() != GLBLDOC_UNKNOWN &&
                    (!pPrevEntry || static_cast<SwGlblDocContent*>(pPrevEntry->GetUserData())->GetType() != GLBLDOC_UNKNOWN))
            nRet |= MenuEnableFlags::InsertText;
        if (pEntry && GLBLDOC_SECTION == static_cast<SwGlblDocContent*>(pEntry->GetUserData())->GetType())
            nRet |= MenuEnableFlags::EditLink;
    }
    else if(!nEntryCount)
    {
        nRet |= MenuEnableFlags::InsertText;
    }
    if(nEntryCount)
        nRet |= MenuEnableFlags::Update|MenuEnableFlags::Delete;
    if(nSelCount)
        nRet |= MenuEnableFlags::UpdateSel;
    return nRet;
}

void     SwGlobalTree::RequestHelp( const HelpEvent& rHEvt )
{
    bool bParent = true;
    Update(true);
    Display(true);
    if( rHEvt.GetMode() & HelpEventMode::QUICK )
    {
        Point aPos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
        SvTreeListEntry* pEntry = GetEntry( aPos );
        const SwGlblDocContent* pCont = pEntry ?
                            static_cast<const SwGlblDocContent*>(pEntry->GetUserData()) : nullptr;
        if( pCont &&  GLBLDOC_SECTION == pCont->GetType())
        {
            bParent = false;
            SvLBoxTab* pTab;
            SvLBoxItem* pItem = GetItem( pEntry, aPos.X(), &pTab );
            if (pItem && SvLBoxItemType::String == pItem->GetType())
            {
                const SwSection* pSect = pCont->GetSection();
                OUString sEntry = pSect->GetLinkFileName().getToken(0, sfx2::cTokenSeparator);
                if(!pSect->IsConnectFlag())
                    sEntry = m_aContextStrings[IDX_STR_BROKEN_LINK] + sEntry;
                Point aEntryPos = GetEntryPosition( pEntry );

                aEntryPos.setX( GetTabPos( pEntry, pTab ) );
                Size aSize( pItem->GetSize( this, pEntry ) );

                if((aEntryPos.X() + aSize.Width()) > GetSizePixel().Width())
                    aSize.setWidth( GetSizePixel().Width() - aEntryPos.X() );

                aEntryPos = OutputToScreenPixel(aEntryPos);
                tools::Rectangle aItemRect( aEntryPos, aSize );
                if(Help::IsBalloonHelpEnabled())
                {
                    aEntryPos.AdjustX(aSize.Width() );
                    Help::ShowBalloon( this, aEntryPos, aItemRect, sEntry );
                }
                else
                    Help::ShowQuickHelp( this, aItemRect, sEntry,
                        QuickHelpFlags::Left|QuickHelpFlags::VCenter );
            }
        }
    }

    if(bParent)
        SvTreeListBox::RequestHelp(rHEvt);
}

void     SwGlobalTree::SelectHdl()
{

    sal_uLong nSelCount = GetSelectionCount();
    SvTreeListEntry* pSel = FirstSelected();
    sal_uLong nAbsPos = pSel ? GetModel()->GetAbsPos(pSel) : 0;
    SwNavigationPI* pNavi = GetParentWindow();
    bool bReadonly = !m_pActiveShell ||
                m_pActiveShell->GetView().GetDocShell()->IsReadOnly();
    pNavi->m_aGlobalToolBox->EnableItem(pNavi->m_aGlobalToolBox->GetItemId("edit"),  nSelCount == 1 && !bReadonly);
    pNavi->m_aGlobalToolBox->EnableItem(pNavi->m_aGlobalToolBox->GetItemId("insert"),  nSelCount <= 1 && !bReadonly);
    pNavi->m_aGlobalToolBox->EnableItem(pNavi->m_aGlobalToolBox->GetItemId("update"),  GetEntryCount() > 0 && !bReadonly);
    pNavi->m_aGlobalToolBox->EnableItem(pNavi->m_aGlobalToolBox->GetItemId("up"),
                    nSelCount == 1 && nAbsPos && !bReadonly);
    pNavi->m_aGlobalToolBox->EnableItem(pNavi->m_aGlobalToolBox->GetItemId("down"),
                    nSelCount == 1 && nAbsPos < GetEntryCount() - 1 && !bReadonly);

}

void     SwGlobalTree::DeselectHdl()
{
    SelectHdl();
}

DragDropMode SwGlobalTree::NotifyStartDrag( TransferDataContainer& ,
                                                SvTreeListEntry* pEntry )
{
    m_bIsInternalDrag = true;
    m_pDDSource = pEntry;
    return DragDropMode::CTRL_MOVE;
}

sal_IntPtr SwGlobalTree::GetTabPos( SvTreeListEntry*, SvLBoxTab* pTab)
{
    return pTab->GetPos() - GLBL_TABPOS_SUB;
}

TriState SwGlobalTree::NotifyMoving(   SvTreeListEntry*  pTarget,
                                        SvTreeListEntry*  pSource,
                                        SvTreeListEntry*&,
                                        sal_uLong&
                                    )
{
    SvTreeList* _pModel = GetModel();
    sal_uLong nSource = _pModel->GetAbsPos(pSource);
    sal_uLong nDest   = pTarget ? _pModel->GetAbsPos(pTarget) : m_pSwGlblDocContents->size();

    if( m_pActiveShell->MoveGlobalDocContent(
            *m_pSwGlblDocContents, nSource, nSource + 1, nDest ) &&
            Update( false ))
        Display();

    return TRISTATE_FALSE;
}

TriState SwGlobalTree::NotifyCopying(  SvTreeListEntry*  /*pTarget*/,
                                        SvTreeListEntry*  /*pEntry*/,
                                        SvTreeListEntry*& /*rpNewParent*/,
                                        sal_uLong&        /*rNewChildPos*/
                                    )
{
    return TRISTATE_FALSE;
}

bool SwGlobalTree::NotifyAcceptDrop( SvTreeListEntry* pEntry)
{
    return pEntry != nullptr;
}

void SwGlobalTree::StartDrag( sal_Int8 nAction, const Point& rPt )
{
    if( 1 == GetSelectionCount() )
        SvTreeListBox::StartDrag( nAction, rPt );
}

void SwGlobalTree::DragFinished( sal_Int8 nAction )
{
    SvTreeListBox::DragFinished( nAction );
    m_bIsInternalDrag = false;
}

// If a Ctrl+DoubleClick is executed in an empty area,
// then the base function of the control should be called.

void  SwGlobalTree::MouseButtonDown( const MouseEvent& rMEvt )
{
    Point aPos( rMEvt.GetPosPixel());
    SvTreeListEntry* pEntry = GetEntry( aPos, true );
    if( !pEntry && rMEvt.IsLeft() && rMEvt.IsMod1() && (rMEvt.GetClicks() % 2) == 0)
        Control::MouseButtonDown( rMEvt );
    else
        SvTreeListBox::MouseButtonDown( rMEvt );
}

void     SwGlobalTree::GetFocus()
{
    if(Update( false ))
        Display();
    SvTreeListBox::GetFocus();
}

void     SwGlobalTree::KeyInput(const KeyEvent& rKEvt)
{
    const vcl::KeyCode aCode = rKEvt.GetKeyCode();
    if(aCode.GetCode() == KEY_RETURN)
    {
        switch(aCode.GetModifier())
        {
            case KEY_MOD2:
                // Switch boxes
                GetParentWindow()->ToggleTree();
            break;
        }
    }
    else
        SvTreeListBox::KeyInput(rKEvt);
}

void SwGlobalTree::Clear()
{
    m_pEmphasisEntry = nullptr;
    SvTreeListBox::Clear();
}

void SwGlobalTree::Display(bool bOnlyUpdateUserData)
{
    size_t nCount = m_pSwGlblDocContents->size();
    if(bOnlyUpdateUserData && GetEntryCount() == m_pSwGlblDocContents->size())
    {
        SvTreeListEntry* pEntry = First();
        for (size_t i = 0; i < nCount && pEntry; i++)
        {
            SwGlblDocContent* pCont = (*m_pSwGlblDocContents)[i].get();
            pEntry->SetUserData(pCont);
            pEntry = Next(pEntry);
            assert(pEntry || i == nCount - 1);
        }
    }
    else
    {
        SetUpdateMode( false );
        SvTreeListEntry* pOldSelEntry = FirstSelected();
        OUString sEntryName;  // Name of the entry
        sal_uLong nSelPos = TREELIST_ENTRY_NOTFOUND;
        if(pOldSelEntry)
        {
            sEntryName = GetEntryText(pOldSelEntry);
            nSelPos = GetModel()->GetAbsPos(pOldSelEntry);
        }
        Clear();
        if(!m_pSwGlblDocContents)
            Update( false );

        SvTreeListEntry* pSelEntry = nullptr;
        for( size_t i = 0; i < nCount; i++)
        {
            SwGlblDocContent* pCont = (*m_pSwGlblDocContents)[i].get();
            OUString sEntry;
            Image aImage;
            switch( pCont->GetType()  )
            {
                case GLBLDOC_UNKNOWN:
                {
                    sEntry = m_aContextStrings[IDX_STR_INSERT_TEXT];
                }
                break;
                case GLBLDOC_TOXBASE:
                {
                    const SwTOXBase* pBase = pCont->GetTOX();
                    sEntry = pBase->GetTitle();
                    aImage = Image(StockImage::Yes, RID_BMP_NAVI_INDEX);
                }
                break;
                case GLBLDOC_SECTION:
                {
                    const SwSection* pSect = pCont->GetSection();
                    sEntry = pSect->GetSectionName();
                    aImage = Image(StockImage::Yes, RID_BMP_DROP_REGION);
                }
                break;
            }
            SvTreeListEntry* pEntry = InsertEntry(sEntry, aImage, aImage,
                        nullptr, false, TREELIST_APPEND, pCont);
            if(sEntry == sEntryName)
            {
                pSelEntry = pEntry;
            }
        }
        if(pSelEntry)
        {
            Select(pSelEntry);
        }
        else if(nSelPos != TREELIST_ENTRY_NOTFOUND && nSelPos < nCount)
        {
            Select(GetEntry(nSelPos));
        }
        else if(nCount)
            Select(First());
        else
            SelectHdl();
        SetUpdateMode( true );
    }
}

void SwGlobalTree::InsertRegion( const SwGlblDocContent* pCont, const OUString* pFileName )
{
    Sequence< OUString > aFileNames;
    if ( !pFileName )
    {
        m_pDocInserter.reset(new ::sfx2::DocumentInserter(GetFrameWeld(), "swriter", sfx2::DocumentInserter::Mode::InsertMulti));
        m_pDocInserter->StartExecuteModal( LINK( this, SwGlobalTree, DialogClosedHdl ) );
    }
    else if ( !pFileName->isEmpty() )
    {
        aFileNames.realloc(1);
        INetURLObject aFileName;
        aFileName.SetSmartURL( *pFileName );
        aFileNames.getArray()[0] = aFileName.GetMainURL( INetURLObject::DecodeMechanism::NONE );
        InsertRegion( pCont, aFileNames );
    }
}

void    SwGlobalTree::EditContent(const SwGlblDocContent* pCont )
{
    sal_uInt16 nSlot = 0;
    switch( pCont->GetType() )
    {
        case GLBLDOC_UNKNOWN:
            m_pActiveShell->GetView().GetEditWin().GrabFocus();
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
            pCont = nullptr;
        }
        break;
    }
    if(pCont)
        GotoContent(pCont);
    if(nSlot)
    {
        m_pActiveShell->GetView().GetViewFrame()->GetDispatcher()->Execute(nSlot);
        if(Update( false ))
            Display();
    }
}

IMPL_LINK( SwGlobalTree, PopupHdl, Menu* , pMenu, bool)
{
    ExecuteContextMenuAction( pMenu->GetCurItemId());
    return true;
}

void    SwGlobalTree::ExecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry )
{
    SvTreeListEntry* pEntry = FirstSelected();
    SwGlblDocContent* pCont = pEntry ? static_cast<SwGlblDocContent*>(pEntry->GetUserData()) : nullptr;
    // If a RequestHelp is called during the dialogue,
    // then the content gets lost. Because of that a copy
    // is created in which only the DocPos is set correctly.
    std::unique_ptr<SwGlblDocContent> pContCopy;
    if(pCont)
        pContCopy.reset(new SwGlblDocContent(pCont->GetDocPos()));
    SfxDispatcher& rDispatch = *m_pActiveShell->GetView().GetViewFrame()->GetDispatcher();
    sal_uInt16 nSlot = 0;
    switch( nSelectedPopupEntry )
    {
        case CTX_UPDATE_SEL:
        {
            // Two passes: first update the areas, then the directories.
            SvTreeListEntry* pSelEntry = FirstSelected();
            while( pSelEntry )
            {
                SwGlblDocContent* pContent = static_cast<SwGlblDocContent*>(pSelEntry->GetUserData());
                if(GLBLDOC_SECTION == pContent->GetType() &&
                    pContent->GetSection()->IsConnected())
                {
                    const_cast<SwSection*>(pContent->GetSection())->UpdateNow();
                }

                pSelEntry = NextSelected(pSelEntry);
            }
            pSelEntry = FirstSelected();
            while( pSelEntry )
            {
                SwGlblDocContent* pContent = static_cast<SwGlblDocContent*>(pSelEntry->GetUserData());
                if(GLBLDOC_TOXBASE == pContent->GetType())
                    m_pActiveShell->UpdateTableOf(*pContent->GetTOX());
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
            m_pActiveShell->GetLinkManager().UpdateAllLinks(true, false, nullptr);
            if(CTX_UPDATE_ALL == nSelectedPopupEntry)
                nSlot = FN_UPDATE_TOX;
            pCont = nullptr;
        }
        break;
        case CTX_EDIT:
        {
            OSL_ENSURE(pCont, "edit without entry ? " );
            if (pCont)
            {
                EditContent(pCont);
            }
        }
        break;
        case CTX_EDIT_LINK:
        {
            OSL_ENSURE(pCont, "edit without entry ? " );
            if (pCont)
            {
                SfxStringItem aName(FN_EDIT_REGION,
                        pCont->GetSection()->GetSectionName());
                rDispatch.ExecuteList(FN_EDIT_REGION, SfxCallMode::ASYNCHRON,
                        { &aName });
            }
        }
        break;
        case CTX_DELETE:
        {
            // If several entries selected, then after each delete the array
            // must be refilled. So you do not have to remember anything,
            // deleting begins at the end.
            SvTreeListEntry* pSelEntry = LastSelected();
            std::unique_ptr<SwGlblDocContents> pTempContents;
            m_pActiveShell->StartAction();
            while(pSelEntry)
            {
                m_pActiveShell->DeleteGlobalDocContent(
                    pTempContents ? *pTempContents : *m_pSwGlblDocContents,
                                     GetModel()->GetAbsPos(pSelEntry));
                pSelEntry = PrevSelected(pSelEntry);
                if(pSelEntry)
                {
                    pTempContents.reset(new SwGlblDocContents);
                    m_pActiveShell->GetGlobalDocContent(*pTempContents);
                }
            }
            pTempContents.reset();
            m_pActiveShell->EndAction();
            pCont = nullptr;
        }
        break;
        case CTX_INSERT_ANY_INDEX:
        {
            if(pContCopy)
            {
                SfxItemSet aSet(
                    m_pActiveShell->GetView().GetPool(),
                    svl::Items<
                        RES_FRM_SIZE, RES_FRM_SIZE,
                        RES_LR_SPACE, RES_LR_SPACE,
                        RES_BACKGROUND, RES_BACKGROUND,
                        RES_COL, RES_COL,
                        SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE,
                        FN_PARAM_TOX_TYPE, FN_PARAM_TOX_TYPE>{});

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                ScopedVclPtr<AbstractMultiTOXTabDialog> pDlg(pFact->CreateMultiTOXTabDialog(
                                                        this, aSet,
                                                        *m_pActiveShell,
                                                        nullptr,
                                                        true));
                if(RET_OK == pDlg->Execute())
                {
                    SwTOXDescription&  rDesc = pDlg->GetTOXDescription(
                                                pDlg->GetCurrentTOXType());
                    SwTOXMgr aMgr(m_pActiveShell);
                    SwTOXBase* pToInsert = nullptr;
                    if(aMgr.UpdateOrInsertTOX(rDesc, &pToInsert, pDlg->GetOutputItemSet()))
                        m_pActiveShell->InsertGlobalDocContent( *pContCopy, *pToInsert );
                }
                pCont = nullptr;
            }
        }
        break;
        case CTX_INSERT_FILE:
        {
            m_pDocContent = std::move(pContCopy);
            InsertRegion( m_pDocContent.get() );
            pCont = nullptr;
        }
        break;
        case CTX_INSERT_NEW_FILE:
        {
            SfxViewFrame* pGlobFrame = m_pActiveShell->GetView().GetViewFrame();
            SwGlobalFrameListener_Impl aFrameListener(*pGlobFrame);

            sal_uLong nEntryPos = pEntry ? GetModel()->GetAbsPos(pEntry) : sal_uLong(-1);
            // Creating a new doc
            SfxStringItem aFactory(SID_NEWDOCDIRECT,
                            SwDocShell::Factory().GetFilterContainer()->GetName());

            const SfxFrameItem* pItem = static_cast<const SfxFrameItem*>(
                            rDispatch.ExecuteList(SID_NEWDOCDIRECT,
                                SfxCallMode::SYNCHRON, { &aFactory }));

            // save at
            SfxFrame* pFrame = pItem ? pItem->GetFrame() : nullptr;
            SfxViewFrame* pViewFrame = pFrame ? pFrame->GetCurrentViewFrame() : nullptr;
            if (pViewFrame)
            {
                const SfxBoolItem* pBool = static_cast<const SfxBoolItem*>(
                        pViewFrame->GetDispatcher()->Execute(
                                SID_SAVEASDOC, SfxCallMode::SYNCHRON ));
                SfxObjectShell& rObj = *pViewFrame->GetObjectShell();
                const SfxMedium* pMedium = rObj.GetMedium();
                OUString sNewFile(pMedium->GetURLObject().GetMainURL(INetURLObject::DecodeMechanism::ToIUri));
                // Insert the area with the Doc-Name
                // Bring the own Doc in the foreground
                if(aFrameListener.IsValid() && !sNewFile.isEmpty())
                {
                    pGlobFrame->ToTop();
                    // Due to the update the entries are invalid
                    if(nEntryPos != sal_uLong(-1))
                    {
                        Update( false );
                        Display();
                        Select(GetModel()->GetEntryAtAbsPos(nEntryPos));
                        pEntry = FirstSelected();
                        pCont = pEntry ? static_cast<SwGlblDocContent*>(pEntry->GetUserData()) : nullptr;
                    }
                    else
                    {
                        pEntry = nullptr;
                        pCont = nullptr;
                    }
                    if(pBool->GetValue())
                    {
                        InsertRegion(pCont, &sNewFile);
                        pViewFrame->ToTop();
                    }
                    else
                        pViewFrame->GetDispatcher()->Execute(SID_CLOSEWIN,
                                                SfxCallMode::SYNCHRON);
                }
                else
                {
                    pViewFrame->ToTop();
                    return;
                }
            }
        }
        break;
        case CTX_INSERT_TEXT:
        {
            if(pCont)
                m_pActiveShell->InsertGlobalDocContent(*pCont);
            else
            {
                m_pActiveShell->SplitNode(); // Empty document
                m_pActiveShell->Up( false );
            }
            m_pActiveShell->GetView().GetEditWin().GrabFocus();
        }
        break;
        case CTX_UPDATE:
            pCont = nullptr;
        break;
        default:;
        // here nothing happens
    }
    if(pCont)
        GotoContent(pCont);
    if(nSlot)
        rDispatch.Execute(nSlot);
    if(Update( false ))
        Display();
}

IMPL_LINK_NOARG(SwGlobalTree, Timeout, Timer *, void)
{
    if(!IsDisposed() && !HasFocus() && Update( false ))
        Display();
}

void SwGlobalTree::GotoContent(const SwGlblDocContent* pCont)
{
    m_pActiveShell->EnterStdMode();

    switch( pCont->GetType()  )
    {
        case GLBLDOC_UNKNOWN:
            m_pActiveShell->GotoGlobalDocContent(*pCont);
        break;
        case GLBLDOC_TOXBASE:
        {
            const OUString sName = pCont->GetTOX()->GetTOXName();
            if (!m_pActiveShell->GotoNextTOXBase(&sName))
                m_pActiveShell->GotoPrevTOXBase(&sName);
        }
        break;
        case GLBLDOC_SECTION:
        break;
    }

}

void    SwGlobalTree::ShowTree()
{
    m_aUpdateTimer.Start();
    SvTreeListBox::Show();
}

void    SwGlobalTree::HideTree()
{
    m_aUpdateTimer.Stop();
    SvTreeListBox::Hide();
}

void    SwGlobalTree::ExecCommand(const OUString &rCmd)
{
    SvTreeListEntry* pEntry = FirstSelected();
    OSL_ENSURE(pEntry, "It explodes in the next moment");
    if (rCmd == "edit")
    {
        const SwGlblDocContent* pCont = static_cast<const SwGlblDocContent*>(
                                                pEntry->GetUserData());
        EditContent(pCont);
    }
    else
    {
        if(GetSelectionCount() == 1)
        {
            bool bMove = false;
            sal_uLong nSource = GetModel()->GetAbsPos(pEntry);
            sal_uLong nDest = nSource;
            if (rCmd == "down")
            {
                sal_uLong nEntryCount = GetEntryCount();
                bMove = nEntryCount > nSource + 1;
                nDest+= 2;
            }
            else if (rCmd == "up")
            {
                bMove = 0 != nSource;
                nDest--;
            }
            if( bMove && m_pActiveShell->MoveGlobalDocContent(
                *m_pSwGlblDocContents, nSource, nSource + 1, nDest ) &&
                    Update( false ))
                Display();
        }
    }
}

bool    SwGlobalTree::Update(bool bHard)
{
    SwView* pActView = GetParentWindow()->GetCreateView();
    bool bRet = false;
    if(pActView && pActView->GetWrtShellPtr())
    {
        const SwWrtShell* pOldShell = m_pActiveShell;
        m_pActiveShell = pActView->GetWrtShellPtr();
        if(m_pActiveShell != pOldShell)
        {
            m_pSwGlblDocContents.reset();
        }
        if(!m_pSwGlblDocContents)
        {
            m_pSwGlblDocContents.reset(new SwGlblDocContents);
            bRet = true;
            m_pActiveShell->GetGlobalDocContent(*m_pSwGlblDocContents);
        }
        else
        {
            bool bCopy = false;
            std::unique_ptr<SwGlblDocContents> pTempContents(new SwGlblDocContents);
            m_pActiveShell->GetGlobalDocContent(*pTempContents);
            if(pTempContents->size() != m_pSwGlblDocContents->size() ||
                    pTempContents->size() != GetEntryCount())
            {
                bRet = true;
                bCopy = true;
            }
            else
            {
                for(size_t i = 0; i < pTempContents->size() && !bCopy; i++)
                {
                    SwGlblDocContent* pLeft = (*pTempContents)[i].get();
                    SwGlblDocContent* pRight = (*m_pSwGlblDocContents)[i].get();
                    GlobalDocContentType eType = pLeft->GetType();
                    SvTreeListEntry* pEntry = GetEntry(i);
                    OUString sTemp = GetEntryText(pEntry);
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
                        bCopy = bRet = true;
                    }
                }
            }
            if(bCopy || bHard)
            {
                *m_pSwGlblDocContents = std::move( *pTempContents );
            }
        }

    }
    else
    {
        Clear();
        if(m_pSwGlblDocContents)
            m_pSwGlblDocContents->clear();
    }
    // FIXME: Implement a test for changes!
    return bRet;
}

void SwGlobalTree::OpenDoc(const SwGlblDocContent* pCont)
{
    const OUString sFileName(pCont->GetSection()->GetLinkFileName().getToken(0,
            sfx2::cTokenSeparator));
    bool bFound = false;
    const SfxObjectShell* pCurr = SfxObjectShell::GetFirst();
    while( !bFound && pCurr )
    {
        if(pCurr->GetMedium() &&
           pCurr->GetMedium()->GetURLObject().GetMainURL(INetURLObject::DecodeMechanism::ToIUri) == sFileName)
        {
            bFound = true;
            SwGlobalTree::SetShowShell(pCurr);
            Application::PostUserEvent( LINK( this, SwGlobalTree, ShowFrameHdl ), nullptr, true );
            pCurr = nullptr;
        }
        else
            pCurr = SfxObjectShell::GetNext(*pCurr);
    }
    if(!bFound)
    {
        SfxStringItem aURL(SID_FILE_NAME, sFileName);
        SfxBoolItem aReadOnly(SID_DOC_READONLY, false);
        SfxStringItem aTargetFrameName( SID_TARGETNAME, "_blank" );
        SfxStringItem aReferer(SID_REFERER, m_pActiveShell->GetView().GetDocShell()->GetTitle());
        m_pActiveShell->GetView().GetViewFrame()->GetDispatcher()->
                ExecuteList(SID_OPENDOC, SfxCallMode::ASYNCHRON,
                        { &aURL, &aReadOnly, &aReferer, &aTargetFrameName });
    }
}

IMPL_LINK_NOARG( SwGlobalTree, DoubleClickHdl, SvTreeListBox*, bool)
{
    SvTreeListEntry* pEntry = GetCurEntry();
    SwGlblDocContent* pCont = static_cast<SwGlblDocContent*>(pEntry->GetUserData());
    if(pCont->GetType() == GLBLDOC_SECTION)
        OpenDoc(pCont);
    else
    {
        GotoContent(pCont);
        m_pActiveShell->GetView().GetEditWin().GrabFocus();
    }
    return false;
}

SwNavigationPI* SwGlobalTree::GetParentWindow()
{
    return m_xDialog;
}

IMPL_STATIC_LINK_NOARG(SwGlobalTree, ShowFrameHdl, void*, void)
{
    SfxViewFrame* pFirst = pShowShell ? SfxViewFrame::GetFirst(pShowShell) : nullptr;
    if (pFirst)
        pFirst->ToTop();
    SwGlobalTree::SetShowShell(nullptr);
}

void SwGlobalTree::InitEntry(SvTreeListEntry* pEntry,
        const OUString& rStr ,const Image& rImg1,const Image& rImg2,
        SvLBoxButtonKind eButtonKind)
{
    const size_t nColToHilite = 1; //0==Bitmap;1=="Column1";2=="Column2"
    SvTreeListBox::InitEntry( pEntry, rStr, rImg1, rImg2, eButtonKind );
    SvLBoxString& rCol = static_cast<SvLBoxString&>(pEntry->GetItem( nColToHilite ));
    pEntry->ReplaceItem(std::make_unique<SwLBoxString>(rCol.GetText()), nColToHilite);
}

void SwLBoxString::Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                         const SvViewDataEntry* pView, const SvTreeListEntry& rEntry)
{
    SwGlblDocContent* pCont = static_cast<SwGlblDocContent*>(rEntry.GetUserData());
    if (pCont->GetType() == GLBLDOC_SECTION &&
      !pCont->GetSection()->IsConnectFlag())
    {
        rRenderContext.Push(PushFlags::FONT);
        vcl::Font aOldFont(rRenderContext.GetFont());
        vcl::Font aFont(rRenderContext.GetFont());
        aFont.SetColor(COL_LIGHTRED);
        rRenderContext.SetFont(aFont);
        rRenderContext.DrawText(rPos, GetText());
        rRenderContext.Pop();
    }
    else
        SvLBoxString::Paint(rPos, rDev, rRenderContext, pView, rEntry);
}

void    SwGlobalTree::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        Update(true);
    }
    SvTreeListBox::DataChanged( rDCEvt );
}

void SwGlobalTree::InsertRegion( const SwGlblDocContent* _pContent, const Sequence< OUString >& _rFiles )
{
    sal_Int32 nFiles = _rFiles.getLength();
    if ( !nFiles )
        return;

    bool bMove = false;
    if ( !_pContent )
    {
        SvTreeListEntry* pLast = LastVisible();
        _pContent = static_cast<SwGlblDocContent*>(pLast->GetUserData());
        bMove = true;
    }
    sal_uLong nEntryCount = GetEntryCount();
    const OUString* pFileNames = _rFiles.getConstArray();
    SwWrtShell& rSh = GetParentWindow()->GetCreateView()->GetWrtShell();
    rSh.StartAction();
    // after insertion of the first new content the 'pCont' parameter becomes invalid
    // find the index of the 'anchor' content to always use a current anchor content
    size_t nAnchorContent = m_pSwGlblDocContents->size() - 1;
    if ( !bMove )
    {
        for (size_t nContent = 0; nContent < m_pSwGlblDocContents->size();
                ++nContent)
        {
            if( *_pContent == *(*m_pSwGlblDocContents)[ nContent ] )
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
        SwGlblDocContent* pAnchorContent = nullptr;
        OSL_ENSURE(aTempContents.size() > (nAnchorContent + nFile), "invalid anchor content -> last insertion failed");
        if ( aTempContents.size() > (nAnchorContent + nFile) )
            pAnchorContent = aTempContents[nAnchorContent + nFile].get();
        else
            pAnchorContent = aTempContents.back().get();
        OUString sFileName(pFileNames[nFile]);
        INetURLObject aFileUrl;
        aFileUrl.SetSmartURL( sFileName );
        OUString sSectionName(aFileUrl.GetLastName(
            INetURLObject::DecodeMechanism::Unambiguous).getToken(0, sfx2::cTokenSeparator));
        sal_uInt16 nSectCount = rSh.GetSectionFormatCount();
        OUString sTempSectionName(sSectionName);
        sal_uInt16 nAddNumber = 0;
        sal_uInt16 nCount = 0;
        // if applicable: add index if the range name is already in use.
        while ( nCount < nSectCount )
        {
            const SwSectionFormat& rFormat = rSh.GetSectionFormat(nCount);
            if ((rFormat.GetSection()->GetSectionName() == sTempSectionName)
                && rFormat.IsInNodesArr())
            {
                nCount = 0;
                nAddNumber++;
                sTempSectionName = sSectionName + ":" + OUString::number( nAddNumber );
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
        aSectionData.SetLinkFilePassword( OUString() );

        rSh.InsertGlobalDocContent( *pAnchorContent, aSectionData );
    }
    if ( bMove )
    {
        Update( false );
        rSh.MoveGlobalDocContent(
            *m_pSwGlblDocContents, nEntryCount, nEntryCount + nFiles, nEntryCount - nFiles );
    }
    rSh.EndAction();
    Update( false );
    Display();

}

IMPL_LINK( SwGlobalTree, DialogClosedHdl, sfx2::FileDialogHelper*, _pFileDlg, void )
{
    if ( ERRCODE_NONE != _pFileDlg->GetError() )
        return;

    SfxMediumList aMedList(m_pDocInserter->CreateMediumList());
    if ( !aMedList.empty() )
    {
        Sequence< OUString >aFileNames( aMedList.size() );
        OUString* pFileNames = aFileNames.getArray();
        sal_Int32 nPos = 0;
        for (std::unique_ptr<SfxMedium>& pMed : aMedList)
        {
            OUString sFileName = pMed->GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE )
                + OUStringLiteral1(sfx2::cTokenSeparator)
                + pMed->GetFilter()->GetFilterName()
                + OUStringLiteral1(sfx2::cTokenSeparator);
            pFileNames[nPos++] = sFileName;
        }
        InsertRegion( m_pDocContent.get(), aFileNames );
        m_pDocContent.reset();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
