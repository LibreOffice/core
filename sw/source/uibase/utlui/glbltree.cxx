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

#include <o3tl/safeint.hxx>
#include <svl/stritem.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/help.hxx>
#include <sot/filelist.hxx>
#include <svl/eitem.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/settings.hxx>

#include <sfx2/docinsert.hxx>
#include <sfx2/filedlghelper.hxx>

#include <wrtsh.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <edglbldc.hxx>
#include <section.hxx>
#include <tox.hxx>
#include <navipi.hxx>
#include <edtwin.hxx>
#include <toxmgr.hxx>

#include <cmdid.h>
#include <helpids.h>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <swabstdlg.hxx>
#include <memory>

using namespace ::com::sun::star::uno;

#define GLOBAL_UPDATE_TIMEOUT 2000

const SfxObjectShell* SwGlobalTree::pShowShell = nullptr;

namespace {

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

}

void SwGlobalFrameListener_Impl::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    if( rHint.GetId() == SfxHintId::Dying)
        bValid = false;
}

namespace {

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

}

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

SwGlobalTree::SwGlobalTree(std::unique_ptr<weld::TreeView> xTreeView, SwNavigationPI* pDialog)
    : m_xTreeView(std::move(xTreeView))
    , m_xDialog(pDialog)
    , m_pActiveShell(nullptr)
#if 0
    , m_pEmphasisEntry(nullptr)
    , m_pDDSource(nullptr)
    , m_bIsInternalDrag(false)
    , m_bLastEntryEmphasis(false)
#endif
{
    Size aSize(m_xDialog->LogicToPixel(Size(110, 112), MapMode(MapUnit::MapAppFont)));;
    m_xTreeView->set_size_request(aSize.Width(), aSize.Height());

#if 0
    SetDragDropMode(DragDropMode::APP_COPY  |
                    DragDropMode::CTRL_MOVE |
                    DragDropMode::ENABLE_TOP );
#endif

    m_aUpdateTimer.SetTimeout(GLOBAL_UPDATE_TIMEOUT);
    m_aUpdateTimer.SetInvokeHandler(LINK(this, SwGlobalTree, Timeout));
    m_aUpdateTimer.Start();
    for (sal_uInt16 i = 0; i < GLOBAL_CONTEXT_COUNT; i++)
    {
        m_aContextStrings[i] = SwResId(GLOBAL_CONTEXT_ARY[i]);
    }
    m_xTreeView->set_help_id(HID_NAVIGATOR_GLOB_TREELIST);
    Select();
    m_xTreeView->connect_row_activated(LINK(this, SwGlobalTree, DoubleClickHdl));
    m_xTreeView->connect_changed(LINK(this, SwGlobalTree, SelectHdl));
    m_xTreeView->connect_focus_in(LINK(this, SwGlobalTree, FocusInHdl));
    m_xTreeView->connect_key_press(LINK(this, SwGlobalTree, KeyInputHdl));
    m_xTreeView->connect_popup_menu(LINK(this, SwGlobalTree, CommandHdl));
    m_xTreeView->connect_query_tooltip(LINK(this, SwGlobalTree, QueryTooltipHdl));
}

SwGlobalTree::~SwGlobalTree()
{
    m_pSwGlblDocContents.reset();
    m_pDocInserter.reset();
    m_aUpdateTimer.Stop();
    m_xDialog.clear();
}

#if 0
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
                                static_cast<int>(GetAbsPos(pDropEntry)):
                                    - 1;
            size_t nEntryCount = m_xTreeView->n_children();

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

#endif

IMPL_LINK(SwGlobalTree, CommandHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    bool bPop = false;
    if (m_pActiveShell && !m_pActiveShell->GetView().GetDocShell()->IsReadOnly())
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(m_xTreeView.get(), "modules/swriter/ui/mastercontextmenu.ui"));
        std::unique_ptr<weld::Menu> xPopup = xBuilder->weld_menu("navmenu");

        const MenuEnableFlags nEnableFlags = GetEnableFlags();

        xPopup->set_sensitive("updatesel", bool(nEnableFlags & MenuEnableFlags::UpdateSel));

        xPopup->set_sensitive("editlink", bool(nEnableFlags & MenuEnableFlags::EditLink));

        //disabling if applicable
        xPopup->set_sensitive("insertindex", bool(nEnableFlags & MenuEnableFlags::InsertIdx ));
        xPopup->set_sensitive("insertfile", bool(nEnableFlags & MenuEnableFlags::InsertFile));
        xPopup->set_sensitive("insertnewfile", bool(nEnableFlags & MenuEnableFlags::InsertFile));
        xPopup->set_sensitive("inserttext", bool(nEnableFlags & MenuEnableFlags::InsertText));

        xPopup->set_sensitive("update", bool(nEnableFlags & MenuEnableFlags::Update));
        xPopup->set_sensitive("insert", bool(nEnableFlags & MenuEnableFlags::InsertIdx));
        xPopup->set_sensitive("editcontent", bool(nEnableFlags & MenuEnableFlags::Edit));
        xPopup->set_sensitive("deleteentry", bool(nEnableFlags & MenuEnableFlags::Delete));

        OString sCommand = xPopup->popup_at_rect(m_xTreeView.get(), tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1,1)));
        if (!sCommand.isEmpty())
            ExecuteContextMenuAction(sCommand);
    }
    return bPop;
}

void SwGlobalTree::TbxMenuHdl(const OString& rCommand, weld::Menu& rMenu)
{
    const MenuEnableFlags nEnableFlags = GetEnableFlags();
    if (rCommand == "insert")
    {
        rMenu.set_sensitive("insertindex", bool(nEnableFlags & MenuEnableFlags::InsertIdx));
        rMenu.set_sensitive("insertfile", bool(nEnableFlags & MenuEnableFlags::InsertFile));
        rMenu.set_sensitive("insertnewfile", bool(nEnableFlags & MenuEnableFlags::InsertFile));
        rMenu.set_sensitive("inserttext", bool(nEnableFlags & MenuEnableFlags::InsertText));
    }
    else if (rCommand == "update")
    {
        rMenu.set_sensitive("updatesel", bool(nEnableFlags & MenuEnableFlags::UpdateSel));
    }
}

MenuEnableFlags SwGlobalTree::GetEnableFlags() const
{
    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
    bool bEntry = m_xTreeView->get_selected(xEntry.get());

    sal_uLong nSelCount = m_xTreeView->count_selected_rows();
    size_t nEntryCount = m_xTreeView->n_children();
    std::unique_ptr<weld::TreeIter> xPrevEntry;
    bool bPrevEntry = false;
    if (bEntry)
    {
        xPrevEntry = m_xTreeView->make_iterator(xEntry.get());
        bPrevEntry = m_xTreeView->iter_previous(*xPrevEntry);
    }

    MenuEnableFlags nRet = MenuEnableFlags::NONE;
    if(nSelCount == 1 || !nEntryCount)
        nRet |= MenuEnableFlags::InsertIdx|MenuEnableFlags::InsertFile;
    if(nSelCount == 1)
    {
        nRet |= MenuEnableFlags::Edit;
        if (bEntry && reinterpret_cast<SwGlblDocContent*>(m_xTreeView->get_id(*xEntry).toInt64())->GetType() != GLBLDOC_UNKNOWN &&
                    (!bPrevEntry || reinterpret_cast<SwGlblDocContent*>(m_xTreeView->get_id(*xPrevEntry).toInt64())->GetType() != GLBLDOC_UNKNOWN))
            nRet |= MenuEnableFlags::InsertText;
        if (bEntry && GLBLDOC_SECTION == reinterpret_cast<SwGlblDocContent*>(m_xTreeView->get_id(*xEntry).toInt64())->GetType())
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

IMPL_LINK(SwGlobalTree, QueryTooltipHdl, const weld::TreeIter&, rIter, OUString)
{
    OUString sEntry;

    const SwGlblDocContent* pCont = reinterpret_cast<const SwGlblDocContent*>(m_xTreeView->get_id(rIter).toInt64());
    if (pCont && GLBLDOC_SECTION == pCont->GetType())
    {
        const SwSection* pSect = pCont->GetSection();
        sEntry = pSect->GetLinkFileName().getToken(0, sfx2::cTokenSeparator);
        if (!pSect->IsConnectFlag())
            sEntry = m_aContextStrings[IDX_STR_BROKEN_LINK] + sEntry;
    }

    return sEntry;
}

IMPL_LINK_NOARG(SwGlobalTree, SelectHdl, weld::TreeView&, void)
{
    Select();
}

void SwGlobalTree::Select()
{
    int nSelCount = m_xTreeView->count_selected_rows();
    int nSel = m_xTreeView->get_selected_index();
    int nAbsPos = nSel != -1 ? nSel : 0;
    SwNavigationPI* pNavi = GetParentWindow();
    bool bReadonly = !m_pActiveShell ||
                m_pActiveShell->GetView().GetDocShell()->IsReadOnly();
    pNavi->m_xGlobalToolBox->set_item_sensitive("edit",  nSelCount == 1 && !bReadonly);
    pNavi->m_xGlobalToolBox->set_item_sensitive("insert",  nSelCount <= 1 && !bReadonly);
    pNavi->m_xGlobalToolBox->set_item_sensitive("update",  m_xTreeView->n_children() > 0 && !bReadonly);
    pNavi->m_xGlobalToolBox->set_item_sensitive("moveup",
                    nSelCount == 1 && nAbsPos && !bReadonly);
    pNavi->m_xGlobalToolBox->set_item_sensitive("movedown",
                    nSelCount == 1 && nAbsPos < m_xTreeView->n_children() - 1 && !bReadonly);

}

#if 0
DragDropMode SwGlobalTree::NotifyStartDrag( TransferDataContainer& ,
                                                SvTreeListEntry* pEntry )
{
    m_bIsInternalDrag = true;
    m_pDDSource = pEntry;
    return DragDropMode::CTRL_MOVE;
}

// TabPos: push to left
#define  GLBL_TABPOS_SUB 5

sal_IntPtr SwGlobalTree::GetTabPos( SvTreeListEntry*, SvLBoxTab* pTab)
{
    return pTab->GetPos() - GLBL_TABPOS_SUB;
}
#endif

#if 0
TriState SwGlobalTree::NotifyMoving(   SvTreeListEntry*  pTarget,
                                        SvTreeListEntry*  pSource,
                                        SvTreeListEntry*&,
                                        sal_uLong&
                                    )
{
    SvTreeList* _pModel = GetModel();
    sal_uLong nSource = GetAbsPos(pSource);
    sal_uLong nDest   = pTarget ? GetAbsPos(pTarget) : m_pSwGlblDocContents->size();

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
    if (1 == m_xTreeView->count_selected_rows())
        SvTreeListBox::StartDrag( nAction, rPt );
}

void SwGlobalTree::DragFinished( sal_Int8 nAction )
{
    SvTreeListBox::DragFinished( nAction );
    m_bIsInternalDrag = false;
}

#endif

IMPL_LINK_NOARG(SwGlobalTree, FocusInHdl, weld::Widget&, void)
{
    if (Update(false))
        Display();
}

IMPL_LINK(SwGlobalTree, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    bool bHandled = false;
    const vcl::KeyCode aCode = rKEvt.GetKeyCode();
    if (aCode.GetCode() == KEY_RETURN)
    {
        switch (aCode.GetModifier())
        {
            case KEY_MOD2:
                // Switch boxes
                GetParentWindow()->ToggleTree();
                bHandled = true;
            break;
        }
    }
    return bHandled;
}

void SwGlobalTree::Clear()
{
//TODO    m_pEmphasisEntry = nullptr;
    m_xTreeView->clear();
}

void SwGlobalTree::Display(bool bOnlyUpdateUserData)
{
    fprintf(stderr, "SwGlobalTree::Display %d\n", bOnlyUpdateUserData);

    size_t nCount = m_pSwGlblDocContents->size();
    size_t nChildren = m_xTreeView->n_children();
    if (bOnlyUpdateUserData && nChildren == m_pSwGlblDocContents->size())
    {
        std::unique_ptr<weld::TreeIter> xEntry = m_xTreeView->make_iterator();
        bool bEntry = m_xTreeView->get_iter_first(*xEntry);
        for (size_t i = 0; i < nCount && bEntry; i++)
        {
            const SwGlblDocContent* pCont = (*m_pSwGlblDocContents)[i].get();
            OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pCont)));
            m_xTreeView->set_id(*xEntry, sId);
            if (pCont->GetType() == GLBLDOC_SECTION && !pCont->GetSection()->IsConnectFlag())
                m_xTreeView->set_font_color(*xEntry, COL_LIGHTRED);
            bEntry = m_xTreeView->iter_next(*xEntry);
            assert(bEntry || i == nCount - 1);
        }
    }
    else
    {
        int nOldSelEntry = m_xTreeView->get_selected_index();
        OUString sEntryName;  // Name of the entry
        int nSelPos = -1;
        if (nOldSelEntry != -1)
        {
            sEntryName = m_xTreeView->get_text(nOldSelEntry);
            nSelPos = nOldSelEntry;
        }
        m_xTreeView->freeze();
        Clear();
        if (!m_pSwGlblDocContents)
            Update( false );

        int nSelEntry = -1;
        for (size_t i = 0; i < nCount; ++i)
        {
            const SwGlblDocContent* pCont = (*m_pSwGlblDocContents)[i].get();

            OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pCont)));
            OUString sEntry;
            OUString aImage;
            switch (pCont->GetType())
            {
                case GLBLDOC_UNKNOWN:
                    sEntry = m_aContextStrings[IDX_STR_INSERT_TEXT];
                break;
                case GLBLDOC_TOXBASE:
                {
                    const SwTOXBase* pBase = pCont->GetTOX();
                    sEntry = pBase->GetTitle();
                    aImage = RID_BMP_NAVI_INDEX;
                }
                break;
                case GLBLDOC_SECTION:
                {
                    const SwSection* pSect = pCont->GetSection();
                    sEntry = pSect->GetSectionName();
                    aImage = RID_BMP_DROP_REGION;
                }
                break;
            }

            m_xTreeView->append(sId, sEntry);
            if (!aImage.isEmpty())
                m_xTreeView->set_image(i, aImage);

            if (pCont->GetType() == GLBLDOC_SECTION && !pCont->GetSection()->IsConnectFlag())
                m_xTreeView->set_font_color(i, COL_LIGHTRED);

            if (sEntry == sEntryName)
                nSelEntry = i;
        }
        m_xTreeView->thaw();
        if (nSelEntry != -1)
            m_xTreeView->select(nSelEntry);
        else if (nSelPos != -1 && o3tl::make_unsigned(nSelPos) < nCount)
            m_xTreeView->select(nSelPos);
        else if (nCount)
            m_xTreeView->select(0);
        Select();
    }
}

void SwGlobalTree::InsertRegion( const SwGlblDocContent* pCont, const OUString* pFileName )
{
    Sequence< OUString > aFileNames;
    if ( !pFileName )
    {
        m_pDocInserter.reset(new ::sfx2::DocumentInserter(GetParentWindow()->GetFrameWeld(), "swriter", sfx2::DocumentInserter::Mode::InsertMulti));
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

void SwGlobalTree::EditContent(const SwGlblDocContent* pCont )
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

void SwGlobalTree::ExecuteContextMenuAction(const OString& rSelectedPopupEntry)
{
    bool bUpdateHard = false;

    int nEntry = m_xTreeView->get_selected_index();
    SwGlblDocContent* pCont = nEntry != -1 ? reinterpret_cast<SwGlblDocContent*>(m_xTreeView->get_id(nEntry).toInt64()) : nullptr;
    // If a RequestHelp is called during the dialogue,
    // then the content gets lost. Because of that a copy
    // is created in which only the DocPos is set correctly.
    std::unique_ptr<SwGlblDocContent> pContCopy;
    if(pCont)
        pContCopy.reset(new SwGlblDocContent(pCont->GetDocPos()));
    SfxDispatcher& rDispatch = *m_pActiveShell->GetView().GetViewFrame()->GetDispatcher();
    sal_uInt16 nSlot = 0;
    if (rSelectedPopupEntry == "updatesel")
    {
        // Two passes: first update the areas, then the directories.
        m_xTreeView->selected_foreach([this](weld::TreeIter& rSelEntry){
            SwGlblDocContent* pContent = reinterpret_cast<SwGlblDocContent*>(m_xTreeView->get_id(rSelEntry).toInt64());
            if (GLBLDOC_SECTION == pContent->GetType() &&
                pContent->GetSection()->IsConnected())
            {
                const_cast<SwSection*>(pContent->GetSection())->UpdateNow();
            }
            return false;
        });
        m_xTreeView->selected_foreach([this](weld::TreeIter& rSelEntry){
            SwGlblDocContent* pContent = reinterpret_cast<SwGlblDocContent*>(m_xTreeView->get_id(rSelEntry).toInt64());
            if (GLBLDOC_TOXBASE == pContent->GetType())
                m_pActiveShell->UpdateTableOf(*pContent->GetTOX());
            return false;
        });

        bUpdateHard = true;
    }
    else if (rSelectedPopupEntry == "updateindex")
    {
        nSlot = FN_UPDATE_TOX;
        bUpdateHard = true;
    }
    else if (rSelectedPopupEntry == "updatelinks" || rSelectedPopupEntry == "updateall")
    {
        m_pActiveShell->GetLinkManager().UpdateAllLinks(true, false, nullptr);
        if (rSelectedPopupEntry == "updateall")
            nSlot = FN_UPDATE_TOX;
        pCont = nullptr;
        bUpdateHard = true;
    }
    else if (rSelectedPopupEntry == "edit")
    {
        OSL_ENSURE(pCont, "edit without entry ? " );
        if (pCont)
        {
            EditContent(pCont);
        }
    }
    else if (rSelectedPopupEntry == "editlink")
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
    else if (rSelectedPopupEntry == "deleteentry")
    {
        // If several entries selected, then after each delete the array
        // must be refilled. So you do not have to remember anything,
        // deleting begins at the end.
        std::vector<int> aRows = m_xTreeView->get_selected_rows();
        std::sort(aRows.begin(), aRows.end());

        std::unique_ptr<SwGlblDocContents> pTempContents;
        m_pActiveShell->StartAction();
        for (auto iter = aRows.rbegin(); iter != aRows.rend(); ++iter)
        {
            m_pActiveShell->DeleteGlobalDocContent(
                pTempContents ? *pTempContents : *m_pSwGlblDocContents,
                                 *iter);
            pTempContents.reset(new SwGlblDocContents);
            m_pActiveShell->GetGlobalDocContent(*pTempContents);
        }
        pTempContents.reset();
        m_pActiveShell->EndAction();
        pCont = nullptr;
    }
    else if (rSelectedPopupEntry == "insertindex")
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
                                                    m_xDialog->GetFrameWeld(), aSet,
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
    else if (rSelectedPopupEntry == "insertfile")
    {
        m_pDocContent = std::move(pContCopy);
        InsertRegion( m_pDocContent.get() );
        pCont = nullptr;
    }
    else if (rSelectedPopupEntry == "insertnewfile")
    {
        SfxViewFrame* pGlobFrame = m_pActiveShell->GetView().GetViewFrame();
        SwGlobalFrameListener_Impl aFrameListener(*pGlobFrame);

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
                if (nEntry != -1)
                {
                    Update( false );
                    Display();
                    m_xTreeView->select(nEntry);
                    Select();
                    nEntry = m_xTreeView->get_selected_index();
                    pCont = nEntry != -1 ? reinterpret_cast<SwGlblDocContent*>(m_xTreeView->get_id(nEntry).toInt64()) : nullptr;
                }
                else
                {
                    nEntry = -1;
                    pCont = nullptr;
                }
                if(pBool->GetValue())
                {
                    InsertRegion(pCont, &sNewFile);
                    pViewFrame->ToTop();
                }
                else
                    pViewFrame->GetDispatcher()->Execute(SID_CLOSEWIN, SfxCallMode::SYNCHRON);
            }
            else
            {
                pViewFrame->ToTop();
                return;
            }
        }
    }
    else if (rSelectedPopupEntry == "inserttext")
    {
        if (pCont)
            m_pActiveShell->InsertGlobalDocContent(*pCont);
        else
        {
            m_pActiveShell->SplitNode(); // Empty document
            m_pActiveShell->Up( false );
        }
        m_pActiveShell->GetView().GetEditWin().GrabFocus();
    }
    else if (rSelectedPopupEntry == "update")
        pCont = nullptr;

    if (pCont)
        GotoContent(pCont);
    if (nSlot)
        rDispatch.Execute(nSlot);
    if (Update(bUpdateHard))
        Display();
}

IMPL_LINK_NOARG(SwGlobalTree, Timeout, Timer *, void)
{
    if (!m_xTreeView->has_focus() && Update(false))
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

void SwGlobalTree::ShowTree()
{
    m_aUpdateTimer.Start();
    m_xTreeView->show();
}

void SwGlobalTree::HideTree()
{
    m_aUpdateTimer.Stop();
    m_xTreeView->hide();
}

void SwGlobalTree::ExecCommand(const OString &rCmd)
{
    int nEntry = m_xTreeView->get_selected_index();
    if (nEntry == -1)
        return;
    if (rCmd == "edit")
    {
        const SwGlblDocContent* pCont = reinterpret_cast<const SwGlblDocContent*>(
                                                m_xTreeView->get_id(nEntry).toInt64());
        EditContent(pCont);
    }
    else
    {
        if (m_xTreeView->count_selected_rows() == 1)
        {
            bool bMove = false;
            sal_uLong nSource = nEntry;
            sal_uLong nDest = nSource;
            if (rCmd == "movedown")
            {
                size_t nEntryCount = m_xTreeView->n_children();
                bMove = nEntryCount > nSource + 1;
                nDest+= 2;
            }
            else if (rCmd == "moveup")
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

bool SwGlobalTree::Update(bool bHard)
{
    SwView* pActView = GetParentWindow()->GetCreateView();
    bool bRet = false;
    if (pActView && pActView->GetWrtShellPtr())
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
            size_t nChildren = m_xTreeView->n_children();
            if (pTempContents->size() != m_pSwGlblDocContents->size() ||
                    pTempContents->size() != nChildren)
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
                    OUString sTemp = m_xTreeView->get_text(i);
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
                        bCopy = true;
                    }
                }
            }
            if (bCopy || bHard)
            {
                *m_pSwGlblDocContents = std::move( *pTempContents );
                bRet = true;
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
            Application::PostUserEvent(LINK(this, SwGlobalTree, ShowFrameHdl));
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

IMPL_LINK_NOARG( SwGlobalTree, DoubleClickHdl, weld::TreeView&, bool)
{
    int nEntry = m_xTreeView->get_cursor_index();
    SwGlblDocContent* pCont = nEntry != -1 ? reinterpret_cast<SwGlblDocContent*>(m_xTreeView->get_id(nEntry).toInt64()) : nullptr;
    if (pCont->GetType() == GLBLDOC_SECTION)
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

void SwGlobalTree::InsertRegion( const SwGlblDocContent* _pContent, const Sequence< OUString >& _rFiles )
{
    sal_Int32 nFiles = _rFiles.getLength();
    if (!nFiles)
        return;

    size_t nEntryCount = m_xTreeView->n_children();

    bool bMove = _pContent == nullptr;
    const OUString* pFileNames = _rFiles.getConstArray();
    SwWrtShell& rSh = GetParentWindow()->GetCreateView()->GetWrtShell();
    rSh.StartAction();
    // after insertion of the first new content the 'pCont' parameter becomes invalid
    // find the index of the 'anchor' content to always use a current anchor content
    size_t nAnchorContent = m_pSwGlblDocContents->size() - 1;
    if (!bMove)
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

        SwSectionData aSectionData(SectionType::Content, sSectionName);
        aSectionData.SetProtectFlag(true);
        aSectionData.SetHidden(false);

        aSectionData.SetLinkFileName(sFileName);
        aSectionData.SetType(SectionType::FileLink);
        aSectionData.SetLinkFilePassword( OUString() );

        rSh.InsertGlobalDocContent( *pAnchorContent, aSectionData );
    }
    if (bMove)
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
        for (const std::unique_ptr<SfxMedium>& pMed : aMedList)
        {
            OUString sFileName = pMed->GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE )
                + OUStringChar(sfx2::cTokenSeparator)
                + pMed->GetFilter()->GetFilterName()
                + OUStringChar(sfx2::cTokenSeparator);
            pFileNames[nPos++] = sFileName;
        }
        InsertRegion( m_pDocContent.get(), aFileNames );
        m_pDocContent.reset();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
