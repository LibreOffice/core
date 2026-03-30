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

#include <memory>

#include <vcl/commandinfoprovider.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/style.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/UnknownModuleException.hpp>
#include <officecfg/Office/Common.hxx>

#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/templdlg.hxx>
#include <templdgi.hxx>
#include <sfx2/styfitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/tplpitem.hxx>
#include <sfx2/sfxresid.hxx>
#include <svl/itemset.hxx>

#include <sfx2/sfxsids.hrc>
#include <sfx2/strings.hrc>
#include <helpids.h>
#include <sfx2/viewfrm.hxx>
#include <vcl/weld/Menu.hxx>

#include <comphelper/string.hxx>

using namespace css;
using namespace css::beans;
using namespace css::frame;
using namespace css::uno;

class SfxCommonTemplateDialog_Impl::DeletionWatcher
{
    typedef void (DeletionWatcher::* bool_type)();

public:
    explicit DeletionWatcher(SfxCommonTemplateDialog_Impl& rDialog)
        : m_pDialog(&rDialog)
        , m_pPrevious(m_pDialog->impl_setDeletionWatcher(this))
    {
    }

    ~DeletionWatcher()
    {
        if (m_pDialog)
            m_pDialog->impl_setDeletionWatcher(m_pPrevious);
    }

    DeletionWatcher(const DeletionWatcher&) = delete;
    DeletionWatcher& operator=(const DeletionWatcher&) = delete;

    // Signal that the dialog was deleted
    void signal()
    {
        m_pDialog = nullptr;
        if (m_pPrevious)
            m_pPrevious->signal();
    }

    // Return true if the dialog was deleted
    operator bool_type() const
    {
        return m_pDialog ? nullptr : &DeletionWatcher::signal;
    }

private:
    SfxCommonTemplateDialog_Impl* m_pDialog;
    DeletionWatcher *const m_pPrevious; /// let's add more epicycles!
};

sal_Int8 SfxCommonTemplateDialog_Impl::ExecuteDrop(const ExecuteDropEvent& rEvt)
{
    // handle drop of content into the treeview to create a new style
    m_aStyleListExecuteDrop.Call(rEvt);
    return DND_ACTION_NONE;
}

IMPL_LINK(SfxCommonTemplateDialog_Impl, OnAsyncExecuteDrop, void*, pStyleList, void)
{
    StyleList* pStyle = static_cast<StyleList*>(pStyleList);
    if (pStyle == &m_aStyleList)
        ActionSelect(u"new"_ustr, m_aStyleList);
}

namespace SfxTemplate
{
    // converts from SFX_STYLE_FAMILY Ids to 1-6
    sal_uInt16 SfxFamilyIdToNId(SfxStyleFamily nFamily)
    {
        switch ( nFamily )
        {
            case SfxStyleFamily::Char:   return 1;
            case SfxStyleFamily::Para:   return 2;
            case SfxStyleFamily::Frame:  return 3;
            case SfxStyleFamily::Page:   return 4;
            case SfxStyleFamily::Pseudo: return 5;
            case SfxStyleFamily::Table:  return 6;
            default:                     return 0xffff;
        }
    }
    // converts from 1-6 to SFX_STYLE_FAMILY Ids
    SfxStyleFamily NIdToSfxFamilyId(sal_uInt16 nId)
    {
        switch (nId)
        {
            case 1:
                return SfxStyleFamily::Char;
            case 2:
                return SfxStyleFamily::Para;
            case 3:
                return SfxStyleFamily::Frame;
            case 4:
                return SfxStyleFamily::Page;
            case 5:
                return SfxStyleFamily::Pseudo;
            case 6:
                return SfxStyleFamily::Table;
            default:
                return SfxStyleFamily::All;
        }
    }
}

SfxTemplatePanelControl::SfxTemplatePanelControl(SfxBindings* pBindings, weld::Widget* pParent)
    : PanelLayout(pParent, u"TemplatePanel"_ustr, u"sfx/ui/templatepanel.ui"_ustr)
    , m_aSpotlightParaStyles(SID_SPOTLIGHT_PARASTYLES, *pBindings, *this)
    , m_aSpotlightCharStyles(SID_SPOTLIGHT_CHARSTYLES, *pBindings, *this)
    , pImpl(new SfxTemplateDialog_Impl(pBindings, this))
{
    OSL_ASSERT(pBindings!=nullptr);
}

SfxTemplatePanelControl::~SfxTemplatePanelControl()
{
    m_aSpotlightParaStyles.dispose();
    m_aSpotlightCharStyles.dispose();
}

void SfxTemplatePanelControl::NotifyItemUpdate(const sal_uInt16 nSId, const SfxItemState eState,
                                               const SfxPoolItem* pState)
{
    switch (nSId)
    {
        case SID_SPOTLIGHT_PARASTYLES:
            if (eState >= SfxItemState::DEFAULT)
            {
                const SfxBoolItem* pItem = dynamic_cast<const SfxBoolItem*>(pState);
                if (pItem)
                {
                    bool bValue = pItem->GetValue();
                    if (bValue || (!bValue && pImpl->m_aStyleList.IsSpotlightParaStyles()))
                    {
                        pImpl->m_aStyleList.SetSpotlightParaStyles(bValue);
                        pImpl->FamilySelect(SfxTemplate::SfxFamilyIdToNId(SfxStyleFamily::Para));
                    }
                }
            }
            break;
        case SID_SPOTLIGHT_CHARSTYLES:
            if (eState >= SfxItemState::DEFAULT)
            {
                const SfxBoolItem* pItem = dynamic_cast<const SfxBoolItem*>(pState);
                if (pItem)
                {
                    bool bValue = pItem->GetValue();
                    if (bValue || (!bValue && pImpl->m_aStyleList.IsSpotlightCharStyles()))
                    {
                        pImpl->m_aStyleList.SetSpotlightCharStyles(bValue);
                        pImpl->FamilySelect(SfxTemplate::SfxFamilyIdToNId(SfxStyleFamily::Char));
                    }
                }
            }
            break;
    }
}

void SfxCommonTemplateDialog_Impl::connect_stylelist_execute_drop(
    const Link<const ExecuteDropEvent&, sal_Int8>& rLink)
{
    m_aStyleListExecuteDrop = rLink;
}

void SfxCommonTemplateDialog_Impl::connect_stylelist_has_selected_style(const Link<void*, bool>& rLink)
{
    m_aStyleListHasSelectedStyle = rLink;
}

void SfxCommonTemplateDialog_Impl::connect_stylelist_update_style_dependents(const Link<void*, void>& rLink)
{
    m_aStyleListUpdateStyleDependents = rLink;
}

void SfxCommonTemplateDialog_Impl::connect_stylelist_enable_tree_drag(const Link<bool, void> rLink)
{
    m_aStyleListEnableTreeDrag = rLink;
}

void SfxCommonTemplateDialog_Impl::connect_stylelist_enable_delete(const Link<void*, void> rLink)
{
    m_aStyleListEnableDelete = rLink;
}

void SfxCommonTemplateDialog_Impl::connect_stylelist_set_water_can_state(
    const Link<const SfxBoolItem*, void> rLink)
{
    m_aStyleListSetWaterCanState = rLink;
}

constexpr OUString FILTERPOPOVER = u"stylefiltersmenutoolbutton"_ustr;

SfxCommonTemplateDialog_Impl::StyleFiltersToolbarPopup::StyleFiltersToolbarPopup(
    weld::Toolbar* pParent, StyleList& rStyleList)
    : WeldToolbarPopup(nullptr, pParent, u"sfx/ui/templatepanel.ui"_ustr,
                       u"StyleFiltersPopover"_ustr)
    , m_xFiltersTreeView(m_xBuilder->weld_tree_view(u"stylefilterstreeview"_ustr))
    , m_xOKButton(m_xBuilder->weld_button(u"ok"_ustr))

{
    m_xFiltersTreeView->enable_toggle_buttons(weld::ColumnToggleType::Check);
    for (const SfxFilterTuple& i : rStyleList.GetFamilyItem()->GetFilterList())
    {
        m_xFiltersTreeView->append();
        const int nRow = m_xFiltersTreeView->n_children() - 1;
        if (rStyleList.m_aFamilySelectedFiltersSet[rStyleList.GetFamilyItem()->GetFamily()]
                .contains(i.nFlags))
            m_xFiltersTreeView->set_toggle(nRow, TriState::TRISTATE_TRUE);
        else
            m_xFiltersTreeView->set_toggle(nRow, TriState::TRISTATE_FALSE);
        m_xFiltersTreeView->set_text(nRow, i.aName, 0);
        m_xFiltersTreeView->set_id(
            nRow, OUString::number(
                      static_cast<std::underlying_type<SfxStyleSearchBits>::type>(i.nFlags)));
    }
}

void SfxCommonTemplateDialog_Impl::StyleFiltersToolbarPopup::GrabFocus()
{
    m_xFiltersTreeView->grab_focus();
}

// Constructor

SfxCommonTemplateDialog_Impl::SfxCommonTemplateDialog_Impl(SfxBindings* pB, weld::Container* pC, weld::Builder* pBuilder)
    : pBindings(pB)
    , xModuleManager(frame::ModuleManager::create(::comphelper::getProcessComponentContext()))
    , m_pDeletionWatcher(nullptr)
    , m_aStyleList(pBuilder, pB, this, pC, u"treeview"_ustr, u"flatview"_ustr)
    , mxHierarchicalCheckbox(pBuilder->weld_check_button(u"hierarchical"_ustr))
    , mxPreviewCheckbox(pBuilder->weld_check_button(u"showpreview"_ustr))
    , mxSpotlightCheckbox(pBuilder->weld_check_button(u"spotlightstyles"_ustr))
    , m_xStyleFiltersToolbar(pBuilder->weld_toolbar(u"stylefilterstoolbar"_ustr))
    , m_xStyleFiltersToolbarPopoverContainer(
          new ToolbarPopupContainer(m_xStyleFiltersToolbar.get()))
    , bIsWater(false)
    , bUpdate(false)
    , bWaterDisabled(false)
    , bNewByExampleDisabled(false)
    , bUpdateByExampleDisabled(false)
{
    m_xStyleFiltersToolbar->set_help_id(HID_TEMPLATE_FILTER);
    m_xStyleFiltersToolbar->set_item_popover(FILTERPOPOVER,
                                             m_xStyleFiltersToolbarPopoverContainer->getTopLevel());
    m_xStyleFiltersToolbar->connect_clicked(
        LINK(this, SfxTemplateDialog_Impl, FilterToolbarSelectHdl));
    m_xStyleFiltersToolbar->connect_menu_toggled(
        LINK(this, SfxTemplateDialog_Impl, FilterToolbarMenuHdl));
    mxPreviewCheckbox->set_active(officecfg::Office::Common::StylesAndFormatting::Preview::get());
}

IMPL_LINK_NOARG(SfxCommonTemplateDialog_Impl, FilterToolbarSelectHdl, const OUString&, void)
{
    m_xStyleFiltersToolbar->set_menu_item_active(
        FILTERPOPOVER, !m_xStyleFiltersToolbar->get_menu_item_active(FILTERPOPOVER));
}

IMPL_LINK_NOARG(SfxCommonTemplateDialog_Impl, FilterToolbarPopupOKButtonHdl, weld::Button&, void)
{
    FilterToolbarSelectHdl(FILTERPOPOVER);
}

// called when the toolbar menu is opened and closed
IMPL_LINK_NOARG(SfxCommonTemplateDialog_Impl, FilterToolbarMenuHdl, const OUString&, void)
{
    if (!m_xStyleFiltersToolbarPopoverContainer->getPopover())
    {
        m_xStyleFiltersToolbarPopoverContainer->setPopover(
            std::make_unique<StyleFiltersToolbarPopup>(m_xStyleFiltersToolbar.get(), m_aStyleList));
        m_xStyleFiltersToolbarPopoverContainer->getPopover()->GrabFocus();

        StyleFiltersToolbarPopup* pFilterToolbarPopup = static_cast<StyleFiltersToolbarPopup*>(
            m_xStyleFiltersToolbarPopoverContainer->getPopover());
        pFilterToolbarPopup->m_xOKButton->connect_clicked(
            LINK(this, SfxCommonTemplateDialog_Impl, FilterToolbarPopupOKButtonHdl));
    }
    else // close menu (popover)
    {
        StyleFiltersToolbarPopup* pFilterToolbarPopup = static_cast<StyleFiltersToolbarPopup*>(
            m_xStyleFiltersToolbarPopoverContainer->getPopover());

        weld::TreeView* pFiltersTreeView = pFilterToolbarPopup->m_xFiltersTreeView.get();

        auto& rFamilySelectedFiltersSet
            = m_aStyleList
                  .m_aFamilySelectedFiltersSet[m_aStyleList.GetFamilyItem()->GetFamily()];
        rFamilySelectedFiltersSet.clear();
        // fill the style filters set from the selected filters
        pFiltersTreeView->all_foreach([&pFiltersTreeView,
                                       &rFamilySelectedFiltersSet](weld::TreeIter& rEntry) {
            if (pFiltersTreeView->get_toggle(rEntry) == TriState::TRISTATE_TRUE)
            {
                rFamilySelectedFiltersSet.insert(
                    static_cast<SfxStyleSearchBits>(pFiltersTreeView->get_id(rEntry).toUInt32()));
            }
            return false;
        });

        m_aStyleList.m_bHierarchical
            ? m_aStyleList.FillHierarchicalTreeView(/*bExpandRootParents*/ true)
            : m_aStyleList.FillFlatTreeView();

        SaveFactoryStyleFilter();

        m_xStyleFiltersToolbarPopoverContainer->unsetPopover();
    }
}

void SfxTemplateDialog_Impl::EnableEdit(bool bEnable, StyleList* rStyleList)
{
    if(rStyleList == &m_aStyleList || rStyleList == nullptr)
        SfxCommonTemplateDialog_Impl::EnableEdit( bEnable, &m_aStyleList );
    if( !bEnable || !bUpdateByExampleDisabled )
        EnableItem(u"update"_ustr, bEnable);
}

// called from StyleList::Update()
IMPL_LINK(SfxCommonTemplateDialog_Impl, ReadResource_Hdl, StyleList&, rStyleList, void)
{
    size_t nCount = m_aStyleListReadResource.Call(nullptr);

// Insert in the reverse order of occurrence in the Style Families. This is for
// the toolbar of the designer. The list box of the catalog respects the
// correct order by itself.

// Sequences: the order of Resource = the order of Toolbar for example list box.
// Order of ascending SIDs: Low SIDs are displayed first when templates of
// several families are active.

    // in the Writer the UpdateStyleByExample Toolbox button is removed and
    // the NewStyle button gets a PopupMenu
    if(nCount > 4)
        ReplaceUpdateButtonByMenu();

    while (nCount)
    {
        --nCount;
        const SfxStyleFamilyItem &rItem = rStyleList.GetFamilyItemByIndex( nCount );
        sal_uInt16 nId = SfxTemplate::SfxFamilyIdToNId( rItem.GetFamily() );
        InsertFamilyItem(nId, rItem);
    }
}

// called from StyleList::Update
IMPL_LINK_NOARG(SfxCommonTemplateDialog_Impl, ClearResource_Hdl, void*, void)
{
    ClearFamilyList();
    m_aStyleListClear.Call(nullptr);
}

SfxCommonTemplateDialog_Impl::DeletionWatcher *
SfxCommonTemplateDialog_Impl::impl_setDeletionWatcher(
        DeletionWatcher *const pNewWatcher)
{
    DeletionWatcher *const pRet(m_pDeletionWatcher);
    m_pDeletionWatcher = pNewWatcher;
    return pRet;
}

void SfxCommonTemplateDialog_Impl::Initialize()
{
    m_aStyleList.connect_ReadResource(LINK(this, SfxCommonTemplateDialog_Impl, ReadResource_Hdl));
    m_aStyleList.connect_ClearResource(LINK(this, SfxCommonTemplateDialog_Impl, ClearResource_Hdl));
    m_aStyleList.connect_UpdateFamily(LINK(this, SfxCommonTemplateDialog_Impl, UpdateFamily_Hdl));

    mxHierarchicalCheckbox->connect_toggled(
        LINK(this, SfxCommonTemplateDialog_Impl, HierarchicalHdl));
    mxPreviewCheckbox->connect_toggled(LINK(this, SfxCommonTemplateDialog_Impl, PreviewHdl));
    mxSpotlightCheckbox->connect_toggled(LINK(this, SfxCommonTemplateDialog_Impl, SpotlightHdl));

    m_aStyleList.Initialize();

    mxHierarchicalCheckbox->set_active(m_aStyleList.m_bHierarchical);

    SfxStyleFamily eFam = SfxTemplate::NIdToSfxFamilyId(m_aStyleList.m_nActFamily);
    mxSpotlightCheckbox->set_visible(m_aStyleList.HasStylesSpotlightFeature()
                                && (eFam == SfxStyleFamily::Para || eFam == SfxStyleFamily::Char));
}

SfxCommonTemplateDialog_Impl::~SfxCommonTemplateDialog_Impl()
{
    // Set the UNO's in an 'off' state. FN_PARAM_1 is used to prevent the sidebar from trying to
    // reopen while it is being closed here.
    if (m_aStyleList.IsSpotlightParaStyles())
    {
        SfxDispatcher &rDispatcher = *SfxGetpApp()->GetDispatcher_Impl();
        SfxFlagItem aParam(FN_PARAM_1);
        rDispatcher.ExecuteList(SID_SPOTLIGHT_PARASTYLES, SfxCallMode::SYNCHRON, { &aParam });
    }
    if (m_aStyleList.IsSpotlightCharStyles())
    {
        SfxDispatcher &rDispatcher = *SfxGetpApp()->GetDispatcher_Impl();
        SfxFlagItem aParam(FN_PARAM_1);
        rDispatcher.ExecuteList(SID_SPOTLIGHT_CHARSTYLES, SfxCallMode::SYNCHRON, { &aParam });
    }

    if ( bIsWater )
        Execute_Impl(SID_STYLE_WATERCAN, u""_ustr, u""_ustr, 0, m_aStyleList);
    m_aStyleListClear.Call(nullptr);
    m_aStyleListCleanup.Call(nullptr);
    if ( m_pDeletionWatcher )
        m_pDeletionWatcher->signal();
    mxHierarchicalCheckbox.reset();
    mxPreviewCheckbox.reset();
    mxSpotlightCheckbox.reset();
}

/**
 * Is it safe to show the water-can / fill icon. If we've a
 * hierarchical widget - we have only single select, otherwise
 * we need to check if we have a multi-selection. We either have
 * a mxTreeBox showing or an mxFmtLb (which we hide when not shown)
 */
bool SfxCommonTemplateDialog_Impl::IsSafeForWaterCan() const
{
    return m_aStyleListWaterCan.Call(nullptr);
}

// called by StyleList::FillHierarchicalTreeView, StyleList::FillFlatTreeView and
// StyleList::ExecuteDrop
void SfxCommonTemplateDialog_Impl::SelectStyle(const OUString &rStr)
{
    m_aStyleList.SelectStyle(rStr);
    SelectUpdate(rStr);
}

// called from SfxCommonTemplateDialog_Impl::SelectStyle and StyleList::FmtSelectHdl
void SfxCommonTemplateDialog_Impl::SelectUpdate(const OUString& rStr)
{
    m_aStyleList.SelectUpdate(rStr);

    bWaterDisabled = !IsSafeForWaterCan();

    // tdf#134598 call UpdateStyleDependents to update watercan
    UpdateStyleDependents_Hdl(nullptr);
}

void SfxCommonTemplateDialog_Impl::EnableTreeDrag(bool bEnable)
{
    m_aStyleListEnableTreeDrag.Call(bEnable);
}

// called from SfxTemplateControllerItem::SetWaterCanStateHdl_Impl
// Updated display: Watering the house
void SfxCommonTemplateDialog_Impl::SetWaterCanState(const SfxBoolItem *pItem)
{
    bWaterDisabled = (pItem == nullptr);

    if(!bWaterDisabled)
        //make sure the watercan is only activated when there is (only) one selection
        bWaterDisabled = !IsSafeForWaterCan();

    if(pItem && !bWaterDisabled)
    {
        CheckItem(u"watercan"_ustr, pItem->GetValue());
        EnableItem(u"watercan"_ustr);
    }
    else
    {
        if(!bWaterDisabled)
            EnableItem(u"watercan"_ustr);
        else
            EnableItem(u"watercan"_ustr, false);
    }

// Ignore while in watercan mode statusupdates

    m_aStyleListSetWaterCanState.Call(pItem);
}

// Item with the status of a Family is copied and noted
// (is updated when all states have also been updated.)
// See also: <SfxBindings::AddDoneHdl(const Link &)>

// called from SfxTemplateControllerItem::StateChangedAtToolBoxControl
void SfxCommonTemplateDialog_Impl::SetFamilyState( sal_uInt16 nSlotId, const SfxTemplateItem* pItem )
{
    m_aStyleList.SetFamilyState(nSlotId, pItem);
    // this is the only place SfxCommonTemplateDialog_Impl::bUpdate is set true
    // StyleList::Notify - case SfxHintId::UpdateDone is the only place it is set false
    SetNotifyupdate(true);
}

// Internal: Perform functions through the Dispatcher
bool SfxCommonTemplateDialog_Impl::Execute_Impl(
    sal_uInt16 nId, const OUString &rStr, const OUString& rRefStr, sal_uInt16 nFamily, StyleList& rStyleList,
    SfxStyleSearchBits nMask, sal_uInt16 *pIdx, const sal_uInt16* pModifier)
{
    SfxDispatcher &rDispatcher = *SfxGetpApp()->GetDispatcher_Impl();
    SfxStringItem aItem(nId, rStr);
    SfxUInt16Item aFamily(SID_STYLE_FAMILY, nFamily);
    SfxUInt16Item aMask( SID_STYLE_MASK, static_cast<sal_uInt16>(nMask) );
    SfxStringItem aUpdName(SID_STYLE_UPD_BY_EX_NAME, rStr);
    SfxStringItem aRefName( SID_STYLE_REFERENCE, rRefStr );
    const SfxPoolItem* pItems[ 6 ];
    sal_uInt16 nCount = 0;
    if( !rStr.isEmpty() )
        pItems[ nCount++ ] = &aItem;
    pItems[ nCount++ ] = &aFamily;
    if( nMask != SfxStyleSearchBits::Auto )
        pItems[ nCount++ ] = &aMask;
    if(SID_STYLE_UPDATE_BY_EXAMPLE == nId)
    {
        // Special solution for Numbering update in Writer
        const OUString aTemplName(rStyleList.GetSelectedEntry());
        aUpdName.SetValue(aTemplName);
        pItems[ nCount++ ] = &aUpdName;
    }

    if ( !rRefStr.isEmpty() )
        pItems[ nCount++ ] = &aRefName;

    pItems[ nCount++ ] = nullptr;

    DeletionWatcher aDeleted(*this);
    sal_uInt16 nModi = pModifier ? *pModifier : 0;
    const SfxPoolItemHolder aResult(rDispatcher.Execute(
        nId, SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
        pItems, nModi));

    // Dialog can be destroyed while in Execute() because started
    // subdialogs are not modal to it (#i97888#).
    if (!aResult || aDeleted )
        return false;

    if ((nId == SID_STYLE_NEW || SID_STYLE_EDIT == nId)
        && rStyleList.EnableExecute())
    {
        const SfxUInt16Item* pFilterItem(dynamic_cast<const SfxUInt16Item*>(aResult.getItem()));
        assert(pFilterItem);
        SfxStyleSearchBits nFilterFlags = static_cast<SfxStyleSearchBits>(pFilterItem->GetValue()) & ~SfxStyleSearchBits::UserDefined;
        if(nFilterFlags == SfxStyleSearchBits::Auto)       // User Template?
            nFilterFlags = static_cast<SfxStyleSearchBits>(pFilterItem->GetValue());
        const SfxStyleFamilyItem *pFamilyItem = rStyleList.GetFamilyItem();
        const size_t nFilterCount = pFamilyItem->GetFilterList().size();

        for ( size_t i = 0; i < nFilterCount; ++i )
        {
            const SfxFilterTuple &rTupel = pFamilyItem->GetFilterList()[ i ];

            if ( ( rTupel.nFlags & nFilterFlags ) == nFilterFlags && pIdx )
                *pIdx = i;
        }
    }

    return true;
}

// Select-Handler for the Toolbox
// called from StyleList::Update when bFillTreeView is false, SfxTemplatePanelControl::NotifyItemUpdate,
// SfxTemplateDialog_Impl::ToolBoxLSelect, SfxCommonTemplateDialog_Impl::SetFamily
void SfxCommonTemplateDialog_Impl::FamilySelect(sal_uInt16 nEntry, bool bFillTreeView)
{
    SfxDispatcher* pDispatcher = SfxGetpApp()->GetDispatcher_Impl();
    SfxUInt16Item const aItem(SID_STYLE_FAMILY,
                              static_cast<sal_uInt16>(SfxTemplate::NIdToSfxFamilyId(nEntry)));
    pDispatcher->ExecuteList(SID_STYLE_FAMILY, SfxCallMode::SYNCHRON, { &aItem });

    // The first time SfxTemplateControllerItem::StateChangedAtToolBoxControl case SID_STYLE_FAMILY
    // calls SfxCommonTemplateDialog_Impl::SetFamily, nEntry is 0xffff.
    if (nEntry == 0xffff)
        return;

    assert(0 < nEntry && nEntry <= MAX_FAMILIES);

    CheckItem(OUString::number(m_aStyleList.m_nActFamily), false);
    CheckItem(OUString::number(nEntry), true);

    m_aStyleList.FamilySelect(nEntry, bFillTreeView);

    // update Spotlight checkbox
    SfxStyleFamily eFam = m_aStyleList.GetFamilyItem()->GetFamily();
    bool bShowSpotlightCheckbox = m_aStyleList.HasStylesSpotlightFeature()
                                  && (eFam == SfxStyleFamily::Para || eFam == SfxStyleFamily::Char);
    mxSpotlightCheckbox->set_visible(bShowSpotlightCheckbox);
    if (bShowSpotlightCheckbox)
    {
        bool bActive = false;
        if (eFam == SfxStyleFamily::Para)
            bActive = m_aStyleList.IsSpotlightParaStyles();
        else if (eFam == SfxStyleFamily::Char)
            bActive = m_aStyleList.IsSpotlightCharStyles();
        mxSpotlightCheckbox->set_active(bActive);
    }
}

void SfxCommonTemplateDialog_Impl::ActionSelect(const OUString& rEntry, StyleList& rStyleList)
{
    if (rEntry == "watercan")
    {
        const bool bOldState = !IsCheckedItem(rEntry);
        bool bCheck;
        SfxBoolItem aBool;
        // when a template is chosen.
        if (!bOldState && m_aStyleListHasSelectedStyle.Call(nullptr))
        {
            const OUString aTemplName(rStyleList.GetSelectedEntry());
            Execute_Impl(SID_STYLE_WATERCAN, aTemplName, u""_ustr,
                         static_cast<sal_uInt16>(m_aStyleList.GetFamilyItem()->GetFamily()), rStyleList);
            bCheck = true;
        }
        else
        {
            Execute_Impl(SID_STYLE_WATERCAN, u""_ustr, u""_ustr, 0, rStyleList);
            bCheck = false;
        }
        CheckItem(rEntry, bCheck);
        aBool.SetValue(bCheck);
        SetWaterCanState(&aBool);
    }
    else if (rEntry == "new" || rEntry == "newmenu")
    {
        m_aStyleListNewMenu.Call(nullptr);
    }
    else if (rEntry == "update")
    {
        Execute_Impl(SID_STYLE_UPDATE_BY_EXAMPLE,
                u""_ustr, u""_ustr,
                static_cast<sal_uInt16>(m_aStyleList.GetFamilyItem()->GetFamily()), rStyleList);
    }
    else if (rEntry == "load")
    {
        SfxViewFrame* pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
        SfxFrameItem aFrameItem(SID_DOCFRAME, pViewFrame);
        SfxGetpApp()->GetDispatcher_Impl()->ExecuteList(SID_TEMPLATE_LOAD, SfxCallMode::SLOT, { &aFrameItem });
    }
    else
        SAL_WARN("sfx", "not implemented: " << rEntry);
}

static OUString getModuleIdentifier( const Reference< XModuleManager2 >& i_xModMgr, SfxObjectShell const * i_pObjSh )
{
    assert(i_xModMgr.is() && "getModuleIdentifier(): no XModuleManager");
    assert(i_pObjSh && "getModuleIdentifier(): no ObjectShell");

    OUString sIdentifier;

    try
    {
        sIdentifier = i_xModMgr->identify( i_pObjSh->GetModel() );
    }
    catch ( css::frame::UnknownModuleException& )
    {
        SAL_WARN("sfx", "getModuleIdentifier(): unknown module" );
    }
    catch ( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sfx", "getModuleIdentifier(): exception of XModuleManager::identify()" );
    }

    return sIdentifier;
}

// called from StyleList::Update
void SfxCommonTemplateDialog_Impl::LoadFactoryStyleFilter(SfxObjectShell* pObjectShell)
{
    OSL_ENSURE(pObjectShell,
               "SfxCommonTemplateDialog_Impl::LoadFactoryStyleFilter(): no ObjectShell");

    ::comphelper::SequenceAsHashMap aFactoryProps(
        xModuleManager->getByName(getModuleIdentifier(xModuleManager, pObjectShell)));
    OUString sIndexes
        = aFactoryProps.getUnpackedValueOrDefault(u"ooSetupFactoryStyleFilter"_ustr, OUString());

    auto vStyleIndexes = comphelper::string::split(sIndexes, ',');

    // vStyleIndexes[0].toInt32() != 0xffff condition is for legacy purpose
    if (!vStyleIndexes.empty() && vStyleIndexes[0].toInt32() != 0xffff)
    {
        m_aStyleList.m_bHierarchical = (vStyleIndexes[0].toUInt32() & 0x1000) != 0;
        vStyleIndexes[0] = OUString::number(vStyleIndexes[0].toUInt32() & ~0x1000);
    }
    else
    {
        m_aStyleList.m_bHierarchical = true;
        vStyleIndexes[0] = "0"; // All Visible Styles
    }

    // fill active family selected filters set
    if (SfxStyleFamilyItem* pFamilyItem = m_aStyleList.GetFamilyItem())
    {
        auto& rFamilySelectedFiltersSet
            = m_aStyleList.m_aFamilySelectedFiltersSet[pFamilyItem->GetFamily()];
        rFamilySelectedFiltersSet.clear();
        size_t nFilterListSize = pFamilyItem->GetFilterList().size();
        for (const auto& rsIndex : vStyleIndexes)
        {
            if (rsIndex.toUInt32() >= nFilterListSize)
                continue;
            const SfxFilterTuple& rFilterTuple = pFamilyItem->GetFilterList()[rsIndex.toUInt32()];
            rFamilySelectedFiltersSet.insert(rFilterTuple.nFlags);
        }
    }

    m_aStyleList.m_aFamilyExpandedStyleEntriesSet.clear();
    m_aStyleList.m_aFamiliesThatHaveBeenSelectedSet.clear();
}

void SfxCommonTemplateDialog_Impl::SaveFactoryStyleFilter()
{
    SfxViewFrame* pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
    SfxObjectShell* pDocShell = pViewFrame->GetObjectShell();
    if (!pDocShell)
        return;

    OUString sIndexes;
    sal_uInt16 nIndex = 0;
    for (SfxStyleFamilyItem& rFamilyItem : m_aStyleList.m_aStyleFamilies)
    {
        for (const SfxFilterTuple& rFilterTuple : rFamilyItem.GetFilterList())
        {
            if (m_aStyleList.m_aFamilySelectedFiltersSet[rFamilyItem.GetFamily()].contains(
                    rFilterTuple.nFlags))
            {
                if (!sIndexes.isEmpty())
                    sIndexes += "," + OUString::number(nIndex);
                else
                {
                    // store hierarchical flag in first index
                    sIndexes
                        += OUString::number(nIndex | (m_aStyleList.m_bHierarchical ? 0x1000 : 0));
                }
            }
            nIndex++;
        }
    }

    Sequence<PropertyValue> lProps{ comphelper::makePropertyValue(u"ooSetupFactoryStyleFilter"_ustr,
                                                                  sIndexes) };
    xModuleManager->replaceByName(getModuleIdentifier(xModuleManager, pDocShell), Any(lProps));
}

IMPL_LINK_NOARG(SfxCommonTemplateDialog_Impl, HierarchicalHdl, weld::Toggleable&, void)
{
   m_aStyleList.m_bHierarchical = !m_aStyleList.m_bHierarchical;
   m_aStyleList.m_bHierarchical ? m_aStyleList.ShowHierarchicalView() : m_aStyleList.ShowFlatView();
   SaveFactoryStyleFilter();
}

IMPL_LINK_NOARG(SfxCommonTemplateDialog_Impl, PreviewHdl, weld::Toggleable&, void)
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch( comphelper::ConfigurationChanges::create() );
    bool bCustomPreview = mxPreviewCheckbox->get_active();
    officecfg::Office::Common::StylesAndFormatting::Preview::set(bCustomPreview, batch );
    batch->commit();

    m_aStyleList.ShowPreviews(bCustomPreview);
}

IMPL_LINK_NOARG(SfxCommonTemplateDialog_Impl, SpotlightHdl, weld::Toggleable&, void)
{
    SfxDispatcher &rDispatcher = *SfxGetpApp()->GetDispatcher_Impl();
    SfxStyleFamily eFam = SfxTemplate::NIdToSfxFamilyId(m_aStyleList.m_nActFamily);
    if (eFam == SfxStyleFamily::Para)
       rDispatcher.Execute(SID_SPOTLIGHT_PARASTYLES, SfxCallMode::SYNCHRON);
    else if (eFam == SfxStyleFamily::Char)
       rDispatcher.Execute(SID_SPOTLIGHT_CHARSTYLES, SfxCallMode::SYNCHRON);
}

IMPL_LINK_NOARG(SfxCommonTemplateDialog_Impl, UpdateStyleDependents_Hdl, void*, void)
{
    m_aStyleListUpdateStyleDependents.Call(nullptr);
    EnableItem(u"watercan"_ustr, !bWaterDisabled);
    m_aStyleListEnableDelete.Call(nullptr);
}

void SfxCommonTemplateDialog_Impl::EnableExample_Impl(sal_uInt16 nId, bool bEnable)
{
    bool bDisable = !bEnable || !IsSafeForWaterCan();
    if (nId == SID_STYLE_NEW_BY_EXAMPLE)
    {
        bNewByExampleDisabled = bDisable;
        m_aStyleList.EnableNewByExample(bNewByExampleDisabled);
        EnableItem(u"new"_ustr, bEnable);
        EnableItem(u"newmenu"_ustr, bEnable);
    }
    else if( nId == SID_STYLE_UPDATE_BY_EXAMPLE )
    {
        bUpdateByExampleDisabled = bDisable;
        EnableItem(u"update"_ustr, bEnable);
    }
}

class ToolbarDropTarget final : public DropTargetHelper
{
private:
    SfxTemplateDialog_Impl& m_rParent;

public:
    ToolbarDropTarget(SfxTemplateDialog_Impl& rDialog, weld::Toolbar& rToolbar)
        : DropTargetHelper(rToolbar.get_drop_target())
        , m_rParent(rDialog)
    {
    }

    virtual sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt) override
    {
        return m_rParent.AcceptToolbarDrop(rEvt, *this);
    }

    virtual sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt) override
    {
        return m_rParent.ExecuteDrop(rEvt);
    }
};

SfxTemplateDialog_Impl::SfxTemplateDialog_Impl(SfxBindings* pB, SfxTemplatePanelControl* pDlgWindow)
    : SfxCommonTemplateDialog_Impl(pB, pDlgWindow->get_container(), pDlgWindow->get_builder())
    , m_xActionTbL(pDlgWindow->get_builder()->weld_toolbar(u"left"_ustr))
    , m_xActionTbR(pDlgWindow->get_builder()->weld_toolbar(u"right"_ustr))
    , m_xToolMenu(pDlgWindow->get_builder()->weld_menu(u"toolmenu"_ustr))
    , m_nActionTbLVisible(0)
{
    m_xActionTbR->set_item_help_id(u"watercan"_ustr, HID_TEMPLDLG_WATERCAN);
    // shown/hidden in SfxTemplateDialog_Impl::ReplaceUpdateButtonByMenu()
    m_xActionTbR->set_item_help_id(u"new"_ustr, HID_TEMPLDLG_NEWBYEXAMPLE);
    m_xActionTbR->set_item_help_id(u"newmenu"_ustr, HID_TEMPLDLG_NEWBYEXAMPLE);
    m_xActionTbR->set_item_help_id(u"update"_ustr, HID_TEMPLDLG_UPDATEBYEXAMPLE);

    // Features not working in LOK yet
    if (comphelper::LibreOfficeKit::isActive())
    {
        m_xActionTbR->set_item_visible(u"watercan"_ustr, false);
        m_xActionTbR->set_item_visible(u"newmenu"_ustr, false);
        m_xActionTbR->set_item_visible(u"update"_ustr, false);
    }
    else
    {
        m_xActionTbR->set_item_menu(u"newmenu"_ustr, m_xToolMenu.get());
        m_xToolMenu->connect_activate(LINK(this, SfxTemplateDialog_Impl, ToolMenuSelectHdl));
    }

    Initialize();
}

void SfxTemplateDialog_Impl::Initialize()
{
    SfxCommonTemplateDialog_Impl::Initialize();

    m_xActionTbL->connect_clicked(LINK(this, SfxTemplateDialog_Impl, ToolBoxLSelect)); // Style families toolbar
    m_xActionTbR->connect_clicked(LINK(this, SfxTemplateDialog_Impl, ToolBoxRSelect));
    m_xActionTbL->set_help_id(HID_TEMPLDLG_TOOLBOX_LEFT);

    m_xToolbarDropTargetHelper.reset(new ToolbarDropTarget(*this, *m_xActionTbL));
}

void SfxTemplateDialog_Impl::EnableFamilyItem(sal_uInt16 nId, bool bEnable)
{
    m_xActionTbL->set_item_sensitive(OUString::number(nId), bEnable);
}

// Family toolbar button settings
// called from SfxCommonTemplateDialog_Impl::ReadResource
void SfxTemplateDialog_Impl::InsertFamilyItem(sal_uInt16 nId, const SfxStyleFamilyItem &rItem)
{
    OUString sHelpId;
    switch( rItem.GetFamily() )
    {
        case SfxStyleFamily::Char:     sHelpId = ".uno:CharStyle"; break;
        case SfxStyleFamily::Para:     sHelpId = ".uno:ParaStyle"; break;
        case SfxStyleFamily::Frame:    sHelpId = ".uno:FrameStyle"; break;
        case SfxStyleFamily::Page:     sHelpId = ".uno:PageStyle"; break;
        case SfxStyleFamily::Pseudo:   sHelpId = ".uno:ListStyle"; break;
        case SfxStyleFamily::Table:    sHelpId = ".uno:TableStyle"; break;
        default: OSL_FAIL("unknown StyleFamily"); break;
    }

    OUString sId(OUString::number(nId));
    m_xActionTbL->set_item_visible(sId, true);
    m_xActionTbL->set_item_icon_name(sId, rItem.GetImage());
    m_xActionTbL->set_item_tooltip_text(sId, rItem.GetText());
    m_xActionTbL->set_item_accessible_name(sId, rItem.GetText());
    m_xActionTbL->set_item_help_id(sId, sHelpId);
    ++m_nActionTbLVisible;
}

void SfxTemplateDialog_Impl::ReplaceUpdateButtonByMenu()
{
    // in LOK we don't support all options yet
    if (comphelper::LibreOfficeKit::isActive())
        return;

    m_xActionTbR->set_item_visible(u"update"_ustr, false);
    m_xActionTbR->set_item_visible(u"new"_ustr, false);
    m_xActionTbR->set_item_visible(u"newmenu"_ustr, true);
    FillToolMenu();
}

void SfxTemplateDialog_Impl::ClearFamilyList()
{
    for (int i = 0, nCount = m_xActionTbL->get_n_items(); i < nCount; ++i)
        m_xActionTbL->set_item_visible(m_xActionTbL->get_item_ident(i), false);

}

SfxTemplateDialog_Impl::~SfxTemplateDialog_Impl()
{
    m_xToolbarDropTargetHelper.reset();
    m_xActionTbL.reset();
    m_xActionTbR.reset();
}

void SfxTemplateDialog_Impl::EnableItem(const OUString& rMesId, bool bCheck)
{
    if (rMesId == "watercan" && !bCheck && IsCheckedItem(u"watercan"_ustr))
        Execute_Impl(SID_STYLE_WATERCAN, u""_ustr, u""_ustr, 0, m_aStyleList);
    m_xActionTbR->set_item_sensitive(rMesId, bCheck);
}

void SfxTemplateDialog_Impl::CheckItem(const OUString &rMesId, bool bCheck)
{
    if (rMesId == "watercan")
    {
        bIsWater=bCheck;
        m_xActionTbR->set_item_active(u"watercan"_ustr, bCheck);
    }
    else
        m_xActionTbL->set_item_active(rMesId, bCheck);
}

bool SfxTemplateDialog_Impl::IsCheckedItem(const OUString& rMesId)
{
    if (rMesId == "watercan")
        return m_xActionTbR->get_item_active(u"watercan"_ustr);
    return m_xActionTbL->get_item_active(rMesId);
}

IMPL_LINK( SfxTemplateDialog_Impl, ToolBoxLSelect, const OUString&, rEntry, void)
{
    FamilySelect(rEntry.toUInt32());
}

IMPL_LINK(SfxTemplateDialog_Impl, ToolBoxRSelect, const OUString&, rEntry, void)
{
    if (rEntry == "newmenu")
        m_xActionTbR->set_menu_item_active(rEntry, !m_xActionTbR->get_menu_item_active(rEntry));
    else
        ActionSelect(rEntry, m_aStyleList);
}

void SfxTemplateDialog_Impl::FillToolMenu()
{
    //create a popup menu in Writer
    OUString sTextDoc(u"com.sun.star.text.TextDocument"_ustr);

    auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(u".uno:StyleNewByExample"_ustr, sTextDoc);
    OUString sLabel = vcl::CommandInfoProvider::GetPopupLabelForCommand(aProperties);
    m_xToolMenu->append(u"new"_ustr, sLabel);
    aProperties = vcl::CommandInfoProvider::GetCommandProperties(u".uno:StyleUpdateByExample"_ustr, sTextDoc);
    sLabel = vcl::CommandInfoProvider::GetPopupLabelForCommand(aProperties);
    m_xToolMenu->append(u"update"_ustr, sLabel);
    m_xToolMenu->append_separator(u"separator"_ustr);

    aProperties = vcl::CommandInfoProvider::GetCommandProperties(u".uno:LoadStyles"_ustr, sTextDoc);
    sLabel = vcl::CommandInfoProvider::GetPopupLabelForCommand(aProperties);
    m_xToolMenu->append(u"load"_ustr, sLabel);
}

IMPL_LINK(SfxTemplateDialog_Impl, ToolMenuSelectHdl, const OUString&, rMenuId, void)
{
    if (rMenuId.isEmpty())
        return;
    ActionSelect(rMenuId, m_aStyleList);
}

// called from SfxTemplateControllerItem::StateChangedAtToolBoxControl
void SfxCommonTemplateDialog_Impl::SetFamily(SfxStyleFamily const nFamily)
{
    sal_uInt16 const nId(SfxTemplate::SfxFamilyIdToNId(nFamily));
    assert((0 < nId && nId <= MAX_FAMILIES) || 0xffff == nId);
    if (nId != m_aStyleList.m_nActFamily)
    {
        m_aStyleList.m_nActFamily = nId;
        FamilySelect(nId);
    }
}

// called from StyleList::NewMenuExecuteAction
IMPL_LINK(SfxCommonTemplateDialog_Impl, UpdateFamily_Hdl, StyleList&, rStyleList, void)
{
    bWaterDisabled = false;
    bUpdateByExampleDisabled = false;

    if (IsCheckedItem(u"watercan"_ustr) &&
        // only if that area is allowed
        rStyleList.CurrentFamilyHasState())
    {
        Execute_Impl(SID_STYLE_APPLY, rStyleList.GetSelectedEntry(), OUString(),
                     static_cast<sal_uInt16>(rStyleList.GetFamilyItem()->GetFamily()), rStyleList);
    }
}

void SfxCommonTemplateDialog_Impl::ReplaceUpdateButtonByMenu()
{
    //does nothing
}

sal_Int8 SfxTemplateDialog_Impl::AcceptToolbarDrop(const AcceptDropEvent& rEvt, const DropTargetHelper& rHelper)
{
    sal_Int8 nReturn = DND_ACTION_NONE;

    // auto flip to the category under the mouse
    int nIndex = m_xActionTbL->get_drop_index(rEvt.maPosPixel);
    if (nIndex >= m_nActionTbLVisible)
        nIndex = m_nActionTbLVisible - 1;

    OUString sIdent = m_xActionTbL->get_item_ident(nIndex);
    if (!sIdent.isEmpty() && !m_xActionTbL->get_item_active(sIdent))
        ToolBoxLSelect(sIdent);

    // special case: page styles are allowed to create new styles by example
    // but not allowed to be created by drag and drop
    if (sIdent.toUInt32() != SfxTemplate::SfxFamilyIdToNId(SfxStyleFamily::Page) &&
        rHelper.IsDropFormatSupported(SotClipboardFormatId::OBJECTDESCRIPTOR) &&
        !bNewByExampleDisabled)
    {
        nReturn = DND_ACTION_COPY;
    }
    return nReturn;
}

void SfxCommonTemplateDialog_Impl::EnableEdit(bool b, StyleList* rStyleList)
{
    if (rStyleList == &m_aStyleList || rStyleList == nullptr)
        m_aStyleList.Enableedit(b);
}
void SfxCommonTemplateDialog_Impl::EnableDel(bool b, const StyleList* rStyleList)
{
    if (rStyleList == &m_aStyleList || rStyleList == nullptr)
        m_aStyleList.Enabledel(b);
}
void SfxCommonTemplateDialog_Impl::EnableNew(bool b, const StyleList* rStyleList)
{
    if (rStyleList == &m_aStyleList || rStyleList == nullptr)
        m_aStyleList.Enablenew(b);
}
void SfxCommonTemplateDialog_Impl::EnableHide(bool b, const StyleList* rStyleList)
{
    if (rStyleList == &m_aStyleList || rStyleList == nullptr)
        m_aStyleList.Enablehide(b);
}
void SfxCommonTemplateDialog_Impl::EnableShow(bool b, const StyleList* rStyleList)
{
    if (rStyleList == &m_aStyleList || rStyleList == nullptr)
        m_aStyleList.Enableshow(b);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
