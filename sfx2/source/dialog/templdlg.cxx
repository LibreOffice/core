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
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/templdlg.hxx>
#include <templdgi.hxx>
#include <sfx2/styfitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/tplpitem.hxx>
#include <sfx2/sfxresid.hxx>

#include <sfx2/sfxsids.hrc>
#include <sfx2/strings.hrc>
#include <helpids.h>
#include <sfx2/viewfrm.hxx>

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
        ActionSelect("new", m_aStyleList);
}

SfxTemplatePanelControl::SfxTemplatePanelControl(SfxBindings* pBindings, weld::Widget* pParent)
    : PanelLayout(pParent, "TemplatePanel", "sfx/ui/templatepanel.ui")
    , pImpl(new SfxTemplateDialog_Impl(pBindings, this))
{
    OSL_ASSERT(pBindings!=nullptr);
}

SfxTemplatePanelControl::~SfxTemplatePanelControl()
{
}

namespace SfxTemplate
{
    // converts from SFX_STYLE_FAMILY Ids to 1-6
    static sal_uInt16 SfxFamilyIdToNId(SfxStyleFamily nFamily)
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

// Constructor

SfxCommonTemplateDialog_Impl::SfxCommonTemplateDialog_Impl(SfxBindings* pB, weld::Container* pC, weld::Builder* pBuilder)
    : pBindings(pB)
    , mpContainer(pC)
    , xModuleManager(frame::ModuleManager::create(::comphelper::getProcessComponentContext()))
    , m_pDeletionWatcher(nullptr)
    , m_aStyleList(pBuilder, pB, this, pC, "treeview", "flatview")
    , mxPreviewCheckbox(pBuilder->weld_check_button("showpreview"))
    , mxFilterLb(pBuilder->weld_combo_box("filter"))
    , nActFamily(0xffff)
    , nActFilter(0)
    , bIsWater(false)
    , bUpdate(false)
    , bWaterDisabled(false)
    , bNewByExampleDisabled(false)
    , bUpdateByExampleDisabled(false)
    , m_bWantHierarchical(false)
{
    mxFilterLb->set_help_id(HID_TEMPLATE_FILTER);
    mxPreviewCheckbox->set_active(officecfg::Office::Common::StylesAndFormatting::Preview::get());
}

void SfxTemplateDialog_Impl::EnableEdit(bool bEnable, StyleList* rStyleList)
{
    if(rStyleList == &m_aStyleList || rStyleList == nullptr)
        SfxCommonTemplateDialog_Impl::EnableEdit( bEnable, &m_aStyleList );
    if( !bEnable || !bUpdateByExampleDisabled )
        EnableItem("update", bEnable);
}

IMPL_LINK(SfxCommonTemplateDialog_Impl, ReadResource_Hdl, StyleList&, rStyleList, void)
{
    nActFilter = 0xffff;

    SfxViewFrame* pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
    SfxObjectShell* pCurObjShell = pViewFrame->GetObjectShell();
    if (pCurObjShell)
    {
        nActFilter = static_cast<sal_uInt16>(LoadFactoryStyleFilter_Hdl(pCurObjShell));
        if (0xffff == nActFilter)
        {
            nActFilter = pCurObjShell->GetAutoStyleFilterIndex();
        }
    }

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

    for( ; nCount--; )
    {
        const SfxStyleFamilyItem &rItem = rStyleList.GetFamilyItemByIndex( nCount );
        sal_uInt16 nId = SfxTemplate::SfxFamilyIdToNId( rItem.GetFamily() );
        InsertFamilyItem(nId, rItem);
    }
}

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
    m_aStyleList.connect_LoadFactoryStyleFilter(LINK(this, SfxCommonTemplateDialog_Impl, LoadFactoryStyleFilter_Hdl));
    m_aStyleList.connect_SaveSelection(LINK(this, SfxCommonTemplateDialog_Impl, SaveSelection_Hdl));
    m_aStyleList.connect_UpdateFamily(LINK(this, SfxCommonTemplateDialog_Impl, UpdateFamily_Hdl));
    m_aStyleList.connect_UpdateStyles(LINK(this, SfxCommonTemplateDialog_Impl, UpdateStyles_Hdl));

    mxFilterLb->connect_changed(LINK(this, SfxCommonTemplateDialog_Impl, FilterSelectHdl));
    mxPreviewCheckbox->connect_toggled(LINK(this, SfxCommonTemplateDialog_Impl, PreviewHdl));
    m_aStyleList.Initialize();
}

IMPL_LINK(SfxCommonTemplateDialog_Impl, UpdateStyles_Hdl, StyleFlags, nFlags, void)
{
    const SfxStyleFamilyItem* pItem = m_aStyleList.GetFamilyItem();

    if (nFlags & StyleFlags::UpdateFamily) // Update view type list (Hierarchical, All, etc.
    {
        CheckItem(OString::number(nActFamily)); // check Button in Toolbox

        mxFilterLb->freeze();
        mxFilterLb->clear();

        //insert hierarchical at the beginning
        mxFilterLb->append(OUString::number(static_cast<int>(SfxStyleSearchBits::All)),
                           SfxResId(STR_STYLE_FILTER_HIERARCHICAL));
        const SfxStyleFilter& rFilter = pItem->GetFilterList();
        for (const SfxFilterTuple& i : rFilter)
            mxFilterLb->append(OUString::number(static_cast<int>(i.nFlags)), i.aName);
        mxFilterLb->thaw();

        if (nActFilter < mxFilterLb->get_count() - 1)
            mxFilterLb->set_active(nActFilter + 1);
        else
        {
            nActFilter = 0;
            m_aStyleList.FilterSelect(nActFilter, false);
            mxFilterLb->set_active(1);
        }

        // if the tree view again, select family hierarchy
        if (m_aStyleList.IsTreeView() || m_bWantHierarchical)
        {
            mxFilterLb->set_active_text(SfxResId(STR_STYLE_FILTER_HIERARCHICAL));
            EnableHierarchical(true, m_aStyleList);
        }
    }
    else
    {
        if (nActFilter < mxFilterLb->get_count() - 1)
            mxFilterLb->set_active(nActFilter + 1);
        else
        {
            nActFilter = 0;
            m_aStyleList.FilterSelect(nActFilter, false);
            mxFilterLb->set_active(1);
        }
    }

    if (!(nFlags & StyleFlags::UpdateFamilyList))
        return;

    EnableItem("watercan", false);
}

SfxCommonTemplateDialog_Impl::~SfxCommonTemplateDialog_Impl()
{
    if ( bIsWater )
        Execute_Impl(SID_STYLE_WATERCAN, "", "", 0, m_aStyleList);
    m_aStyleListClear.Call(nullptr);
    m_aStyleListCleanup.Call(nullptr);
    if ( m_pDeletionWatcher )
        m_pDeletionWatcher->signal();
    mxPreviewCheckbox.reset();
    mxFilterLb.reset();
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

void SfxCommonTemplateDialog_Impl::SelectStyle(const OUString &rStr, bool bIsCallback, StyleList& rStyleList)
{
    rStyleList.SelectStyle(rStr, bIsCallback);

    bWaterDisabled = !IsSafeForWaterCan();

    // tdf#134598 call UpdateStyleDependents to update watercan
    UpdateStyleDependents_Hdl(nullptr);
}

void SfxCommonTemplateDialog_Impl::EnableTreeDrag(bool bEnable)
{
    m_aStyleListEnableTreeDrag.Call(bEnable);
}

// Updated display: Watering the house
void SfxCommonTemplateDialog_Impl::SetWaterCanState(const SfxBoolItem *pItem)
{
    bWaterDisabled = (pItem == nullptr);

    if(!bWaterDisabled)
        //make sure the watercan is only activated when there is (only) one selection
        bWaterDisabled = !IsSafeForWaterCan();

    if(pItem && !bWaterDisabled)
    {
        CheckItem("watercan", pItem->GetValue());
        EnableItem("watercan");
    }
    else
    {
        if(!bWaterDisabled)
            EnableItem("watercan");
        else
            EnableItem("watercan", false);
    }

// Ignore while in watercan mode statusupdates

    m_aStyleListSetWaterCanState.Call(pItem);
}

// Item with the status of a Family is copied and noted
// (is updated when all states have also been updated.)
// See also: <SfxBindings::AddDoneHdl(const Link &)>
void SfxCommonTemplateDialog_Impl::SetFamilyState( sal_uInt16 nSlotId, const SfxTemplateItem* pItem )
{
    m_aStyleList.SetFamilyState(nSlotId, pItem);
    bUpdate = true;
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
    const SfxPoolItem* pItem = rDispatcher.Execute(
        nId, SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
        pItems, nModi );

    // Dialog can be destroyed while in Execute() because started
    // subdialogs are not modal to it (#i97888#).
    if ( !pItem || aDeleted )
        return false;

    if ((nId == SID_STYLE_NEW || SID_STYLE_EDIT == nId)
        && rStyleList.EnableExecute())
    {
        const SfxUInt16Item *pFilterItem = dynamic_cast< const SfxUInt16Item* >(pItem);
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

// Handler Listbox of Filter
void SfxCommonTemplateDialog_Impl::EnableHierarchical(bool const bEnable, StyleList& rStyleList)
{
    if (bEnable)
    {
        if (!rStyleList.IsHierarchical())
        {
            // Turn on treeView
            m_bWantHierarchical = true;
            SaveSelection_Hdl(rStyleList); // fdo#61429 store "hierarchical"
            m_aStyleList.SetHierarchical();
        }
    }
    else
    {
        m_aStyleList.SetFilterControlsHandle();
        // If bHierarchical, then the family can have changed
        // minus one since hierarchical is inserted at the start
        m_bWantHierarchical = false; // before FilterSelect
        FilterSelect(mxFilterLb->get_active() - 1, rStyleList.IsHierarchical() );
    }
}

// Other filters; can be switched by the users or as a result of new or
// editing, if the current document has been assigned a different filter.
void SfxCommonTemplateDialog_Impl::FilterSelect(
    sal_uInt16 nEntry, // Idx of the new Filters
    bool bForce) // Force update, even if the new filter is equal to the current
{
    if (nEntry == nActFilter && !bForce)
        return;

    nActFilter = nEntry;
    m_aStyleList.FilterSelect(nActFilter, true);
}

void SfxCommonTemplateDialog_Impl::IsUpdate(StyleList&)
{
    SfxViewFrame* pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
    SfxObjectShell* pDocShell = pViewFrame->GetObjectShell();
    nActFilter = static_cast<sal_uInt16>(LoadFactoryStyleFilter_Hdl(pDocShell));
    if (0xffff == nActFilter)
    {
        nActFilter = pDocShell->GetAutoStyleFilterIndex();
    }
}

IMPL_LINK(SfxCommonTemplateDialog_Impl, FilterSelectHdl, weld::ComboBox&, rBox, void)
{
    if (SfxResId(STR_STYLE_FILTER_HIERARCHICAL) == rBox.get_active_text())
    {
        EnableHierarchical(true, m_aStyleList);
    }
    else
    {
        EnableHierarchical(false, m_aStyleList);
    }
}

// Select-Handler for the Toolbox
void SfxCommonTemplateDialog_Impl::FamilySelect(sal_uInt16 nEntry, StyleList&, bool bPreviewRefresh)
{
    assert((0 < nEntry && nEntry <= MAX_FAMILIES) || 0xffff == nEntry);
    if( nEntry != nActFamily || bPreviewRefresh )
    {
        CheckItem(OString::number(nActFamily), false);
        nActFamily = nEntry;
        m_aStyleList.FamilySelect(nEntry);
    }
}

void SfxCommonTemplateDialog_Impl::ActionSelect(const OString& rEntry, StyleList& rStyleList)
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
            Execute_Impl(SID_STYLE_WATERCAN, aTemplName, "",
                         static_cast<sal_uInt16>(m_aStyleList.GetFamilyItem()->GetFamily()), rStyleList);
            bCheck = true;
        }
        else
        {
            Execute_Impl(SID_STYLE_WATERCAN, "", "", 0, rStyleList);
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
                "", "",
                static_cast<sal_uInt16>(m_aStyleList.GetFamilyItem()->GetFamily()), rStyleList);
    }
    else if (rEntry == "load")
        SfxGetpApp()->GetDispatcher_Impl()->Execute(SID_TEMPLATE_LOAD);
    else
        SAL_WARN("sfx", "not implemented: " << rEntry);
}

static OUString getModuleIdentifier( const Reference< XModuleManager2 >& i_xModMgr, SfxObjectShell const * i_pObjSh )
{
    OSL_ENSURE( i_xModMgr.is(), "getModuleIdentifier(): no XModuleManager" );
    OSL_ENSURE( i_pObjSh, "getModuleIdentifier(): no ObjectShell" );

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

IMPL_LINK(SfxCommonTemplateDialog_Impl, LoadFactoryStyleFilter_Hdl, SfxObjectShell const*, i_pObjSh, sal_Int32)
{
    OSL_ENSURE( i_pObjSh, "SfxCommonTemplateDialog_Impl::LoadFactoryStyleFilter(): no ObjectShell" );

    ::comphelper::SequenceAsHashMap aFactoryProps(
        xModuleManager->getByName( getModuleIdentifier( xModuleManager, i_pObjSh ) ) );
    sal_Int32 nFilter = aFactoryProps.getUnpackedValueOrDefault( "ooSetupFactoryStyleFilter", sal_Int32(-1) );

    m_bWantHierarchical = (nFilter & 0x1000) != 0;
    nFilter &= ~0x1000; // clear it

    return nFilter;
}

void SfxCommonTemplateDialog_Impl::SaveFactoryStyleFilter( SfxObjectShell const * i_pObjSh, sal_Int32 i_nFilter )
{
    OSL_ENSURE( i_pObjSh, "SfxCommonTemplateDialog_Impl::LoadFactoryStyleFilter(): no ObjectShell" );
    Sequence< PropertyValue > lProps{ comphelper::makePropertyValue(
        "ooSetupFactoryStyleFilter", i_nFilter | (m_bWantHierarchical ? 0x1000 : 0)) };
    xModuleManager->replaceByName( getModuleIdentifier( xModuleManager, i_pObjSh ), Any( lProps ) );
}

IMPL_LINK_NOARG(SfxCommonTemplateDialog_Impl, SaveSelection_Hdl, StyleList&, SfxObjectShell*)
{
    SfxViewFrame *const pViewFrame(pBindings->GetDispatcher_Impl()->GetFrame());
    SfxObjectShell *const pDocShell(pViewFrame->GetObjectShell());
    if (pDocShell)
    {
        pDocShell->SetAutoStyleFilterIndex(nActFilter);
        SaveFactoryStyleFilter( pDocShell, nActFilter );
    }
    return pDocShell;
}

IMPL_LINK_NOARG(SfxCommonTemplateDialog_Impl, PreviewHdl, weld::Toggleable&, void)
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch( comphelper::ConfigurationChanges::create() );
    bool bCustomPreview = mxPreviewCheckbox->get_active();
    officecfg::Office::Common::StylesAndFormatting::Preview::set(bCustomPreview, batch );
    batch->commit();

    m_aStyleList.EnablePreview(bCustomPreview);

    FamilySelect(nActFamily, m_aStyleList, true);
}

IMPL_LINK_NOARG(SfxCommonTemplateDialog_Impl, UpdateStyleDependents_Hdl, void*, void)
{
    m_aStyleListUpdateStyleDependents.Call(nullptr);
    EnableItem("watercan", !bWaterDisabled);
    m_aStyleListEnableDelete.Call(nullptr);
}

void SfxCommonTemplateDialog_Impl::EnableExample_Impl(sal_uInt16 nId, bool bEnable)
{
    bool bDisable = !bEnable || !IsSafeForWaterCan();
    if (nId == SID_STYLE_NEW_BY_EXAMPLE)
    {
        bNewByExampleDisabled = bDisable;
        m_aStyleList.EnableNewByExample(bNewByExampleDisabled);
        EnableItem("new", bEnable);
        EnableItem("newmenu", bEnable);
    }
    else if( nId == SID_STYLE_UPDATE_BY_EXAMPLE )
    {
        bUpdateByExampleDisabled = bDisable;
        EnableItem("update", bEnable);
    }
}

SfxTemplateDialog_Impl::SfxTemplateDialog_Impl(SfxBindings* pB, SfxTemplatePanelControl* pDlgWindow)
    : SfxCommonTemplateDialog_Impl(pB, pDlgWindow->get_container(), pDlgWindow->get_builder())
    , m_xActionTbL(pDlgWindow->get_builder()->weld_toolbar("left"))
    , m_xActionTbR(pDlgWindow->get_builder()->weld_toolbar("right"))
    , m_xToolMenu(pDlgWindow->get_builder()->weld_menu("toolmenu"))
    , m_nActionTbLVisible(0)
{
    m_xActionTbR->set_item_help_id("watercan", HID_TEMPLDLG_WATERCAN);
    // shown/hidden in SfxTemplateDialog_Impl::ReplaceUpdateButtonByMenu()
    m_xActionTbR->set_item_help_id("new", HID_TEMPLDLG_NEWBYEXAMPLE);
    m_xActionTbR->set_item_help_id("newmenu", HID_TEMPLDLG_NEWBYEXAMPLE);
    m_xActionTbR->set_item_menu("newmenu", m_xToolMenu.get());
    m_xToolMenu->connect_activate(LINK(this, SfxTemplateDialog_Impl, ToolMenuSelectHdl));
    m_xActionTbR->set_item_help_id("update", HID_TEMPLDLG_UPDATEBYEXAMPLE);

    Initialize();
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

void SfxTemplateDialog_Impl::Initialize()
{
    SfxCommonTemplateDialog_Impl::Initialize();

    m_xActionTbL->connect_clicked(LINK(this, SfxTemplateDialog_Impl, ToolBoxLSelect));
    m_xActionTbR->connect_clicked(LINK(this, SfxTemplateDialog_Impl, ToolBoxRSelect));
    m_xActionTbL->set_help_id(HID_TEMPLDLG_TOOLBOX_LEFT);

    m_xToolbarDropTargetHelper.reset(new ToolbarDropTarget(*this, *m_xActionTbL));
}

void SfxTemplateDialog_Impl::EnableFamilyItem(sal_uInt16 nId, bool bEnable)
{
    m_xActionTbL->set_item_sensitive(OString::number(nId), bEnable);
}

// Insert element into dropdown filter "Frame Styles", "List Styles", etc.
void SfxTemplateDialog_Impl::InsertFamilyItem(sal_uInt16 nId, const SfxStyleFamilyItem &rItem)
{
    OString sHelpId;
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

    OString sId(OString::number(nId));
    m_xActionTbL->set_item_visible(sId, true);
    m_xActionTbL->set_item_icon_name(sId, rItem.GetImage());
    m_xActionTbL->set_item_tooltip_text(sId, rItem.GetText());
    m_xActionTbL->set_item_help_id(sId, sHelpId);
    ++m_nActionTbLVisible;
}

void SfxTemplateDialog_Impl::ReplaceUpdateButtonByMenu()
{
    m_xActionTbR->set_item_visible("update", false);
    m_xActionTbR->set_item_visible("new", false);
    m_xActionTbR->set_item_visible("newmenu", true);
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

void SfxTemplateDialog_Impl::EnableItem(const OString& rMesId, bool bCheck)
{
    if (rMesId == "watercan" && !bCheck && IsCheckedItem("watercan"))
        Execute_Impl(SID_STYLE_WATERCAN, "", "", 0, m_aStyleList);
    m_xActionTbR->set_item_sensitive(rMesId, bCheck);
}

void SfxTemplateDialog_Impl::CheckItem(const OString &rMesId, bool bCheck)
{
    if (rMesId == "watercan")
    {
        bIsWater=bCheck;
        m_xActionTbR->set_item_active("watercan", bCheck);
    }
    else
        m_xActionTbL->set_item_active(rMesId, bCheck);
}

bool SfxTemplateDialog_Impl::IsCheckedItem(const OString& rMesId)
{
    if (rMesId == "watercan")
        return m_xActionTbR->get_item_active("watercan");
    return m_xActionTbL->get_item_active(rMesId);
}

IMPL_LINK( SfxTemplateDialog_Impl, ToolBoxLSelect, const OString&, rEntry, void)
{
    FamilySelect(rEntry.toUInt32(), m_aStyleList);
}

IMPL_LINK(SfxTemplateDialog_Impl, ToolBoxRSelect, const OString&, rEntry, void)
{
    if (rEntry == "newmenu")
        m_xActionTbR->set_menu_item_active(rEntry, !m_xActionTbR->get_menu_item_active(rEntry));
    else
        ActionSelect(rEntry, m_aStyleList);
}

void SfxTemplateDialog_Impl::FillToolMenu()
{
    //create a popup menu in Writer
    OUString sTextDoc("com.sun.star.text.TextDocument");

    auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(".uno:StyleNewByExample", sTextDoc);
    OUString sLabel = vcl::CommandInfoProvider::GetPopupLabelForCommand(aProperties);
    m_xToolMenu->append("new", sLabel);
    aProperties = vcl::CommandInfoProvider::GetCommandProperties(".uno:StyleUpdateByExample", sTextDoc);
    sLabel = vcl::CommandInfoProvider::GetPopupLabelForCommand(aProperties);
    m_xToolMenu->append("update", sLabel);
    m_xToolMenu->append_separator("separator");

    aProperties = vcl::CommandInfoProvider::GetCommandProperties(".uno:LoadStyles", sTextDoc);
    sLabel = vcl::CommandInfoProvider::GetPopupLabelForCommand(aProperties);
    m_xToolMenu->append("load", sLabel);
}

IMPL_LINK(SfxTemplateDialog_Impl, ToolMenuSelectHdl, const OString&, rMenuId, void)
{
    if (rMenuId.isEmpty())
        return;
    ActionSelect(rMenuId, m_aStyleList);
}

void SfxCommonTemplateDialog_Impl::SetFamily(SfxStyleFamily const nFamily)
{
    sal_uInt16 const nId(SfxTemplate::SfxFamilyIdToNId(nFamily));
    assert((0 < nId && nId <= MAX_FAMILIES) || 0xffff == nId);
    if ( nId != nActFamily )
    {
        m_aStyleListSetFamily.Call(nId);
        nActFamily = nId;
    }
}

IMPL_LINK(SfxCommonTemplateDialog_Impl, UpdateFamily_Hdl, StyleList&, rStyleList, void)
{
    bWaterDisabled = false;
    bUpdateByExampleDisabled = false;

    if (IsCheckedItem("watercan") &&
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

    OString sIdent = m_xActionTbL->get_item_ident(nIndex);
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
