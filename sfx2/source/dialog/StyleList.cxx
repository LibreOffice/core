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
#include <com/sun/star/frame/XModel3.hpp>
#include <utility>
#include <vcl/commandevent.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld/Menu.hxx>
#include <vcl/weld/MessageDialog.hxx>
#include <vcl/weld/weldutils.hxx>
#include <vcl/window.hxx>
#include <svl/intitem.hxx>
#include <svl/style.hxx>
#include <svl/itemset.hxx>
#include <comphelper/lok.hxx>
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

Color ColorHash(std::u16string_view rString)
{
    static constexpr auto aSaturationArray = std::to_array<sal_uInt16>({ 90, 75, 60 });
    static constexpr auto aBrightnessArray = std::to_array<sal_uInt16>({ 100, 80, 60 });
    static constexpr auto aTintOrShadeArray
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

namespace
{
// used to disallow the default character style in the styles spotlight character styles color map
std::optional<OUString> sDefaultCharStyleUIName;
}

// Constructor

StyleList::StyleList(weld::Builder* pBuilder, SfxBindings* pBindings,
                     SfxCommonTemplateDialog_Impl* Parent, weld::Container* pC,
                     const OUString& treeviewname, const OUString& flatviewname)
    : m_bHierarchical(false)
    , m_bAllowReParentDrop(false)
    , m_bNewByExampleDisabled(false)
    , m_bDontUpdate(false)
    , m_bTreeDrag(true)
    , m_bCanEdit(false)
    , m_bCanHide(true)
    , m_bCanShow(false)
    , m_bCanNew(true)
    , m_bCanDel(false)
    , m_bBindingUpdate(true)
    , m_pStyleSheetPool(nullptr)
    , m_xFmtLb(pBuilder->weld_tree_view(flatviewname))
    , m_xTreeBox(pBuilder->weld_tree_view(treeviewname))
    , m_pCurObjShell(nullptr)
    , m_nActFamily(0xffff)
    , m_pParentDialog(Parent)
    , m_pBindings(pBindings)
    , m_Module(nullptr)
    , m_nModifier(0)
    , m_pContainer(pC)
{
    m_xFmtLb->set_help_id(HID_TEMPLATE_FMT);

    uno::Reference<frame::XFrame> xFrame
        = m_pBindings->GetDispatcher()->GetFrame()->GetFrame().GetFrameInterface();
    m_bModuleHasStylesSpotlightFeature
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
    mxMenuBuilder = Application::CreateBuilder(m_pContainer, u"sfx/ui/stylecontextmenu.ui"_ustr);
    mxMenu = mxMenuBuilder->weld_menu(u"menu"_ustr);
    mxMenu->set_sensitive(u"edit"_ustr, m_bCanEdit);
    mxMenu->set_sensitive(u"delete"_ustr, m_bCanDel);
    mxMenu->set_sensitive(u"new"_ustr, m_bCanNew);
    mxMenu->set_sensitive(u"hide"_ustr, m_bCanHide);
    mxMenu->set_sensitive(u"show"_ustr, m_bCanShow);

    const SfxStyleFamilyItem* pItem = GetFamilyItem();
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

// called from SfxCommonTemplateDialog_Impl::ReadResource
IMPL_LINK_NOARG(StyleList, ReadResource, void*, size_t)
{
    // Read global user resource
    for (auto& i : m_pFamilyState)
        i.reset();

    SfxViewFrame* pViewFrame = m_pBindings->GetDispatcher_Impl()->GetFrame();
    m_pCurObjShell = pViewFrame->GetObjectShell();
    m_Module = m_pCurObjShell ? m_pCurObjShell->GetModule() : nullptr;
    if (m_Module)
        m_aStyleFamilies = m_Module->CreateStyleFamilies();

    for (SfxStyleFamilyItem& rFamilyItem : m_aStyleFamilies)
        m_aFamilySelectedFiltersSet[rFamilyItem.GetFamily()] = { SfxStyleSearchBits::AllVisible };

    if (m_pCurObjShell)
    {
        if (m_bModuleHasStylesSpotlightFeature)
            sDefaultCharStyleUIName = getDefaultStyleName(SfxStyleFamily::Char);
    }
    size_t nCount = m_aStyleFamilies.size();
    m_pBindings->ENTERREGISTRATIONS();

    size_t i;
    for (i = 0; i < nCount; ++i)
    {
        sal_uInt16 nSlot = 0;
        switch (m_aStyleFamilies.at(i).GetFamily())
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

    m_xFmtLb->connect_item_activated(LINK(this, StyleList, TreeListApplyHdl));
    m_xFmtLb->connect_mouse_press(LINK(this, StyleList, MousePressHdl));
    m_xFmtLb->connect_query_tooltip(LINK(this, StyleList, QueryTooltipHdl));
    m_xFmtLb->connect_selection_changed(LINK(this, StyleList, FmtSelectHdl));
    m_xFmtLb->connect_command(LINK(this, StyleList, PopupFlatMenuHdl));
    m_xFmtLb->connect_key_press(LINK(this, StyleList, KeyInputHdl));
    m_xFmtLb->set_selection_mode(SelectionMode::Multiple);
    m_xTreeBox->connect_expanding(LINK(this, StyleList, ExpandHdl));
    m_xTreeBox->connect_collapsing(LINK(this, StyleList, CollapseHdl));
    m_xTreeBox->connect_selection_changed(LINK(this, StyleList, FmtSelectHdl));
    m_xTreeBox->connect_item_activated(LINK(this, StyleList, TreeListApplyHdl));
    m_xTreeBox->connect_mouse_press(LINK(this, StyleList, MousePressHdl));
    m_xTreeBox->connect_query_tooltip(LINK(this, StyleList, QueryTooltipHdl));
    m_xTreeBox->connect_command(LINK(this, StyleList, PopupTreeMenuHdl));
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

IMPL_LINK(StyleList, ExpandHdl, const weld::TreeIter&, rIter, bool)
{
    m_aFamilyExpandedStyleEntriesSet[GetActualFamily()].insert(m_xTreeBox->get_text(rIter));
    return true;
}

IMPL_LINK(StyleList, CollapseHdl, const weld::TreeIter&, rIter, bool)
{
    m_aFamilyExpandedStyleEntriesSet[GetActualFamily()].erase(m_xTreeBox->get_text(rIter));
    return true;
}

// called by SfxCommonTemplateDialog_Impl::PreviewHdl
void StyleList::ShowPreviews(bool bEnable)
{
    m_xFmtLb->clear();
    m_xFmtLb->set_column_custom_renderer(1, bEnable);
    m_xTreeBox->clear();
    m_xTreeBox->set_column_custom_renderer(1, bEnable);

    m_bHierarchical ? FillHierarchicalTreeView() : FillFlatTreeView();
}

bool StyleList::EnableExecute()
{
    return m_xTreeBox->get_visible() || m_xFmtLb->count_selected_rows() <= 1;
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
    pTreeView->get_dest_row_at_pos(rEvt.maPosPixel, true);
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

    std::unique_ptr<weld::TreeIter> xSource = m_xTreeBox->get_selected();
    if (!xSource)
        return DND_ACTION_NONE;

    std::unique_ptr<weld::TreeIter> xTarget
        = m_xTreeBox->get_dest_row_at_pos(rEvt.maPosPixel, true);
    if (!xTarget)
    {
        // if nothing under the mouse, use the last row
        int nChildren = m_xTreeBox->n_children();
        if (!nChildren)
            return DND_ACTION_NONE;
        xTarget = m_xTreeBox->make_iterator();
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
    FillHierarchicalTreeView();
    m_pParentDialog->SelectStyle(aTargetStyle);
    return DND_ACTION_NONE;
}

IMPL_LINK_NOARG(StyleList, NewMenuExecuteAction, void*, void)
{
    if (!m_pStyleSheetPool || m_nActFamily == 0xffff)
        return;

    if (const SfxStyleFamilyItem* pFamilyItem = GetFamilyItem())
    {
        sal_uInt16 nActFilter = GetActiveFilter();
        SfxStyleSearchBits nFilter(SfxStyleSearchBits::Auto);
        if (nActFilter != 0xffff)
            nFilter = pFamilyItem->GetFilterList()[nActFilter].nFlags;

        // why? : FloatingWindow must not be parent of a modal dialog
        const SfxStyleFamily eFam = pFamilyItem->GetFamily();
        SfxNewStyleDlg aDlg(m_pContainer, *m_pStyleSheetPool, eFam);
        auto nResult = aDlg.run();
        if (nResult == RET_OK)
        {
            const OUString aTemplName(aDlg.GetName());
            m_pParentDialog->Execute_Impl(SID_STYLE_NEW_BY_EXAMPLE, aTemplName, u""_ustr,
                                          static_cast<sal_uInt16>(eFam), *this, nFilter);
            m_bHierarchical ? FillHierarchicalTreeView() : FillFlatTreeView();
            m_pParentDialog->SelectStyle(aTemplName);
            m_aUpdateFamily.Call(*this);
        }
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

Point StyleList::PrepareMenu(const CommandEvent& rContextMenuEvent)
{
    weld::TreeView* pTreeView = m_xTreeBox->get_visible() ? m_xTreeBox.get() : m_xFmtLb.get();
    if (rContextMenuEvent.IsMouseEvent())
    {
        const Point& rPos = rContextMenuEvent.GetMousePosPixel();
        std::unique_ptr<weld::TreeIter> xIter = pTreeView->get_dest_row_at_pos(rPos, false);
        if (xIter && !pTreeView->is_selected(*xIter))
        {
            pTreeView->unselect_all();
            pTreeView->set_cursor(*xIter);
            pTreeView->select(*xIter);
        }
        FmtSelectHdl(*pTreeView);
        return rPos;
    }
    else
    {
        if (std::unique_ptr<weld::TreeIter> pSelected = pTreeView->get_selected())
            return pTreeView->get_row_area(*pSelected).Center();
        return {};
    }
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
    sal_Int32 nSpotlightId;
    StyleTreeArr_Impl pChildren;

public:
    bool HasParent() const { return !aParent.isEmpty(); }

    StyleTree_Impl(OUString _aName, OUString _aParent, sal_Int32 _nSpotlightId)
        : aName(std::move(_aName))
        , aParent(std::move(_aParent))
        , nSpotlightId(_nSpotlightId)
        , pChildren(0)
    {
    }

    const OUString& getName() const { return aName; }
    const OUString& getParent() const { return aParent; }
    sal_Int32 getSpotlightId() const { return nSpotlightId; }
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
    // Paradoxically, with a list and non-Latin style names,
    // sorting twice is faster than sorting once.
    // The first sort has a cheap comparator, and gets the list into mostly-sorted order.
    // Then the second sort needs to call its (much more expensive) comparator less often.
    std::sort(rArr.begin(), rArr.end(),
              [](std::unique_ptr<StyleTree_Impl> const& pEntry1,
                 std::unique_ptr<StyleTree_Impl> const& pEntry2) {
                  return pEntry1->getName() < pEntry2->getName();
              });
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

static void InsertSpotlightEntry(weld::TreeView& rTreeView, const weld::TreeIter& rIter,
                                 const StyleTree_Impl& rEntry, SfxStyleFamily eFam,
                                 SfxViewShell* pViewSh)
{
    const OUString& rName = rEntry.getName();

    Color aColor = ColorHash(rName);

    // For kit keep the id used for spotlight/number-image for a style stable
    // regardless of the selection mode of the style panel, so multiple views
    // on a document all share the same id for a style.
    sal_Int32 nSpotlightId;
    if (comphelper::LibreOfficeKit::isActive())
        nSpotlightId = rEntry.getSpotlightId();
    else
    {
        StylesSpotlightColorMap& rColorMap = (eFam == SfxStyleFamily::Para)
                                                 ? pViewSh->GetStylesSpotlightParaColorMap()
                                                 : pViewSh->GetStylesSpotlightCharColorMap();
        nSpotlightId = rColorMap.size() + 1;
        rColorMap[rName] = std::pair(aColor, nSpotlightId);
    }

    if (eFam == SfxStyleFamily::Char)
    {
        // don't show a color or number for default character style 'No Character Style' entry
        if (rName == sDefaultCharStyleUIName.value() /*"No Character Style"*/)
        {
            rTreeView.set_id(rIter, rName);
            rTreeView.set_text(rIter, rName);
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
    // In kit mode, unused styles are -1, so we can just skip the number image for those
    if (nSpotlightId != -1)
    {
        xDevice->SetTextColor(COL_BLACK);
        xDevice->DrawText(aRect, OUString::number(nSpotlightId),
                          DrawTextFlags::Center | DrawTextFlags::VCenter);
    }

    rTreeView.set_id(rIter, rName);
    rTreeView.set_text(rIter, rName);
    rTreeView.set_image(rIter, *xDevice, 0);
}

static void RemoveHiddenEntriesWithNoVisibleChildren(StyleTreeArr_Impl& rTreeArr,
                                                     SfxStyleFamily eFam,
                                                     SfxStyleSheetBasePool* pStyleSheetPool)
{
    for (size_t i = 0; i < rTreeArr.size(); i++)
    {
        StyleTree_Impl* pChildEntry = rTreeArr[i].get();
        StyleTreeArr_Impl& rChildren = pChildEntry->getChildren();
        if (!rChildren.empty())
            RemoveHiddenEntriesWithNoVisibleChildren(rChildren, eFam, pStyleSheetPool);
        SfxStyleSheetBase* pStyle = pStyleSheetPool->Find(pChildEntry->getName(), eFam);
        if (pStyle->IsHidden() && rChildren.empty())
        {
            rTreeArr.erase(rTreeArr.begin() + i);
            --i;
        }
    }
}

static void FillBox_Impl(weld::TreeView& rBox, StyleTreeArr_Impl& rTreeArray,
                         SfxStyleFamily eStyleFamily, const weld::TreeIter* pParent,
                         bool bSpotlightFill, SfxViewShell* pViewShell,
                         SfxStyleSheetBasePool* pStyleSheetPool)
{
    if (rTreeArray.empty())
        return;
    rBox.bulk_insert_for_each(
        rTreeArray.size(),
        [&rTreeArray, bSpotlightFill, pStyleSheetPool, eStyleFamily, &rBox,
         pViewShell](weld::TreeIter& rIter, int i) {
            StyleTree_Impl* pChildEntry = rTreeArray[i].get();
            const OUString& rChildName = pChildEntry->getName();
            const SfxStyleSheetBase* pStyle = pStyleSheetPool->Find(rChildName, eStyleFamily);
            if (bSpotlightFill)
            {
                if (pStyle && pStyle->IsUsed())
                    InsertSpotlightEntry(rBox, rIter, *pChildEntry, eStyleFamily, pViewShell);
                else
                {
                    rBox.set_id(rIter, rChildName);
                    rBox.set_text(rIter, rChildName);
                }
            }
            else
            {
                rBox.set_id(rIter, rChildName);
                rBox.set_text(rIter, rChildName);
            }
            if (pStyle && pStyle->IsHidden())
                rBox.set_font_color(
                    rIter, Application::GetSettings().GetStyleSettings().GetDisableColor());
        },
        pParent, nullptr, /*bGoingToSetText*/ true,
        /*ForceForwardInsert*/ bSpotlightFill ? true : false);

    std::unique_ptr<weld::TreeIter> xChildParentIter = rBox.make_iterator(pParent);
    if (!pParent)
        (void)rBox.get_iter_first(*xChildParentIter);
    else
        (void)rBox.iter_children(*xChildParentIter);
    for (size_t i = 0; i < rTreeArray.size(); ++i)
    {
        FillBox_Impl(rBox, rTreeArray[i]->getChildren(), eStyleFamily, xChildParentIter.get(),
                     bSpotlightFill, pViewShell, pStyleSheetPool);
        (void)rBox.iter_next_sibling(*xChildParentIter);
    }
}

// Helper function: Access to the current family item
SfxStyleFamilyItem* StyleList::GetFamilyItem()
{
    const size_t nCount = m_aStyleFamilies.size();
    for (size_t i = 0; i < nCount; ++i)
    {
        SfxStyleFamilyItem& rItem = m_aStyleFamilies.at(i);
        sal_uInt16 nId = SfxTemplate::SfxFamilyIdToNId(rItem.GetFamily());
        if (nId == m_nActFamily)
            return &rItem;
    }
    return nullptr;
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

// called from SfxCommonTemplateDialog_Impl::SetWaterCanState
IMPL_LINK(StyleList, SetWaterCanState, const SfxBoolItem*, pItem, void)
{
    size_t nCount = m_aStyleFamilies.size();
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

// BASED ON THE REMOVED StyleList::UpdateFamily FUNCTION
// called from SfxCommonTemplateDialog_Impl::FamilySelect
void StyleList::FamilySelect(sal_uInt16 nEntry, bool bFillTreeView)
{
    m_nActFamily = nEntry;

    // update style sheet pool listener
    SfxViewFrame* pViewFrame = m_pBindings->GetDispatcher_Impl()->GetFrame();
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

    if (bFillTreeView)
        m_bHierarchical ? FillHierarchicalTreeView() : FillFlatTreeView();

    InvalidateBindings();
}

sal_uInt16 StyleList::GetActiveFilter()
{
    sal_uInt16 nActFilter = 0Xffff;

    for (sal_uInt16 n = 0; const SfxFilterTuple& rFilterTuple : GetFamilyItem()->GetFilterList())
    {
        if (rFilterTuple.nFlags == SfxStyleSearchBits::AllVisible
            || rFilterTuple.nFlags == SfxStyleSearchBits::All
            || rFilterTuple.nFlags == SfxStyleSearchBits::Used
            || rFilterTuple.nFlags == SfxStyleSearchBits::Auto)
            ;
        else if (m_aFamilySelectedFiltersSet[GetFamilyItem()->GetFamily()].contains(
                     rFilterTuple.nFlags))
        {
            nActFilter = n;
            break;
        }
        n++;
    }

    if (nActFilter == 0xffff)
        nActFilter = m_pCurObjShell->GetAutoStyleFilterIndex();

    return nActFilter;
}

// called by: SfxCommonTemplateDialog_Impl::SelectUpdate
void StyleList::SelectUpdate(const OUString& rStr)
{
    const SfxStyleFamilyItem* pFamilyItem = GetFamilyItem();
    if (!pFamilyItem)
        return;
    const SfxStyleFamily eFam = pFamilyItem->GetFamily();
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
}

// It selects the style in treeview
// called from SfxCommonTemplateDialog_Impl::SelectStyle
void StyleList::SelectStyle(std::u16string_view rStr)
{
    const SfxStyleFamilyItem* pFamilyItem = GetFamilyItem();
    if (!pFamilyItem)
        return;
    const SfxStyleFamily eFam = pFamilyItem->GetFamily();
    if (m_bHierarchical)
    {
        if (!rStr.empty())
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
        bool bSelect = !rStr.empty();
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

// this gets called from SfxTempateControllerItem::StateChangedAtToolBoxControl
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

void StyleList::FillHierarchicalTreeView(bool bExpandRootParents)
{
    assert(m_xTreeBox && "FillHierarchicalTreeView() without treebox");
    if (!m_pStyleSheetPool || m_nActFamily == 0xffff)
        return;

    SfxStyleFamilyItem* pItem = GetFamilyItem();
    if (!pItem)
        return;

    SfxStyleFamily eFam = pItem->GetFamily();

    StyleTreeArr_Impl aArr;

    auto& rFamilySelectedFiltersSet = m_aFamilySelectedFiltersSet[GetFamilyItem()->GetFamily()];

    bool bShowHiddenInFilter = rFamilySelectedFiltersSet.contains(SfxStyleSearchBits::Hidden)
                               && rFamilySelectedFiltersSet.size() > 1;

    // std::set<std::pair<OUString, OUString>> aStyleSheetSet is used to check for styles that are
    // already added to aArr.
    for (std::set<std::pair<OUString, OUString>> aStyleSheetSet;
         SfxStyleSearchBits eStyleSearchBits : rFamilySelectedFiltersSet)
    {
        // only include all hidden styles when the hidden styles filter is the only filter selected
        if (eStyleSearchBits == SfxStyleSearchBits::Hidden && rFamilySelectedFiltersSet.size() > 1)
            continue;

        // used styles can't be hidden but hidden styles can be used - hmm
        if (eStyleSearchBits != SfxStyleSearchBits::Hidden
            && eStyleSearchBits != SfxStyleSearchBits::Auto
            && eStyleSearchBits != SfxStyleSearchBits::Used)
        {
            eStyleSearchBits |= SfxStyleSearchBits::Hidden;
        }

        SfxStyleSheetBase* pStyle = m_pStyleSheetPool->First(eFam, eStyleSearchBits);
        while (pStyle)
        {
            if (aStyleSheetSet.insert(std::pair(pStyle->GetName(), pStyle->GetParent())).second)
            {
                std::unique_ptr<StyleTree_Impl> pNew = std::make_unique<StyleTree_Impl>(
                    pStyle->GetName(), pStyle->GetParent(), pStyle->GetSpotlightId());
                aArr.push_back(std::move(pNew));
            }
            pStyle = m_pStyleSheetPool->Next();
        }
    }

    OUString aUIName = getDefaultStyleName(eFam);
    MakeTree_Impl(aArr, aUIName);

    // When the Hidden Style filter is not selected remove hidden entries with no visible children
    if (!rFamilySelectedFiltersSet.contains(SfxStyleSearchBits::Hidden) && !bShowHiddenInFilter)
        RemoveHiddenEntriesWithNoVisibleChildren(aArr, eFam, m_pStyleSheetPool);

    m_xTreeBox->freeze();
    m_xTreeBox->clear();

    SfxViewShell* pViewShell = m_pCurObjShell->GetViewShell();
    StylesSpotlightColorMap* pSpotlightColorMap = nullptr;
    bool bOrigMapHasEntries = false;
    if (pViewShell && m_bModuleHasStylesSpotlightFeature)
    {
        if (eFam == SfxStyleFamily::Para)
            pSpotlightColorMap = &pViewShell->GetStylesSpotlightParaColorMap();
        else if (eFam == SfxStyleFamily::Char)
            pSpotlightColorMap = &pViewShell->GetStylesSpotlightCharColorMap();
    }

    if (pSpotlightColorMap && !pSpotlightColorMap->empty())
    {
        bOrigMapHasEntries = true;
        pSpotlightColorMap->clear();
    }

    bool bSpotlightFill = pViewShell && m_bModuleHasStylesSpotlightFeature
                          && ((eFam == SfxStyleFamily::Para && m_bSpotlightParaStyles)
                              || (eFam == SfxStyleFamily::Char && m_bSpotlightCharStyles));

    FillBox_Impl(*m_xTreeBox, aArr, eFam, nullptr, bSpotlightFill, pViewShell, m_pStyleSheetPool);

    m_xTreeBox->columns_autosize();

    m_xTreeBox->thaw();

    // make view update
    if (pViewShell && pSpotlightColorMap && (!pSpotlightColorMap->empty() || bOrigMapHasEntries))
        static_cast<SfxListener*>(pViewShell)
            ->Notify(*m_pStyleSheetPool, SfxHint(SfxHintId::StylesSpotlightModified));

    // expand all root parents the first time the family is visited
    if (!m_aFamiliesThatHaveBeenSelectedSet.contains(GetActualFamily()))
    {
        m_aFamiliesThatHaveBeenSelectedSet.insert(GetActualFamily());
        bExpandRootParents = true;
    }

    // bExpandRootParents is set true when filter selections are made
    if (bExpandRootParents)
    {
        std::unique_ptr<weld::TreeIter> xEntry = m_xTreeBox->make_iterator();
        bool bEntry = m_xTreeBox->get_iter_first(*xEntry);
        while (bEntry)
        {
            if (m_xTreeBox->iter_has_child(*xEntry))
            {
                m_aFamilyExpandedStyleEntriesSet[GetActualFamily()].insert(
                    m_xTreeBox->get_text(*xEntry));
            }
            bEntry = m_xTreeBox->iter_next_sibling(*xEntry);
        }
    }

    // restore the tree expand state
    std::unique_ptr<weld::TreeIter> xEntry = m_xTreeBox->make_iterator();
    bool bEntry = m_xTreeBox->get_iter_first(*xEntry);
    while (bEntry)
    {
        if (m_aFamilyExpandedStyleEntriesSet[GetActualFamily()].contains(
                m_xTreeBox->get_text(*xEntry)))
        {
            m_xTreeBox->expand_row(*xEntry);
        }
        bEntry = m_xTreeBox->iter_next(*xEntry);
    }

    // select style
    OUString aStyle;
    if (SfxTemplateItem* pState = m_pFamilyState[m_nActFamily - 1].get())
        aStyle = pState->GetStyleName();
    m_pParentDialog->SelectStyle(aStyle);
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

SfxStyleFamily StyleList::GetActualFamily()
{
    SfxStyleFamilyItem* pFamilyItem = GetFamilyItem();
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

// called from SfxCommonTemplateDialog_Impl::SelectUpdate
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

void StyleList::FillFlatTreeView()
{
    OSL_ENSURE(m_pStyleSheetPool, "no StyleSheetPool");
    if (!m_pStyleSheetPool)
        return;

    SfxStyleFamilyItem* pFamilyItem = GetFamilyItem();
    if (!pFamilyItem)
        return;

    const SfxStyleFamily eFam = pFamilyItem->GetFamily();

    std::unique_ptr<weld::TreeIter> xEntry = m_xFmtLb->make_iterator();
    std::vector<StyleTree_Impl> aStyles;

    comphelper::string::NaturalStringSorter aSorter(
        ::comphelper::getProcessComponentContext(),
        Application::GetSettings().GetLanguageTag().getLocale());

    auto& rFamilySelectedFiltersSet = m_aFamilySelectedFiltersSet[GetFamilyItem()->GetFamily()];

    bool bShowHiddenInFilter = rFamilySelectedFiltersSet.contains(SfxStyleSearchBits::Hidden)
                               && rFamilySelectedFiltersSet.size() > 1;

    // std::set<std::pair<OUString, OUString>> aStyleSheetSet is used to prevent duplicate entries
    for (std::set<std::pair<OUString, OUString>> aStyleSheetSet;
         SfxStyleSearchBits eStyleSearchBits : rFamilySelectedFiltersSet)
    {
        if (eStyleSearchBits == SfxStyleSearchBits::Hidden
            && bShowHiddenInFilter /*rFamilySelectedFiltersSet.size() > 1*/)
            continue;

        // used styles can't be hidden but hidden styles can be used - hmm
        if (bShowHiddenInFilter && eStyleSearchBits != SfxStyleSearchBits::Auto
            && eStyleSearchBits != SfxStyleSearchBits::Used)
            eStyleSearchBits |= SfxStyleSearchBits::Hidden;

        SfxStyleSheetBase* pStyle = m_pStyleSheetPool->First(eFam, eStyleSearchBits);
        while (pStyle)
        {
            if (eStyleSearchBits != SfxStyleSearchBits::Hidden && pStyle->IsHidden()
                && !bShowHiddenInFilter)
                ;
            else if (aStyleSheetSet.insert(std::pair(pStyle->GetName(), pStyle->GetParent()))
                         .second)
            {
                aStyles.emplace_back(pStyle->GetName(), pStyle->GetParent(),
                                     pStyle->GetSpotlightId());
            }
            pStyle = m_pStyleSheetPool->Next();
        }
    }

    OUString aUIName = getDefaultStyleName(eFam);

    // Paradoxically, with a list and non-Latin style names,
    // sorting twice is faster than sorting once.
    // The first sort has a cheap comparator, and gets the list into mostly-sorted order.
    // Then the second sort needs to call its (much more expensive) comparator less often.
    std::sort(aStyles.begin(), aStyles.end(),
              [](const StyleTree_Impl& rLHS, const StyleTree_Impl& rRHS) {
                  return rLHS.getName() < rRHS.getName();
              });
    std::sort(aStyles.begin(), aStyles.end(),
              [&aSorter, &aUIName](const StyleTree_Impl& rLHSS, const StyleTree_Impl& rRHSS) {
                  const OUString& rLHS = rLHSS.getName();
                  const OUString& rRHS = rRHSS.getName();
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
    StylesSpotlightColorMap* pSpotlightColorMap = nullptr;
    bool bOrigMapHasEntries = false;
    if (pViewShell && m_bModuleHasStylesSpotlightFeature)
    {
        if (eFam == SfxStyleFamily::Para)
            pSpotlightColorMap = &pViewShell->GetStylesSpotlightParaColorMap();
        else if (eFam == SfxStyleFamily::Char)
            pSpotlightColorMap = &pViewShell->GetStylesSpotlightCharColorMap();
    }

    if (pSpotlightColorMap && !pSpotlightColorMap->empty())
    {
        bOrigMapHasEntries = true;
        pSpotlightColorMap->clear();
    }

    size_t nCount = aStyles.size();

    if (pViewShell && m_bModuleHasStylesSpotlightFeature
        && ((eFam == SfxStyleFamily::Para && m_bSpotlightParaStyles)
            || (eFam == SfxStyleFamily::Char && m_bSpotlightCharStyles)))
    {
        m_xFmtLb->bulk_insert_for_each(
            nCount,
            [this, &aStyles, eFam, pViewShell](weld::TreeIter& rIter, int nIdx) {
                const OUString& rName = aStyles[nIdx].getName();
                auto pChildStyle = m_pStyleSheetPool->Find(rName, eFam);
                if (pChildStyle && pChildStyle->IsUsed())
                    InsertSpotlightEntry(*m_xFmtLb, rIter, aStyles[nIdx], eFam, pViewShell);
                else
                {
                    m_xFmtLb->set_id(rIter, rName);
                    m_xFmtLb->set_text(rIter, rName);
                }
                if (pChildStyle && pChildStyle->IsHidden())
                    m_xFmtLb->set_font_color(
                        rIter, Application::GetSettings().GetStyleSettings().GetDisableColor());
            },
            nullptr, nullptr, /*bGoingToSetText*/ true, /*ForceForwardInsert*/ true);
    }
    else
    {
        m_xFmtLb->bulk_insert_for_each(
            nCount,
            [this, &aStyles, eFam](weld::TreeIter& rIter, int nIdx) {
                const OUString& rName = aStyles[nIdx].getName();
                m_xFmtLb->set_id(rIter, rName);
                m_xFmtLb->set_text(rIter, rName);
                m_xFmtLb->set_id(rIter, rName);
                m_xFmtLb->set_text(rIter, rName);
                auto pStyle = m_pStyleSheetPool->Find(rName, eFam);
                if (pStyle && pStyle->IsHidden())
                    m_xFmtLb->set_font_color(
                        rIter, Application::GetSettings().GetStyleSettings().GetDisableColor());
            },
            nullptr, nullptr, /*bGoingToSetText*/ true);
    }

    m_xFmtLb->columns_autosize();

    m_xFmtLb->thaw();

    // make view update
    if (pViewShell && pSpotlightColorMap && (!pSpotlightColorMap->empty() || bOrigMapHasEntries))
        static_cast<SfxListener*>(pViewShell)
            ->Notify(*m_pStyleSheetPool, SfxHint(SfxHintId::StylesSpotlightModified));

    // Selects the current style if any
    SfxTemplateItem* pState = m_pFamilyState[m_nActFamily - 1].get();
    OUString aStyle;
    if (pState)
        aStyle = pState->GetStyleName();
    m_pParentDialog->SelectStyle(aStyle);
}

// called from SfxCommonTemplateDialog_Impl::SetFamilyState
void StyleList::SetFamilyState(sal_uInt16 nSlotId, const SfxTemplateItem* pItem)
{
    sal_uInt16 nIdx = nSlotId - SID_STYLE_FAMILY_START;
    m_pFamilyState[nIdx].reset();
    if (pItem)
        m_pFamilyState[nIdx].reset(new SfxTemplateItem(*pItem));
}

// called from StyleList::Update and SfxCommonTemplateDialog_Impl::HierarchicalHdl
void StyleList::ShowHierarchicalView()
{
    m_bHierarchical = true;
    m_xFmtLb->hide();
    FillHierarchicalTreeView();
    m_xTreeBox->show();
}

// called from StyleList::Update and SfxCommonTemplateDialog_Impl::HierarchicalHdl
void StyleList::ShowFlatView()
{
    m_bHierarchical = false;
    m_xTreeBox->hide();
    FillFlatTreeView();
    m_xFmtLb->show();
}

// Handler for the New-Buttons
void StyleList::NewHdl()
{
    if (m_nActFamily == 0xffff
        || !(m_xTreeBox->get_visible() || m_xFmtLb->count_selected_rows() <= 1))
        return;

    sal_uInt16 nActFilter = GetActiveFilter();

    const SfxStyleFamilyItem* pItem = GetFamilyItem();
    const SfxStyleFamily eFam = pItem->GetFamily();
    SfxStyleSearchBits nMask(SfxStyleSearchBits::Auto);
    if (nActFilter != 0xffff)
        nMask = pItem->GetFilterList()[nActFilter].nFlags;

    m_pParentDialog->Execute_Impl(SID_STYLE_NEW, u""_ustr, GetSelectedEntry(),
                                  static_cast<sal_uInt16>(eFam), *this, nMask);
}

// Handler for the edit-Buttons
void StyleList::EditHdl()
{
    if (m_nActFamily != 0xffff && HasSelectedStyle(nullptr))
    {
        sal_uInt16 nFilter = GetActiveFilter();
        OUString aTemplName(GetSelectedEntry());
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
        if (m_bHierarchical)
        {
            weld::RemoveParentKeepChildren(*m_xTreeBox, *elem);
            m_bDontUpdate = false;
        }
    }
    m_bDontUpdate = false; // if everything is deleted set m_bDontUpdate back to false

    if (!m_bHierarchical)
        FillFlatTreeView();
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

// only called from SfxCommonTemplateDialog_Impl::SelectUpdate
IMPL_LINK_NOARG(StyleList, EnableDelete, void*, void)
{
    bool bEnableDelete(false);
    if (m_nActFamily != 0xffff && HasSelectedStyle(nullptr))
    {
        OSL_ENSURE(m_pStyleSheetPool, "No StyleSheetPool");
        const OUString aTemplName(GetSelectedEntry());
        const SfxStyleFamily eFam = GetFamilyItem()->GetFamily();
        for (const SfxStyleSearchBits& rFilter : m_aFamilySelectedFiltersSet[eFam])
        {
            const SfxStyleSheetBase* pStyle = m_pStyleSheetPool->Find(aTemplName, eFam, rFilter);
            if (pStyle && pStyle->IsUserDefined())
            {
                if (pStyle->HasClearParentSupport() || !pStyle->IsUsed())
                {
                    bEnableDelete = true;
                    break;
                }
            }
        }
    }
    m_pParentDialog->EnableDel(bEnableDelete, this);
}

// called from SfxCommonTemplateDialog_Impl::ClearResource and
// SfxCommonTemplateDialog_Impl::~SfxCommonTemplateDialog_Impl
IMPL_LINK_NOARG(StyleList, Clear, void*, void)
{
    if (m_pCurObjShell && m_bModuleHasStylesSpotlightFeature)
    {
        SfxViewShell* pViewShell = m_pCurObjShell->GetViewShell();
        if (pViewShell)
        {
            pViewShell->GetStylesSpotlightParaColorMap().clear();
            pViewShell->GetStylesSpotlightCharColorMap().clear();
        }
    }
    m_aStyleFamilies.clear();
    m_aFamilyExpandedStyleEntriesSet.clear();
    m_aFamilySelectedFiltersSet.clear();
    m_aFamiliesThatHaveBeenSelectedSet.clear();
    for (auto& i : m_pFamilyState)
        i.reset();
    m_pCurObjShell = nullptr;
    for (auto& i : pBoundItems)
        i.reset();
}

IMPL_LINK(StyleList, OnPopupEnd, const OUString&, sCommand, void) { MenuSelect(sCommand); }

void StyleList::ShowMenu(const Point& rPos)
{
    CreateContextMenu();
    weld::TreeView* pTreeView = m_xTreeBox->get_visible() ? m_xTreeBox.get() : m_xFmtLb.get();
    mxMenu->connect_activate(LINK(this, StyleList, OnPopupEnd));
    mxMenu->popup_at_rect(pTreeView, tools::Rectangle(rPos, Size(1, 1)));
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
                // This is the only place SfxCommonTemplateDialog_Impl::bUpdate is set false.
                // SfxCommonTemplateDialog_Impl::SetFamilyState and below in SfxHintId::DocChanged
                // are the only places it is set true.
                m_pParentDialog->SetNotifyupdate(false);
                Update();
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
        && (nId == SfxHintId::SfxStyleSheetPool || dynamic_cast<const SfxStyleSheetHint*>(&rHint)
            || nId == SfxHintId::StyleSheetModifiedExtended)) // ie. SfxStyleSheetModifiedHint
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
        m_bHierarchical ? FillHierarchicalTreeView() : FillFlatTreeView();
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

    mxMenu.reset();
    mxMenuBuilder.reset();
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
    bool bSelected = comphelper::LibreOfficeKit::isActive() ? false : std::get<2>(aPayload);
    const OUString& rId = std::get<3>(aPayload);

    auto popIt = rRenderContext.ScopedPush(vcl::PushFlags::TEXTCOLOR);
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    if (bSelected)
        rRenderContext.SetTextColor(rStyleSettings.GetHighlightTextColor());
    else
        rRenderContext.SetTextColor(rStyleSettings.GetWindowTextColor());

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
                auto popIt2 = rRenderContext.ScopedPush(vcl::PushFlags::ALL);
                // tdf#119919 - show "hidden" styles as disabled to not move children onto root node
                if (pStyleSheet->IsHidden())
                    rRenderContext.SetTextColor(rStyleSettings.GetDisableColor());

                sal_Int32 nSize = aRect.GetHeight();
                std::unique_ptr<sfx2::StylePreviewRenderer> pStylePreviewRenderer(
                    pStyleManager->CreateStylePreviewRenderer(rRenderContext, pStyleSheet, nSize));
                bSuccess
                    = pStylePreviewRenderer->recalculate() && pStylePreviewRenderer->render(aRect);
            }
        }
    }

    if (!bSuccess)
        rRenderContext.DrawText(aRect, rId, DrawTextFlags::Left | DrawTextFlags::VCenter);
}

// Selection of a template during the Watercan-Status ?
// see SfxCommonTemplateDialog_Impl::SelectUpdate for the above comment to possibly make sense
IMPL_LINK(StyleList, FmtSelectHdl, weld::ItemView&, rItemView, void)
{
    weld::TreeView& rTreeView = dynamic_cast<weld::TreeView&>(rItemView);
    std::unique_ptr<weld::TreeIter> xEntry = rTreeView.get_cursor();
    if (!xEntry)
        return;

    m_pParentDialog->SelectUpdate(rTreeView.get_text(*xEntry));
}

IMPL_LINK_NOARG(StyleList, TreeListApplyHdl, const weld::TreeIter&, bool)
{
    // only if that region is allowed
    if (m_nActFamily != 0xffff && nullptr != m_pFamilyState[m_nActFamily - 1]
        && !GetSelectedEntry().isEmpty())
    {
        m_pParentDialog->Execute_Impl(SID_STYLE_APPLY, GetSelectedEntry(), OUString(),
                                      static_cast<sal_uInt16>(GetFamilyItem()->GetFamily()), *this,
                                      SfxStyleSearchBits::Auto, nullptr, &m_nModifier);
    }
    // After applying a style from the list, set keyboard focus to the app window if possible.
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
// Called from StyleList::Initialize and StyleList::Notify-SfxHintId::UpdateDone
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

    sal_uInt16 i;
    for (i = 0; i < MAX_FAMILIES; ++i)
        if (m_pFamilyState[i])
            break;

    // i == MAX_FAMILIES happens when a Formula, QR Code, OLE, or Chart object is selected
    if (i == MAX_FAMILIES || !pNewPool)
        // nothing is allowed
        return;

    // When the active family has a nullptr family state switch to the first family that has a
    // family state not nullptr. See: SwDocShell::StateStyleSheet
    if (m_nActFamily == 0xffff || !m_pFamilyState[m_nActFamily - 1])
    {
        const size_t nFamilyCount = m_aStyleFamilies.size();
        size_t n;
        for (n = 0; n < nFamilyCount; n++)
            if (m_pFamilyState[SfxTemplate::SfxFamilyIdToNId(m_aStyleFamilies[n].GetFamily()) - 1])
                break;
        // bFillTreeView is set false here because the tree will be filled below
        m_pParentDialog->FamilySelect(
            SfxTemplate::SfxFamilyIdToNId(m_aStyleFamilies[n].GetFamily()),
            /*bFillTreeView*/ false);
    }

    if (bDocChanged)
        m_pParentDialog->LoadFactoryStyleFilter(pDocShell);

    m_bHierarchical ? ShowHierarchicalView() : ShowFlatView();
}

const SfxStyleFamilyItem& StyleList::GetFamilyItemByIndex(size_t i) const
{
    return m_aStyleFamilies.at(i);
}

IMPL_STATIC_LINK(StyleList, CustomGetSizeHdl, weld::TreeView::get_size_args, aPayload, Size)
{
    vcl::RenderContext& rRenderContext = aPayload.first;
    return Size(comphelper::LibreOfficeKit::isActive() ? 200 : 42,
                32 * rRenderContext.GetDPIScaleFactor());
}

IMPL_LINK(StyleList, PopupFlatMenuHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    const Point aPos = PrepareMenu(rCEvt);

    if (m_xFmtLb->count_selected_rows() <= 0)
    {
        m_pParentDialog->EnableEdit(false, this);
        m_pParentDialog->EnableDel(false, this);
    }

    ShowMenu(aPos);

    return true;
}

IMPL_LINK(StyleList, PopupTreeMenuHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    const Point aPos = PrepareMenu(rCEvt);

    ShowMenu(aPos);

    return true;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
