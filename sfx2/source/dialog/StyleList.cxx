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
#include <unordered_map>

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <utility>
#include <vcl/commandevent.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weldutils.hxx>
#include <vcl/window.hxx>
#include <svl/intitem.hxx>
#include <svl/style.hxx>
#include <svl/itemset.hxx>
#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Common.hxx>

#include <osl/diagnose.h>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <templdgi.hxx>
#include <tplcitem.hxx>
#include <sfx2/styfitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/newstyle.hxx>
#include <sfx2/tplpitem.hxx>
#include <sfx2/sfxresid.hxx>

#include <sfx2/sfxsids.hrc>
#include <sfx2/strings.hrc>
#include <sfx2/docfac.hxx>
#include <sfx2/module.hxx>
#include <helpids.h>
#include <sfx2/viewfrm.hxx>

#include <comphelper/string.hxx>

#include <sfx2/StyleManager.hxx>
#include <sfx2/StylePreviewRenderer.hxx>

#include <StyleList.hxx>

#include <vcl/virdev.hxx>
#include <basegfx/color/bcolortools.hxx>
#include <random>

using namespace css;
using namespace css::beans;
using namespace css::frame;
using namespace css::uno;

class TreeViewDropTarget final : public DropTargetHelper
{
private:
    StyleList& m_rParent;

public:
    TreeViewDropTarget(StyleList& rStyleList, weld::TreeView& rTreeView)
        : DropTargetHelper(rTreeView.get_drop_target())
        , m_rParent(rStyleList)
    {
    }

    virtual sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt) override
    {
        return m_rParent.AcceptDrop(rEvt, *this);
    }

    virtual sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt) override
    {
        return m_rParent.ExecuteDrop(rEvt);
    }
};

namespace
{
Color ColorHash(std::u16string_view rString)
{
    constexpr auto aSaturationArray = std::to_array<sal_uInt16>({ 90, 75, 60 });
    constexpr auto aBrightnessArray = std::to_array<sal_uInt16>({ 100, 80, 60 });
    constexpr auto aTintOrShadeArray
        = std::to_array<sal_Int16>({ 1'500, 3'000, 4'500, 6'500, 7'500 });

    sal_uInt32 nStringHash = rtl_ustr_hashCode_WithLength(rString.data(), rString.length());

    // Twist the hash number with a RNG twister so we can get very different number even when the string hash
    // differs only slightly. For example "Heading 1" and "Heading 2" are very close, so we would get a color
    // that is very similar and with number quantization could result in the same color.
    std::mt19937 twister;
    twister.seed(nStringHash); // setting the hash for
    nStringHash = twister();

    double fHue = (nStringHash % 60) * 6;
    nStringHash = nStringHash / 60;

    double fSaturation = aSaturationArray[nStringHash % aSaturationArray.size()];
    nStringHash = nStringHash / aSaturationArray.size();

    double fBrightness = aBrightnessArray[nStringHash % aBrightnessArray.size()];
    nStringHash = nStringHash / aBrightnessArray.size();

    auto aColor = Color::HSBtoRGB(fHue, fSaturation, fBrightness);
    double fTintOrShade = aTintOrShadeArray[nStringHash % aTintOrShadeArray.size()];
    aColor.ApplyTintOrShade(fTintOrShade);

    return aColor;
}

// used to disallow the default character style in the styles highlighter character styles color map
std::optional<OUString> sDefaultCharStyleUIName;
}

// Constructor

StyleList::StyleList(weld::Builder* pBuilder, SfxBindings* pBindings,
                     SfxCommonTemplateDialog_Impl* Parent, weld::Container* pC,
                     OUString treeviewname, OUString flatviewname)
    : m_bHierarchical(false)
    , m_bAllowReParentDrop(false)
    , m_bNewByExampleDisabled(false)
    , m_bDontUpdate(false)
    , m_bTreeDrag(true)
    , m_bCanEdit(false)
    , m_bCanHide(true)
    , m_bCanShow(false)
    , m_bCanNew(true)
    , m_bUpdateFamily(false)
    , m_bCanDel(false)
    , m_bBindingUpdate(true)
    , m_pStyleSheetPool(nullptr)
    , m_nActFilter(0)
    , m_xFmtLb(pBuilder->weld_tree_view(flatviewname))
    , m_xTreeBox(pBuilder->weld_tree_view(treeviewname))
    , m_pCurObjShell(nullptr)
    , m_nActFamily(0xffff)
    , m_nAppFilter(SfxStyleSearchBits::Auto)
    , m_pParentDialog(Parent)
    , m_pBindings(pBindings)
    , m_Module(nullptr)
    , m_nModifier(0)
    , m_pContainer(pC)
{
    m_xFmtLb->set_help_id(HID_TEMPLATE_FMT);

    uno::Reference<frame::XFrame> xFrame
        = m_pBindings->GetDispatcher()->GetFrame()->GetFrame().GetFrameInterface();
    m_bModuleHasStylesHighlighterFeature
        = vcl::CommandInfoProvider::GetModuleIdentifier(xFrame) == "com.sun.star.text.TextDocument";
}

// Destructor

StyleList::~StyleList() {}

// Called in the destructor of Dialog
// Cleans up the StyleList individual components while closing the application
IMPL_LINK_NOARG(StyleList, Cleanup, void*, void)
{
    if (m_pStyleSheetPool)
        EndListening(*m_pStyleSheetPool);
    m_pStyleSheetPool = nullptr;
    m_xTreeView1DropTargetHelper.reset();
    m_xTreeView2DropTargetHelper.reset();
    m_xTreeBox.reset();
    m_xFmtLb.reset();
    pIdle.reset();
}

void StyleList::CreateContextMenu()
{
    if (m_bBindingUpdate)
    {
        m_pBindings->Invalidate(SID_STYLE_NEW, true);
        m_pBindings->Update(SID_STYLE_NEW);
        m_bBindingUpdate = false;
    }
    mxMenu.reset();
    mxMenuBuilder = Application::CreateBuilder(nullptr, u"sfx/ui/stylecontextmenu.ui"_ustr);
    mxMenu = mxMenuBuilder->weld_menu(u"menu"_ustr);
    mxMenu->set_sensitive(u"edit"_ustr, m_bCanEdit);
    mxMenu->set_sensitive(u"delete"_ustr, m_bCanDel);
    mxMenu->set_sensitive(u"new"_ustr, m_bCanNew);
    mxMenu->set_sensitive(u"hide"_ustr, m_bCanHide);
    mxMenu->set_sensitive(u"show"_ustr, m_bCanShow);

    const SfxStyleFamilyItem* pItem = GetFamilyItem();
    if (pItem && pItem->GetFamily() == SfxStyleFamily::Table) //tdf#101648, no ui for this yet
    {
        mxMenu->set_sensitive(u"edit"_ustr, false);
        mxMenu->set_sensitive(u"new"_ustr, false);
    }
    if (pItem && pItem->GetFamily() == SfxStyleFamily::Pseudo)
    {
        const OUString aTemplName(GetSelectedEntry());
        if (aTemplName == "No List")
        {
            mxMenu->set_sensitive(u"edit"_ustr, false);
            mxMenu->set_sensitive(u"new"_ustr, false);
            mxMenu->set_sensitive(u"hide"_ustr, false);
        }
    }
}

IMPL_LINK_NOARG(StyleList, ReadResource, void*, size_t)
{
    // Read global user resource
    for (auto& i : m_pFamilyState)
        i.reset();

    SfxViewFrame* pViewFrame = m_pBindings->GetDispatcher_Impl()->GetFrame();
    m_pCurObjShell = pViewFrame->GetObjectShell();
    m_Module = m_pCurObjShell ? m_pCurObjShell->GetModule() : nullptr;
    if (m_Module)
        m_xStyleFamilies = m_Module->CreateStyleFamilies();
    if (!m_xStyleFamilies)
        m_xStyleFamilies.emplace();

    m_nActFilter = 0xffff;

    if (m_pCurObjShell)
    {
        m_nActFilter = static_cast<sal_uInt16>(m_aLoadFactoryStyleFilter.Call(m_pCurObjShell));
        if (0xffff == m_nActFilter)
        {
            m_nActFilter = m_pCurObjShell->GetAutoStyleFilterIndex();
        }
        if (m_bModuleHasStylesHighlighterFeature)
            sDefaultCharStyleUIName = getDefaultStyleName(SfxStyleFamily::Char);
    }
    size_t nCount = m_xStyleFamilies->size();
    m_pBindings->ENTERREGISTRATIONS();

    size_t i;
    for (i = 0; i < nCount; ++i)
    {
        sal_uInt16 nSlot = 0;
        switch (m_xStyleFamilies->at(i).GetFamily())
        {
            case SfxStyleFamily::Char:
                nSlot = SID_STYLE_FAMILY1;
                break;
            case SfxStyleFamily::Para:
                nSlot = SID_STYLE_FAMILY2;
                break;
            case SfxStyleFamily::Frame:
                nSlot = SID_STYLE_FAMILY3;
                break;
            case SfxStyleFamily::Page:
                nSlot = SID_STYLE_FAMILY4;
                break;
            case SfxStyleFamily::Pseudo:
                nSlot = SID_STYLE_FAMILY5;
                break;
            case SfxStyleFamily::Table:
                nSlot = SID_STYLE_FAMILY6;
                break;
            default:
                OSL_FAIL("unknown StyleFamily");
                break;
        }
        pBoundItems[i].reset(new SfxTemplateControllerItem(nSlot, *m_pParentDialog, *m_pBindings));
    }
    pBoundItems[i++].reset(
        new SfxTemplateControllerItem(SID_STYLE_WATERCAN, *m_pParentDialog, *m_pBindings));
    pBoundItems[i++].reset(
        new SfxTemplateControllerItem(SID_STYLE_NEW_BY_EXAMPLE, *m_pParentDialog, *m_pBindings));
    pBoundItems[i++].reset(
        new SfxTemplateControllerItem(SID_STYLE_UPDATE_BY_EXAMPLE, *m_pParentDialog, *m_pBindings));
    pBoundItems[i++].reset(
        new SfxTemplateControllerItem(SID_STYLE_NEW, *m_pParentDialog, *m_pBindings));
    pBoundItems[i++].reset(
        new SfxTemplateControllerItem(SID_STYLE_DRAGHIERARCHIE, *m_pParentDialog, *m_pBindings));
    pBoundItems[i++].reset(
        new SfxTemplateControllerItem(SID_STYLE_EDIT, *m_pParentDialog, *m_pBindings));
    pBoundItems[i++].reset(
        new SfxTemplateControllerItem(SID_STYLE_DELETE, *m_pParentDialog, *m_pBindings));
    pBoundItems[i++].reset(
        new SfxTemplateControllerItem(SID_STYLE_FAMILY, *m_pParentDialog, *m_pBindings));
    m_pBindings->LEAVEREGISTRATIONS();

    for (; i < COUNT_BOUND_FUNC; ++i)
        pBoundItems[i] = nullptr;

    StartListening(*m_pBindings);

    for (i = SID_STYLE_FAMILY1; i <= SID_STYLE_FAMILY4; i++)
        m_pBindings->Update(i);

    return nCount;
}

void StyleList::EnableNewByExample(bool newByExampleDisabled)
{
    m_bNewByExampleDisabled = newByExampleDisabled;
}

void StyleList::FilterSelect(sal_uInt16 nActFilter, bool bsetFilter)
{
    m_nActFilter = nActFilter;
    if (bsetFilter)
    {
        SfxObjectShell* const pDocShell = m_aSaveSelection.Call(*this);
        SfxStyleSheetBasePool* pOldStyleSheetPool = m_pStyleSheetPool;
        m_pStyleSheetPool = pDocShell ? pDocShell->GetStyleSheetPool() : nullptr;
        if (pOldStyleSheetPool != m_pStyleSheetPool)
        {
            if (pOldStyleSheetPool)
                EndListening(*pOldStyleSheetPool);
            if (m_pStyleSheetPool)
                StartListening(*m_pStyleSheetPool);
        }
    }
    UpdateStyles(StyleFlags::UpdateFamilyList);
}

IMPL_LINK(StyleList, SetFamily, sal_uInt16, nId, void)
{
    if (m_nActFamily != 0xFFFF)
        m_pParentDialog->CheckItem(OUString::number(m_nActFamily), false);
    m_nActFamily = nId;
    if (nId != 0xFFFF)
    {
        m_bUpdateFamily = true;
    }
}

void StyleList::InvalidateBindings()
{
    m_pBindings->Invalidate(SID_STYLE_NEW_BY_EXAMPLE, true);
    m_pBindings->Update(SID_STYLE_NEW_BY_EXAMPLE);
    m_pBindings->Invalidate(SID_STYLE_UPDATE_BY_EXAMPLE, true);
    m_pBindings->Update(SID_STYLE_UPDATE_BY_EXAMPLE);
    m_pBindings->Invalidate(SID_STYLE_WATERCAN, true);
    m_pBindings->Update(SID_STYLE_WATERCAN);
    m_pBindings->Invalidate(SID_STYLE_NEW, true);
    m_pBindings->Update(SID_STYLE_NEW);
    m_pBindings->Invalidate(SID_STYLE_DRAGHIERARCHIE, true);
    m_pBindings->Update(SID_STYLE_DRAGHIERARCHIE);
}

void StyleList::Initialize()
{
    m_pBindings->Invalidate(SID_STYLE_FAMILY);
    m_pBindings->Update(SID_STYLE_FAMILY);

    m_xFmtLb->connect_row_activated(LINK(this, StyleList, TreeListApplyHdl));
    m_xFmtLb->connect_mouse_press(LINK(this, StyleList, MousePressHdl));
    m_xFmtLb->connect_query_tooltip(LINK(this, StyleList, QueryTooltipHdl));
    m_xFmtLb->connect_changed(LINK(this, StyleList, FmtSelectHdl));
    m_xFmtLb->connect_popup_menu(LINK(this, StyleList, PopupFlatMenuHdl));
    m_xFmtLb->connect_key_press(LINK(this, StyleList, KeyInputHdl));
    m_xFmtLb->set_selection_mode(SelectionMode::Multiple);
    m_xTreeBox->connect_changed(LINK(this, StyleList, FmtSelectHdl));
    m_xTreeBox->connect_row_activated(LINK(this, StyleList, TreeListApplyHdl));
    m_xTreeBox->connect_mouse_press(LINK(this, StyleList, MousePressHdl));
    m_xTreeBox->connect_query_tooltip(LINK(this, StyleList, QueryTooltipHdl));
    m_xTreeBox->connect_popup_menu(LINK(this, StyleList, PopupTreeMenuHdl));
    m_xTreeBox->connect_key_press(LINK(this, StyleList, KeyInputHdl));
    m_xTreeBox->connect_drag_begin(LINK(this, StyleList, DragBeginHdl));
    m_xTreeView1DropTargetHelper.reset(new TreeViewDropTarget(*this, *m_xFmtLb));
    m_xTreeView2DropTargetHelper.reset(new TreeViewDropTarget(*this, *m_xTreeBox));

    m_pParentDialog->connect_stylelist_read_resource(LINK(this, StyleList, ReadResource));
    m_pParentDialog->connect_stylelist_clear(LINK(this, StyleList, Clear));
    m_pParentDialog->connect_stylelist_cleanup(LINK(this, StyleList, Cleanup));
    m_pParentDialog->connect_stylelist_execute_drop(LINK(this, StyleList, ExecuteDrop));
    m_pParentDialog->connect_stylelist_execute_new_menu(
        LINK(this, StyleList, NewMenuExecuteAction));
    m_pParentDialog->connect_stylelist_for_watercan(LINK(this, StyleList, IsSafeForWaterCan));
    m_pParentDialog->connect_stylelist_has_selected_style(LINK(this, StyleList, HasSelectedStyle));
    m_pParentDialog->connect_stylelist_update_style_dependents(
        LINK(this, StyleList, UpdateStyleDependents));
    m_pParentDialog->connect_stylelist_enable_tree_drag(LINK(this, StyleList, EnableTreeDrag));
    m_pParentDialog->connect_stylelist_enable_delete(LINK(this, StyleList, EnableDelete));
    m_pParentDialog->connect_stylelist_set_water_can_state(LINK(this, StyleList, SetWaterCanState));
    m_pParentDialog->connect_set_family(LINK(this, StyleList, SetFamily));

    int nTreeHeight = m_xFmtLb->get_height_rows(8);
    m_xFmtLb->set_size_request(-1, nTreeHeight);
    m_xTreeBox->set_size_request(-1, nTreeHeight);

    m_xFmtLb->connect_custom_get_size(LINK(this, StyleList, CustomGetSizeHdl));
    m_xFmtLb->connect_custom_render(LINK(this, StyleList, CustomRenderHdl));
    m_xTreeBox->connect_custom_get_size(LINK(this, StyleList, CustomGetSizeHdl));
    m_xTreeBox->connect_custom_render(LINK(this, StyleList, CustomRenderHdl));
    bool bCustomPreview = officecfg::Office::Common::StylesAndFormatting::Preview::get();
    m_xFmtLb->set_column_custom_renderer(1, bCustomPreview);
    m_xTreeBox->set_column_custom_renderer(1, bCustomPreview);

    m_xFmtLb->set_visible(!m_bHierarchical);
    m_xTreeBox->set_visible(m_bHierarchical);
    Update();
}

void StyleList::UpdateFamily()
{
    m_bUpdateFamily = false;

    SfxDispatcher* pDispat = m_pBindings->GetDispatcher_Impl();
    SfxViewFrame* pViewFrame = pDispat->GetFrame();
    SfxObjectShell* pDocShell = pViewFrame->GetObjectShell();

    SfxStyleSheetBasePool* pOldStyleSheetPool = m_pStyleSheetPool;
    m_pStyleSheetPool = pDocShell ? pDocShell->GetStyleSheetPool() : nullptr;
    if (pOldStyleSheetPool != m_pStyleSheetPool)
    {
        if (pOldStyleSheetPool)
            EndListening(*pOldStyleSheetPool);
        if (m_pStyleSheetPool)
            StartListening(*m_pStyleSheetPool);
    }

    m_bTreeDrag = true;
    m_bCanNew = m_xTreeBox->get_visible() || m_xFmtLb->count_selected_rows() <= 1;
    m_pParentDialog->EnableNew(m_bCanNew, this);
    m_bTreeDrag = true;
    if (m_pStyleSheetPool)
    {
        if (!m_xTreeBox->get_visible())
            UpdateStyles(StyleFlags::UpdateFamily | StyleFlags::UpdateFamilyList);
        else
        {
            UpdateStyles(StyleFlags::UpdateFamily);
            FillTreeBox(GetActualFamily());
        }
    }

    InvalidateBindings();
}

bool StyleList::EnableExecute()
{
    return m_xTreeBox->get_visible() || m_xFmtLb->count_selected_rows() <= 1;
}

void StyleList::connect_LoadFactoryStyleFilter(const Link<SfxObjectShell const*, sal_Int32>& rLink)
{
    m_aLoadFactoryStyleFilter = rLink;
}

void StyleList::connect_SaveSelection(const Link<StyleList&, SfxObjectShell*> rLink)
{
    m_aSaveSelection = rLink;
}

/** Drop is enabled as long as it is allowed to create a new style by example, i.e. to
    create a style out of the current selection.
*/
sal_Int8 StyleList::AcceptDrop(const AcceptDropEvent& rEvt, const DropTargetHelper& rHelper)
{
    if (rHelper.IsDropFormatSupported(SotClipboardFormatId::OBJECTDESCRIPTOR))
    {
        // special case: page styles are allowed to create new styles by example
        // but not allowed to be created by drag and drop
        if (GetActualFamily() == SfxStyleFamily::Page || m_bNewByExampleDisabled)
            return DND_ACTION_NONE;
        else
            return DND_ACTION_COPY;
    }
    // to enable the autoscroll when we're close to the edges
    weld::TreeView* pTreeView = m_xTreeBox->get_visible() ? m_xTreeBox.get() : m_xFmtLb.get();
    pTreeView->get_dest_row_at_pos(rEvt.maPosPixel, nullptr, true);
    return DND_ACTION_MOVE;
}

// handles drop of content in treeview when creating a new style
IMPL_LINK(StyleList, ExecuteDrop, const ExecuteDropEvent&, rEvt, sal_Int8)
{
    SfxObjectShell* pDocShell = m_pCurObjShell;
    if (pDocShell)
    {
        TransferableDataHelper aHelper(rEvt.maDropEvent.Transferable);
        sal_uInt32 nFormatCount = aHelper.GetFormatCount();

        sal_Int8 nRet = DND_ACTION_NONE;

        bool bFormatFound = false;

        for (sal_uInt32 i = 0; i < nFormatCount; ++i)
        {
            SotClipboardFormatId nId = aHelper.GetFormat(i);
            TransferableObjectDescriptor aDesc;

            if (aHelper.GetTransferableObjectDescriptor(nId, aDesc))
            {
                if (aDesc.maClassName == pDocShell->GetFactory().GetClassId())
                {
                    Application::PostUserEvent(
                        LINK(m_pParentDialog, SfxCommonTemplateDialog_Impl, OnAsyncExecuteDrop),
                        this);

                    bFormatFound = true;
                    nRet = rEvt.mnAction;
                    break;
                }
            }
        }

        if (bFormatFound)
            return nRet;
    }

    if (!m_xTreeBox->get_visible())
        return DND_ACTION_NONE;

    if (!m_bAllowReParentDrop)
        return DND_ACTION_NONE;

    // otherwise if we're dragging with the treeview to set a new parent of the dragged style
    weld::TreeView* pSource = m_xTreeBox->get_drag_source();
    // only dragging within the same widget allowed
    if (!pSource || pSource != m_xTreeBox.get())
        return DND_ACTION_NONE;

    std::unique_ptr<weld::TreeIter> xSource(m_xTreeBox->make_iterator());
    if (!m_xTreeBox->get_selected(xSource.get()))
        return DND_ACTION_NONE;

    std::unique_ptr<weld::TreeIter> xTarget(m_xTreeBox->make_iterator());
    if (!m_xTreeBox->get_dest_row_at_pos(rEvt.maPosPixel, xTarget.get(), true))
    {
        // if nothing under the mouse, use the last row
        int nChildren = m_xTreeBox->n_children();
        if (!nChildren)
            return DND_ACTION_NONE;
        if (!m_xTreeBox->get_iter_first(*xTarget)
            || !m_xTreeBox->iter_nth_sibling(*xTarget, nChildren - 1))
            return DND_ACTION_NONE;
        while (m_xTreeBox->get_row_expanded(*xTarget))
        {
            nChildren = m_xTreeBox->iter_n_children(*xTarget);
            if (!m_xTreeBox->iter_children(*xTarget)
                || !m_xTreeBox->iter_nth_sibling(*xTarget, nChildren - 1))
                return DND_ACTION_NONE;
        }
    }
    OUString aTargetStyle = m_xTreeBox->get_text(*xTarget);
    DropHdl(m_xTreeBox->get_text(*xSource), aTargetStyle);
    m_xTreeBox->unset_drag_dest_row();
    FillTreeBox(GetActualFamily());
    m_pParentDialog->SelectStyle(aTargetStyle, false, *this);
    return DND_ACTION_NONE;
}

IMPL_LINK_NOARG(StyleList, NewMenuExecuteAction, void*, void)
{
    if (!m_pStyleSheetPool || m_nActFamily == 0xffff)
        return;

    const SfxStyleFamily eFam = GetFamilyItem()->GetFamily();
    const SfxStyleFamilyItem* pItem = GetFamilyItem();
    SfxStyleSearchBits nFilter(SfxStyleSearchBits::Auto);
    if (pItem && m_nActFilter != 0xffff)
        nFilter = pItem->GetFilterList()[m_nActFilter].nFlags;
    if (nFilter == SfxStyleSearchBits::Auto) // automatic
        nFilter = m_nAppFilter;

    // why? : FloatingWindow must not be parent of a modal dialog
    SfxNewStyleDlg aDlg(m_pContainer, *m_pStyleSheetPool, eFam);
    auto nResult = aDlg.run();
    if (nResult == RET_OK)
    {
        const OUString aTemplName(aDlg.GetName());
        m_pParentDialog->Execute_Impl(SID_STYLE_NEW_BY_EXAMPLE, aTemplName, u""_ustr,
                                      static_cast<sal_uInt16>(GetFamilyItem()->GetFamily()), *this,
                                      nFilter);
        UpdateFamily();
        m_aUpdateFamily.Call(*this);
    }
}

void StyleList::DropHdl(const OUString& rStyle, const OUString& rParent)
{
    m_bDontUpdate = true;
    const SfxStyleFamilyItem* pItem = GetFamilyItem();
    const SfxStyleFamily eFam = pItem->GetFamily();
    if (auto pStyle = m_pStyleSheetPool->Find(rStyle, eFam))
        pStyle->SetParent(rParent);
    m_bDontUpdate = false;
}

void StyleList::PrepareMenu(const Point& rPos)
{
    weld::TreeView* pTreeView = m_xTreeBox->get_visible() ? m_xTreeBox.get() : m_xFmtLb.get();
    std::unique_ptr<weld::TreeIter> xIter(pTreeView->make_iterator());
    if (pTreeView->get_dest_row_at_pos(rPos, xIter.get(), false) && !pTreeView->is_selected(*xIter))
    {
        pTreeView->unselect_all();
        pTreeView->set_cursor(*xIter);
        pTreeView->select(*xIter);
    }
    FmtSelectHdl(*pTreeView);
}

/** Internal structure for the establishment of the hierarchical view */
namespace
{
class StyleTree_Impl;
}

typedef std::vector<std::unique_ptr<StyleTree_Impl>> StyleTreeArr_Impl;

namespace
{
class StyleTree_Impl
{
private:
    OUString aName;
    OUString aParent;
    StyleTreeArr_Impl pChildren;

public:
    bool HasParent() const { return !aParent.isEmpty(); }

    StyleTree_Impl(OUString _aName, OUString _aParent)
        : aName(std::move(_aName))
        , aParent(std::move(_aParent))
        , pChildren(0)
    {
    }

    const OUString& getName() const { return aName; }
    const OUString& getParent() const { return aParent; }
    StyleTreeArr_Impl& getChildren() { return pChildren; }
};
}

static void MakeTree_Impl(StyleTreeArr_Impl& rArr, const OUString& aUIName)
{
    const comphelper::string::NaturalStringSorter aSorter(
        ::comphelper::getProcessComponentContext(),
        Application::GetSettings().GetLanguageTag().getLocale());

    std::unordered_map<OUString, StyleTree_Impl*> styleFinder;
    styleFinder.reserve(rArr.size());
    for (const auto& pEntry : rArr)
    {
        styleFinder.emplace(pEntry->getName(), pEntry.get());
    }

    // Arrange all under their Parents
    for (auto& pEntry : rArr)
    {
        if (!pEntry->HasParent())
            continue;
        auto it = styleFinder.find(pEntry->getParent());
        if (it != styleFinder.end())
        {
            StyleTree_Impl* pCmp = it->second;
            // Insert child entries sorted
            auto iPos = std::lower_bound(
                pCmp->getChildren().begin(), pCmp->getChildren().end(), pEntry,
                [&aSorter](std::unique_ptr<StyleTree_Impl> const& pEntry1,
                           std::unique_ptr<StyleTree_Impl> const& pEntry2) {
                    return aSorter.compare(pEntry1->getName(), pEntry2->getName()) < 0;
                });
            pCmp->getChildren().insert(iPos, std::move(pEntry));
        }
    }

    // Only keep tree roots in rArr, child elements can be accessed through the hierarchy
    std::erase_if(rArr, [](std::unique_ptr<StyleTree_Impl> const& pEntry) { return !pEntry; });

    // tdf#91106 sort top level styles
    std::sort(rArr.begin(), rArr.end());
    std::sort(rArr.begin(), rArr.end(),
              [&aSorter, &aUIName](std::unique_ptr<StyleTree_Impl> const& pEntry1,
                                   std::unique_ptr<StyleTree_Impl> const& pEntry2) {
                  if (pEntry2->getName() == aUIName)
                      return false;
                  if (pEntry1->getName() == aUIName)
                      return true; // default always first
                  return aSorter.compare(pEntry1->getName(), pEntry2->getName()) < 0;
              });
}

static bool IsExpanded_Impl(const std::vector<OUString>& rEntries, std::u16string_view rStr)
{
    for (const auto& rEntry : rEntries)
    {
        if (rEntry == rStr)
            return true;
    }
    return false;
}

static void lcl_Insert(weld::TreeView& rTreeView, const OUString& rName, SfxStyleFamily eFam,
                       const weld::TreeIter* pParent, weld::TreeIter* pRet, SfxViewShell* pViewSh)
{
    Color aColor = ColorHash(rName);

    int nColor;
    if (eFam == SfxStyleFamily::Para)
    {
        StylesHighlighterColorMap& rParaStylesColorMap
            = pViewSh->GetStylesHighlighterParaColorMap();
        nColor = rParaStylesColorMap.size();
        rParaStylesColorMap[rName] = std::pair(aColor, nColor);
    }
    else
    {
        StylesHighlighterColorMap& rCharStylesColorMap
            = pViewSh->GetStylesHighlighterCharColorMap();
        nColor = rCharStylesColorMap.size();
        rCharStylesColorMap[rName] = std::pair(aColor, nColor);
        // don't show a color or number for default character style 'No Character Style' entry
        if (rName == sDefaultCharStyleUIName.value() /*"No Character Style"*/)
        {
            rTreeView.insert(pParent, -1, &rName, &rName, nullptr, nullptr, false, pRet);
            return;
        }
    }

    // draw the color rectangle and number image
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Size aImageSize = rStyleSettings.GetListBoxPreviewDefaultPixelSize();
    ScopedVclPtrInstance<VirtualDevice> xDevice;
    xDevice->SetOutputSize(aImageSize);
    xDevice->SetFillColor(aColor);
    const tools::Rectangle aRect(Point(0, 0), aImageSize);
    xDevice->DrawRect(aRect);
    xDevice->SetTextColor(COL_BLACK);
    xDevice->DrawText(aRect, OUString::number(nColor),
                      DrawTextFlags::Center | DrawTextFlags::VCenter);

    rTreeView.insert(pParent, -1, &rName, &rName, nullptr, xDevice.get(), false, pRet);
}

static void FillBox_Impl(weld::TreeView& rBox, StyleTree_Impl* pEntry,
                         const std::vector<OUString>& rEntries, SfxStyleFamily eStyleFamily,
                         const weld::TreeIter* pParent, bool blcl_insert, SfxViewShell* pViewShell,
                         SfxStyleSheetBasePool* pStyleSheetPool)
{
    std::unique_ptr<weld::TreeIter> xResult = rBox.make_iterator();
    const OUString& rName = pEntry->getName();
    if (blcl_insert)
    {
        const SfxStyleSheetBase* pStyle = nullptr;
        if (pStyleSheetPool)
            pStyle = pStyleSheetPool->Find(rName, eStyleFamily);
        if (pStyle && pStyle->IsUsed())
            lcl_Insert(rBox, rName, eStyleFamily, pParent, xResult.get(), pViewShell);
        else
            rBox.insert(pParent, -1, &rName, &rName, nullptr, nullptr, false, xResult.get());
    }
    else
        rBox.insert(pParent, -1, &rName, &rName, nullptr, nullptr, false, xResult.get());

    for (size_t i = 0; i < pEntry->getChildren().size(); ++i)
        FillBox_Impl(rBox, pEntry->getChildren()[i].get(), rEntries, eStyleFamily, xResult.get(),
                     blcl_insert, pViewShell, pStyleSheetPool);
}

namespace SfxTemplate
{
// converts from SFX_STYLE_FAMILY Ids to 1-6
static sal_uInt16 SfxFamilyIdToNId(SfxStyleFamily nFamily)
{
    switch (nFamily)
    {
        case SfxStyleFamily::Char:
            return 1;
        case SfxStyleFamily::Para:
            return 2;
        case SfxStyleFamily::Frame:
            return 3;
        case SfxStyleFamily::Page:
            return 4;
        case SfxStyleFamily::Pseudo:
            return 5;
        case SfxStyleFamily::Table:
            return 6;
        default:
            return 0xffff;
    }
}
// converts from 1-6 to SFX_STYLE_FAMILY Ids
static SfxStyleFamily NIdToSfxFamilyId(sal_uInt16 nId)
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

sal_uInt16 StyleList::StyleNrToInfoOffset(sal_uInt16 nId)
{
    const SfxStyleFamilyItem& rItem = m_xStyleFamilies->at(nId);
    return SfxTemplate::SfxFamilyIdToNId(rItem.GetFamily()) - 1;
}

// Helper function: Access to the current family item
const SfxStyleFamilyItem* StyleList::GetFamilyItem() const
{
    const size_t nCount = m_xStyleFamilies->size();
    for (size_t i = 0; i < nCount; ++i)
    {
        const SfxStyleFamilyItem& rItem = m_xStyleFamilies->at(i);
        sal_uInt16 nId = SfxTemplate::SfxFamilyIdToNId(rItem.GetFamily());
        if (nId == m_nActFamily)
            return &rItem;
    }
    return nullptr;
}

void StyleList::GetSelectedStyle() const
{
    const OUString aTemplName(GetSelectedEntry());
    const SfxStyleFamilyItem* pItem = GetFamilyItem();
    m_pStyleSheetPool->Find(aTemplName, pItem->GetFamily());
}

// Used to get the current selected entry in visible treeview
OUString StyleList::GetSelectedEntry() const
{
    OUString aRet;
    if (m_xTreeBox->get_visible())
        aRet = m_xTreeBox->get_selected_text();
    else
        aRet = m_xFmtLb->get_selected_text();
    return aRet;
}

/**
 * Is it safe to show the water-can / fill icon. If we've a
 * hierarchical widget - we have only single select, otherwise
 * we need to check if we have a multi-selection. We either have
 * a m_xTreeBox showing or an m_xFmtLb (which we hide when not shown)
 */
IMPL_LINK_NOARG(StyleList, IsSafeForWaterCan, void*, bool)
{
    if (m_xTreeBox->get_visible())
        return m_xTreeBox->get_selected_index() != -1;
    else
        return m_xFmtLb->count_selected_rows() == 1;
}

IMPL_LINK(StyleList, SetWaterCanState, const SfxBoolItem*, pItem, void)
{
    size_t nCount = m_xStyleFamilies->size();
    m_pBindings->EnterRegistrations();
    for (size_t n = 0; n < nCount; n++)
    {
        SfxControllerItem* pCItem = pBoundItems[n].get();
        bool bChecked = pItem && pItem->GetValue();
        if (pCItem->IsBound() == bChecked)
        {
            if (!bChecked)
                pCItem->ReBind();
            else
                pCItem->UnBind();
        }
    }
    m_pBindings->LeaveRegistrations();
}

void StyleList::FamilySelect(sal_uInt16 nEntry, bool bRefresh)
{
    if (bRefresh)
    {
        bool bCustomPreview = officecfg::Office::Common::StylesAndFormatting::Preview::get();
        m_xFmtLb->clear();
        m_xFmtLb->set_column_custom_renderer(1, bCustomPreview);
        m_xTreeBox->clear();
        m_xTreeBox->set_column_custom_renderer(1, bCustomPreview);
    }
    m_nActFamily = nEntry;
    SfxDispatcher* pDispat = m_pBindings->GetDispatcher_Impl();
    SfxUInt16Item const aItem(SID_STYLE_FAMILY,
                              static_cast<sal_uInt16>(SfxTemplate::NIdToSfxFamilyId(nEntry)));
    pDispat->ExecuteList(SID_STYLE_FAMILY, SfxCallMode::SYNCHRON, { &aItem });
    m_pBindings->Invalidate(SID_STYLE_FAMILY);
    m_pBindings->Update(SID_STYLE_FAMILY);
    UpdateFamily();
    m_aUpdateFamily.Call(*this);
}

// It selects the style in treeview
// bIsCallBack is true for the selected style. For eg. if "Addressee" is selected in
// styles, bIsCallBack will be true for it.
void StyleList::SelectStyle(const OUString& rStr, bool bIsCallback)
{
    const SfxStyleFamilyItem* pItem = GetFamilyItem();
    if (!pItem)
        return;
    const SfxStyleFamily eFam = pItem->GetFamily();
    SfxStyleSheetBase* pStyle = m_pStyleSheetPool->Find(rStr, eFam);
    if (pStyle)
    {
        bool bReadWrite = !(pStyle->GetMask() & SfxStyleSearchBits::ReadOnly);
        m_pParentDialog->EnableEdit(bReadWrite, this);
        m_pParentDialog->EnableHide(bReadWrite && !pStyle->IsHidden() && !pStyle->IsUsed(), this);
        m_pParentDialog->EnableShow(bReadWrite && pStyle->IsHidden(), this);
    }
    else
    {
        m_pParentDialog->EnableEdit(false, this);
        m_pParentDialog->EnableHide(false, this);
        m_pParentDialog->EnableShow(false, this);
    }

    if (bIsCallback)
        return;

    if (m_xTreeBox->get_visible())
    {
        if (!rStr.isEmpty())
        {
            std::unique_ptr<weld::TreeIter> xEntry = m_xTreeBox->make_iterator();
            bool bEntry = m_xTreeBox->get_iter_first(*xEntry);
            while (bEntry)
            {
                if (m_xTreeBox->get_text(*xEntry) == rStr)
                {
                    m_xTreeBox->scroll_to_row(*xEntry);
                    m_xTreeBox->select(*xEntry);
                    break;
                }
                bEntry = m_xTreeBox->iter_next(*xEntry);
            }
        }
        else if (eFam == SfxStyleFamily::Pseudo)
        {
            std::unique_ptr<weld::TreeIter> xEntry = m_xTreeBox->make_iterator();
            if (m_xTreeBox->get_iter_first(*xEntry))
            {
                m_xTreeBox->scroll_to_row(*xEntry);
                m_xTreeBox->select(*xEntry);
            }
        }
        else
            m_xTreeBox->unselect_all();
    }
    else
    {
        bool bSelect = !rStr.isEmpty();
        if (bSelect)
        {
            std::unique_ptr<weld::TreeIter> xEntry = m_xFmtLb->make_iterator();
            bool bEntry = m_xFmtLb->get_iter_first(*xEntry);
            while (bEntry && m_xFmtLb->get_text(*xEntry) != rStr)
                bEntry = m_xFmtLb->iter_next(*xEntry);
            if (!bEntry)
                bSelect = false;
            else
            {
                if (!m_xFmtLb->is_selected(*xEntry))
                {
                    m_xFmtLb->unselect_all();
                    m_xFmtLb->scroll_to_row(*xEntry);
                    m_xFmtLb->select(*xEntry);
                }
            }
        }

        if (!bSelect)
        {
            m_xFmtLb->unselect_all();
            m_pParentDialog->EnableEdit(false, this);
            m_pParentDialog->EnableHide(false, this);
            m_pParentDialog->EnableShow(false, this);
        }
    }
}

static void MakeExpanded_Impl(const weld::TreeView& rBox, std::vector<OUString>& rEntries)
{
    std::unique_ptr<weld::TreeIter> xEntry = rBox.make_iterator();
    if (rBox.get_iter_first(*xEntry))
    {
        do
        {
            if (rBox.get_row_expanded(*xEntry))
                rEntries.push_back(rBox.get_text(*xEntry));
        } while (rBox.iter_next(*xEntry));
    }
}

IMPL_LINK(StyleList, EnableTreeDrag, bool, m_bEnable, void)
{
    if (m_pStyleSheetPool)
    {
        const SfxStyleFamilyItem* pItem = GetFamilyItem();
        SfxStyleSheetBase* pStyle = pItem ? m_pStyleSheetPool->First(pItem->GetFamily()) : nullptr;
        m_bAllowReParentDrop = pStyle && pStyle->HasParentSupport() && m_bEnable;
    }
    m_bTreeDrag = m_bEnable;
}

// Fill the treeview

void StyleList::FillTreeBox(SfxStyleFamily eFam)
{
    assert(m_xTreeBox && "FillTreeBox() without treebox");
    if (!m_pStyleSheetPool || m_nActFamily == 0xffff)
        return;

    const SfxStyleFamilyItem* pItem = GetFamilyItem();
    if (!pItem)
        return;

    StyleTreeArr_Impl aArr;
    SfxStyleSheetBase* pStyle = m_pStyleSheetPool->First(eFam, SfxStyleSearchBits::All);

    m_bAllowReParentDrop = pStyle && pStyle->HasParentSupport() && m_bTreeDrag;

    while (pStyle)
    {
        StyleTree_Impl* pNew = new StyleTree_Impl(pStyle->GetName(), pStyle->GetParent());
        aArr.emplace_back(pNew);
        pStyle = m_pStyleSheetPool->Next();
    }
    OUString aUIName = getDefaultStyleName(eFam);
    MakeTree_Impl(aArr, aUIName);
    std::vector<OUString> aEntries;
    MakeExpanded_Impl(*m_xTreeBox, aEntries);
    m_xTreeBox->freeze();
    m_xTreeBox->clear();
    const sal_uInt16 nCount = aArr.size();

    SfxViewShell* pViewShell = m_pCurObjShell->GetViewShell();
    if (pViewShell && m_bModuleHasStylesHighlighterFeature)
    {
        if (eFam == SfxStyleFamily::Para)
            pViewShell->GetStylesHighlighterParaColorMap().clear();
        else if (eFam == SfxStyleFamily::Char)
            pViewShell->GetStylesHighlighterCharColorMap().clear();
    }

    bool blcl_insert = pViewShell && m_bModuleHasStylesHighlighterFeature
                       && ((eFam == SfxStyleFamily::Para && m_bHighlightParaStyles)
                           || (eFam == SfxStyleFamily::Char && m_bHighlightCharStyles));

    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        FillBox_Impl(*m_xTreeBox, aArr[i].get(), aEntries, eFam, nullptr, blcl_insert, pViewShell,
                     m_pStyleSheetPool);
        aArr[i].reset();
    }

    m_xTreeBox->columns_autosize();

    m_pParentDialog->EnableItem(u"watercan"_ustr, false);

    SfxTemplateItem* pState = m_pFamilyState[m_nActFamily - 1].get();

    m_xTreeBox->thaw();

    // hack for x11 to make view update
    if (pViewShell && m_bModuleHasStylesHighlighterFeature)
    {
        SfxViewFrame* pViewFrame = m_pBindings->GetDispatcher_Impl()->GetFrame();
        pViewFrame->Resize(true);
    }

    std::unique_ptr<weld::TreeIter> xEntry = m_xTreeBox->make_iterator();
    bool bEntry = m_xTreeBox->get_iter_first(*xEntry);
    if (bEntry && nCount)
        m_xTreeBox->expand_row(*xEntry);

    while (bEntry)
    {
        if (IsExpanded_Impl(aEntries, m_xTreeBox->get_text(*xEntry)))
            m_xTreeBox->expand_row(*xEntry);
        bEntry = m_xTreeBox->iter_next(*xEntry);
    }

    OUString aStyle;
    if (pState) // Select current entry
        aStyle = pState->GetStyleName();
    m_pParentDialog->SelectStyle(aStyle, false, *this);
    EnableDelete(nullptr);
}

static OUString lcl_GetStyleFamilyName(SfxStyleFamily nFamily)
{
    if (nFamily == SfxStyleFamily::Char)
        return u"CharacterStyles"_ustr;
    if (nFamily == SfxStyleFamily::Para)
        return u"ParagraphStyles"_ustr;
    if (nFamily == SfxStyleFamily::Page)
        return u"PageStyles"_ustr;
    if (nFamily == SfxStyleFamily::Table)
        return u"TableStyles"_ustr;
    if (nFamily == SfxStyleFamily::Pseudo)
        return u"NumberingStyles"_ustr;
    return OUString();
}

OUString StyleList::getDefaultStyleName(const SfxStyleFamily eFam)
{
    OUString sDefaultStyle;
    OUString aFamilyName = lcl_GetStyleFamilyName(eFam);
    if (aFamilyName == "TableStyles")
        sDefaultStyle = "Default Style";
    else if (aFamilyName == "NumberingStyles")
        sDefaultStyle = "No List";
    else
        sDefaultStyle = "Standard";
    uno::Reference<style::XStyleFamiliesSupplier> xModel(m_pCurObjShell->GetModel(),
                                                         uno::UNO_QUERY);
    OUString aUIName;
    try
    {
        uno::Reference<container::XNameAccess> xStyles;
        uno::Reference<container::XNameAccess> xCont = xModel->getStyleFamilies();
        xCont->getByName(aFamilyName) >>= xStyles;
        uno::Reference<beans::XPropertySet> xInfo;
        xStyles->getByName(sDefaultStyle) >>= xInfo;
        xInfo->getPropertyValue(u"DisplayName"_ustr) >>= aUIName;
    }
    catch (const uno::Exception&)
    {
    }
    return aUIName;
}

SfxStyleFamily StyleList::GetActualFamily() const
{
    const SfxStyleFamilyItem* pFamilyItem = GetFamilyItem();
    if (!pFamilyItem || m_nActFamily == 0xffff)
        return SfxStyleFamily::Para;
    else
        return pFamilyItem->GetFamily();
}

IMPL_LINK_NOARG(StyleList, HasSelectedStyle, void*, bool)
{
    return m_xTreeBox->get_visible() ? m_xTreeBox->get_selected_index() != -1
                                     : m_xFmtLb->count_selected_rows() != 0;
}

IMPL_LINK_NOARG(StyleList, UpdateStyleDependents, void*, void)
{
    // Trigger Help PI. Only when the watercan is on
    if (m_nActFamily != 0xffff && m_pParentDialog->IsCheckedItem(u"watercan"_ustr) &&
        // only if that region is allowed
        nullptr != m_pFamilyState[m_nActFamily - 1] && IsSafeForWaterCan(nullptr))
    {
        m_pParentDialog->Execute_Impl(SID_STYLE_WATERCAN, u""_ustr, u""_ustr, 0, *this);
        m_pParentDialog->Execute_Impl(SID_STYLE_WATERCAN, GetSelectedEntry(), u""_ustr,
                                      static_cast<sal_uInt16>(GetFamilyItem()->GetFamily()), *this);
    }
}

// Comes into action when the current style is changed
void StyleList::UpdateStyles(StyleFlags nFlags)
{
    OSL_ENSURE(nFlags != StyleFlags::NONE, "nothing to do");
    const SfxStyleFamilyItem* pItem = GetFamilyItem();
    if (!pItem)
    {
        // Is the case for the template catalog
        const size_t nFamilyCount = m_xStyleFamilies->size();
        size_t n;
        for (n = 0; n < nFamilyCount; n++)
            if (m_pFamilyState[StyleNrToInfoOffset(n)])
                break;
        if (n == nFamilyCount)
            // It happens sometimes, God knows why
            return;
        m_nAppFilter = m_pFamilyState[StyleNrToInfoOffset(n)]->GetValue();
        m_pParentDialog->FamilySelect(StyleNrToInfoOffset(n) + 1, *this);
        pItem = GetFamilyItem();
    }

    const SfxStyleFamily eFam = pItem->GetFamily();

    SfxStyleSearchBits nFilter(m_nActFilter < pItem->GetFilterList().size()
                                   ? pItem->GetFilterList()[m_nActFilter].nFlags
                                   : SfxStyleSearchBits::Auto);
    if (nFilter == SfxStyleSearchBits::Auto) // automatic
        nFilter = m_nAppFilter;

    OSL_ENSURE(m_pStyleSheetPool, "no StyleSheetPool");
    if (!m_pStyleSheetPool)
        return;

    m_aUpdateStyles.Call(nFlags);

    SfxStyleSheetBase* pStyle = m_pStyleSheetPool->First(eFam, nFilter);

    std::unique_ptr<weld::TreeIter> xEntry = m_xFmtLb->make_iterator();
    std::vector<OUString> aStrings;

    comphelper::string::NaturalStringSorter aSorter(
        ::comphelper::getProcessComponentContext(),
        Application::GetSettings().GetLanguageTag().getLocale());

    while (pStyle)
    {
        aStrings.push_back(pStyle->GetName());
        pStyle = m_pStyleSheetPool->Next();
    }
    OUString aUIName = getDefaultStyleName(eFam);

    // Paradoxically, with a list and non-Latin style names,
    // sorting twice is faster than sorting once.
    // The first sort has a cheap comparator, and gets the list into mostly-sorted order.
    // Then the second sort needs to call its (much more expensive) comparator less often.
    std::sort(aStrings.begin(), aStrings.end());
    std::sort(aStrings.begin(), aStrings.end(),
              [&aSorter, &aUIName](const OUString& rLHS, const OUString& rRHS) {
                  if (rRHS == aUIName)
                      return false;
                  if (rLHS == aUIName)
                      return true; // default always first
                  return aSorter.compare(rLHS, rRHS) < 0;
              });

    // Fill the display box
    m_xFmtLb->freeze();
    m_xFmtLb->clear();

    SfxViewShell* pViewShell = m_pCurObjShell->GetViewShell();
    if (pViewShell && m_bModuleHasStylesHighlighterFeature)
    {
        if (eFam == SfxStyleFamily::Para)
            pViewShell->GetStylesHighlighterParaColorMap().clear();
        else if (eFam == SfxStyleFamily::Char)
            pViewShell->GetStylesHighlighterCharColorMap().clear();
    }

    size_t nCount = aStrings.size();
    size_t nPos = 0;

    if (pViewShell && m_bModuleHasStylesHighlighterFeature
        && ((eFam == SfxStyleFamily::Para && m_bHighlightParaStyles)
            || (eFam == SfxStyleFamily::Char && m_bHighlightCharStyles)))
    {
        for (nPos = 0; nPos < nCount; ++nPos)
        {
            pStyle = m_pStyleSheetPool->Find(aStrings[nPos], eFam);
            if (pStyle && pStyle->IsUsed())
                lcl_Insert(*m_xFmtLb, aStrings[nPos], eFam, nullptr, nullptr, pViewShell);
            else
                m_xFmtLb->append(aStrings[nPos], aStrings[nPos]);
        }
    }
    else
    {
        for (nPos = 0; nPos < nCount; ++nPos)
            m_xFmtLb->append(aStrings[nPos], aStrings[nPos]);
    }

    m_xFmtLb->columns_autosize();

    m_xFmtLb->thaw();

    // hack for x11 to make view update
    if (pViewShell && m_bModuleHasStylesHighlighterFeature)
    {
        SfxViewFrame* pViewFrame = m_pBindings->GetDispatcher_Impl()->GetFrame();
        pViewFrame->Resize(true);
    }

    // Selects the current style if any
    SfxTemplateItem* pState = m_pFamilyState[m_nActFamily - 1].get();
    OUString aStyle;
    if (pState)
        aStyle = pState->GetStyleName();
    m_pParentDialog->SelectStyle(aStyle, false, *this);
    EnableDelete(nullptr);
}

void StyleList::SetFamilyState(sal_uInt16 nSlotId, const SfxTemplateItem* pItem)
{
    sal_uInt16 nIdx = nSlotId - SID_STYLE_FAMILY_START;
    m_pFamilyState[nIdx].reset();
    if (pItem)
        m_pFamilyState[nIdx].reset(new SfxTemplateItem(*pItem));
    m_bUpdateFamily = true;
}

void StyleList::SetHierarchical()
{
    m_bHierarchical = true;
    const OUString aSelectEntry(GetSelectedEntry());
    m_xFmtLb->hide();
    FillTreeBox(GetActualFamily());
    m_pParentDialog->SelectStyle(aSelectEntry, false, *this);
    m_xTreeBox->show();
}

void StyleList::SetFilterControlsHandle()
{
    m_xTreeBox->hide();
    m_xFmtLb->show();
    m_bHierarchical = false;
}

// Handler for the New-Buttons
void StyleList::NewHdl()
{
    if (m_nActFamily == 0xffff
        || !(m_xTreeBox->get_visible() || m_xFmtLb->count_selected_rows() <= 1))
        return;

    const SfxStyleFamilyItem* pItem = GetFamilyItem();
    const SfxStyleFamily eFam = pItem->GetFamily();
    SfxStyleSearchBits nMask(SfxStyleSearchBits::Auto);
    if (m_nActFilter != 0xffff)
        nMask = pItem->GetFilterList()[m_nActFilter].nFlags;
    if (nMask == SfxStyleSearchBits::Auto) // automatic
        nMask = m_nAppFilter;

    m_pParentDialog->Execute_Impl(SID_STYLE_NEW, u""_ustr, GetSelectedEntry(),
                                  static_cast<sal_uInt16>(eFam), *this, nMask);
}

// Handler for the edit-Buttons
void StyleList::EditHdl()
{
    if (m_nActFamily != 0xffff && HasSelectedStyle(nullptr))
    {
        sal_uInt16 nFilter = m_nActFilter;
        OUString aTemplName(GetSelectedEntry());
        GetSelectedStyle(); // -Wall required??
        m_pParentDialog->Execute_Impl(SID_STYLE_EDIT, aTemplName, OUString(),
                                      static_cast<sal_uInt16>(GetFamilyItem()->GetFamily()), *this,
                                      SfxStyleSearchBits::Auto, &nFilter);
    }
}

// Handler for the Delete-Buttons
void StyleList::DeleteHdl()
{
    if (m_nActFamily == 0xffff || !HasSelectedStyle(nullptr))
        return;

    bool bUsedStyle = false; // one of the selected styles are used in the document?

    std::vector<std::unique_ptr<weld::TreeIter>> aList;
    weld::TreeView* pTreeView = m_xTreeBox->get_visible() ? m_xTreeBox.get() : m_xFmtLb.get();
    const SfxStyleFamilyItem* pItem = GetFamilyItem();

    OUStringBuffer aMsg(SfxResId(STR_DELETE_STYLE_USED) + SfxResId(STR_DELETE_STYLE));

    pTreeView->selected_foreach(
        [this, pTreeView, pItem, &aList, &bUsedStyle, &aMsg](weld::TreeIter& rEntry) {
            aList.emplace_back(pTreeView->make_iterator(&rEntry));
            // check the style is used or not
            const OUString aTemplName(pTreeView->get_text(rEntry));

            SfxStyleSheetBase* pStyle = m_pStyleSheetPool->Find(aTemplName, pItem->GetFamily());

            if (pStyle->IsUsed()) // pStyle is in use in the document?
            {
                if (bUsedStyle) // add a separator for the second and later styles
                    aMsg.append(", ");
                aMsg.append(aTemplName);
                bUsedStyle = true;
            }

            return false;
        });

    bool aApproved = false;

    // we only want to show the dialog once and if we want to delete a style in use (UX-advice)
    if (bUsedStyle)
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
            pTreeView, VclMessageType::Question, VclButtonsType::YesNo, aMsg.makeStringAndClear()));
        aApproved = xBox->run() == RET_YES;
    }

    // if there are no used styles selected or the user approved the changes
    if (bUsedStyle && !aApproved)
        return;

    for (auto const& elem : aList)
    {
        const OUString aTemplName(pTreeView->get_text(*elem));
        m_bDontUpdate = true; // To prevent the Treelistbox to shut down while deleting
        m_pParentDialog->Execute_Impl(SID_STYLE_DELETE, aTemplName, OUString(),
                                      static_cast<sal_uInt16>(GetFamilyItem()->GetFamily()), *this);

        if (m_xTreeBox->get_visible())
        {
            weld::RemoveParentKeepChildren(*m_xTreeBox, *elem);
            m_bDontUpdate = false;
        }
    }
    m_bDontUpdate = false; // if everything is deleted set m_bDontUpdate back to false
    UpdateStyles(StyleFlags::UpdateFamilyList); // and force-update the list
}

void StyleList::HideHdl()
{
    if (m_nActFamily == 0xffff || !HasSelectedStyle(nullptr))
        return;

    weld::TreeView* pTreeView = m_xTreeBox->get_visible() ? m_xTreeBox.get() : m_xFmtLb.get();
    pTreeView->selected_foreach([this, pTreeView](weld::TreeIter& rEntry) {
        OUString aTemplName = pTreeView->get_text(rEntry);

        m_pParentDialog->Execute_Impl(SID_STYLE_HIDE, aTemplName, OUString(),
                                      static_cast<sal_uInt16>(GetFamilyItem()->GetFamily()), *this);

        return false;
    });
}

void StyleList::ShowHdl()
{
    if (m_nActFamily == 0xffff || !HasSelectedStyle(nullptr))
        return;

    weld::TreeView* pTreeView = m_xTreeBox->get_visible() ? m_xTreeBox.get() : m_xFmtLb.get();
    pTreeView->selected_foreach([this, pTreeView](weld::TreeIter& rEntry) {
        OUString aTemplName = pTreeView->get_text(rEntry);

        m_pParentDialog->Execute_Impl(SID_STYLE_SHOW, aTemplName, OUString(),
                                      static_cast<sal_uInt16>(GetFamilyItem()->GetFamily()), *this);

        return false;
    });
}

IMPL_LINK_NOARG(StyleList, EnableDelete, void*, void)
{
    bool bEnableDelete(false);
    if (m_nActFamily != 0xffff && HasSelectedStyle(nullptr))
    {
        OSL_ENSURE(m_pStyleSheetPool, "No StyleSheetPool");
        const OUString aTemplName(GetSelectedEntry());
        const SfxStyleFamilyItem* pItem = GetFamilyItem();
        const SfxStyleFamily eFam = pItem->GetFamily();
        SfxStyleSearchBits nFilter = SfxStyleSearchBits::Auto;
        if (pItem->GetFilterList().size() > m_nActFilter)
            nFilter = pItem->GetFilterList()[m_nActFilter].nFlags;
        if (nFilter == SfxStyleSearchBits::Auto) // automatic
            nFilter = m_nAppFilter;
        const SfxStyleSheetBase* pStyle = m_pStyleSheetPool->Find(
            aTemplName, eFam, m_xTreeBox->get_visible() ? SfxStyleSearchBits::All : nFilter);

        OSL_ENSURE(pStyle, "Style not found");
        if (pStyle && pStyle->IsUserDefined())
        {
            if (pStyle->HasClearParentSupport() || !pStyle->IsUsed())
            {
                bEnableDelete = true;
            }
        }
    }
    m_pParentDialog->EnableDel(bEnableDelete, this);
}

IMPL_LINK_NOARG(StyleList, Clear, void*, void)
{
    if (m_pCurObjShell && m_bModuleHasStylesHighlighterFeature)
    {
        SfxViewShell* pViewShell = m_pCurObjShell->GetViewShell();
        if (pViewShell)
        {
            pViewShell->GetStylesHighlighterParaColorMap().clear();
            pViewShell->GetStylesHighlighterCharColorMap().clear();
        }
    }
    m_xStyleFamilies.reset();
    for (auto& i : m_pFamilyState)
        i.reset();
    m_pCurObjShell = nullptr;
    for (auto& i : pBoundItems)
        i.reset();
}

void StyleList::ShowMenu(const CommandEvent& rCEvt)
{
    CreateContextMenu();
    weld::TreeView* pTreeView = m_xTreeBox->get_visible() ? m_xTreeBox.get() : m_xFmtLb.get();
    OUString sCommand(
        mxMenu->popup_at_rect(pTreeView, tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1, 1))));
    MenuSelect(sCommand);
}

void StyleList::MenuSelect(const OUString& rIdent)
{
    sLastItemIdent = rIdent;
    if (sLastItemIdent.isEmpty())
        return;
    Application::PostUserEvent(LINK(this, StyleList, MenuSelectAsyncHdl)); /***check this****/
}

void StyleList::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    const SfxHintId nId = rHint.GetId();

    switch (nId)
    {
        case SfxHintId::UpdateDone:
        {
            SfxViewFrame* pViewFrame = m_pBindings->GetDispatcher_Impl()->GetFrame();
            SfxObjectShell* pDocShell = pViewFrame->GetObjectShell();
            if (m_pParentDialog->GetNotifyUpdate()
                && (!m_pParentDialog->IsCheckedItem(u"watercan"_ustr)
                    || (pDocShell && pDocShell->GetStyleSheetPool() != m_pStyleSheetPool)))
            {
                m_pParentDialog->SetNotifyupdate(false);
                Update();
            }
            else if (m_bUpdateFamily)
            {
                UpdateFamily();
                m_aUpdateFamily.Call(*this);
            }

            if (m_pStyleSheetPool)
            {
                OUString aStr = GetSelectedEntry();
                if (!aStr.isEmpty())
                {
                    const SfxStyleFamilyItem* pItem = GetFamilyItem();
                    if (!pItem)
                        break;
                    const SfxStyleFamily eFam = pItem->GetFamily();
                    SfxStyleSheetBase* pStyle = m_pStyleSheetPool->Find(aStr, eFam);
                    if (pStyle)
                    {
                        bool bReadWrite = !(pStyle->GetMask() & SfxStyleSearchBits::ReadOnly);
                        m_pParentDialog->EnableEdit(bReadWrite, this);
                        m_pParentDialog->EnableHide(
                            bReadWrite && !pStyle->IsUsed() && !pStyle->IsHidden(), this);
                        m_pParentDialog->EnableShow(bReadWrite && pStyle->IsHidden(), this);
                    }
                    else
                    {
                        m_pParentDialog->EnableEdit(false, this);
                        m_pParentDialog->EnableHide(false, this);
                        m_pParentDialog->EnableShow(false, this);
                    }
                }
            }
            break;
        }

        // Necessary if switching between documents and in both documents
        // the same template is used. Do not immediately call Update_Impl,
        // for the case that one of the documents is an internal InPlaceObject!
        case SfxHintId::DocChanged:
            m_pParentDialog->SetNotifyupdate(true);
            break;
        case SfxHintId::Dying:
        {
            EndListening(*m_pStyleSheetPool);
            m_pStyleSheetPool = nullptr;
            break;
        }
        default:
            break;
    }

    // Do not set timer when the stylesheet pool is in the box, because it is
    // possible that a new one is registered after the timer is up -
    // works bad in UpdateStyles_Impl ()!

    if (!m_bDontUpdate && nId != SfxHintId::Dying
        && (dynamic_cast<const SfxStyleSheetPoolHint*>(&rHint)
            || dynamic_cast<const SfxStyleSheetHint*>(&rHint)
            || dynamic_cast<const SfxStyleSheetModifiedHint*>(&rHint)))
    {
        if (!pIdle)
        {
            pIdle.reset(new Idle("SfxCommonTemplate"));
            pIdle->SetPriority(TaskPriority::LOWEST);
            pIdle->SetInvokeHandler(LINK(this, StyleList, TimeOut));
        }
        pIdle->Start();
    }
}

IMPL_LINK_NOARG(StyleList, TimeOut, Timer*, void)
{
    if (!m_bDontUpdate)
    {
        m_bDontUpdate = true;
        if (!m_xTreeBox->get_visible())
            UpdateStyles(StyleFlags::UpdateFamilyList);
        else
        {
            FillTreeBox(GetActualFamily());
            SfxTemplateItem* pState = m_pFamilyState[m_nActFamily - 1].get();
            if (pState)
            {
                m_pParentDialog->SelectStyle(pState->GetStyleName(), false, *this);
                EnableDelete(nullptr);
            }
        }
        m_bDontUpdate = false;
        pIdle.reset();
    }
    else
        pIdle->Start();
}

IMPL_LINK_NOARG(StyleList, MenuSelectAsyncHdl, void*, void)
{
    if (sLastItemIdent == "new")
        NewHdl();
    else if (sLastItemIdent == "edit")
        EditHdl();
    else if (sLastItemIdent == "delete")
        DeleteHdl();
    else if (sLastItemIdent == "hide")
        HideHdl();
    else if (sLastItemIdent == "show")
        ShowHdl();
}

// Double-click on a style sheet in the ListBox is applied.
IMPL_LINK(StyleList, DragBeginHdl, bool&, rUnsetDragIcon, bool)
{
    rUnsetDragIcon = false;
    // Allow normal processing. only if bAllowReParentDrop is true
    return !m_bAllowReParentDrop;
}

IMPL_LINK(StyleList, KeyInputHdl, const KeyEvent&, rKeyEvent, bool)
{
    bool bRet = false;
    const vcl::KeyCode& rKeyCode = rKeyEvent.GetKeyCode();
    if (m_bCanDel && !rKeyCode.GetModifier() && rKeyCode.GetCode() == KEY_DELETE)
    {
        DeleteHdl();
        bRet = true;
    }
    return bRet;
}

IMPL_LINK(StyleList, QueryTooltipHdl, const weld::TreeIter&, rEntry, OUString)
{
    weld::TreeView* pTreeView = m_xTreeBox->get_visible() ? m_xTreeBox.get() : m_xFmtLb.get();
    const OUString aTemplName(pTreeView->get_text(rEntry));
    OUString sQuickHelpText(aTemplName);

    const SfxStyleFamilyItem* pItem = GetFamilyItem();
    if (!pItem)
        return sQuickHelpText;
    SfxStyleSheetBase* pStyle = m_pStyleSheetPool->Find(aTemplName, pItem->GetFamily());
    if (pStyle && pStyle->IsUsed()) // pStyle is in use in the document?
    {
        OUString sUsedBy;
        if (pStyle->GetFamily() == SfxStyleFamily::Pseudo)
            sUsedBy = pStyle->GetUsedBy();

        if (!sUsedBy.isEmpty())
        {
            const sal_Int32 nMaxLen = 80;
            if (sUsedBy.getLength() > nMaxLen)
            {
                sUsedBy = OUString::Concat(sUsedBy.subView(0, nMaxLen)) + "...";
            }

            OUString aMessage = SfxResId(STR_STYLEUSEDBY);
            aMessage = aMessage.replaceFirst("%STYLELIST", sUsedBy);
            sQuickHelpText = aTemplName + " " + aMessage;
        }
    }

    return sQuickHelpText;
}

IMPL_LINK(StyleList, CustomRenderHdl, weld::TreeView::render_args, aPayload, void)
{
    vcl::RenderContext& rRenderContext = std::get<0>(aPayload);
    const ::tools::Rectangle& rRect = std::get<1>(aPayload);
    ::tools::Rectangle aRect(
        rRect.TopLeft(),
        Size(rRenderContext.GetOutputSize().Width() - rRect.Left(), rRect.GetHeight()));
    bool bSelected = std::get<2>(aPayload);
    const OUString& rId = std::get<3>(aPayload);

    rRenderContext.Push(vcl::PushFlags::TEXTCOLOR);
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    if (bSelected)
        rRenderContext.SetTextColor(rStyleSettings.GetHighlightTextColor());
    else
        rRenderContext.SetTextColor(rStyleSettings.GetDialogTextColor());

    bool bSuccess = false;

    SfxObjectShell* pShell = SfxObjectShell::Current();
    sfx2::StyleManager* pStyleManager = pShell ? pShell->GetStyleManager() : nullptr;

    if (pStyleManager)
    {
        if (const SfxStyleFamilyItem* pItem = GetFamilyItem())
        {
            SfxStyleSheetBase* pStyleSheet = pStyleManager->Search(rId, pItem->GetFamily());

            if (pStyleSheet)
            {
                rRenderContext.Push(vcl::PushFlags::ALL);
                // tdf#119919 - show "hidden" styles as disabled to not move children onto root node
                if (pStyleSheet->IsHidden())
                    rRenderContext.SetTextColor(rStyleSettings.GetDisableColor());

                sal_Int32 nSize = aRect.GetHeight();
                std::unique_ptr<sfx2::StylePreviewRenderer> pStylePreviewRenderer(
                    pStyleManager->CreateStylePreviewRenderer(rRenderContext, pStyleSheet, nSize));
                bSuccess
                    = pStylePreviewRenderer->recalculate() && pStylePreviewRenderer->render(aRect);
                rRenderContext.Pop();
            }
        }
    }

    if (!bSuccess)
        rRenderContext.DrawText(aRect, rId, DrawTextFlags::Left | DrawTextFlags::VCenter);

    rRenderContext.Pop();
}

// Selection of a template during the Watercan-Status
IMPL_LINK(StyleList, FmtSelectHdl, weld::TreeView&, rListBox, void)
{
    std::unique_ptr<weld::TreeIter> xHdlEntry = rListBox.make_iterator();
    if (!rListBox.get_cursor(xHdlEntry.get()))
        return;

    m_pParentDialog->SelectStyle(rListBox.get_text(*xHdlEntry), true, *this);
}

IMPL_LINK_NOARG(StyleList, TreeListApplyHdl, weld::TreeView&, bool)
{
    // only if that region is allowed
    if (m_nActFamily != 0xffff && nullptr != m_pFamilyState[m_nActFamily - 1]
        && !GetSelectedEntry().isEmpty())
    {
        m_pParentDialog->Execute_Impl(SID_STYLE_APPLY, GetSelectedEntry(), OUString(),
                                      static_cast<sal_uInt16>(GetFamilyItem()->GetFamily()), *this,
                                      SfxStyleSearchBits::Auto, nullptr, &m_nModifier);
    }
    // After selecting a focused item if possible again on the app window
    if (dynamic_cast<const SfxTemplateDialog_Impl*>(m_pParentDialog) != nullptr)
    {
        SfxViewFrame* pViewFrame = m_pBindings->GetDispatcher_Impl()->GetFrame();
        SfxViewShell* pVu = pViewFrame->GetViewShell();
        vcl::Window* pAppWin = pVu ? pVu->GetWindow() : nullptr;
        if (pAppWin)
            pAppWin->GrabFocus();
    }

    return true;
}

IMPL_LINK(StyleList, MousePressHdl, const MouseEvent&, rMEvt, bool)
{
    m_nModifier = rMEvt.GetModifier();
    return false;
}

// Notice from SfxBindings that the update is completed. Pushes out the update
// of the display.
void StyleList::Update()
{
    bool bDocChanged = false;
    SfxStyleSheetBasePool* pNewPool = nullptr;
    SfxViewFrame* pViewFrame = m_pBindings->GetDispatcher_Impl()->GetFrame();
    SfxObjectShell* pDocShell = pViewFrame->GetObjectShell();
    if (pDocShell)
        pNewPool = pDocShell->GetStyleSheetPool();

    if (pNewPool != m_pStyleSheetPool && pDocShell)
    {
        SfxModule* pNewModule = pDocShell->GetModule();
        if (pNewModule && pNewModule != m_Module)
        {
            m_aClearResource.Call(nullptr);
            m_aReadResource.Call(*this);
        }
        if (m_pStyleSheetPool)
        {
            EndListening(*m_pStyleSheetPool);
            m_pStyleSheetPool = nullptr;
        }

        if (pNewPool)
        {
            StartListening(*pNewPool);
            m_pStyleSheetPool = pNewPool;
            bDocChanged = true;
        }
    }

    if (m_bUpdateFamily)
    {
        UpdateFamily();
        m_aUpdateFamily.Call(*this);
    }

    sal_uInt16 i;
    for (i = 0; i < MAX_FAMILIES; ++i)
        if (m_pFamilyState[i])
            break;
    if (i == MAX_FAMILIES || !pNewPool)
        // nothing is allowed
        return;

    SfxTemplateItem* pItem = nullptr;
    // current region not within the allowed region or default
    if (m_nActFamily == 0xffff || nullptr == (pItem = m_pFamilyState[m_nActFamily - 1].get()))
    {
        m_pParentDialog->CheckItem(OUString::number(m_nActFamily), false);
        const size_t nFamilyCount = m_xStyleFamilies->size();
        size_t n;
        for (n = 0; n < nFamilyCount; n++)
            if (m_pFamilyState[StyleNrToInfoOffset(n)])
                break;

        std::unique_ptr<SfxTemplateItem>& pNewItem = m_pFamilyState[StyleNrToInfoOffset(n)];
        m_nAppFilter = pNewItem->GetValue();
        m_pParentDialog->FamilySelect(StyleNrToInfoOffset(n) + 1, *this);
        pItem = pNewItem.get();
    }
    else if (bDocChanged)
    {
        // other DocShell -> all new
        m_pParentDialog->CheckItem(OUString::number(m_nActFamily));
        m_nActFilter = static_cast<sal_uInt16>(m_aLoadFactoryStyleFilter.Call(pDocShell));
        m_pParentDialog->IsUpdate(*this);
        if (0xffff == m_nActFilter)
        {
            m_nActFilter = pDocShell->GetAutoStyleFilterIndex();
        }

        m_nAppFilter = pItem->GetValue();
        if (!m_xTreeBox->get_visible())
        {
            UpdateStyles(StyleFlags::UpdateFamilyList);
        }
        else
            FillTreeBox(GetActualFamily());
    }
    else
    {
        // other filters for automatic
        m_pParentDialog->CheckItem(OUString::number(m_nActFamily));
        const SfxStyleFamilyItem* pStyleItem = GetFamilyItem();
        if (pStyleItem
            && SfxStyleSearchBits::Auto == pStyleItem->GetFilterList()[m_nActFilter].nFlags
            && m_nAppFilter != pItem->GetValue())
        {
            m_nAppFilter = pItem->GetValue();
            if (!m_xTreeBox->get_visible())
                UpdateStyles(StyleFlags::UpdateFamilyList);
            else
                FillTreeBox(GetActualFamily());
        }
        else
        {
            m_nAppFilter = pItem->GetValue();
        }
    }
    const OUString aStyle(pItem->GetStyleName());
    m_pParentDialog->SelectStyle(aStyle, false, *this);
    EnableDelete(nullptr);
    m_pParentDialog->EnableNew(m_bCanNew, this);
}

const SfxStyleFamilyItem& StyleList::GetFamilyItemByIndex(size_t i) const
{
    return m_xStyleFamilies->at(i);
}

IMPL_STATIC_LINK(StyleList, CustomGetSizeHdl, weld::TreeView::get_size_args, aPayload, Size)
{
    vcl::RenderContext& rRenderContext = aPayload.first;
    return Size(42, 32 * rRenderContext.GetDPIScaleFactor());
}

IMPL_LINK(StyleList, PopupFlatMenuHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    PrepareMenu(rCEvt.GetMousePosPixel());

    if (m_xFmtLb->count_selected_rows() <= 0)
    {
        m_pParentDialog->EnableEdit(false, this);
        m_pParentDialog->EnableDel(false, this);
    }

    ShowMenu(rCEvt);

    return true;
}

IMPL_LINK(StyleList, PopupTreeMenuHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    PrepareMenu(rCEvt.GetMousePosPixel());

    ShowMenu(rCEvt);

    return true;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
