/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <o3tl/string_view.hxx>
#include <svl/stritem.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <sot/filelist.hxx>
#include <svl/eitem.hxx>
#include <vcl/graphicfilter.hxx>
#include <osl/diagnose.h>

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

#include <sfx2/event.hxx>
#include <unotxvw.hxx>
#include <tools/json_writer.hxx>
#include <COKit/COKitTypes.hxx>
#include <sfx2/cokitfilepicker.hxx>
#include <sfx2/kit/helper.hxx>
#include <comphelper/scopeguard.hxx>
#include <osl/file.hxx>
#include <doc.hxx>
#include <iodetect.hxx>
#include <numrule.hxx>
#include <IDocumentLinksAdministration.hxx>

using namespace ::cpo::uno;
using namespace ::cpo::uno;

#define GLOBAL_UPDATE_TIMEOUT 2000


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

const TranslateId GLOBAL_CONTEXT_ARY[] =
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
    , m_aDropTargetHelper(*this)
    , m_pDialog(pDialog)
    , m_aUpdateTimer("SwGlobalTree m_aUpdateTimer")
    , m_aConnectChangedIdle("SwGlobalTree m_aConnectChangedIdle")
    , m_pActiveShell(nullptr)
{
    m_xTreeView->set_size_request(m_xTreeView->get_approximate_digit_width() * 30,
                                  m_xTreeView->get_text_height() * 14);

    m_aUpdateTimer.SetTimeout(GLOBAL_UPDATE_TIMEOUT);
    m_aUpdateTimer.SetInvokeHandler(LINK(this, SwGlobalTree, Timeout));
    m_aUpdateTimer.Start();
    m_aConnectChangedIdle.SetInvokeHandler(LINK(this, SwGlobalTree, ConnectChangedHdl));
    for (sal_uInt16 i = 0; i < GLOBAL_CONTEXT_COUNT; i++)
    {
        m_aContextStrings[i] = SwResId(GLOBAL_CONTEXT_ARY[i]);
    }
    m_xTreeView->set_help_id(HID_NAVIGATOR_GLOB_TREELIST);
    Select();
    m_xTreeView->connect_row_activated(LINK(this, SwGlobalTree, DoubleClickHdl));
    m_xTreeView->connect_selection_changed(LINK(this, SwGlobalTree, SelectHdl));
    m_xTreeView->connect_focus_in(LINK(this, SwGlobalTree, FocusInHdl));
    m_xTreeView->connect_key_press(LINK(this, SwGlobalTree, KeyInputHdl));
    m_xTreeView->connect_popup_menu(LINK(this, SwGlobalTree, CommandHdl));
    m_xTreeView->connect_query_tooltip(LINK(this, SwGlobalTree, QueryTooltipHdl));
}

SwGlobalTree::~SwGlobalTree()
{
    m_xAlive.reset();
    m_pSwGlblDocContents.reset();
    m_pDocInserter.reset();
    m_aUpdateTimer.Stop();
    m_aConnectChangedIdle.Stop();
}

SwGlobalTreeDropTarget::SwGlobalTreeDropTarget(SwGlobalTree& rTreeView)
    : DropTargetHelper(rTreeView.get_widget().get_drop_target())
    , m_rTreeView(rTreeView)
{
}

sal_Int8 SwGlobalTreeDropTarget::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    weld::TreeView& rWidget = m_rTreeView.get_widget();
    std::unique_ptr<weld::TreeIter> xDropEntry(rWidget.make_iterator());
    if (!rWidget.get_dest_row_at_pos(rEvt.maPosPixel, xDropEntry.get(), true))
        xDropEntry.reset();

    if (rWidget.get_drag_source() == &rWidget)  // internal drag
        m_rTreeView.MoveSelectionTo(xDropEntry.get());
    else
    {
        TransferableDataHelper aData( rEvt.maDropEvent.Transferable );

        OUString sFileName;
        const SwGlblDocContent* pCnt = xDropEntry ?
                    weld::fromId<const SwGlblDocContent*>(rWidget.get_id(*xDropEntry)) :
                            nullptr;
        if( aData.HasFormat( SotClipboardFormatId::FILE_LIST ))
        {
            nRet = rEvt.mnAction;
            SwGlblDocContents aTempContents;
            int nAbsContPos = xDropEntry ?
                                rWidget.get_iter_index_in_parent(*xDropEntry):
                                    - 1;
            size_t nEntryCount = rWidget.n_children();

            // Get data
            FileList aFileList;
            aData.GetFileList( SotClipboardFormatId::FILE_LIST, aFileList );
            size_t n = aFileList.Count();
            while (n)
            {
                --n;
                sFileName = aFileList.GetFile(n);
                m_rTreeView.InsertRegion(pCnt, &sFileName);
                // The list of contents must be newly fetched after inserting,
                // to not work on an old content.
                if(n)
                {
                    if (const SwWrtShell* pSh = m_rTreeView.GetActiveWrtShell())
                    {
                        pSh->GetGlobalDocContent(aTempContents);
                        // If the file was successfully inserted,
                        // then the next content must also be fetched.
                        if(nEntryCount < aTempContents.size())
                        {
                            nEntryCount++;
                            nAbsContPos++;
                            pCnt = aTempContents[ nAbsContPos ].get();
                        }
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
                m_rTreeView.InsertRegion(pCnt, &sFileName);
            }
        }
    }
    return nRet;
}

sal_Int8 SwGlobalTreeDropTarget::AcceptDrop( const AcceptDropEvent& rEvt )
{
    // to enable the autoscroll when we're close to the edges
    weld::TreeView& rWidget = m_rTreeView.get_widget();
    rWidget.get_dest_row_at_pos(rEvt.maPosPixel, nullptr, true);

    sal_Int8 nRet = rEvt.mnAction;

    if (rWidget.get_drag_source() == &rWidget)  // internal drag
        return nRet;

    if (IsDropFormatSupported( SotClipboardFormatId::SIMPLE_FILE) ||
        IsDropFormatSupported( SotClipboardFormatId::STRING) ||
        IsDropFormatSupported( SotClipboardFormatId::FILE_LIST) ||
        IsDropFormatSupported( SotClipboardFormatId::SOLK) ||
        IsDropFormatSupported( SotClipboardFormatId::NETSCAPE_BOOKMARK )||
        IsDropFormatSupported( SotClipboardFormatId::FILECONTENT) ||
        IsDropFormatSupported( SotClipboardFormatId::FILEGRPDESCRIPTOR) ||
        IsDropFormatSupported( SotClipboardFormatId::UNIFORMRESOURCELOCATOR) ||
        IsDropFormatSupported( SotClipboardFormatId::FILENAME))
    {
        nRet = DND_ACTION_LINK;
    }

    return nRet;
}

IMPL_LINK(SwGlobalTree, CommandHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    bool bPop = false;
    if (m_pActiveShell && !m_pActiveShell->GetView().GetDocShell()->IsReadOnly())
    {
        // tdf#166902 ensure right clicked entry is selected
        if (std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
                rCEvt.IsMouseEvent() && m_xTreeView->get_dest_row_at_pos(
                    rCEvt.GetMousePosPixel(), xEntry.get(), false))
        {
            // if clicked entry is not currently selected then clear selections and select it
            if (!m_xTreeView->is_selected(*xEntry))
                m_xTreeView->set_cursor(*xEntry);
        }

        m_xContextSubMenu.reset();
        m_xContextMenu.reset();
        m_xContextMenuBuilder = Application::CreateBuilder(m_xTreeView.get(), u"modules/swriter/ui/mastercontextmenu.ui"_ustr);
        m_xContextMenu = m_xContextMenuBuilder->weld_menu(u"navmenu"_ustr);
        m_xContextSubMenu = m_xContextMenuBuilder->weld_menu(u"insertmenu"_ustr);

        const MenuEnableFlags nEnableFlags = GetEnableFlags();

        m_xContextMenu->set_sensitive(u"updatesel"_ustr, bool(nEnableFlags & MenuEnableFlags::UpdateSel));

        m_xContextMenu->set_sensitive(u"editlink"_ustr, bool(nEnableFlags & MenuEnableFlags::EditLink));

        //disabling if applicable
        m_xContextSubMenu->set_sensitive(u"insertindex"_ustr, bool(nEnableFlags & MenuEnableFlags::InsertIdx ));
        m_xContextSubMenu->set_sensitive(u"insertfile"_ustr, bool(nEnableFlags & MenuEnableFlags::InsertFile));
        m_xContextSubMenu->set_sensitive(u"insertnewfile"_ustr, bool(nEnableFlags & MenuEnableFlags::InsertFile));
        m_xContextSubMenu->set_sensitive(u"inserttext"_ustr, bool(nEnableFlags & MenuEnableFlags::InsertText));
        HideInsertFileEntries(*m_xContextSubMenu);

        m_xContextMenu->set_sensitive(u"update"_ustr, bool(nEnableFlags & MenuEnableFlags::Update));
        m_xContextMenu->set_sensitive(u"insert"_ustr, bool(nEnableFlags & MenuEnableFlags::InsertIdx));
        m_xContextMenu->set_sensitive(u"editcontent"_ustr, bool(nEnableFlags & MenuEnableFlags::Edit));
        m_xContextMenu->set_sensitive(u"deleteentry"_ustr, bool(nEnableFlags & MenuEnableFlags::Delete));

        // The choice may arrive after the menu was shown. A choice in the submenu is reported by
        // the menu too.
        m_xContextMenu->connect_activate(LINK(this, SwGlobalTree, ContextMenuSelectHdl));
        m_xContextMenu->popup_at_rect(m_xTreeView.get(), tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1,1)));

        bPop = true;
    }
    return bPop;
}

IMPL_LINK(SwGlobalTree, ContextMenuSelectHdl, const OUString&, rIdent, void)
{
    if (!rIdent.isEmpty())
        ExecuteContextMenuAction(rIdent);
}

void SwGlobalTree::HideInsertFileEntries(weld::Menu& rMenu)
{
    // An inserted file is linked to, so it has to stay where it is picked: a picker that gives a
    // copy, as an upload to a server is, offers nothing to link to, unless the host also says
    // where the file stays.
    rMenu.set_visible(u"insertfile"_ustr, sfx2::COKitFilePicker::picksFilesInPlace()
                                              || sfx2::COKitFilePicker::canPickLinkTarget());
    // A new document is linked to as well, so it is created where the user chooses.
    rMenu.set_visible(u"insertnewfile"_ustr, sfx2::COKitFilePicker::canPickSaveLocation());
}

void SwGlobalTree::TbxMenuHdl(std::u16string_view rCommand, weld::Menu& rMenu)
{
    const MenuEnableFlags nEnableFlags = GetEnableFlags();
    if (rCommand == u"insert")
    {
        rMenu.set_sensitive(u"insertindex"_ustr, bool(nEnableFlags & MenuEnableFlags::InsertIdx));
        rMenu.set_sensitive(u"insertfile"_ustr, bool(nEnableFlags & MenuEnableFlags::InsertFile));
        rMenu.set_sensitive(u"insertnewfile"_ustr, bool(nEnableFlags & MenuEnableFlags::InsertFile));
        rMenu.set_sensitive(u"inserttext"_ustr, bool(nEnableFlags & MenuEnableFlags::InsertText));
        HideInsertFileEntries(rMenu);
    }
    else if (rCommand == u"update")
    {
        rMenu.set_sensitive(u"updatesel"_ustr, bool(nEnableFlags & MenuEnableFlags::UpdateSel));
    }
}

MenuEnableFlags SwGlobalTree::GetEnableFlags() const
{
    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
    bool bEntry = m_xTreeView->get_selected(xEntry.get());

    int nSelCount = m_xTreeView->count_selected_rows();
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
        if (bEntry && weld::fromId<SwGlblDocContent*>(m_xTreeView->get_id(*xEntry))->GetType() != GLBLDOC_UNKNOWN &&
                    (!bPrevEntry || weld::fromId<SwGlblDocContent*>(m_xTreeView->get_id(*xPrevEntry))->GetType() != GLBLDOC_UNKNOWN))
            nRet |= MenuEnableFlags::InsertText;
        if (bEntry && GLBLDOC_SECTION == weld::fromId<SwGlblDocContent*>(m_xTreeView->get_id(*xEntry))->GetType())
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

    const SwGlblDocContent* pCont = weld::fromId<const SwGlblDocContent*>(m_xTreeView->get_id(rIter));
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
    pNavi->m_xGlobalToolBox->set_item_sensitive(u"edit"_ustr,  nSelCount == 1 && !bReadonly);
    pNavi->m_xGlobalToolBox->set_item_sensitive(u"insert"_ustr,  nSelCount <= 1 && !bReadonly);
    pNavi->m_xGlobalToolBox->set_item_sensitive(u"update"_ustr,  m_xTreeView->n_children() > 0 && !bReadonly);
    pNavi->m_xGlobalToolBox->set_item_sensitive(u"moveup"_ustr,
                    nSelCount == 1 && nAbsPos && !bReadonly);
    pNavi->m_xGlobalToolBox->set_item_sensitive(u"movedown"_ustr,
                    nSelCount == 1 && nAbsPos < m_xTreeView->n_children() - 1 && !bReadonly);

}

void SwGlobalTree::MoveSelectionTo(const weld::TreeIter* pDropEntry)
{
    int nSource = m_xTreeView->get_selected_index();

    int nDest = pDropEntry ? m_xTreeView->get_iter_index_in_parent(*pDropEntry)
                           : m_pSwGlblDocContents->size();

    if (m_pActiveShell->MoveGlobalDocContent(
            *m_pSwGlblDocContents, nSource, nSource + 1, nDest ) &&
            Update( false ))
        Display();
}

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

void SwGlobalTree::Display(bool bOnlyUpdateUserData)
{
    size_t nCount = m_pSwGlblDocContents->size();
    size_t nChildren = m_xTreeView->n_children();
    m_aShownBroken.assign(nCount, false);
    for (size_t i = 0; i < nCount; ++i)
    {
        const SwGlblDocContent* pCont = (*m_pSwGlblDocContents)[i].get();
        m_aShownBroken[i]
            = pCont->GetType() == GLBLDOC_SECTION && !pCont->GetSection()->IsConnectFlag();
    }
    if (bOnlyUpdateUserData && nChildren == m_pSwGlblDocContents->size())
    {
        std::unique_ptr<weld::TreeIter> xEntry = m_xTreeView->make_iterator();
        bool bEntry = m_xTreeView->get_iter_first(*xEntry);
        for (size_t i = 0; i < nCount && bEntry; i++)
        {
            const SwGlblDocContent* pCont = (*m_pSwGlblDocContents)[i].get();
            OUString sId(weld::toId(pCont));
            m_xTreeView->set_id(*xEntry, sId);
            if (pCont->GetType() == GLBLDOC_SECTION && !pCont->GetSection()->IsConnectFlag())
                m_xTreeView->set_font_color(*xEntry, COL_LIGHTRED);
            else
                m_xTreeView->set_font_color(*xEntry, COL_AUTO);
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
        m_xTreeView->clear();

        int nSelEntry = -1;
        for (size_t i = 0; i < nCount; ++i)
        {
            const SwGlblDocContent* pCont = (*m_pSwGlblDocContents)[i].get();

            OUString sId(weld::toId(pCont));
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
                    sEntry = pSect->GetSectionName().toString();
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
        SwNavigationPI* pNavi = GetParentWindow();
        m_pDocInserter.reset(new ::sfx2::DocumentInserter(pNavi->GetFrameWeld(), u"swriter"_ustr, sfx2::DocumentInserter::Mode::InsertMulti));
        m_pDocInserter->StartExecuteModal( LINK( this, SwGlobalTree, DialogClosedHdl ) );
    }
    else if ( !pFileName->isEmpty() )
    {
        aFileNames.realloc(1);
        INetURLObject aFileName;
        aFileName.SetSmartURL( *pFileName );
        // tdf#127978 - don't URL encode filename for navigator's tooltip
        aFileNames.getArray()[0]
            = aFileName.GetMainURL(INetURLObject::DecodeMechanism::WithCharset);
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
        m_pActiveShell->GetView().GetViewFrame().GetDispatcher()->Execute(nSlot);
        if(Update( false ))
            Display();
    }
}

void SwGlobalTree::ExecuteContextMenuAction(std::u16string_view rSelectedPopupEntry)
{
    bool bUpdateHard = false;

    int nEntry = m_xTreeView->get_selected_index();
    SwGlblDocContent* pCont = nEntry != -1 ? weld::fromId<SwGlblDocContent*>(m_xTreeView->get_id(nEntry)) : nullptr;
    // If a RequestHelp is called during the dialogue,
    // then the content gets lost. Because of that a copy
    // is created in which only the DocPos is set correctly.
    std::optional<SwGlblDocContent> oContCopy;
    if(pCont)
        oContCopy.emplace(pCont->GetDocPos());
    SfxDispatcher& rDispatch = *m_pActiveShell->GetView().GetViewFrame().GetDispatcher();
    sal_uInt16 nSlot = 0;
    if (rSelectedPopupEntry == u"updatesel")
    {
        // asking for the update is the consent to updating the links of the document; only these
        // sections are updated now
        m_pActiveShell->GetView().GetDocShell()->getEmbeddedObjectContainer().setUserAllowsLinkUpdate(
            true);
        // Two passes: first update the areas, then the directories.
        std::vector<OUString> aUpdatedFiles;
        m_xTreeView->selected_foreach([this, &aUpdatedFiles](weld::TreeIter& rSelEntry){
            SwGlblDocContent* pContent = weld::fromId<SwGlblDocContent*>(m_xTreeView->get_id(rSelEntry));
            if (GLBLDOC_SECTION == pContent->GetType() &&
                pContent->GetSection()->IsConnected())
            {
                const_cast<SwSection*>(pContent->GetSection())->UpdateNow();
                aUpdatedFiles.push_back(LinkedFileId(*pContent->GetSection()));
            }
            return false;
        });
        m_xTreeView->selected_foreach([this](weld::TreeIter& rSelEntry){
            SwGlblDocContent* pContent = weld::fromId<SwGlblDocContent*>(m_xTreeView->get_id(rSelEntry));
            if (GLBLDOC_TOXBASE == pContent->GetType())
                m_pActiveShell->UpdateTableOf(*pContent->GetTOX());
            return false;
        });

        bUpdateHard = true;
        if (!aUpdatedFiles.empty())
            RequestFreshLinkedFiles(aUpdatedFiles);
    }
    else if (rSelectedPopupEntry == u"updateindex")
    {
        nSlot = FN_UPDATE_TOX;
        bUpdateHard = true;
    }
    else if (rSelectedPopupEntry == u"updatelinks" || rSelectedPopupEntry == u"updateall")
    {
        // asking for the update is the consent to it
        m_pActiveShell->GetView().GetDocShell()->AllowLinkUpdate();
        RequestFreshLinkedFiles();
        if (rSelectedPopupEntry == u"updateall")
            nSlot = FN_UPDATE_TOX;
        pCont = nullptr;
        bUpdateHard = true;
    }
    else if (rSelectedPopupEntry == u"editcontent")
    {
        OSL_ENSURE(pCont, "edit without entry ? " );
        if (pCont)
        {
            EditContent(pCont);
        }
    }
    else if (rSelectedPopupEntry == u"editlink")
    {
        OSL_ENSURE(pCont, "edit without entry ? " );
        if (pCont)
        {
            SfxStringItem aName(FN_EDIT_REGION,
                    pCont->GetSection()->GetSectionName().toString());
            rDispatch.ExecuteList(FN_EDIT_REGION, SfxCallMode::ASYNCHRON,
                    { &aName });
        }
    }
    else if (rSelectedPopupEntry == u"deleteentry")
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
    else if (rSelectedPopupEntry == u"insertindex")
    {
        if(oContCopy)
        {
            SfxItemSetFixed<
                    RES_FRM_SIZE, RES_FRM_SIZE,
                    RES_LR_SPACE, RES_LR_SPACE,
                    RES_BACKGROUND, RES_BACKGROUND,
                    RES_COL, RES_COL,
                    SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE,
                    FN_PARAM_TOX_TYPE, FN_PARAM_TOX_TYPE>
                aSet( m_pActiveShell->GetView().GetPool() );

            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            ScopedVclPtr<AbstractMultiTOXTabDialog> pDlg(pFact->CreateMultiTOXTabDialog(
                                                    m_xTreeView.get(), aSet,
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
                    m_pActiveShell->InsertGlobalDocContent( *oContCopy, *pToInsert );
            }
            pCont = nullptr;
        }
    }
    else if (rSelectedPopupEntry == u"insertfile")
    {
        if (!sfx2::COKitFilePicker::picksFilesInPlace()
            && sfx2::COKitFilePicker::canPickLinkTarget())
            InsertLinkTarget(std::move(oContCopy));
        else
        {
            m_oDocContent = std::move(oContCopy);
            InsertRegion(m_oDocContent ? &*m_oDocContent : nullptr);
        }
        pCont = nullptr;
    }
    else if (rSelectedPopupEntry == u"insertnewfile")
    {
        InsertNewDocument(std::move(oContCopy));
        pCont = nullptr;
    }
    else if (rSelectedPopupEntry == u"inserttext")
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
    else if (rSelectedPopupEntry == u"update")
        pCont = nullptr;

    if (pCont)
        GotoContent(pCont);
    if (nSlot)
        rDispatch.Execute(nSlot);
    if (Update(bUpdateHard))
        Display();
}

// A linked file was found missing, or read again, while the document may still be in the middle
// of updating its links, so the list is redrawn once that is done, wherever the focus is.
IMPL_LINK_NOARG(SwGlobalTree, ConnectChangedHdl, Timer*, void)
{
    if (Update(false))
        Display();
}

IMPL_LINK_NOARG(SwGlobalTree, Timeout, Timer *, void)
{
    SwView* pView = GetParentWindow()->GetCreateView();
    if (pView && pView->GetEditWin().HasFocus())
    {
        if (Update(false))
            Display();
        UpdateTracking();
    }
}

// track GlobalDocContentType at the cursor position in the document
void SwGlobalTree::UpdateTracking()
{
    if (!m_pActiveShell)
        return;

    m_xTreeView->unselect_all();

    const SwSection* pActiveShellCurrSection = m_pActiveShell->GetCurrSection();
    if (pActiveShellCurrSection)
    {
        const SwSection* pSection = pActiveShellCurrSection;
        while (SwSection* pParent = pSection->GetParent())
            pSection = pParent;
        for (const std::unique_ptr<SwGlblDocContent>& rGlblDocContent : *m_pSwGlblDocContents)
        {
            if (rGlblDocContent->GetType() == GlobalDocContentType::GLBLDOC_UNKNOWN)
                continue;
            if ((pSection->GetType() == SectionType::ToxContent
                 && rGlblDocContent->GetTOX() == pSection->GetTOXBase())
                    || (pSection->GetType() != SectionType::ToxContent
                        && rGlblDocContent->GetSection() == pSection))
            {
                const OUString aId(weld::toId(rGlblDocContent.get()));
                m_xTreeView->select(m_xTreeView->find_id(aId));
                break;
            }
        }
    }
    else
    {
        const SwCursor* pCursor = m_pActiveShell->GetCursor();
        const SwNode& rNode = pCursor->GetPoint()->GetNode();
        if (rNode.IsTextNode())
        {
            // only the first text node in each series of text nodes is stored in the
            // SwGlblDocContents array
            SwNodeIndex aIdx(rNode);
            do
            {
                --aIdx;
            } while (aIdx.GetNode().IsTextNode());
            ++aIdx;
            SwNodeOffset aTextNodeIndex(aIdx.GetNode().GetIndex());
            for (const std::unique_ptr<SwGlblDocContent>& rGlblDocContent : *m_pSwGlblDocContents)
            {
                if (rGlblDocContent->GetType() == GlobalDocContentType::GLBLDOC_UNKNOWN
                        && rGlblDocContent->GetDocPos() == aTextNodeIndex)
                {
                    const OUString aId(weld::toId(rGlblDocContent.get()));
                    m_xTreeView->select(m_xTreeView->find_id(aId));
                }
            }
        }
    }

    Select();
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
            const UIName sName = pCont->GetTOX()->GetTOXName();
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
    UpdateTracking();
}

void SwGlobalTree::HideTree()
{
    m_aUpdateTimer.Stop();
    m_xTreeView->hide();
}

void SwGlobalTree::ExecCommand(std::u16string_view rCmd)
{
    int nEntry = m_xTreeView->get_selected_index();
    if (nEntry == -1)
        return;
    if (rCmd == u"edit")
    {
        const SwGlblDocContent* pCont = weld::fromId<const SwGlblDocContent*>(
                                                m_xTreeView->get_id(nEntry));
        EditContent(pCont);
    }
    else
    {
        if (m_xTreeView->count_selected_rows() == 1)
        {
            bool bMove = false;
            int nSource = nEntry;
            int nDest = nSource;
            if (rCmd == u"movedown")
            {
                int nEntryCount = m_xTreeView->n_children();
                bMove = nEntryCount > nSource + 1;
                nDest+= 2;
            }
            else if (rCmd == u"moveup")
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
            if (!IsListening(*m_pActiveShell->GetView().GetDocShell()))
                StartListening(*m_pActiveShell->GetView().GetDocShell());
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
            SwGlblDocContents aTempContents;
            m_pActiveShell->GetGlobalDocContent(aTempContents);
            size_t nChildren = m_xTreeView->n_children();
            if (aTempContents.size() != m_pSwGlblDocContents->size() ||
                    aTempContents.size() != nChildren)
            {
                bRet = true;
                bCopy = true;
            }
            else
            {
                for(size_t i = 0; i < aTempContents.size() && !bCopy; i++)
                {
                    SwGlblDocContent* pLeft = aTempContents[i].get();
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
                         ) ||
                         // a linked file found missing, or found again
                         (
                           eType == GLBLDOC_SECTION && i < m_aShownBroken.size() &&
                           m_aShownBroken[i] == pLeft->GetSection()->IsConnectFlag()
                         )
                       )
                    {
                        bCopy = true;
                    }
                }
            }
            if (bCopy || bHard)
            {
                *m_pSwGlblDocContents = std::move( aTempContents );
                bRet = true;
            }
        }
    }
    else
    {
        m_xTreeView->clear();
        if(m_pSwGlblDocContents)
            m_pSwGlblDocContents->clear();
    }
    // FIXME: Implement a test for changes!
    return bRet;
}

void SwGlobalTree::RequestFreshLinkedFiles(const std::vector<OUString>& rIds)
{
    tools::JsonWriter aJson;
    if (!rIds.empty())
    {
        tools::JsonWriter::ScopedJsonWriterNode<']'> aIds = aJson.startArray("ids");
        for (const OUString& rId : rIds)
            aJson.putSimpleValue(rId);
    }
    m_pActiveShell->GetView().viewCallback(COKitCallbackType::REFRESH_LINKED_FILES,
                                           aJson.finishAndGetAsOString());
}

OUString SwGlobalTree::LinkedFileId(const SwSection& rSection)
{
    // The host knows the file by its path relative to this document, as the document links to it.
    return m_pActiveShell->GetView().GetDocShell()->GetLinkedFileId(
        rSection.GetLinkFileName().getToken(0, sfx2::cTokenSeparator));
}

void SwGlobalTree::OpenDoc(const SwGlblDocContent* pCont)
{
    // The document is opened by the host, which knows where to show it and how to reach the file.
    const OUString sFileName(pCont->GetSection()->GetLinkFileName().getToken(0,
            sfx2::cTokenSeparator));
    tools::JsonWriter aJson;
    aJson.put("url", sFileName);
    aJson.put("id", LinkedFileId(*pCont->GetSection()));
    aJson.put("section", pCont->GetSection()->GetSectionName().toString());
    m_pActiveShell->GetView().viewCallback(COKitCallbackType::OPEN_LINKED_DOCUMENT,
                                           aJson.finishAndGetAsOString());
}

namespace
{
/// Creates an empty text document at rUrl, with the styles and the chapter numbering of rMaster.
bool CreateChapterFile(const OUString& rUrl, const SwDoc& rMaster)
{
    std::shared_ptr<const SfxFilter> pFilter = SwIoSystem::GetFilterOfFormat(FILTER_XML);
    if (!pFilter)
        return false;

    SfxObjectShellLock xDocSh(new SwDocShell(SfxObjectCreateMode::INTERNAL));
    if (!xDocSh->DoInitNew(SfxNewDocumentMacros::Disallow))
        return false;

    SwDoc* pDoc = static_cast<SwDocShell*>(&xDocSh)->GetDoc();
    pDoc->ReplaceStyles(rMaster);
    if (const SwNumRule* pOutlineRule = rMaster.GetOutlineNumRule())
        pDoc->SetOutlineNumRule(*pOutlineRule);

    SfxMedium* pMedium = new SfxMedium(rUrl, StreamMode::STD_READWRITE);
    pMedium->SetFilter(pFilter);
    xDocSh->DoSaveAs(*pMedium);
    xDocSh->DoSaveCompleted(pMedium);
    const bool bCreated = !xDocSh->GetErrorIgnoreWarning();
    xDocSh->DoClose();
    return bCreated;
}

/// A file next to the master document rMasterUrl, named after it, that does not exist yet.
OUString SuggestChapterUrl(std::u16string_view rMasterUrl)
{
    INetURLObject aUrl(rMasterUrl);
    if (rMasterUrl.empty() || aUrl.GetProtocol() != INetProtocol::File)
        return OUString();

    const OUString aBase = aUrl.getBase(INetURLObject::LAST_SEGMENT, true,
                                        INetURLObject::DecodeMechanism::WithCharset);
    // A folder that cannot be looked into gets no suggestion, nor does one that is full.
    for (sal_Int32 nNumber = 1; nNumber <= 1000; ++nNumber)
    {
        aUrl.setName(Concat2View(aBase + "-" + OUString::number(nNumber) + ".odt"));
        const OUString aCandidate = aUrl.GetMainURL(INetURLObject::DecodeMechanism::NONE);
        osl::DirectoryItem aItem;
        const osl::FileBase::RC eResult = osl::DirectoryItem::get(aCandidate, aItem);
        if (eResult == osl::FileBase::E_NOENT)
            return aCandidate;
        if (eResult != osl::FileBase::E_None)
            break;
    }
    return OUString();
}
}

void SwGlobalTree::InsertLinkTarget(std::optional<SwGlblDocContent> oDocContent)
{
    std::weak_ptr<bool> xAlive = m_xAlive;
    const int nView = KitHelper::getCurrentView();
    sfx2::COKitFilePicker::pickLinkTarget(
        SwResId(STR_FILE), sfx2::COKitFilePicker::documentImportFilters(u"swriter"_ustr),
        [this, xAlive, nView, oDocContent = std::move(oDocContent)](
            const std::optional<sfx2::COKitFilePicker::LinkTarget>& roTarget)
        {
            // The Navigator can be gone by the time the user picks.
            if (xAlive.expired() || !m_pActiveShell || !roTarget)
                return;

            // Switch to the view that asked, and switch back when done.
            comphelper::ScopeGuard aRestoreView(
                [nSavedView = KitHelper::getCurrentView()]
                {
                    if (nSavedView >= 0 && KitHelper::getCurrentView() != nSavedView)
                        KitHelper::setView(nSavedView);
                });
            if (nView >= 0 && KitHelper::getCurrentView() != nView)
                KitHelper::setView(nView);

            // The document links to the file where the host keeps it, and reads it from the copy,
            // which the section reads as soon as it is inserted. The host names the file by an
            // absolute URL, or by its path relative to the document as plain text, which is
            // encoded as it is made absolute.
            SfxMedium* pMedium = m_pActiveShell->GetView().GetDocShell()->GetMedium();
            const OUString aBaseUrl = pMedium ? pMedium->GetBaseURL() : OUString();
            const INetURLObject aAbsoluteTarget(roTarget->sLinkUrl);
            OUString aLinkUrl = roTarget->sLinkUrl;
            if (!aAbsoluteTarget.HasError())
                aLinkUrl = aAbsoluteTarget.GetMainURL(INetURLObject::DecodeMechanism::NONE);
            else if (!aBaseUrl.isEmpty())
                aLinkUrl = INetURLObject::GetAbsURL(aBaseUrl, roTarget->sLinkUrl,
                                                    INetURLObject::EncodeMechanism::All,
                                                    INetURLObject::DecodeMechanism::NONE);
            INetURLObject aSectionUrl;
            aSectionUrl.SetSmartURL(aLinkUrl);
            m_pActiveShell->GetDoc()->getIDocumentLinksAdministration().GetLinkManager().SetLinkReadURL(
                aSectionUrl.GetMainURL(INetURLObject::DecodeMechanism::WithCharset),
                roTarget->sReadUrl);
            InsertRegion(oDocContent ? &*oDocContent : nullptr, &aLinkUrl);
        });
}

void SwGlobalTree::InsertNewDocument(std::optional<SwGlblDocContent> oDocContent)
{
    std::shared_ptr<const SfxFilter> pFilter = SwIoSystem::GetFilterOfFormat(FILTER_XML);
    std::vector<sfx2::COKitFilePicker::Filter> aFilters;
    if (pFilter)
        aFilters.push_back({ pFilter->GetUIName(), pFilter->GetWildcard().getGlob() });

    // Next to the master document as the user knows it, which an app may have opened as a copy.
    SfxMedium* pMedium = m_pActiveShell->GetView().GetDocShell()->GetMedium();
    const OUString aSuggested = SuggestChapterUrl(pMedium ? pMedium->GetBaseURL() : OUString());

    std::weak_ptr<bool> xAlive = m_xAlive;
    const int nView = KitHelper::getCurrentView();
    sfx2::COKitFilePicker::pickSaveLocation(
        SwResId(STR_NEW_FILE), aSuggested, aFilters,
        [this, xAlive, nView, oDocContent = std::move(oDocContent)](
            const std::optional<OUString>& roUrl)
        {
            // The Navigator can be gone by the time the user picks.
            if (xAlive.expired() || !m_pActiveShell || !roUrl)
                return;

            // Switch to the view that asked, and switch back when done.
            comphelper::ScopeGuard aRestoreView(
                [nSavedView = KitHelper::getCurrentView()]
                {
                    if (nSavedView >= 0 && KitHelper::getCurrentView() != nSavedView)
                        KitHelper::setView(nSavedView);
                });
            if (nView >= 0 && KitHelper::getCurrentView() != nView)
                KitHelper::setView(nView);

            if (!CreateChapterFile(*roUrl, *m_pActiveShell->GetDoc()))
            {
                SAL_WARN("sw.ui", "Cannot create the chapter " << *roUrl);
                return;
            }
            InsertRegion(oDocContent ? &*oDocContent : nullptr, &*roUrl);

            // The new chapter is written where the host shows it.
            const INetURLObject aNewUrl(*roUrl);
            if (!m_pSwGlblDocContents)
                return;
            for (const std::unique_ptr<SwGlblDocContent>& pContent : *m_pSwGlblDocContents)
            {
                if (pContent->GetType() == GLBLDOC_SECTION
                    && INetURLObject(o3tl::getToken(pContent->GetSection()->GetLinkFileName(), 0,
                                                    sfx2::cTokenSeparator))
                           == aNewUrl)
                {
                    OpenDoc(pContent.get());
                    break;
                }
            }
        });
}

IMPL_LINK_NOARG( SwGlobalTree, DoubleClickHdl, weld::TreeView&, bool)
{
    int nEntry = m_xTreeView->get_cursor_index();
    SwGlblDocContent* pCont = weld::fromId<SwGlblDocContent*>(m_xTreeView->get_id(nEntry));
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
    return m_pDialog;
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

        SwSectionData aSectionData(SectionType::Content, UIName(sSectionName));
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
    if ( aMedList.empty() )
        return;

    Sequence< OUString >aFileNames( aMedList.size() );
    OUString* pFileNames = aFileNames.getArray();
    sal_Int32 nPos = 0;
    for (const std::unique_ptr<SfxMedium>& pMed : aMedList)
    {
        // tdf#127978 - don't URL encode filename for navigator's tooltip
        pFileNames[nPos++]
            = pMed->GetURLObject().GetMainURL(INetURLObject::DecodeMechanism::Unambiguous)
            + OUStringChar(sfx2::cTokenSeparator)
            + pMed->GetFilter()->GetFilterName()
            + OUStringChar(sfx2::cTokenSeparator);
    }
    InsertRegion( &*m_oDocContent, aFileNames );
    m_oDocContent.reset();
}

void SwGlobalTree::Notify(SfxBroadcaster& rBC, SfxHint const& rHint)
{
    if (rHint.GetId() == SfxHintId::ThisIsAnSfxEventHint)
    {
        const SfxEventHint* pEventHint = static_cast<const SfxEventHint*>(&rHint);
        if (pEventHint->GetEventId() == SfxEventHintId::CloseView)
        {
            SfxViewEventHint const*const pVEHint(static_cast<SfxViewEventHint const*>(&rHint));
            if (m_pActiveShell)
            {
                SwXTextView* pDyingShell = dynamic_cast<SwXTextView*>(pVEHint->GetController().get());
                if (pDyingShell && pDyingShell->GetView() == &m_pActiveShell->GetView())
                {
                    EndListening(*m_pActiveShell->GetView().GetDocShell());
                    m_pActiveShell = nullptr;
                }
            }
            return;
        }
    }
    SfxListener::Notify(rBC, rHint);
    if (rHint.GetId() == SfxHintId::SwNavigatorUpdateTracking)
        UpdateTracking();
    else if (rHint.GetId() == SfxHintId::SwLinkedSectionConnectChanged)
        m_aConnectChangedIdle.Start();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
