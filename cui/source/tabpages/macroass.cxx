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
#include <svl/macitem.hxx>
#include <svx/svxids.hrc>
#include <tools/debug.hxx>
#include <vcl/idle.hxx>
#include <cfgutil.hxx>
#include <sfx2/evntconf.hxx>
#include <headertablistbox.hxx>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::frame::XFrame;

class SfxMacroTabPage_Impl
{
public:
    SfxMacroTabPage_Impl();

    OUString                               m_aStaticMacroLBLabel;
    std::unique_ptr<weld::Button>          m_xAssignPB;
    std::unique_ptr<weld::Button>          m_xDeletePB;
    std::unique_ptr<MacroEventListBox>     m_xEventLB;
    std::unique_ptr<weld::Widget>          m_xGroupFrame;
    std::unique_ptr<CuiConfigGroupListBox> m_xGroupLB;
    std::unique_ptr<weld::Frame>           m_xMacroFrame;
    std::unique_ptr<CuiConfigFunctionListBox> m_xMacroLB;

    Idle                            m_aFillGroupIdle { "cui SfxMacroTabPage m_aFillGroupIdle" };
    bool                            m_bGotEvents;
};

SfxMacroTabPage_Impl::SfxMacroTabPage_Impl()
    : m_bGotEvents(false)
{
}

static sal_uInt16 aPageRg[] = {
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
    weld::TreeView& rTreeView = mpImpl->m_xEventLB->GetListBox();
    int nSelected = rTreeView.get_selected_index();
    if (nSelected != -1)
    {
        // get bound macro
        const SvxMacro* pM = aTbl.Get(static_cast<SvMacroItemId>(rTreeView.get_selected_id().toInt32()));
        mpImpl->m_xDeletePB->set_sensitive(nullptr != pM);

        OUString sEventMacro = rTreeView.get_text(nSelected, 1);

        OUString sScriptURI = mpImpl->m_xMacroLB->GetSelectedScriptURI();
        mpImpl->m_xAssignPB->set_sensitive(!sScriptURI.equalsIgnoreAsciiCase(sEventMacro));
    }
    else
        mpImpl->m_xAssignPB->set_sensitive(false);
}

SfxMacroTabPage::SfxMacroTabPage(weld::Container* pPage, weld::DialogController* pController, const Reference< XFrame >& rxDocumentFrame, const SfxItemSet& rAttrSet )
    : SfxTabPage(pPage, pController, u"cui/ui/eventassignpage.ui"_ustr, u"EventAssignPage"_ustr, &rAttrSet)
{
    mpImpl.reset(new SfxMacroTabPage_Impl);

    mpImpl->m_aFillGroupIdle.SetInvokeHandler( LINK( this, SfxMacroTabPage, TimeOut_Impl ) );
    mpImpl->m_aFillGroupIdle.SetPriority( TaskPriority::HIGHEST );

    mpImpl->m_xEventLB.reset(new MacroEventListBox(m_xBuilder->weld_tree_view(u"assignments"_ustr)));
    mpImpl->m_xAssignPB = m_xBuilder->weld_button(u"assign"_ustr);
    mpImpl->m_xDeletePB = m_xBuilder->weld_button(u"delete"_ustr);
    mpImpl->m_xGroupFrame = m_xBuilder->weld_widget(u"groupframe"_ustr);
    mpImpl->m_xGroupLB.reset(new CuiConfigGroupListBox(m_xBuilder->weld_tree_view(u"libraries"_ustr)));
    mpImpl->m_xMacroFrame = m_xBuilder->weld_frame(u"macroframe"_ustr);
    mpImpl->m_aStaticMacroLBLabel = mpImpl->m_xMacroFrame->get_label();
    mpImpl->m_xMacroLB.reset(new CuiConfigFunctionListBox(m_xBuilder->weld_tree_view(u"macros"_ustr)));

    SetFrame( rxDocumentFrame );

    InitAndSetHandler();

    ScriptChanged();
}

SfxMacroTabPage::~SfxMacroTabPage()
{
    mpImpl.reset();
}

void SfxMacroTabPage::AddEvent(const OUString& rEventName, SvMacroItemId nEventId)
{
    weld::TreeView& rTreeView = mpImpl->m_xEventLB->GetListBox();
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
    mpImpl->m_xGroupFrame->show();
    mpImpl->m_xMacroFrame->show();

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
    if (! mpImpl->m_aFillGroupIdle.IsActive() )
        mpImpl->m_aFillGroupIdle.Start();
}

void SfxMacroTabPage::ActivatePage( const SfxItemSet& )
{
    LaunchFillGroup();
}

void SfxMacroTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    if( mpImpl->m_bGotEvents )
        return;
    if( const SfxEventNamesItem* pEventsItem = aSet.GetItemIfSet( SID_EVENTCONFIG ) )
    {
        mpImpl->m_bGotEvents = true;
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
    if( !mpImpl->m_bGotEvents && (pEventsItem = rSet->GetItemIfSet( SID_EVENTCONFIG ) ) )
    {
        mpImpl->m_bGotEvents = true;
        const SfxEventNamesList& rList = pEventsItem->GetEvents();
        for ( size_t nNo = 0, nCnt = rList.size(); nNo < nCnt; ++nNo )
        {
            const SfxEventName &rOwn = rList.at(nNo);
            AddEvent( rOwn.maUIName, rOwn.mnId );
        }
    }

    FillEvents();

    weld::TreeView& rListBox = mpImpl->m_xEventLB->GetListBox();
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

IMPL_LINK_NOARG(SfxMacroTabPage, SelectEvent_Impl, weld::TreeView&, void)
{
    weld::TreeView& rListBox = mpImpl->m_xEventLB->GetListBox();
    int nSelected = rListBox.get_selected_index();
    if (nSelected == -1)
    {
        DBG_ASSERT(nSelected != -1, "Where does the empty entry come from?");
        return;
    }

    ScriptChanged();
    EnableButtons();
}

IMPL_LINK_NOARG(SfxMacroTabPage, SelectGroup_Impl, weld::TreeView&, void)
{
    mpImpl->m_xGroupLB->GroupSelected();
    const OUString sScriptURI = mpImpl->m_xMacroLB->GetSelectedScriptURI();
    OUString       aLabelText;
    if( !sScriptURI.isEmpty() )
        aLabelText = mpImpl->m_aStaticMacroLBLabel;
    mpImpl->m_xMacroFrame->set_label( aLabelText );

    EnableButtons();
}

IMPL_LINK_NOARG(SfxMacroTabPage, SelectMacro_Impl, weld::TreeView&, void)
{
    EnableButtons();
}

IMPL_LINK(SfxMacroTabPage, AssignDeleteClickHdl_Impl, weld::Button&, rBtn, void)
{
    AssignDeleteHdl(&rBtn);
}

IMPL_LINK(SfxMacroTabPage, AssignDeleteHdl_Impl, weld::TreeView&, rBtn, bool)
{
    AssignDeleteHdl(&rBtn);
    return true;
}

void SfxMacroTabPage::AssignDeleteHdl(const weld::Widget* pBtn)
{
    weld::TreeView& rListBox = mpImpl->m_xEventLB->GetListBox();
    int nSelected = rListBox.get_selected_index();
    if (nSelected == -1)
    {
        DBG_ASSERT(nSelected != -1, "Where does the empty entry come from?");
        return;
    }

    const bool bAssEnabled = pBtn != mpImpl->m_xDeletePB.get() && mpImpl->m_xAssignPB->get_sensitive();

    // remove from the table
    SvMacroItemId nEvent = static_cast<SvMacroItemId>(rListBox.get_selected_id().toInt32());
    aTbl.Erase( nEvent );

    OUString sScriptURI;
    if( bAssEnabled )
    {
        sScriptURI = mpImpl->m_xMacroLB->GetSelectedScriptURI();
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
    mpImpl->m_xGroupLB->Init(comphelper::getProcessComponentContext(), GetFrame(),
                             OUString(), false);
}

void SfxMacroTabPage::InitAndSetHandler()
{
    weld::TreeView& rListBox = mpImpl->m_xEventLB->GetListBox();
    Link<weld::TreeView&,bool> aLnk(LINK(this, SfxMacroTabPage, AssignDeleteHdl_Impl));
    mpImpl->m_xMacroLB->connect_row_activated( aLnk);
    mpImpl->m_xDeletePB->connect_clicked(LINK(this, SfxMacroTabPage, AssignDeleteClickHdl_Impl));
    mpImpl->m_xAssignPB->connect_clicked(LINK(this, SfxMacroTabPage, AssignDeleteClickHdl_Impl));
    rListBox.connect_row_activated(aLnk);

    rListBox.connect_changed(LINK(this, SfxMacroTabPage, SelectEvent_Impl));
    mpImpl->m_xGroupLB->connect_changed(LINK(this, SfxMacroTabPage, SelectGroup_Impl));
    mpImpl->m_xMacroLB->connect_changed(LINK(this, SfxMacroTabPage, SelectMacro_Impl));

    std::vector<int> aWidths
    {
        o3tl::narrowing<int>(rListBox.get_approximate_digit_width() * 35)
    };
    rListBox.set_column_fixed_widths(aWidths);

    mpImpl->m_xEventLB->show();

    mpImpl->m_xEventLB->set_sensitive(true);
    mpImpl->m_xGroupLB->set_sensitive(true);
    mpImpl->m_xMacroLB->set_sensitive(true);

    mpImpl->m_xGroupLB->SetFunctionListBox(mpImpl->m_xMacroLB.get());
}

void SfxMacroTabPage::FillEvents()
{
    weld::TreeView& rListBox = mpImpl->m_xEventLB->GetListBox();

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
    const Reference< XFrame >& rxDocumentFrame, const SfxItemSet& rSet)
    : SfxSingleTabDialogController(pParent, &rSet,u"cui/ui/eventassigndialog.ui"_ustr,
                                   u"EventAssignDialog"_ustr)
{
    std::unique_ptr<SfxMacroTabPage> xPage = CreateSfxMacroTabPage(get_content_area(), this, rSet);
    xPage->SetFrame(rxDocumentFrame);
    SetTabPage(std::move(xPage));
    GetTabPage()->LaunchFillGroup();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
