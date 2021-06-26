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

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <vcl/commandevent.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weldutils.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/style.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/UnknownModuleException.hpp>
#include <officecfg/Office/Common.hxx>

#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/templdlg.hxx>
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
#include <vcl/toolbox.hxx>
#include <vcl/menu.hxx>

using namespace css;
using namespace css::beans;
using namespace css::frame;
using namespace css::uno;

//Constructor

StyleList::StyleList(weld::Builder* pBuilder, std::optional<SfxStyleFamilies> mxFamilies,
                     SfxBindings* m_pBindings, SfxCommonTemplateDialog_Impl* mParent,
                     SfxModule* mModule, weld::Container* pC)
    : m_bCanEdit(false)
    , m_bCanHide(true)
    , m_bCanShow(false)
    , m_nAppFilter(SfxStyleSearchBits::Auto)
    , m_nActFilter(0)
    , m_bHierarchical(false)
    , m_xStyleFamilies(mxFamilies)
    , m_bAllowReParentDrop(false)
    , m_pStyleSheetPool(nullptr)
    , m_bDontUpdate(false)
    , m_nActFamily(0xffff)
    , m_bTreeDrag(true)
    , m_bNewByExampleDisabled(false)
    , m_bCanNew(true)
    , m_bUpdateFamily(false)
    , m_pBindings(m_pBindings)
    , m_pParentDialog(mParent)
    , m_Module(mModule)
    , m_nModifier(0)
    , m_pContainer(pC)
{
    m_xFmtLb = pBuilder->weld_tree_view("flatview");
    m_xTreeBox = pBuilder->weld_tree_view("treeview");
    m_xFmtLb->set_help_id(HID_TEMPLATE_FMT);
}

//Destructor

StyleList::~StyleList()
{
    if (m_pStyleSheetPool)
    m_pParentDialog->EndListening(*m_pStyleSheetPool);
    m_pStyleSheetPool = nullptr;
    m_xTreeView1DropTargetHelper.reset();
    m_xTreeView2DropTargetHelper.reset();
    m_xTreeBox.reset();
    m_xFmtLb.reset();
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
    mxMenuBuilder.reset(Application::CreateBuilder(nullptr, "sfx/ui/stylecontextmenu.ui"));
    mxMenu = mxMenuBuilder->weld_menu("menu");
    mxMenu->set_sensitive("edit", m_bCanEdit);
    mxMenu->set_sensitive("delete", m_bCanDel);
    mxMenu->set_sensitive("new", m_bCanNew);
    mxMenu->set_sensitive("hide", m_bCanHide);
    mxMenu->set_sensitive("show", m_bCanShow);

    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
    if (pItem && pItem->GetFamily() == SfxStyleFamily::Table) //tdf#101648, no ui for this yet
    {
        mxMenu->set_sensitive("edit", false);
        mxMenu->set_sensitive("new", false);
    }
    if (pItem && pItem->GetFamily() == SfxStyleFamily::Pseudo)
    {
        const OUString aTemplName(GetSelectedEntry());
        if (aTemplName == "No List")
        {
            mxMenu->set_sensitive("edit", false);
            mxMenu->set_sensitive("new", false);
            mxMenu->set_sensitive("hide", false);
        }
    }
}

//Partial code in templdlg.cxx

void StyleList::ReadResource()
{
    // Read global user resource
    for (auto& i : m_pFamilyState)
        i.reset();

    SfxViewFrame* pViewFrame = m_pBindings->GetDispatcher_Impl()->GetFrame();
    m_pCurObjShell = pViewFrame->GetObjectShell();
    m_pParentDialog->SetObjectShell(m_pCurObjShell);
    m_Module = m_pCurObjShell ? m_pCurObjShell->GetModule() : nullptr;
    if (m_Module)
        m_xStyleFamilies = m_Module->CreateStyleFamilies();
    if (!m_xStyleFamilies)
        m_xStyleFamilies.emplace();

    m_nActFilter = 0xffff;
    m_pParentDialog->NActFilter(m_nActFilter);
    if (m_pCurObjShell)
    {
        m_nActFilter = static_cast<sal_uInt16>(m_pParentDialog->LoadFactoryStyleFilter(m_pCurObjShell));
        if (0xffff == m_nActFilter)
        {
            m_nActFilter = m_pCurObjShell->GetAutoStyleFilterIndex();
        }
    }
    // Paste in the toolbox
    // reverse order, since always inserted at the head
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
    }

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

void StyleList::Initialize()                     //This is giving a weird error that StyleList::Initialize cannot be redeclared outside its class
{
    // Read global user resource
    m_pBindings->Invalidate(SID_STYLE_FAMILY);
    m_pBindings->Update(SID_STYLE_FAMILY);

    m_xFmtLb->connect_row_activated(
        LINK(this, StyleList, TreeListApplyHdl));
    m_xFmtLb->connect_mouse_press(
        LINK(this, StyleList, MousePressHdl));
    m_xFmtLb->connect_query_tooltip(
        LINK(this, StyleList, QueryTooltipHdl));
    m_xFmtLb->connect_changed(LINK(this, StyleList, FmtSelectHdl));
    m_xFmtLb->connect_popup_menu(
        LINK(this, StyleList, PopupFlatMenuHdl));
    m_xFmtLb->connect_key_press(LINK(this, StyleList, KeyInputHdl));
    m_xFmtLb->set_selection_mode(SelectionMode::Multiple);
    m_xTreeBox->connect_changed(LINK(this, StyleList, FmtSelectHdl));
    m_xTreeBox->connect_row_activated(
        LINK(this, StyleList, TreeListApplyHdl));
    m_xTreeBox->connect_mouse_press(
        LINK(this, StyleList, MousePressHdl));
    m_xTreeBox->connect_query_tooltip(
        LINK(this, StyleList, QueryTooltipHdl));
    m_xTreeBox->connect_popup_menu(
        LINK(this, StyleList, PopupTreeMenuHdl));
    m_xTreeBox->connect_key_press(
        LINK(this, StyleList, KeyInputHdl));
    m_xTreeBox->connect_drag_begin(
        LINK(this, StyleList, DragBeginHdl));
    m_xTreeView1DropTargetHelper.reset(new TreeViewDropTarget(*this, *m_xFmtLb));          // This is giving error that TreeViewDropTarget* is incompatible with a parameter of type TreeViewDropTarget* 
    m_xTreeView2DropTargetHelper.reset(new TreeViewDropTarget(*this, *m_xTreeBox));

    int nTreeHeight = m_xFmtLb->get_height_rows(8);
    m_xFmtLb->set_size_request(-1, nTreeHeight);
    m_xTreeBox->set_size_request(-1, nTreeHeight);

    m_xFmtLb->connect_custom_get_size(
        LINK(this, StyleList, CustomGetSizeHdl));
    m_xFmtLb->connect_custom_render(
        LINK(this, StyleList, CustomRenderHdl));
    m_xTreeBox->connect_custom_get_size(
        LINK(this, StyleList, CustomGetSizeHdl));
    m_xTreeBox->connect_custom_render(
        LINK(this, StyleList, CustomRenderHdl));
    bool bCustomPreview = officecfg::Office::Common::StylesAndFormatting::Preview::get();
    m_xFmtLb->set_column_custom_renderer(0, bCustomPreview);
    m_xTreeBox->set_column_custom_renderer(0, bCustomPreview);

    m_xFmtLb->set_visible(!m_bHierarchical);
    m_xTreeBox->set_visible(m_bHierarchical);

    Update_Impl();
}


void StyleList::UpdateFamily_Impl()
{
    m_bUpdateFamily = false;
    m_pParentDialog->BUpdateFamily(m_bUpdateFamily);

    SfxDispatcher* pDispat = m_pBindings->GetDispatcher_Impl();
    SfxViewFrame* pViewFrame = pDispat->GetFrame();
    SfxObjectShell* pDocShell = pViewFrame->GetObjectShell();

    SfxStyleSheetBasePool* pOldStyleSheetPool = m_pStyleSheetPool;
    m_pStyleSheetPool = pDocShell ? pDocShell->GetStyleSheetPool() : nullptr;
    if (pOldStyleSheetPool != m_pStyleSheetPool)
    {
        if (pOldStyleSheetPool)
            m_pParentDialog->EndListening(*pOldStyleSheetPool);
        if (m_pStyleSheetPool)
            m_pParentDialog->StartListening(*m_pStyleSheetPool);
    }
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

sal_Int8 StyleList::ExecuteDrop(const ExecuteDropEvent& rEvt)
{
    // handle drop of content into the treeview to create a new style
    SfxObjectShell* pDocShell = GetObjectShell();
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
                        LINK(m_pParentDialog, SfxCommonTemplateDialog_Impl, OnAsyncExecuteDrop));

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
    m_pParentDialog->SelectStyle(aTargetStyle, false);
    return DND_ACTION_NONE;
}

// part of action select in dialog
void StyleList::ExecuteAction()
{
    if (m_pStyleSheetPool && m_nActFamily != 0xffff)
    {
        const SfxStyleFamily eFam = GetFamilyItem_Impl()->GetFamily();
        const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
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
            m_pParentDialog->Execute_Impl(SID_STYLE_NEW_BY_EXAMPLE, aTemplName, "",
                         static_cast<sal_uInt16>(GetFamilyItem_Impl()->GetFamily()),
                         nFilter);
            m_pParentDialog->UpdateFamily_Impl();
        }
    }
}

void StyleList::DropHdl(const OUString& rStyle, const OUString& rParent)
{
    m_bDontUpdate = true;
    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
    const SfxStyleFamily eFam = pItem->GetFamily();
    m_pStyleSheetPool->SetParent(eFam, rStyle, rParent);
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

    StyleTree_Impl(const OUString& rName, const OUString& rParent)
        : aName(rName)
        , aParent(rParent)
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
    rArr.erase(
        std::remove_if(rArr.begin(), rArr.end(),
                       [](std::unique_ptr<StyleTree_Impl> const& pEntry) { return !pEntry; }),
        rArr.end());

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

static void FillBox_Impl(weld::TreeView& rBox, StyleTree_Impl* pEntry,
                         const std::vector<OUString>& rEntries, SfxStyleFamily eStyleFamily,
                         const weld::TreeIter* pParent)
{
    std::unique_ptr<weld::TreeIter> xResult = rBox.make_iterator();
    const OUString& rName = pEntry->getName();
    rBox.insert(pParent, -1, &rName, &rName, nullptr, nullptr, false, xResult.get());

    for (size_t i = 0; i < pEntry->getChildren().size(); ++i)
        FillBox_Impl(rBox, pEntry->getChildren()[i].get(), rEntries, eStyleFamily, xResult.get());
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
const SfxStyleFamilyItem* StyleList::GetFamilyItem_Impl() const
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
    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
    m_pStyleSheetPool->Find(aTemplName, pItem->GetFamily());
}

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
bool StyleList::IsSafeForWaterCan() const
{
    if (m_xTreeBox->get_visible())
        return m_xTreeBox->get_selected_index() != -1;
    else
        return m_xFmtLb->count_selected_rows() == 1;
}

void StyleList::SelectStyle(const OUString& rStr, bool bIsCallback)
{
    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
    if (!pItem)
        return;
    const SfxStyleFamily eFam = pItem->GetFamily();
    SfxStyleSheetBase* pStyle = m_pStyleSheetPool->Find(rStr, eFam);
    if (pStyle)
    {
        bool bReadWrite = !(pStyle->GetMask() & SfxStyleSearchBits::ReadOnly);
        m_pParentDialog->EnableEdit(bReadWrite);
        EnableHide(bReadWrite && !pStyle->IsHidden() && !pStyle->IsUsed());
        EnableShow(bReadWrite && pStyle->IsHidden());
    }
    else
    {
        m_pParentDialog->EnableEdit(false);
        EnableHide(false);
        EnableShow(false);
    }


    if (!bIsCallback)
    {
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
                m_pParentDialog->EnableEdit(false);
                EnableHide(false);
                EnableShow(false);
            }
        }
    }
}

void StyleList::EnableTreeDrag(bool bEnable)
{
    if (m_pStyleSheetPool)
    {
        const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
        SfxStyleSheetBase* pStyle
            = pItem ? m_pStyleSheetPool->First(pItem->GetFamily()) : nullptr;
        m_bAllowReParentDrop = pStyle && pStyle->HasParentSupport() && bEnable;
    }
    m_bTreeDrag = bEnable;
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

//Fill the treeview

void StyleList::FillTreeBox(SfxStyleFamily eFam)
{
    assert(m_xTreeBox && "FillTreeBox() without treebox");
    if (!m_pStyleSheetPool || m_nActFamily == 0xffff)
        return;

    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
    if (!pItem)
        return;
    const SfxStyleFamily eFam = pItem->GetFamily();

    StyleTreeArr_Impl aArr;
    SfxStyleSheetBase* pStyle = m_pStyleSheetPool->First(eFam, SfxStyleSearchBits::AllVisible);

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

    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        FillBox_Impl(*m_xTreeBox, aArr[i].get(), aEntries, eFam, nullptr);
        aArr[i].reset();
    }

    m_pParentDialog->EnableItem("watercan", false);

    SfxTemplateItem* pState = m_pFamilyState[m_nActFamily - 1].get();

    m_xTreeBox->thaw();

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
    m_pParentDialog->SelectStyle(aStyle, false);
    EnableDelete();
}

static OUString lcl_GetStyleFamilyName(SfxStyleFamily nFamily)
{
    if (nFamily == SfxStyleFamily::Char)
        return "CharacterStyles";
    if (nFamily == SfxStyleFamily::Para)
        return "ParagraphStyles";
    if (nFamily == SfxStyleFamily::Page)
        return "PageStyles";
    if (nFamily == SfxStyleFamily::Table)
        return "TableStyles";
    if (nFamily == SfxStyleFamily::Pseudo)
        return "NumberingStyles";
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
    uno::Reference<style::XStyleFamiliesSupplier> xModel(GetObjectShell()->GetModel(),
                                                         uno::UNO_QUERY);
    OUString aUIName;
    try
    {
        uno::Reference<container::XNameAccess> xStyles;
        uno::Reference<container::XNameAccess> xCont = xModel->getStyleFamilies();
        xCont->getByName(aFamilyName) >>= xStyles;
        uno::Reference<beans::XPropertySet> xInfo;
        xStyles->getByName(sDefaultStyle) >>= xInfo;
        xInfo->getPropertyValue("DisplayName") >>= aUIName;
    }
    catch (const uno::Exception&)
    {
    }
    return aUIName;
}

SfxStyleFamily StyleList::GetActualFamily() const
{
    const SfxStyleFamilyItem* pFamilyItem = GetFamilyItem_Impl();
    if (!pFamilyItem || m_nActFamily == 0xffff)
        return SfxStyleFamily::Para;
    else
        return pFamilyItem->GetFamily();
}

bool StyleList::HasSelectedStyle() const
{
    return m_xTreeBox->get_visible() ? m_xTreeBox->get_selected_index() != -1
                                    : m_xFmtLb->count_selected_rows() != 0;
}

void StyleList::UpdateStyles_Impl(SfxStyleFamily eFam, SfxStyleSearchBits nFilter)
{

    SfxStyleSheetBase* pStyle = m_pStyleSheetPool->First(eFam, nFilter);

    std::unique_ptr<weld::TreeIter> xEntry = m_xFmtLb->make_iterator();
    bool bEntry = m_xFmtLb->get_iter_first(*xEntry);
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

    size_t nCount = aStrings.size();
    size_t nPos = 0;
    while (nPos < nCount && bEntry && aStrings[nPos] == m_xFmtLb->get_text(*xEntry))
    {
        ++nPos;
        bEntry = m_xFmtLb->iter_next(*xEntry);
    }

    if (nPos < nCount || bEntry)
    {
        // Fills the display box
        m_xFmtLb->freeze();
        m_xFmtLb->clear();

        for (nPos = 0; nPos < nCount; ++nPos)
            m_xFmtLb->append(aStrings[nPos], aStrings[nPos]);

        m_xFmtLb->thaw();
    }
    // Selects the current style if any
    SfxTemplateItem* pState = m_pFamilyState[m_nActFamily - 1].get();
    OUString aStyle;
    if (pState)
        aStyle = pState->GetStyleName();
    m_pParentDialog->SelectStyle(aStyle, false);
    EnableDelete();
}

//Part of Enable Hierarchical
void StyleList::SetHierarchical()
{
    const OUString aSelectEntry(GetSelectedEntry());
    m_xFmtLb->hide();
    FillTreeBox(GetActualFamily());
    m_pParentDialog->SelectStyle(aSelectEntry, false);
    m_xTreeBox->show();
}

//Part of Enable Hierarchical
void StyleList::SetFilter()
{
    m_xTreeBox->hide();
    m_xFmtLb->show();
}

// Handler for the New-Buttons
void StyleList::NewHdl()
{
    if (m_nActFamily == 0xffff || !(m_xTreeBox->get_visible() || m_xFmtLb->count_selected_rows() <= 1))
        return;

    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
    const SfxStyleFamily eFam = pItem->GetFamily();
    SfxStyleSearchBits nMask(SfxStyleSearchBits::Auto);
    if (m_nActFilter != 0xffff)
        nMask = pItem->GetFilterList()[m_nActFilter].nFlags;
    if (nMask == SfxStyleSearchBits::Auto) // automatic
        nMask = m_nAppFilter;

    m_pParentDialog->Execute_Impl(SID_STYLE_NEW, "", GetSelectedEntry(),
                                static_cast<sal_uInt16>(eFam), nMask);
}

// Handler for the edit-Buttons
void StyleList::EditHdl()
{
    if (m_pParentDialog->IsInitialized() && HasSelectedStyle())
    {
        sal_uInt16 nFilter = m_nActFilter;
        OUString aTemplName(GetSelectedEntry());
        GetSelectedStyle(); // -Wall required??
        m_pParentDialog->Execute_Impl(SID_STYLE_EDIT, aTemplName, OUString(),
                                    static_cast<sal_uInt16>(GetFamilyItem_Impl()->GetFamily()),
                                    SfxStyleSearchBits::Auto, &nFilter);
    }
}

// Handler for the Delete-Buttons
void StyleList::DeleteHdl()
{
    if (!m_pParentDialog->IsInitialized() || !HasSelectedStyle())
        return;

    bool bUsedStyle = false; // one of the selected styles are used in the document?

    std::vector<std::unique_ptr<weld::TreeIter>> aList;
    weld::TreeView* pTreeView = m_xTreeBox->get_visible() ? m_xTreeBox.get() : m_xFmtLb.get();
    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();

    OUStringBuffer aMsg;
    aMsg.append(SfxResId(STR_DELETE_STYLE_USED) + SfxResId(STR_DELETE_STYLE));

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
                                    static_cast<sal_uInt16>(GetFamilyItem_Impl()->GetFamily()));

        if (m_xTreeBox->get_visible())
        {
            weld::RemoveParentKeepChildren(*m_xTreeBox, *elem);
            m_bDontUpdate = false;
        }
    }
    m_bDontUpdate = false; //if everything is deleted set m_bDontUpdate back to false
    m_pParentDialog->UpdateStyles_Impl(StyleFlags::UpdateFamilyList); //and force-update the list
}

void StyleList::HideHdl()
{
    if (!m_pParentDialog->IsInitialized() || !HasSelectedStyle())
        return;

    weld::TreeView* pTreeView = m_xTreeBox->get_visible() ? m_xTreeBox.get() : m_xFmtLb.get();
    pTreeView->selected_foreach([this, pTreeView](weld::TreeIter& rEntry) {
        OUString aTemplName = pTreeView->get_text(rEntry);

        m_pParentDialog->Execute_Impl(SID_STYLE_HIDE, aTemplName, OUString(),
                                    static_cast<sal_uInt16>(GetFamilyItem_Impl()->GetFamily()));

        return false;
    });
}

void StyleList::ShowHdl()
{
    if (!m_pParentDialog->IsInitialized() || !HasSelectedStyle())
        return;

    weld::TreeView* pTreeView = m_xTreeBox->get_visible() ? m_xTreeBox.get() : m_xFmtLb.get();
    pTreeView->selected_foreach([this, pTreeView](weld::TreeIter& rEntry) {
        OUString aTemplName = pTreeView->get_text(rEntry);

        m_pParentDialog->Execute_Impl(SID_STYLE_SHOW, aTemplName, OUString(),
                                    static_cast<sal_uInt16>(GetFamilyItem_Impl()->GetFamily()));

        return false;
    });
}

void StyleList::EnableDelete()
{
    bool bEnableDelete(false);
    if (m_pParentDialog->IsInitialized() && HasSelectedStyle())
    {
        OSL_ENSURE(m_pStyleSheetPool, "No StyleSheetPool");
        const OUString aTemplName(GetSelectedEntry());
        const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
        const SfxStyleFamily eFam = pItem->GetFamily();
        SfxStyleSearchBits nFilter = SfxStyleSearchBits::Auto;
        if (pItem->GetFilterList().size() > m_nActFilter)
            nFilter = pItem->GetFilterList()[m_nActFilter].nFlags;
        if (nFilter == SfxStyleSearchBits::Auto) // automatic
            nFilter = m_nAppFilter;
        const SfxStyleSheetBase* pStyle = m_pStyleSheetPool->Find(
            aTemplName, eFam,
            m_xTreeBox->get_visible() ? SfxStyleSearchBits::All : nFilter);

        OSL_ENSURE(pStyle, "Style not found");
        if (pStyle && pStyle->IsUserDefined())
        {
            if (pStyle->HasClearParentSupport() || !pStyle->IsUsed())
            {
                bEnableDelete = true;
            }
            else if (pStyle->GetFamily() == SfxStyleFamily::Page)
            {
                // Hack to allow Calc page styles to be deleted,
                // remove when IsUsed is fixed for Calc page styles.
                SfxViewFrame* pFrame = GetObjectShell()->GetFrame();
                if (pFrame)
                {
                    uno::Reference<frame::XFrame> xFrame = pFrame->GetFrame().GetFrameInterface();
                    if (vcl::CommandInfoProvider::GetModuleIdentifier(xFrame)
                        == "com.sun.star.sheet.SpreadsheetDocument")
                    {
                        bEnableDelete = true;
                    }
                }
            }
        }
    }
    EnableDel(bEnableDelete);
}

void StyleList::ShowMenu(const CommandEvent& rCEvt)
{
    CreateContextMenu();
    weld::TreeView* pTreeView = m_xTreeBox->get_visible() ? m_xTreeBox.get() : m_xFmtLb.get();
     OString sCommand(
        mxMenu->popup_at_rect(pTreeView, tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1, 1))));
     MenuSelect(sCommand);
    
}

void StyleList::MenuSelect(const OString& rIdent)
{
    sLastItemIdent = rIdent;
    if (sLastItemIdent.isEmpty())
        return;
    Application::PostUserEvent(
        LINK(m_pParentDialog, StyleList, MenuSelectAsyncHdl));
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
    weld::TreeView* pTreeView = m_xTreeBox->get_visible() ? m_xTreeBox.get()
                                                                      : m_xFmtLb.get();
    const OUString aTemplName(pTreeView->get_text(rEntry));
    OUString sQuickHelpText(aTemplName);

    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
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

IMPL_LINK(StyleList, CustomRenderHdl, weld::TreeView::render_args, aPayload,
          void)
{
    vcl::RenderContext& rRenderContext = std::get<0>(aPayload);
    const ::tools::Rectangle& rRect = std::get<1>(aPayload);
    ::tools::Rectangle aRect(
        rRect.TopLeft(),
        Size(rRenderContext.GetOutputSize().Width() - rRect.Left(), rRect.GetHeight()));
    bool bSelected = std::get<2>(aPayload);
    const OUString& rId = std::get<3>(aPayload);

    rRenderContext.Push(PushFlags::TEXTCOLOR);
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
        const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
        SfxStyleSheetBase* pStyleSheet = pStyleManager->Search(rId, pItem->GetFamily());

        if (pStyleSheet)
        {
            rRenderContext.Push(PushFlags::ALL);
            sal_Int32 nSize = aRect.GetHeight();
            std::unique_ptr<sfx2::StylePreviewRenderer> pStylePreviewRenderer(
                pStyleManager->CreateStylePreviewRenderer(rRenderContext, pStyleSheet, nSize));
            bSuccess = pStylePreviewRenderer->recalculate() && pStylePreviewRenderer->render(aRect);
            rRenderContext.Pop();
        }
    }

    if (!bSuccess)
        rRenderContext.DrawText(aRect, rId, DrawTextFlags::Left | DrawTextFlags::VCenter);

    rRenderContext.Pop();
}

IMPL_LINK(StyleList, PopupFlatMenuHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    PrepareMenu(rCEvt.GetMousePosPixel());

    if (m_xFmtLb->count_selected_rows() <= 0)
    {
        m_pParentDialog->EnableEdit(false);
        EnableDel(false);
    }

    ShowMenu(rCEvt);

    return true;
}

// Selection of a template during the Watercan-Status
IMPL_LINK(StyleList, FmtSelectHdl, weld::TreeView&, rListBox, void)
{
    std::unique_ptr<weld::TreeIter> xHdlEntry = rListBox.make_iterator();
    if (!rListBox.get_cursor(xHdlEntry.get()))
        return;

    if (rListBox.is_selected(*xHdlEntry))
        m_pParentDialog->UpdateStyleDependents();

    m_pParentDialog->SelectStyle(rListBox.get_text(*xHdlEntry), true);
}

IMPL_LINK_NOARG(StyleList, TreeListApplyHdl, weld::TreeView&, bool)
{
    // only if that region is allowed
    if (m_pParentDialog->IsInitialized() && nullptr != m_pFamilyState[m_nActFamily - 1]
        && !GetSelectedEntry().isEmpty())
    {
        m_pParentDialog->Execute_Impl(SID_STYLE_APPLY, GetSelectedEntry(), OUString(),
                     static_cast<sal_uInt16>(GetFamilyItem_Impl()->GetFamily()),
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
void StyleList::Update_Impl()
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
            m_pParentDialog->ClearResource();
            m_pParentDialog->ReadResource();
        }
        if (m_pStyleSheetPool)
        {
            m_pParentDialog->EndListening(*m_pStyleSheetPool);
            m_pStyleSheetPool = nullptr;
        }

        if (pNewPool)
        {
            m_pParentDialog->StartListening(*pNewPool);
            m_pStyleSheetPool = pNewPool;
            bDocChanged = true;
        }
    }

    if (m_bUpdateFamily)
        m_pParentDialog->UpdateFamily_Impl();

    sal_uInt16 i;
    for (i = 0; i < MAX_FAMILIES; ++i)
        if (m_pFamilyState[i])
            break;
    if (i == MAX_FAMILIES || !pNewPool)
        // nothing is allowed
        return;

    SfxTemplateItem* pItem = nullptr;
    // current region not within the allowed region or default
    if (m_nActFamily == 0xffff
        || nullptr == (pItem = m_pFamilyState[m_nActFamily - 1].get()))
    {
        m_pParentDialog->CheckItem(OString::number(m_nActFamily), false);
        const size_t nFamilyCount = m_xStyleFamilies->size();
        size_t n;
        for (n = 0; n < nFamilyCount; n++)
            if (m_pFamilyState[StyleNrToInfoOffset(n)])
                break;

        std::unique_ptr<SfxTemplateItem>& pNewItem
            = m_pFamilyState[StyleNrToInfoOffset(n)];
        m_nAppFilter = pNewItem->GetValue();
        m_pParentDialog->NAppFilter(m_nAppFilter);
        m_pParentDialog->FamilySelect(StyleNrToInfoOffset(n) + 1);
        pItem = pNewItem.get();
    }
    else if (bDocChanged)
    {
        // other DocShell -> all new
        m_pParentDialog->CheckItem(OString::number(m_nActFamily));
        m_nActFilter = static_cast<sal_uInt16>(m_pParentDialog->LoadFactoryStyleFilter(pDocShell));
        m_pParentDialog->NActFilter(m_nActFilter);
        if (0xffff == m_nActFilter)
        {
            m_nActFilter = pDocShell->GetAutoStyleFilterIndex();
            m_pParentDialog->NActFilter(m_nActFilter);
        }

        m_nAppFilter = pItem->GetValue();
        m_pParentDialog->NAppFilter(m_nAppFilter);
        if (!m_xTreeBox->get_visible())
        {
            m_pParentDialog->UpdateStyles_Impl(StyleFlags::UpdateFamilyList);
        }
        else
            FillTreeBox(GetActualFamily());
    }
    else
    {
        // other filters for automatic
        m_pParentDialog->CheckItem(OString::number(m_nActFamily));
        const SfxStyleFamilyItem* pStyleItem = GetFamilyItem_Impl();
        if (pStyleItem && SfxStyleSearchBits::Auto == pStyleItem->GetFilterList()[m_nActFilter].nFlags
            && m_nAppFilter != pItem->GetValue())
        {
            m_nAppFilter = pItem->GetValue();
            m_pParentDialog->NAppFilter(m_nAppFilter);
            if (!m_xTreeBox->get_visible())
                m_pParentDialog->UpdateStyles_Impl(StyleFlags::UpdateFamilyList);
            else
                FillTreeBox(GetActualFamily());
        }
        else
        {
            m_nAppFilter = pItem->GetValue();
            m_pParentDialog->NAppFilter(m_nAppFilter);
        }
    }
    const OUString aStyle(pItem->GetStyleName());
    m_pParentDialog->SelectStyle(aStyle, false);
    EnableDelete();
    EnableNew(m_bCanNew);
}

void StyleList::UpdateFamImpl()
{
    m_bCanNew = bCanNewGetter_updateFamImpl();
    m_bTreeDrag = bTreeDragSetter_updateFam_Impl();
    if (m_pStyleSheetPool)
    {
        if (!m_xTreeBox->get_visible())
            m_pParentDialog->UpdateStyles_Impl(StyleFlags::UpdateFamily | StyleFlags::UpdateFamilyList);
        else
        {
            m_pParentDialog->UpdateStyles_Impl(StyleFlags::UpdateFamily);
            FillTreeBox(GetActualFamily());
        }
    }

}


void StyleList::UpdateFamily_Impl()
{
    m_bUpdateFamily = false;
    m_pParentDialog->BUpdateFamily(m_bUpdateFamily);

    SfxDispatcher* pDispat = m_pBindings->GetDispatcher_Impl();
    SfxViewFrame* pViewFrame = pDispat->GetFrame();
    SfxObjectShell* pDocShell = pViewFrame->GetObjectShell();

    SfxStyleSheetBasePool* pOldStyleSheetPool = m_pStyleSheetPool;
    m_pStyleSheetPool = pDocShell ? pDocShell->GetStyleSheetPool() : nullptr;
    if (pOldStyleSheetPool != m_pStyleSheetPool)
    {
        if (pOldStyleSheetPool)
            m_pParentDialog->EndListening(*pOldStyleSheetPool);
        if (m_pStyleSheetPool)
            m_pParentDialog->StartListening(*m_pStyleSheetPool);
    }
}

IMPL_STATIC_LINK(StyleList, CustomGetSizeHdl, weld::TreeView::get_size_args,
                 aPayload, Size)
{
    vcl::RenderContext& rRenderContext = aPayload.first;
    return Size(42, 32 * rRenderContext.GetDPIScaleFactor());
}

IMPL_LINK(StyleList, PopupFlatMenuHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    PrepareMenu(rCEvt.GetMousePosPixel());

    if (getSelectedRowsM_xFmtlb() <= 0)
    {
        m_pParentDialog->EnableEdit(false);
        EnableDel(false);
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
