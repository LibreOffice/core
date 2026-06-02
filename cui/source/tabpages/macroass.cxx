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

#include <macroass.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <comphelper/string.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/macitem.hxx>
#include <tools/debug.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/WaitObject.hxx>
#include <sfx2/evntconf.hxx>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::frame::XFrame;

const sal_uInt16 aPageRg[] = {
    SID_ATTR_MACROITEM, SID_ATTR_MACROITEM,
    0
};

static OUString ConvertToUIName_Impl( SvxMacro const *pMacro )
{
    OUString aName( pMacro->GetMacName() );
    if ( pMacro->GetLanguage() != "JavaScript" )
    {
        const sal_Int32 nCount = comphelper::string::getTokenCount(aName, '.');
        OUString aEntry = aName.getToken( nCount-1, '.' );
        if ( nCount > 2 )
        {
            aEntry += OUString::Concat("(") + o3tl::getToken(aName, 0, '.' ) + "." + o3tl::getToken(aName, nCount-2, '.' ) + ")";
        }
        return aEntry;
    }
    else
        return aName;
}

void SfxMacroTabPage::EnableButtons()
{
    // don't do anything as long as the eventbox is empty
    weld::TreeView& rTreeView = m_xEventLB->GetListBox();
    int nSelected = rTreeView.get_selected_index();
    if (nSelected != -1)
    {
        // get bound macro
        const SvxMacro* pM = aTbl.Get(static_cast<SvMacroItemId>(rTreeView.get_selected_id().toInt32()));
        m_xDeletePB->set_sensitive(nullptr != pM);

        OUString sEventMacro = rTreeView.get_text(nSelected, 1);

        OUString sScriptURI = m_xMacroLB->GetSelectedScriptURI();
        m_xAssignPB->set_sensitive(!sScriptURI.equalsIgnoreAsciiCase(sEventMacro));
    }
    else
        m_xAssignPB->set_sensitive(false);
}

SfxMacroTabPage::SfxMacroTabPage(weld::Container* pPage, weld::DialogController* pController, const Reference< XFrame >& rxDocumentFrame, const SfxItemSet& rAttrSet )
    : SfxTabPage(pPage, pController, u"cui/ui/eventassignpage.ui"_ustr, u"EventAssignPage"_ustr, &rAttrSet)
    , m_bGotEvents(false)
{
    m_aFillGroupIdle.SetInvokeHandler(LINK(this, SfxMacroTabPage, TimeOut_Impl));
    m_aFillGroupIdle.SetPriority(TaskPriority::HIGHEST);

    m_xEventLB.reset(new MacroEventListBox(m_xBuilder->weld_tree_view(u"assignments"_ustr)));
    m_xAssignPB = m_xBuilder->weld_button(u"assign"_ustr);
    m_xDeletePB = m_xBuilder->weld_button(u"delete"_ustr);
    m_xGroupFrame = m_xBuilder->weld_widget(u"groupframe"_ustr);
    m_xGroupLB.reset(new CuiConfigGroupListBox(m_xBuilder->weld_tree_view(u"libraries"_ustr)));
    m_xMacroFrame = m_xBuilder->weld_frame(u"macroframe"_ustr);
    m_aStaticMacroLBLabel = m_xMacroFrame->get_label();
    m_xMacroLB.reset(new CuiConfigFunctionListBox(m_xBuilder->weld_tree_view(u"macros"_ustr)));

    SetFrame( rxDocumentFrame );

    weld::TreeView& rListBox = m_xEventLB->GetListBox();
    m_xMacroLB->connect_item_activated(LINK(this, SfxMacroTabPage, MacroTreeViewActivatedHdl));
    m_xDeletePB->connect_clicked(LINK(this, SfxMacroTabPage, AssignDeleteClickHdl_Impl));
    m_xAssignPB->connect_clicked(LINK(this, SfxMacroTabPage, AssignDeleteClickHdl_Impl));
    rListBox.connect_item_activated(LINK(this, SfxMacroTabPage, AssignmentsTreeViewActivatedHdl));

    rListBox.connect_selection_changed(LINK(this, SfxMacroTabPage, SelectEvent_Impl));
    m_xGroupLB->connect_changed(LINK(this, SfxMacroTabPage, SelectGroup_Impl));
    m_xMacroLB->connect_changed(LINK(this, SfxMacroTabPage, SelectMacro_Impl));

    std::vector<int> aWidths{ o3tl::narrowing<int>(rListBox.get_approximate_digit_width() * 35) };
    rListBox.set_column_fixed_widths(aWidths);

    m_xEventLB->show();

    m_xEventLB->set_sensitive(true);
    m_xGroupLB->set_sensitive(true);
    m_xMacroLB->set_sensitive(true);

    m_xGroupLB->SetFunctionListBox(m_xMacroLB.get());

    ScriptChanged();
}

SfxMacroTabPage::~SfxMacroTabPage() {}

void SfxMacroTabPage::AddEvent(const OUString& rEventName, SvMacroItemId nEventId)
{
    weld::TreeView& rTreeView = m_xEventLB->GetListBox();
    rTreeView.append(OUString::number(static_cast<sal_Int32>(nEventId)), rEventName);

    // if the table is valid already
    SvxMacro* pM = aTbl.Get(nEventId);
    if (pM)
    {
        OUString sNew(ConvertToUIName_Impl(pM));
        rTreeView.set_text(rTreeView.n_children() - 1, sNew, 1);
    }
}

void SfxMacroTabPage::ScriptChanged()
{
    // get new areas and their functions
    m_xGroupFrame->show();
    m_xMacroFrame->show();

    EnableButtons();
}

bool SfxMacroTabPage::FillItemSet( SfxItemSet* rSet )
{
    SvxMacroItem aItem( GetWhich( aPageRg[0] ) );
    const_cast<SvxMacroTableDtor&>(aItem.GetMacroTable()) = aTbl;

    const SfxPoolItem* pItem = nullptr;
    SfxItemState eState = GetItemSet().GetItemState(aItem.Which(), true, &pItem);
    if (eState == SfxItemState::DEFAULT && aTbl.empty())
    {
        // Don't touch the item set if there was no input and our table is empty.
        return false;
    }
    if (SfxItemState::SET != eState || aItem != *static_cast<const SvxMacroItem*>(pItem))
    {
        rSet->Put( aItem );
        return true;
    }
    return false;
}

void SfxMacroTabPage::LaunchFillGroup()
{
    if (!m_aFillGroupIdle.IsActive())
        m_aFillGroupIdle.Start();
}

void SfxMacroTabPage::ActivatePage( const SfxItemSet& )
{
    LaunchFillGroup();
}

void SfxMacroTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    if (m_bGotEvents)
        return;
    if( const SfxEventNamesItem* pEventsItem = aSet.GetItemIfSet( SID_EVENTCONFIG ) )
    {
        m_bGotEvents = true;
        const SfxEventNamesList& rList = pEventsItem->GetEvents();
        for ( size_t nNo = 0, nCnt = rList.size(); nNo < nCnt; ++nNo )
        {
            const SfxEventName &rOwn = rList.at(nNo);
            AddEvent( rOwn.maUIName, rOwn.mnId );
        }
    }
}

void SfxMacroTabPage::Reset( const SfxItemSet* rSet )
{
    const SfxPoolItem* pItem;
    if( SfxItemState::SET == rSet->GetItemState( GetWhich( aPageRg[0] ), true, &pItem ))
        aTbl = static_cast<const SvxMacroItem*>(pItem)->GetMacroTable();

    const SfxEventNamesItem* pEventsItem;
    if (!m_bGotEvents && (pEventsItem = rSet->GetItemIfSet(SID_EVENTCONFIG)))
    {
        m_bGotEvents = true;
        const SfxEventNamesList& rList = pEventsItem->GetEvents();
        for ( size_t nNo = 0, nCnt = rList.size(); nNo < nCnt; ++nNo )
        {
            const SfxEventName &rOwn = rList.at(nNo);
            AddEvent( rOwn.maUIName, rOwn.mnId );
        }
    }

    FillEvents();

    weld::TreeView& rListBox = m_xEventLB->GetListBox();
    std::unique_ptr<weld::TreeIter> xIter(rListBox.make_iterator());
    if (rListBox.get_iter_first(*xIter))
    {
        rListBox.set_cursor(*xIter);
        EnableButtons();
    }
}

bool SfxMacroTabPage::IsReadOnly() const
{
    return false;
}

IMPL_LINK_NOARG(SfxMacroTabPage, SelectEvent_Impl, weld::ItemView&, void)
{
    weld::TreeView& rListBox = m_xEventLB->GetListBox();
    int nSelected = rListBox.get_selected_index();
    if (nSelected == -1)
    {
        DBG_ASSERT(nSelected != -1, "Where does the empty entry come from?");
        return;
    }

    ScriptChanged();
    EnableButtons();
}

IMPL_LINK_NOARG(SfxMacroTabPage, SelectGroup_Impl, weld::ItemView&, void)
{
    m_xGroupLB->GroupSelected();
    const OUString sScriptURI = m_xMacroLB->GetSelectedScriptURI();
    OUString       aLabelText;
    if( !sScriptURI.isEmpty() )
        aLabelText = m_aStaticMacroLBLabel;
    m_xMacroFrame->set_label(aLabelText);

    EnableButtons();
}

IMPL_LINK_NOARG(SfxMacroTabPage, SelectMacro_Impl, weld::ItemView&, void)
{
    EnableButtons();
}

IMPL_LINK(SfxMacroTabPage, AssignDeleteClickHdl_Impl, weld::Button&, rBtn, void)
{
    AssignDeleteHdl(&rBtn);
}

IMPL_LINK_NOARG(SfxMacroTabPage, MacroTreeViewActivatedHdl, const weld::TreeIter&, bool)
{
    AssignDeleteHdl(&m_xMacroLB->get_widget());
    return true;
}

IMPL_LINK_NOARG(SfxMacroTabPage, AssignmentsTreeViewActivatedHdl, const weld::TreeIter&, bool)
{
    AssignDeleteHdl(&m_xEventLB->GetListBox());
    return true;
}

void SfxMacroTabPage::AssignDeleteHdl(const weld::Widget* pBtn)
{
    weld::TreeView& rListBox = m_xEventLB->GetListBox();
    int nSelected = rListBox.get_selected_index();
    if (nSelected == -1)
    {
        DBG_ASSERT(nSelected != -1, "Where does the empty entry come from?");
        return;
    }

    const bool bAssEnabled = pBtn != m_xDeletePB.get() && m_xAssignPB->get_sensitive();

    // remove from the table
    SvMacroItemId nEvent = static_cast<SvMacroItemId>(rListBox.get_selected_id().toInt32());
    aTbl.Erase( nEvent );

    OUString sScriptURI;
    if( bAssEnabled )
    {
        sScriptURI = m_xMacroLB->GetSelectedScriptURI();
        if( sScriptURI.startsWith( "vnd.sun.star.script:" ) )
        {
            aTbl.Insert(
                nEvent, SvxMacro( sScriptURI, SVX_MACRO_LANGUAGE_SF ) );
        }
        else
        {
            OSL_ENSURE( false, "SfxMacroTabPage::AssignDeleteHdl_Impl: this branch is *not* dead? (out of interest: tell fs, please!)" );
            aTbl.Insert(
                nEvent, SvxMacro( sScriptURI, SVX_MACRO_LANGUAGE_STARBASIC ) );
        }
    }

    rListBox.set_text(nSelected, sScriptURI, 1);

    EnableButtons();
}

IMPL_LINK( SfxMacroTabPage, TimeOut_Impl, Timer*,, void )
{
    // FillMacroList() can take a long time -> show wait cursor and disable input
    weld::Window* pDialog = GetFrameWeld();
    // perhaps the tabpage is part of a SingleTabDialog then pDialog == nullptr
    std::unique_ptr<weld::WaitObject> xWait(pDialog ? new weld::WaitObject(pDialog) : nullptr);
    // fill macro list
    m_xGroupLB->Init(comphelper::getProcessComponentContext(), GetFrame(), OUString(), false);
}

void SfxMacroTabPage::FillEvents()
{
    weld::TreeView& rListBox = m_xEventLB->GetListBox();

    int nEntryCnt = rListBox.n_children();

    // get events from the table and fill the EventListBox respectively
    for (int n = 0 ; n < nEntryCnt; ++n)
    {
        OUString sOld = rListBox.get_text(n, 1);
        OUString sNew;
        SvMacroItemId nEventId = static_cast<SvMacroItemId>(rListBox.get_id(n).toInt32());
        if (aTbl.IsKeyValid(nEventId))
            sNew = ConvertToUIName_Impl(aTbl.Get(nEventId));

        if (sOld == sNew)
            continue;

        rListBox.set_text(n, sNew, 1);
    }
}

namespace
{
    std::unique_ptr<SfxMacroTabPage> CreateSfxMacroTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rAttrSet)
    {
        return std::make_unique<SfxMacroTabPage>( pPage, pController, nullptr, rAttrSet );
    }
}

std::unique_ptr<SfxTabPage> SfxMacroTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet)
{
    return CreateSfxMacroTabPage(pPage, pController, *rAttrSet);
}

SfxMacroAssignDlg::SfxMacroAssignDlg(weld::Widget* pParent,
    const Reference< XFrame >& rxDocumentFrame, std::unique_ptr<const SfxItemSet> xSet)
    : SfxSingleTabDialogController(pParent, nullptr, u"cui/ui/eventassigndialog.ui"_ustr,
                                   u"EventAssignDialog"_ustr),
      mxItemSet(std::move(xSet))
{
    SetInputSet(mxItemSet.get());
    std::unique_ptr<SfxMacroTabPage> xPage = CreateSfxMacroTabPage(get_content_area(), this, *mxItemSet);
    xPage->SetFrame(rxDocumentFrame);
    SetTabPage(std::move(xPage));
    GetTabPage()->LaunchFillGroup();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
