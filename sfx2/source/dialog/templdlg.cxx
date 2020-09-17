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
#include <vcl/help.hxx>
#include <vcl/menu.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weldutils.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/style.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <unotools/intlwrapper.hxx>
#include <unotools/collatorwrapper.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/UnknownModuleException.hpp>
#include <officecfg/Office/Common.hxx>

#include <sal/log.hxx>
#include <osl/diagnose.h>
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
#include <bitmaps.hlst>
#include <sfx2/viewfrm.hxx>

#include <comphelper/string.hxx>

#include <sfx2/StyleManager.hxx>
#include <sfx2/StylePreviewRenderer.hxx>

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

/** Drop is enabled as long as it is allowed to create a new style by example, i.e. to
    create a style out of the current selection.
*/
sal_Int8 SfxCommonTemplateDialog_Impl::AcceptDrop(const AcceptDropEvent& rEvt, const DropTargetHelper& rHelper)
{
    if (rHelper.IsDropFormatSupported(SotClipboardFormatId::OBJECTDESCRIPTOR))
    {
        // special case: page styles are allowed to create new styles by example
        // but not allowed to be created by drag and drop
        if (GetActualFamily() == SfxStyleFamily::Page || bNewByExampleDisabled)
            return DND_ACTION_NONE;
        else
            return DND_ACTION_COPY;
    }

    // to enable the autoscroll when we're close to the edges
    weld::TreeView* pTreeView = mxTreeBox->get_visible() ? mxTreeBox.get() : mxFmtLb.get();
    pTreeView->get_dest_row_at_pos(rEvt.maPosPixel, nullptr, true);
    return DND_ACTION_MOVE;
}

sal_Int8 SfxCommonTemplateDialog_Impl::ExecuteDrop(const ExecuteDropEvent& rEvt)
{
    // handle drop of content into the treeview to create a new style
    SfxObjectShell* pDocShell = GetObjectShell();
    if (pDocShell)
    {
        TransferableDataHelper aHelper(rEvt.maDropEvent.Transferable);
        sal_uInt32 nFormatCount = aHelper.GetFormatCount();

        sal_Int8 nRet = DND_ACTION_NONE;

        bool bFormatFound = false;

        for ( sal_uInt32 i = 0; i < nFormatCount; ++i )
        {
            SotClipboardFormatId nId = aHelper.GetFormat(i);
            TransferableObjectDescriptor aDesc;

            if ( aHelper.GetTransferableObjectDescriptor( nId, aDesc ) )
            {
                if ( aDesc.maClassName == pDocShell->GetFactory().GetClassId() )
                {
                    Application::PostUserEvent(LINK(this, SfxCommonTemplateDialog_Impl, OnAsyncExecuteDrop));

                    bFormatFound = true;
                    nRet =  rEvt.mnAction;
                    break;
                }
            }
        }

        if (bFormatFound)
            return nRet;
    }

    if (!mxTreeBox->get_visible())
        return DND_ACTION_NONE;

    if (!bAllowReParentDrop)
        return DND_ACTION_NONE;

    // otherwise if we're dragging with the treeview to set a new parent of the dragged style
    weld::TreeView* pSource = mxTreeBox->get_drag_source();
    // only dragging within the same widget allowed
    if (!pSource || pSource != mxTreeBox.get())
        return DND_ACTION_NONE;

    std::unique_ptr<weld::TreeIter> xSource(mxTreeBox->make_iterator());
    if (!mxTreeBox->get_selected(xSource.get()))
        return DND_ACTION_NONE;

    std::unique_ptr<weld::TreeIter> xTarget(mxTreeBox->make_iterator());
    if (!mxTreeBox->get_dest_row_at_pos(rEvt.maPosPixel, xTarget.get(), true))
    {
        // if nothing under the mouse, use the last row
        int nChildren = mxTreeBox->n_children();
        if (!nChildren)
            return DND_ACTION_NONE;
        if (!mxTreeBox->get_iter_first(*xTarget) || !mxTreeBox->iter_nth_sibling(*xTarget, nChildren - 1))
            return DND_ACTION_NONE;
        while (mxTreeBox->get_row_expanded(*xTarget))
        {
            nChildren = mxTreeBox->iter_n_children(*xTarget);
            if (!mxTreeBox->iter_children(*xTarget) || !mxTreeBox->iter_nth_sibling(*xTarget, nChildren - 1))
                return DND_ACTION_NONE;
        }
    }
    OUString aTargetStyle = mxTreeBox->get_text(*xTarget);
    DropHdl(mxTreeBox->get_text(*xSource), aTargetStyle);
    mxTreeBox->unset_drag_dest_row();
    FillTreeBox();
    SelectStyle(aTargetStyle, false);
    return DND_ACTION_NONE;
}

IMPL_LINK(SfxCommonTemplateDialog_Impl, DragBeginHdl, bool&, rUnsetDragIcon, bool)
{
    rUnsetDragIcon = false;
    // Allow normal processing. only if bAllowReParentDrop is true
    return !bAllowReParentDrop;
}

IMPL_LINK_NOARG(SfxCommonTemplateDialog_Impl, OnAsyncExecuteDrop, void*, void)
{
    ActionSelect("new");
}

IMPL_LINK(SfxCommonTemplateDialog_Impl, KeyInputHdl, const KeyEvent&, rKeyEvent, bool)
{
    bool bRet = false;
    const vcl::KeyCode& rKeyCode = rKeyEvent.GetKeyCode();
    if (bCanDel && !rKeyCode.GetModifier() && rKeyCode.GetCode() == KEY_DELETE)
    {
        DeleteHdl();
        bRet =  true;
    }
    return bRet;
}

IMPL_LINK(SfxCommonTemplateDialog_Impl, QueryTooltipHdl, const weld::TreeIter&, rEntry, OUString)
{
    weld::TreeView* pTreeView = mxTreeBox->get_visible() ? mxTreeBox.get() : mxFmtLb.get();
    const OUString aTemplName(pTreeView->get_text(rEntry));
    OUString sQuickHelpText(aTemplName);

    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
    SfxStyleSheetBase* pStyle = pStyleSheetPool->Find(aTemplName, pItem->GetFamily());

    if (pStyle && pStyle->IsUsed())  // pStyle is in use in the document?
    {
        OUString sUsedBy;
        if (pStyle->GetFamily() == SfxStyleFamily::Pseudo)
            sUsedBy = pStyle->GetUsedBy();

        if (!sUsedBy.isEmpty())
        {
            const sal_Int32 nMaxLen = 80;
            if (sUsedBy.getLength() > nMaxLen)
            {
                sUsedBy = sUsedBy.copy(0, nMaxLen) + "...";
            }

            OUString aMessage = SfxResId(STR_STYLEUSEDBY);
            aMessage = aMessage.replaceFirst("%STYLELIST", sUsedBy);
            sQuickHelpText = aTemplName + " " + aMessage;
        }
    }

    return sQuickHelpText;
}

IMPL_STATIC_LINK(SfxCommonTemplateDialog_Impl, CustomGetSizeHdl, weld::TreeView::get_size_args, aPayload, Size)
{
    vcl::RenderContext& rRenderContext = aPayload.first;
    return Size(42, 32 * rRenderContext.GetDPIScaleFactor());
}

IMPL_LINK(SfxCommonTemplateDialog_Impl, CustomRenderHdl, weld::TreeView::render_args, aPayload, void)
{
    vcl::RenderContext& rRenderContext = std::get<0>(aPayload);
    const ::tools::Rectangle& rRect = std::get<1>(aPayload);
    ::tools::Rectangle aRect(rRect.TopLeft(), Size(rRenderContext.GetOutputSize().Width() - rRect.Left(), rRect.GetHeight()));
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
    sfx2::StyleManager* pStyleManager = pShell ? pShell->GetStyleManager(): nullptr;

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

IMPL_LINK(SfxCommonTemplateDialog_Impl, PopupFlatMenuHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    PrepareMenu(rCEvt.GetMousePosPixel());

    if (mxFmtLb->count_selected_rows() <= 0)
    {
        EnableEdit(false);
        EnableDel(false);
    }

    ShowMenu(rCEvt);

    return true;
}

void SfxCommonTemplateDialog_Impl::PrepareMenu(const Point& rPos)
{
    weld::TreeView* pTreeView = mxTreeBox->get_visible() ? mxTreeBox.get() : mxFmtLb.get();
    std::unique_ptr<weld::TreeIter> xIter(pTreeView->make_iterator());
    if (pTreeView->get_dest_row_at_pos(rPos, xIter.get(), false) && !pTreeView->is_selected(*xIter))
    {
        pTreeView->unselect_all();
        pTreeView->set_cursor(*xIter);
        pTreeView->select(*xIter);
        FmtSelectHdl(*pTreeView);
    }
}

void SfxCommonTemplateDialog_Impl::ShowMenu(const CommandEvent& rCEvt)
{
    CreateContextMenu();

    weld::TreeView* pTreeView = mxTreeBox->get_visible() ? mxTreeBox.get() : mxFmtLb.get();
    OString sCommand(mxMenu->popup_at_rect(pTreeView, tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1,1))));
    MenuSelect(sCommand);
}

IMPL_LINK(SfxCommonTemplateDialog_Impl, PopupTreeMenuHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    PrepareMenu(rCEvt.GetMousePosPixel());

    ShowMenu(rCEvt);

    return true;
}

SfxTemplatePanelControl::SfxTemplatePanelControl(SfxBindings* pBindings, vcl::Window* pParentWindow)
    : PanelLayout(pParentWindow, "TemplatePanel", "sfx/ui/templatepanel.ui", nullptr)
    , pImpl(new SfxTemplateDialog_Impl(pBindings, this))
{
    OSL_ASSERT(pBindings!=nullptr);
}

SfxTemplatePanelControl::~SfxTemplatePanelControl()
{
    disposeOnce();
}

void SfxTemplatePanelControl::dispose()
{
    pImpl.reset();
    PanelLayout::dispose();
}

static void MakeExpanded_Impl(weld::TreeView& rBox, std::vector<OUString>& rEntries)
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

/** Internal structure for the establishment of the hierarchical view */
namespace {

class StyleTree_Impl;

}

typedef std::vector<std::unique_ptr<StyleTree_Impl>> StyleTreeArr_Impl;

namespace {

class StyleTree_Impl
{
private:
    OUString aName;
    OUString aParent;
    StyleTreeArr_Impl pChildren;

public:
    bool HasParent() const { return !aParent.isEmpty(); }

    StyleTree_Impl(const OUString &rName, const OUString &rParent):
        aName(rName), aParent(rParent), pChildren(0) {}

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
            auto iPos = std::lower_bound(pCmp->getChildren().begin(), pCmp->getChildren().end(), pEntry,
                [&aSorter](std::unique_ptr<StyleTree_Impl> const & pEntry1, std::unique_ptr<StyleTree_Impl> const & pEntry2) { return aSorter.compare(pEntry1->getName(), pEntry2->getName()) < 0; });
            pCmp->getChildren().insert(iPos, std::move(pEntry));
        }
    }

    // Only keep tree roots in rArr, child elements can be accessed through the hierarchy
    rArr.erase(std::remove_if(rArr.begin(), rArr.end(), [](std::unique_ptr<StyleTree_Impl> const & pEntry) { return !pEntry; }), rArr.end());

    // tdf#91106 sort top level styles
    std::sort(rArr.begin(), rArr.end());
    std::sort(rArr.begin(), rArr.end(),
        [&aSorter, &aUIName](std::unique_ptr<StyleTree_Impl> const & pEntry1, std::unique_ptr<StyleTree_Impl> const & pEntry2) {
            if (pEntry2->getName() == aUIName)
                return false;
            if (pEntry1->getName() == aUIName)
                return true; // default always first
            return aSorter.compare(pEntry1->getName(), pEntry2->getName()) < 0;
        });
}

static bool IsExpanded_Impl( const std::vector<OUString>& rEntries,
                             const OUString &rStr)
{
    for (const auto & rEntry : rEntries)
    {
        if (rEntry == rStr)
            return true;
    }
    return false;
}

static void FillBox_Impl(weld::TreeView& rBox,
                         StyleTree_Impl* pEntry,
                         const std::vector<OUString>& rEntries,
                         SfxStyleFamily eStyleFamily,
                         weld::TreeIter* pParent)
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
    static SfxStyleFamily NIdToSfxFamilyId(sal_uInt16 nId)
    {
        switch (nId)
        {
            case 1: return SfxStyleFamily::Char;
            case 2: return SfxStyleFamily::Para;
            case 3: return SfxStyleFamily::Frame;
            case 4: return SfxStyleFamily::Page;
            case 5: return SfxStyleFamily::Pseudo;
            case 6: return SfxStyleFamily::Table;
            default: return SfxStyleFamily::All;
        }
    }
}

// Constructor

SfxCommonTemplateDialog_Impl::SfxCommonTemplateDialog_Impl(SfxBindings* pB, weld::Container* pC, weld::Builder* pBuilder)
    : pBindings(pB)
    , mpContainer(pC)
    , pModule(nullptr)
    , pStyleSheetPool(nullptr)
    , pCurObjShell(nullptr)
    , xModuleManager(frame::ModuleManager::create(::comphelper::getProcessComponentContext()))
    , m_pDeletionWatcher(nullptr)
    , mxFmtLb(pBuilder->weld_tree_view("flatview"))
    , mxTreeBox(pBuilder->weld_tree_view("treeview"))
    , mxPreviewCheckbox(pBuilder->weld_check_button("showpreview"))
    , mxFilterLb(pBuilder->weld_combo_box("filter"))

    , nActFamily(0xffff)
    , nActFilter(0)
    , nAppFilter(SfxStyleSearchBits::Auto)

    , m_nModifier(0)
    , bDontUpdate(false)
    , bIsWater(false)
    , bUpdate(false)
    , bUpdateFamily(false)
    , bCanEdit(false)
    , bCanDel(false)
    , bCanNew(true)
    , bCanHide(true)
    , bCanShow(false)
    , bWaterDisabled(false)
    , bNewByExampleDisabled(false)
    , bUpdateByExampleDisabled(false)
    , bTreeDrag(true)
    , bAllowReParentDrop(false)
    , bHierarchical(false)
    , m_bWantHierarchical(false)
    , bBindingUpdate(true)
{
    mxFmtLb->set_help_id(HID_TEMPLATE_FMT);
    mxFilterLb->set_help_id(HID_TEMPLATE_FILTER);
    mxPreviewCheckbox->set_active(officecfg::Office::Common::StylesAndFormatting::Preview::get());
}

sal_uInt16 SfxCommonTemplateDialog_Impl::StyleNrToInfoOffset(sal_uInt16 nId)
{
    const SfxStyleFamilyItem& rItem = pStyleFamilies->at( nId );
    return SfxTemplate::SfxFamilyIdToNId(rItem.GetFamily())-1;
}

void SfxTemplateDialog_Impl::EnableEdit(bool bEnable)
{
    SfxCommonTemplateDialog_Impl::EnableEdit( bEnable );
    if( !bEnable || !bUpdateByExampleDisabled )
        EnableItem("update", bEnable);
}

void SfxCommonTemplateDialog_Impl::ReadResource()
{
    // Read global user resource
    for (auto & i : pFamilyState)
        i.reset();

    SfxViewFrame* pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
    pCurObjShell = pViewFrame->GetObjectShell();
    pModule = pCurObjShell ? pCurObjShell->GetModule() : nullptr;
    if (pModule)
        pStyleFamilies = pModule->CreateStyleFamilies();
    if (!pStyleFamilies)
        pStyleFamilies.reset(new SfxStyleFamilies);

    nActFilter = 0xffff;
    if (pCurObjShell)
    {
        nActFilter = static_cast< sal_uInt16 >( LoadFactoryStyleFilter( pCurObjShell ) );
        if ( 0xffff == nActFilter )
            nActFilter = pCurObjShell->GetAutoStyleFilterIndex();
    }

    // Paste in the toolbox
    // reverse order, since always inserted at the head
    size_t nCount = pStyleFamilies->size();

    pBindings->ENTERREGISTRATIONS();

    size_t i;
    for (i = 0; i < nCount; ++i)
    {
        sal_uInt16 nSlot = 0;
        switch (pStyleFamilies->at(i).GetFamily())
        {
            case SfxStyleFamily::Char:
                nSlot = SID_STYLE_FAMILY1; break;
            case SfxStyleFamily::Para:
                nSlot = SID_STYLE_FAMILY2; break;
            case SfxStyleFamily::Frame:
                nSlot = SID_STYLE_FAMILY3; break;
            case SfxStyleFamily::Page:
                nSlot = SID_STYLE_FAMILY4; break;
            case SfxStyleFamily::Pseudo:
                nSlot = SID_STYLE_FAMILY5; break;
            case SfxStyleFamily::Table:
                nSlot = SID_STYLE_FAMILY6; break;
            default: OSL_FAIL("unknown StyleFamily"); break;
        }
        pBoundItems[i].reset(
            new SfxTemplateControllerItem(nSlot, *this, *pBindings) );
    }
    pBoundItems[i++].reset( new SfxTemplateControllerItem(
        SID_STYLE_WATERCAN, *this, *pBindings) );
    pBoundItems[i++].reset( new SfxTemplateControllerItem(
        SID_STYLE_NEW_BY_EXAMPLE, *this, *pBindings) );
    pBoundItems[i++].reset( new SfxTemplateControllerItem(
        SID_STYLE_UPDATE_BY_EXAMPLE, *this, *pBindings) );
    pBoundItems[i++].reset( new SfxTemplateControllerItem(
        SID_STYLE_NEW, *this, *pBindings) );
    pBoundItems[i++].reset( new SfxTemplateControllerItem(
        SID_STYLE_DRAGHIERARCHIE, *this, *pBindings) );
    pBoundItems[i++].reset( new SfxTemplateControllerItem(
        SID_STYLE_EDIT, *this, *pBindings) );
    pBoundItems[i++].reset( new SfxTemplateControllerItem(
        SID_STYLE_DELETE, *this, *pBindings) );
    pBoundItems[i++].reset( new SfxTemplateControllerItem(
        SID_STYLE_FAMILY, *this, *pBindings) );
    pBindings->LEAVEREGISTRATIONS();

    for(; i < COUNT_BOUND_FUNC; ++i)
        pBoundItems[i] = nullptr;

    StartListening(*pBindings);

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
        const SfxStyleFamilyItem &rItem = pStyleFamilies->at( nCount );
        sal_uInt16 nId = SfxTemplate::SfxFamilyIdToNId( rItem.GetFamily() );
        InsertFamilyItem(nId, rItem);
    }

    for ( i = SID_STYLE_FAMILY1; i <= SID_STYLE_FAMILY4; i++ )
        pBindings->Update(i);
}

void SfxCommonTemplateDialog_Impl::ClearResource()
{
    ClearFamilyList();
    impl_clear();
}

void SfxCommonTemplateDialog_Impl::impl_clear()
{
    pStyleFamilies.reset();
    for (auto & i : pFamilyState)
        i.reset();
    for (auto & i : pBoundItems)
        i.reset();
    pCurObjShell = nullptr;
}

SfxCommonTemplateDialog_Impl::DeletionWatcher *
SfxCommonTemplateDialog_Impl::impl_setDeletionWatcher(
        DeletionWatcher *const pNewWatcher)
{
    DeletionWatcher *const pRet(m_pDeletionWatcher);
    m_pDeletionWatcher = pNewWatcher;
    return pRet;
}

class TreeViewDropTarget final : public DropTargetHelper
{
private:
    SfxCommonTemplateDialog_Impl& m_rParent;

public:
    TreeViewDropTarget(SfxCommonTemplateDialog_Impl& rDialog, weld::TreeView& rTreeView)
        : DropTargetHelper(rTreeView.get_drop_target())
        , m_rParent(rDialog)
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

void SfxCommonTemplateDialog_Impl::Initialize()
{
    // Read global user resource
    ReadResource();
    pBindings->Invalidate( SID_STYLE_FAMILY );
    pBindings->Update( SID_STYLE_FAMILY );

    mxFilterLb->connect_changed(LINK(this, SfxCommonTemplateDialog_Impl, FilterSelectHdl));
    mxFmtLb->connect_row_activated(LINK( this, SfxCommonTemplateDialog_Impl, TreeListApplyHdl));
    mxFmtLb->connect_mouse_press(LINK(this, SfxCommonTemplateDialog_Impl, MousePressHdl));
    mxFmtLb->connect_query_tooltip(LINK(this, SfxCommonTemplateDialog_Impl, QueryTooltipHdl));
    mxFmtLb->connect_changed(LINK(this, SfxCommonTemplateDialog_Impl, FmtSelectHdl));
    mxFmtLb->connect_popup_menu(LINK(this, SfxCommonTemplateDialog_Impl, PopupFlatMenuHdl));
    mxFmtLb->connect_key_press(LINK(this, SfxCommonTemplateDialog_Impl, KeyInputHdl));
    mxFmtLb->set_selection_mode(SelectionMode::Multiple);
    mxTreeBox->connect_changed(LINK(this, SfxCommonTemplateDialog_Impl, FmtSelectHdl));
    mxTreeBox->connect_row_activated(LINK( this, SfxCommonTemplateDialog_Impl, TreeListApplyHdl));
    mxTreeBox->connect_mouse_press(LINK(this, SfxCommonTemplateDialog_Impl, MousePressHdl));
    mxTreeBox->connect_query_tooltip(LINK(this, SfxCommonTemplateDialog_Impl, QueryTooltipHdl));
    mxTreeBox->connect_popup_menu(LINK(this, SfxCommonTemplateDialog_Impl, PopupTreeMenuHdl));
    mxTreeBox->connect_key_press(LINK(this, SfxCommonTemplateDialog_Impl, KeyInputHdl));
    mxTreeBox->connect_drag_begin(LINK(this, SfxCommonTemplateDialog_Impl, DragBeginHdl));
    mxPreviewCheckbox->connect_clicked(LINK(this, SfxCommonTemplateDialog_Impl, PreviewHdl));
    m_xTreeView1DropTargetHelper.reset(new TreeViewDropTarget(*this, *mxFmtLb));
    m_xTreeView2DropTargetHelper.reset(new TreeViewDropTarget(*this, *mxTreeBox));

    int nTreeHeight = mxFmtLb->get_height_rows(8);
    mxFmtLb->set_size_request(-1, nTreeHeight);
    mxTreeBox->set_size_request(-1, nTreeHeight);

    mxFmtLb->connect_custom_get_size(LINK(this, SfxCommonTemplateDialog_Impl, CustomGetSizeHdl));
    mxFmtLb->connect_custom_render(LINK(this, SfxCommonTemplateDialog_Impl, CustomRenderHdl));
    mxTreeBox->connect_custom_get_size(LINK(this, SfxCommonTemplateDialog_Impl, CustomGetSizeHdl));
    mxTreeBox->connect_custom_render(LINK(this, SfxCommonTemplateDialog_Impl, CustomRenderHdl));
    bool bCustomPreview = officecfg::Office::Common::StylesAndFormatting::Preview::get();
    mxFmtLb->set_column_custom_renderer(0, bCustomPreview);
    mxTreeBox->set_column_custom_renderer(0, bCustomPreview);

    mxFmtLb->set_visible(!bHierarchical);
    mxTreeBox->set_visible(bHierarchical);

    Update_Impl();
}

SfxCommonTemplateDialog_Impl::~SfxCommonTemplateDialog_Impl()
{
    if ( bIsWater )
        Execute_Impl(SID_STYLE_WATERCAN, "", "", 0);
    impl_clear();
    if ( pStyleSheetPool )
        EndListening(*pStyleSheetPool);
    pStyleSheetPool = nullptr;
    m_xTreeView1DropTargetHelper.reset();
    m_xTreeView2DropTargetHelper.reset();
    mxTreeBox.reset();
    pIdle.reset();
    if ( m_pDeletionWatcher )
        m_pDeletionWatcher->signal();
    mxFmtLb.reset();
    mxPreviewCheckbox.reset();
    mxFilterLb.reset();
}

// Helper function: Access to the current family item
const SfxStyleFamilyItem *SfxCommonTemplateDialog_Impl::GetFamilyItem_Impl() const
{
    const size_t nCount = pStyleFamilies->size();
    for(size_t i = 0; i < nCount; ++i)
    {
        const SfxStyleFamilyItem &rItem = pStyleFamilies->at( i );
        sal_uInt16 nId = SfxTemplate::SfxFamilyIdToNId(rItem.GetFamily());
        if(nId == nActFamily)
            return &rItem;
    }
    return nullptr;
}

void SfxCommonTemplateDialog_Impl::GetSelectedStyle() const
{
    if (!IsInitialized() || !pStyleSheetPool || !HasSelectedStyle())
        return;
    const OUString aTemplName( GetSelectedEntry() );
    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
    pStyleSheetPool->Find( aTemplName, pItem->GetFamily() );
}

/**
 * Is it safe to show the water-can / fill icon. If we've a
 * hierarchical widget - we have only single select, otherwise
 * we need to check if we have a multi-selection. We either have
 * a mxTreeBox showing or an mxFmtLb (which we hide when not shown)
 */
bool SfxCommonTemplateDialog_Impl::IsSafeForWaterCan() const
{
    if (mxTreeBox->get_visible())
        return mxTreeBox->get_selected_index() != -1;
    else
        return mxFmtLb->count_selected_rows() == 1;
}

void SfxCommonTemplateDialog_Impl::SelectStyle(const OUString &rStr, bool bIsCallback)
{
    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
    if ( !pItem )
        return;
    const SfxStyleFamily eFam = pItem->GetFamily();
    SfxStyleSheetBase* pStyle = pStyleSheetPool->Find( rStr, eFam );
    if( pStyle )
    {
        bool bReadWrite = !(pStyle->GetMask() & SfxStyleSearchBits::ReadOnly);
        EnableEdit( bReadWrite );
        EnableHide( bReadWrite && !pStyle->IsHidden( ) && !pStyle->IsUsed( ) );
        EnableShow( bReadWrite && pStyle->IsHidden( ) );
    }
    else
    {
        EnableEdit(false);
        EnableHide(false);
        EnableShow(false);
    }

    if (!bIsCallback)
    {
        if (mxTreeBox->get_visible())
        {
            if (!rStr.isEmpty())
            {
                std::unique_ptr<weld::TreeIter> xEntry = mxTreeBox->make_iterator();
                bool bEntry = mxTreeBox->get_iter_first(*xEntry);
                while (bEntry)
                {
                    if (mxTreeBox->get_text(*xEntry) == rStr)
                    {
                        mxTreeBox->scroll_to_row(*xEntry);
                        mxTreeBox->select(*xEntry);
                        break;
                    }
                    bEntry = mxTreeBox->iter_next(*xEntry);
                }
            }
            else
                mxTreeBox->unselect_all();
        }
        else
        {
            bool bSelect = !rStr.isEmpty();
            if (bSelect)
            {
                std::unique_ptr<weld::TreeIter> xEntry = mxFmtLb->make_iterator();
                bool bEntry = mxFmtLb->get_iter_first(*xEntry);
                while (bEntry && mxFmtLb->get_text(*xEntry) != rStr)
                    bEntry = mxFmtLb->iter_next(*xEntry);
                if (!bEntry)
                    bSelect = false;
                else
                {
                    if (!mxFmtLb->is_selected(*xEntry))
                    {
                        mxFmtLb->unselect_all();
                        mxFmtLb->scroll_to_row(*xEntry);
                        mxFmtLb->select(*xEntry);
                    }
                }
            }

            if (!bSelect)
            {
                mxFmtLb->unselect_all();
                EnableEdit(false);
                EnableHide(false);
                EnableShow(false);
            }
        }
    }

    bWaterDisabled = !IsSafeForWaterCan();

    if (!bIsCallback)
    {
        // tdf#134598 call UpdateStyleDependents to update watercan
        UpdateStyleDependents();
    }
}

OUString SfxCommonTemplateDialog_Impl::GetSelectedEntry() const
{
    OUString aRet;
    if (mxTreeBox->get_visible())
        aRet = mxTreeBox->get_selected_text();
    else
        aRet = mxFmtLb->get_selected_text();
    return aRet;
}

void SfxCommonTemplateDialog_Impl::EnableTreeDrag(bool bEnable)
{
    if (pStyleSheetPool)
    {
        const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
        SfxStyleSheetBase* pStyle = pItem ? pStyleSheetPool->First(pItem->GetFamily()) : nullptr;
        bAllowReParentDrop = pStyle && pStyle->HasParentSupport() && bEnable;
    }
    bTreeDrag = bEnable;
}

static OUString lcl_GetStyleFamilyName( SfxStyleFamily nFamily )
{
    if(nFamily == SfxStyleFamily::Char)
        return "CharacterStyles" ;
    if(nFamily == SfxStyleFamily::Para)
        return "ParagraphStyles";
    if(nFamily == SfxStyleFamily::Page)
        return "PageStyles";
    if(nFamily == SfxStyleFamily::Table)
        return "TableStyles";
    return OUString();
}

OUString SfxCommonTemplateDialog_Impl::getDefaultStyleName( const SfxStyleFamily eFam )
{
    OUString sDefaultStyle;
    OUString aFamilyName = lcl_GetStyleFamilyName(eFam);
    if( aFamilyName == "TableStyles" )
        sDefaultStyle = "Default Style";
    else
        sDefaultStyle = "Standard";
    uno::Reference< style::XStyleFamiliesSupplier > xModel(GetObjectShell()->GetModel(), uno::UNO_QUERY);
    OUString aUIName;
    try
    {
        uno::Reference< container::XNameAccess > xStyles;
        uno::Reference< container::XNameAccess > xCont = xModel->getStyleFamilies();
        xCont->getByName( aFamilyName ) >>= xStyles;
        uno::Reference< beans::XPropertySet > xInfo;
        xStyles->getByName( sDefaultStyle ) >>= xInfo;
        xInfo->getPropertyValue("DisplayName") >>= aUIName;
    }
    catch (const uno::Exception&)
    {
    }
    return aUIName;
}

void SfxCommonTemplateDialog_Impl::FillTreeBox()
{
    assert(mxTreeBox && "FillTreeBox() without treebox");
    if (!pStyleSheetPool || nActFamily == 0xffff)
        return;

    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
    if (!pItem)
        return;
    const SfxStyleFamily eFam = pItem->GetFamily();
    StyleTreeArr_Impl aArr;
    SfxStyleSheetBase* pStyle = pStyleSheetPool->First(eFam, SfxStyleSearchBits::AllVisible);

    bAllowReParentDrop = pStyle && pStyle->HasParentSupport() && bTreeDrag;

    while (pStyle)
    {
        StyleTree_Impl* pNew = new StyleTree_Impl(pStyle->GetName(), pStyle->GetParent());
        aArr.emplace_back(pNew);
        pStyle = pStyleSheetPool->Next();
    }
    OUString aUIName = getDefaultStyleName(eFam);
    MakeTree_Impl(aArr, aUIName);
    std::vector<OUString> aEntries;
    MakeExpanded_Impl(*mxTreeBox, aEntries);
    mxTreeBox->freeze();
    mxTreeBox->clear();
    const sal_uInt16 nCount = aArr.size();

    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        FillBox_Impl(*mxTreeBox, aArr[i].get(), aEntries, eFam, nullptr);
        aArr[i].reset();
    }

    EnableItem("watercan", false);

    SfxTemplateItem* pState = pFamilyState[nActFamily - 1].get();

    mxTreeBox->thaw();

    std::unique_ptr<weld::TreeIter> xEntry = mxTreeBox->make_iterator();
    bool bEntry = mxTreeBox->get_iter_first(*xEntry);
    if (bEntry && nCount)
        mxTreeBox->expand_row(*xEntry);

    while (bEntry)
    {
        if (IsExpanded_Impl(aEntries, mxTreeBox->get_text(*xEntry)))
            mxTreeBox->expand_row(*xEntry);
        bEntry = mxTreeBox->iter_next(*xEntry);
    }

    OUString aStyle;
    if(pState)  // Select current entry
        aStyle = pState->GetStyleName();
    SelectStyle(aStyle, false);
    EnableDelete();
}

bool SfxCommonTemplateDialog_Impl::HasSelectedStyle() const
{
    return mxTreeBox->get_visible() ? mxTreeBox->get_selected_index() != -1
                                    : mxFmtLb->count_selected_rows() != 0;
}

// internal: Refresh the display
// nFlags: what we should update.
void SfxCommonTemplateDialog_Impl::UpdateStyles_Impl(StyleFlags nFlags)
{
    OSL_ENSURE(nFlags != StyleFlags::NONE, "nothing to do");
    const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
    if (!pItem)
    {
        // Is the case for the template catalog
        const size_t nFamilyCount = pStyleFamilies->size();
        size_t n;
        for( n = 0; n < nFamilyCount; n++ )
            if( pFamilyState[ StyleNrToInfoOffset(n) ] ) break;
        if ( n == nFamilyCount )
            // It happens sometimes, God knows why
            return;
        nAppFilter = pFamilyState[StyleNrToInfoOffset(n)]->GetValue();
        FamilySelect(  StyleNrToInfoOffset(n)+1 );
        pItem = GetFamilyItem_Impl();
    }

    const SfxStyleFamily eFam = pItem->GetFamily();

    SfxStyleSearchBits nFilter (nActFilter < pItem->GetFilterList().size() ? pItem->GetFilterList()[nActFilter].nFlags : SfxStyleSearchBits::Auto);
    if (nFilter == SfxStyleSearchBits::Auto)   // automatic
        nFilter = nAppFilter;

    OSL_ENSURE(pStyleSheetPool, "no StyleSheetPool");
    if(!pStyleSheetPool)
        return;

    pItem = GetFamilyItem_Impl();
    if(nFlags & StyleFlags::UpdateFamily)   // Update view type list (Hierarchical, All, etc.
    {
        CheckItem(OString::number(nActFamily));    // check Button in Toolbox

        mxFilterLb->freeze();
        mxFilterLb->clear();

        //insert hierarchical at the beginning
        mxFilterLb->append(OUString::number(static_cast<int>(SfxStyleSearchBits::All)), SfxResId(STR_STYLE_FILTER_HIERARCHICAL));
        const SfxStyleFilter& rFilter = pItem->GetFilterList();
        for (const SfxFilterTuple& i : rFilter)
            mxFilterLb->append(OUString::number(static_cast<int>(i.nFlags)), i.aName);
        mxFilterLb->thaw();

        if (nActFilter < mxFilterLb->get_count() - 1)
            mxFilterLb->set_active(nActFilter + 1);
        else
        {
            nActFilter = 0;
            mxFilterLb->set_active(1);
            nFilter = (nActFilter < rFilter.size()) ? rFilter[nActFilter].nFlags : SfxStyleSearchBits::Auto;
        }

        // if the tree view again, select family hierarchy
        if (mxTreeBox->get_visible() || m_bWantHierarchical)
        {
            mxFilterLb->set_active_text(SfxResId(STR_STYLE_FILTER_HIERARCHICAL));
            EnableHierarchical(true);
        }
    }
    else
    {
        if (nActFilter < mxFilterLb->get_count() - 1)
            mxFilterLb->set_active(nActFilter + 1);
        else
        {
            nActFilter = 0;
            mxFilterLb->set_active(1);
        }
    }

    if(!(nFlags & StyleFlags::UpdateFamilyList))
        return;

    EnableItem("watercan", false);

    SfxStyleSheetBase *pStyle = pStyleSheetPool->First(eFam, nFilter);

    std::unique_ptr<weld::TreeIter> xEntry = mxFmtLb->make_iterator();
    bool bEntry = mxFmtLb->get_iter_first(*xEntry);
    std::vector<OUString> aStrings;

    comphelper::string::NaturalStringSorter aSorter(
        ::comphelper::getProcessComponentContext(),
        Application::GetSettings().GetLanguageTag().getLocale());

    while( pStyle )
    {
        aStrings.push_back(pStyle->GetName());
        pStyle = pStyleSheetPool->Next();
    }
    OUString aUIName = getDefaultStyleName(eFam);

    // Paradoxically, with a list and non-Latin style names,
    // sorting twice is faster than sorting once.
    // The first sort has a cheap comparator, and gets the list into mostly-sorted order.
    // Then the second sort needs to call its (much more expensive) comparator less often.
    std::sort(aStrings.begin(), aStrings.end());
    std::sort(aStrings.begin(), aStrings.end(),
       [&aSorter, &aUIName](const OUString& rLHS, const OUString& rRHS) {
            if(rRHS == aUIName)
                return false;
            if(rLHS == aUIName)
                return true; // default always first
            return aSorter.compare(rLHS, rRHS) < 0;
       });

    size_t nCount = aStrings.size();
    size_t nPos = 0;
    while (nPos < nCount && bEntry &&
           aStrings[nPos] == mxFmtLb->get_text(*xEntry))
    {
        ++nPos;
        bEntry = mxFmtLb->iter_next(*xEntry);
    }

    if (nPos < nCount || bEntry)
    {
        // Fills the display box
        mxFmtLb->freeze();
        mxFmtLb->clear();

        for (nPos = 0; nPos < nCount; ++nPos)
            mxFmtLb->append(aStrings[nPos], aStrings[nPos]);

        mxFmtLb->thaw();
    }
    // Selects the current style if any
    SfxTemplateItem *pState = pFamilyState[nActFamily-1].get();
    OUString aStyle;
    if(pState)
        aStyle = pState->GetStyleName();
    SelectStyle(aStyle, false);
    EnableDelete();
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

    size_t nCount = pStyleFamilies->size();
    pBindings->EnterRegistrations();
    for(size_t n = 0; n < nCount; n++)
    {
        SfxControllerItem *pCItem=pBoundItems[n].get();
        bool bChecked = pItem && pItem->GetValue();
        if( pCItem->IsBound() == bChecked )
        {
            if( !bChecked )
                pCItem->ReBind();
            else
                pCItem->UnBind();
        }
    }
    pBindings->LeaveRegistrations();
}

// Item with the status of a Family is copied and noted
// (is updated when all states have also been updated.)
// See also: <SfxBindings::AddDoneHdl(const Link &)>
void SfxCommonTemplateDialog_Impl::SetFamilyState( sal_uInt16 nSlotId, const SfxTemplateItem* pItem )
{
    sal_uInt16 nIdx = nSlotId - SID_STYLE_FAMILY_START;
    pFamilyState[nIdx].reset();
    if ( pItem )
        pFamilyState[nIdx].reset( new SfxTemplateItem(*pItem) );
    bUpdate = true;

    // If used templates (how the hell you find this out??)
    bUpdateFamily = true;
}

// Notice from SfxBindings that the update is completed. Pushes out the update
// of the display.
void SfxCommonTemplateDialog_Impl::Update_Impl()
{
    bool bDocChanged=false;
    SfxStyleSheetBasePool* pNewPool = nullptr;
    SfxViewFrame* pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
    SfxObjectShell* pDocShell = pViewFrame->GetObjectShell();
    if( pDocShell )
        pNewPool = pDocShell->GetStyleSheetPool();

    if ( pNewPool != pStyleSheetPool && pDocShell )
    {
        SfxModule* pNewModule = pDocShell->GetModule();
        if( pNewModule && pNewModule != pModule )
        {
            ClearResource();
            ReadResource();
        }
        if ( pStyleSheetPool )
        {
            EndListening(*pStyleSheetPool);
            pStyleSheetPool = nullptr;
        }

        if ( pNewPool )
        {
            StartListening(*pNewPool);
            pStyleSheetPool = pNewPool;
            bDocChanged=true;
        }
    }

    if (bUpdateFamily)
        UpdateFamily_Impl();

    sal_uInt16 i;
    for(i = 0; i < MAX_FAMILIES; ++i)
        if(pFamilyState[i])
            break;
    if(i == MAX_FAMILIES || !pNewPool)
        // nothing is allowed
        return;

    SfxTemplateItem *pItem = nullptr;
    // current region not within the allowed region or default
    if(nActFamily == 0xffff || nullptr == (pItem = pFamilyState[nActFamily-1].get() ) )
    {
         CheckItem(OString::number(nActFamily), false);
         const size_t nFamilyCount = pStyleFamilies->size();
         size_t n;
         for( n = 0; n < nFamilyCount; n++ )
             if( pFamilyState[ StyleNrToInfoOffset(n) ] ) break;

         std::unique_ptr<SfxTemplateItem> & pNewItem = pFamilyState[StyleNrToInfoOffset(n)];
         nAppFilter = pNewItem->GetValue();
         FamilySelect( StyleNrToInfoOffset(n) + 1 );
         pItem = pNewItem.get();
    }
    else if( bDocChanged )
    {
         // other DocShell -> all new
         CheckItem(OString::number(nActFamily));
         nActFilter = static_cast< sal_uInt16 >( LoadFactoryStyleFilter( pDocShell ) );
         if ( 0xffff == nActFilter )
            nActFilter = pDocShell->GetAutoStyleFilterIndex();

         nAppFilter = pItem->GetValue();
         if (!mxTreeBox->get_visible())
         {
             UpdateStyles_Impl(StyleFlags::UpdateFamilyList);
         }
         else
             FillTreeBox();
    }
    else
    {
         // other filters for automatic
         CheckItem(OString::number(nActFamily));
         const SfxStyleFamilyItem *pStyleItem =  GetFamilyItem_Impl();
         if ( pStyleItem && SfxStyleSearchBits::Auto == pStyleItem->GetFilterList()[nActFilter].nFlags
            && nAppFilter != pItem->GetValue())
         {
             nAppFilter = pItem->GetValue();
             if (!mxTreeBox->get_visible())
                 UpdateStyles_Impl(StyleFlags::UpdateFamilyList);
             else
                 FillTreeBox();
         }
         else
             nAppFilter = pItem->GetValue();
    }
    const OUString aStyle(pItem->GetStyleName());
    SelectStyle(aStyle, false);
    EnableDelete();
    EnableNew( bCanNew );
}

IMPL_LINK_NOARG( SfxCommonTemplateDialog_Impl, TimeOut, Timer *, void )
{
    if(!bDontUpdate)
    {
        bDontUpdate=true;
        if (!mxTreeBox->get_visible())
            UpdateStyles_Impl(StyleFlags::UpdateFamilyList);
        else
        {
            FillTreeBox();
            SfxTemplateItem *pState = pFamilyState[nActFamily-1].get();
            if(pState)
            {
                SelectStyle(pState->GetStyleName(), false);
                EnableDelete();
            }
        }
        bDontUpdate=false;
        pIdle.reset();
    }
    else
        pIdle->Start();
}

void SfxCommonTemplateDialog_Impl::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    const SfxHintId nId = rHint.GetId();

    // tap update
    switch(nId)
    {
        case SfxHintId::UpdateDone:
            {
                SfxViewFrame *pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
                SfxObjectShell *pDocShell = pViewFrame->GetObjectShell();
                if (
                        bUpdate &&
                        (
                         !IsCheckedItem("watercan") ||
                         (pDocShell && pDocShell->GetStyleSheetPool() != pStyleSheetPool)
                        )
                   )
                {
                    bUpdate = false;
                    Update_Impl();
                }
                else if ( bUpdateFamily )
                {
                    UpdateFamily_Impl();
                }

                if( pStyleSheetPool )
                {
                    OUString aStr = GetSelectedEntry();
                    if (!aStr.isEmpty())
                    {
                        const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
                        if( !pItem ) break;
                        const SfxStyleFamily eFam = pItem->GetFamily();
                        SfxStyleSheetBase *pStyle = pStyleSheetPool->Find( aStr, eFam );
                        if( pStyle )
                        {
                            bool bReadWrite = !(pStyle->GetMask() & SfxStyleSearchBits::ReadOnly);
                            EnableEdit( bReadWrite );
                            EnableHide( bReadWrite && !pStyle->IsUsed( ) && !pStyle->IsHidden( ) );
                            EnableShow( bReadWrite && pStyle->IsHidden( ) );
                        }
                        else
                        {
                            EnableEdit(false);
                            EnableHide(false);
                            EnableShow(false);
                        }
                    }
                }
                break;
            }

            // Necessary if switching between documents and in both documents
            // the same template is used. Do not immediately call Update_Impl,
            // for the case that one of the documents is an internal InPlaceObject!
        case SfxHintId::DocChanged:
            bUpdate = true;
        break;
        case SfxHintId::Dying:
            {
                EndListening(*pStyleSheetPool);
                pStyleSheetPool=nullptr;
                break;
            }
        default: break;
    }

    // Do not set timer when the stylesheet pool is in the box, because it is
    // possible that a new one is registered after the timer is up -
    // works bad in UpdateStyles_Impl ()!

    if(!bDontUpdate && nId != SfxHintId::Dying &&
       (dynamic_cast<const SfxStyleSheetPoolHint*>(&rHint) ||
        dynamic_cast<const SfxStyleSheetHint*>(&rHint) ||
        dynamic_cast<const SfxStyleSheetModifiedHint*>(&rHint) ||
        nId == SfxHintId::StyleSheetModified))
    {
        if(!pIdle)
        {
            pIdle.reset(new Idle("SfxCommonTemplate"));
            pIdle->SetPriority(TaskPriority::LOWEST);
            pIdle->SetInvokeHandler(LINK(this,SfxCommonTemplateDialog_Impl,TimeOut));
        }
        pIdle->Start();

    }
}

// Other filters; can be switched by the users or as a result of new or
// editing, if the current document has been assigned a different filter.
void SfxCommonTemplateDialog_Impl::FilterSelect(
                sal_uInt16 nEntry,  // Idx of the new Filters
                bool bForce )   // Force update, even if the new filter is
                                // equal to the current
{
    if( nEntry == nActFilter && !bForce )
        return;

    nActFilter = nEntry;
    SfxObjectShell *const pDocShell = SaveSelection();
    SfxStyleSheetBasePool *pOldStyleSheetPool = pStyleSheetPool;
    pStyleSheetPool = pDocShell? pDocShell->GetStyleSheetPool(): nullptr;
    if ( pOldStyleSheetPool != pStyleSheetPool )
    {
        if ( pOldStyleSheetPool )
            EndListening(*pOldStyleSheetPool);
        if ( pStyleSheetPool )
            StartListening(*pStyleSheetPool);
    }

    UpdateStyles_Impl(StyleFlags::UpdateFamilyList);
}

// Internal: Perform functions through the Dispatcher
bool SfxCommonTemplateDialog_Impl::Execute_Impl(
    sal_uInt16 nId, const OUString &rStr, const OUString& rRefStr, sal_uInt16 nFamily,
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
        const OUString aTemplName(GetSelectedEntry());
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

    if ( (nId == SID_STYLE_NEW || SID_STYLE_EDIT == nId) && (mxTreeBox->get_visible() || mxFmtLb->count_selected_rows() <= 1) )
    {
        const SfxUInt16Item *pFilterItem = dynamic_cast< const SfxUInt16Item* >(pItem);
        assert(pFilterItem);
        SfxStyleSearchBits nFilterFlags = static_cast<SfxStyleSearchBits>(pFilterItem->GetValue()) & ~SfxStyleSearchBits::UserDefined;
        if(nFilterFlags == SfxStyleSearchBits::Auto)       // User Template?
            nFilterFlags = static_cast<SfxStyleSearchBits>(pFilterItem->GetValue());
        const SfxStyleFamilyItem *pFamilyItem = GetFamilyItem_Impl();
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
void SfxCommonTemplateDialog_Impl::EnableHierarchical(bool const bEnable)
{
    if (bEnable)
    {
        if (!bHierarchical)
        {
            // Turn on treeView
            bHierarchical=true;
            m_bWantHierarchical = true;
            SaveSelection(); // fdo#61429 store "hierarchical"
            const OUString aSelectEntry( GetSelectedEntry());
            mxFmtLb->hide();
            FillTreeBox();
            SelectStyle(aSelectEntry, false);
            mxTreeBox->show();
        }
    }
    else
    {
        mxTreeBox->hide();
        mxFmtLb->show();
        // If bHierarchical, then the family can have changed
        // minus one since hierarchical is inserted at the start
        m_bWantHierarchical = false; // before FilterSelect
        FilterSelect(mxFilterLb->get_active() - 1, bHierarchical );
        bHierarchical=false;
    }
}

IMPL_LINK(SfxCommonTemplateDialog_Impl, FilterSelectHdl, weld::ComboBox&, rBox, void)
{
    if (SfxResId(STR_STYLE_FILTER_HIERARCHICAL) == rBox.get_active_text())
    {
        EnableHierarchical(true);
    }
    else
    {
        EnableHierarchical(false);
    }
}

// Select-Handler for the Toolbox
void SfxCommonTemplateDialog_Impl::FamilySelect(sal_uInt16 nEntry, bool bPreviewRefresh)
{
    assert((0 < nEntry && nEntry <= MAX_FAMILIES) || 0xffff == nEntry);
    if( nEntry != nActFamily || bPreviewRefresh )
    {
        CheckItem(OString::number(nActFamily), false);
        nActFamily = nEntry;
        SfxDispatcher* pDispat = pBindings->GetDispatcher_Impl();
        SfxUInt16Item const aItem(SID_STYLE_FAMILY,
                static_cast<sal_uInt16>(SfxTemplate::NIdToSfxFamilyId(nEntry)));
        pDispat->ExecuteList(SID_STYLE_FAMILY, SfxCallMode::SYNCHRON, { &aItem });
        pBindings->Invalidate( SID_STYLE_FAMILY );
        pBindings->Update( SID_STYLE_FAMILY );
        UpdateFamily_Impl();
    }
}

void SfxCommonTemplateDialog_Impl::ActionSelect(const OString& rEntry)
{
    if (rEntry == "watercan")
    {
        const bool bOldState = !IsCheckedItem(rEntry);
        bool bCheck;
        SfxBoolItem aBool;
        // when a template is chosen.
        if (!bOldState && HasSelectedStyle())
        {
            const OUString aTemplName(
                GetSelectedEntry());
            Execute_Impl(
                SID_STYLE_WATERCAN, aTemplName, "",
                static_cast<sal_uInt16>(GetFamilyItem_Impl()->GetFamily()) );
            bCheck = true;
        }
        else
        {
            Execute_Impl(SID_STYLE_WATERCAN, "", "", 0);
            bCheck = false;
        }
        CheckItem(rEntry, bCheck);
        aBool.SetValue(bCheck);
        SetWaterCanState(&aBool);
    }
    else if (rEntry == "new" || rEntry == "newmenu")
    {
        if(pStyleSheetPool && nActFamily != 0xffff)
        {
            const SfxStyleFamily eFam=GetFamilyItem_Impl()->GetFamily();
            const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
            SfxStyleSearchBits nFilter(SfxStyleSearchBits::Auto);
            if (pItem && nActFilter != 0xffff)
                nFilter = pItem->GetFilterList()[nActFilter].nFlags;
            if (nFilter == SfxStyleSearchBits::Auto)    // automatic
                nFilter = nAppFilter;

            // why? : FloatingWindow must not be parent of a modal dialog
            SfxNewStyleDlg aDlg(mpContainer, *pStyleSheetPool, eFam);
            auto nResult = aDlg.run();
            if (nResult ==  RET_OK)
            {
                const OUString aTemplName(aDlg.GetName());
                Execute_Impl(SID_STYLE_NEW_BY_EXAMPLE,
                             aTemplName, "",
                             static_cast<sal_uInt16>(GetFamilyItem_Impl()->GetFamily()),
                             nFilter);
                UpdateFamily_Impl();
            }
        }
    }
    else if (rEntry == "update")
    {
        Execute_Impl(SID_STYLE_UPDATE_BY_EXAMPLE,
                "", "",
                static_cast<sal_uInt16>(GetFamilyItem_Impl()->GetFamily()));
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
        OSL_FAIL( "getModuleIdentifier(): exception of XModuleManager::identify()" );
    }

    return sIdentifier;
}

sal_Int32 SfxCommonTemplateDialog_Impl::LoadFactoryStyleFilter( SfxObjectShell const * i_pObjSh )
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
    Sequence< PropertyValue > lProps(1);
    lProps[0].Name = "ooSetupFactoryStyleFilter";
    lProps[0].Value <<= i_nFilter | (m_bWantHierarchical ? 0x1000 : 0);
    xModuleManager->replaceByName( getModuleIdentifier( xModuleManager, i_pObjSh ), makeAny( lProps ) );
}

SfxObjectShell* SfxCommonTemplateDialog_Impl::SaveSelection()
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

void SfxCommonTemplateDialog_Impl::DropHdl(const OUString& rStyle, const OUString& rParent)
{
    bDontUpdate = true;
    const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
    const SfxStyleFamily eFam = pItem->GetFamily();
    pStyleSheetPool->SetParent(eFam, rStyle, rParent);
    bDontUpdate = false;
}

// Handler for the New-Buttons
void SfxCommonTemplateDialog_Impl::NewHdl()
{
    if ( nActFamily == 0xffff || !(mxTreeBox->get_visible() || mxFmtLb->count_selected_rows() <= 1))
        return;

    const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
    const SfxStyleFamily eFam = pItem->GetFamily();
    SfxStyleSearchBits nMask(SfxStyleSearchBits::Auto);
    if (nActFilter != 0xffff)
        nMask = pItem->GetFilterList()[nActFilter].nFlags;
    if (nMask == SfxStyleSearchBits::Auto)    // automatic
        nMask = nAppFilter;

    Execute_Impl(SID_STYLE_NEW,
                 "", GetSelectedEntry(),
                 static_cast<sal_uInt16>(eFam),
                 nMask);
}

// Handler for the edit-Buttons
void SfxCommonTemplateDialog_Impl::EditHdl()
{
    if(IsInitialized() && HasSelectedStyle())
    {
        sal_uInt16 nFilter = nActFilter;
        OUString aTemplName(GetSelectedEntry());
        GetSelectedStyle(); // -Wall required??
        Execute_Impl( SID_STYLE_EDIT, aTemplName, OUString(),
                          static_cast<sal_uInt16>(GetFamilyItem_Impl()->GetFamily()), SfxStyleSearchBits::Auto, &nFilter );
    }
}

// Handler for the Delete-Buttons
void SfxCommonTemplateDialog_Impl::DeleteHdl()
{
    if ( !IsInitialized() || !HasSelectedStyle() )
        return;

    bool bUsedStyle = false;     // one of the selected styles are used in the document?

    std::vector<std::unique_ptr<weld::TreeIter>> aList;
    weld::TreeView* pTreeView = mxTreeBox->get_visible() ? mxTreeBox.get() : mxFmtLb.get();
    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();

    OUStringBuffer aMsg;
    aMsg.append(SfxResId(STR_DELETE_STYLE_USED)).append(SfxResId(STR_DELETE_STYLE));

    pTreeView->selected_foreach([this, pTreeView, pItem, &aList, &bUsedStyle, &aMsg](weld::TreeIter& rEntry){
        aList.emplace_back(pTreeView->make_iterator(&rEntry));
        // check the style is used or not
        const OUString aTemplName(pTreeView->get_text(rEntry));

        SfxStyleSheetBase* pStyle = pStyleSheetPool->Find( aTemplName, pItem->GetFamily() );

        if ( pStyle->IsUsed() )  // pStyle is in use in the document?
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
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pTreeView,
                                                  VclMessageType::Question, VclButtonsType::YesNo,
                                                  aMsg.makeStringAndClear()));
        aApproved = xBox->run() == RET_YES;
    }

    // if there are no used styles selected or the user approved the changes
    if ( bUsedStyle && !aApproved )
        return;

    for (auto const& elem : aList)
    {
        const OUString aTemplName(pTreeView->get_text(*elem));
        bDontUpdate = true; // To prevent the Treelistbox to shut down while deleting
        Execute_Impl( SID_STYLE_DELETE, aTemplName,
                      OUString(), static_cast<sal_uInt16>(GetFamilyItem_Impl()->GetFamily()) );

        if (mxTreeBox->get_visible())
        {
            weld::RemoveParentKeepChildren(*mxTreeBox, *elem);
            bDontUpdate = false;
        }
    }
    bDontUpdate = false; //if everything is deleted set bDontUpdate back to false
    UpdateStyles_Impl(StyleFlags::UpdateFamilyList); //and force-update the list
}

void SfxCommonTemplateDialog_Impl::HideHdl()
{
    if ( !IsInitialized() || !HasSelectedStyle() )
        return;

    weld::TreeView* pTreeView = mxTreeBox->get_visible() ? mxTreeBox.get() : mxFmtLb.get();
    pTreeView->selected_foreach([this, pTreeView](weld::TreeIter& rEntry){
        OUString aTemplName = pTreeView->get_text(rEntry);

        Execute_Impl( SID_STYLE_HIDE, aTemplName,
                      OUString(), static_cast<sal_uInt16>(GetFamilyItem_Impl()->GetFamily()) );

        return false;
    });
}

void SfxCommonTemplateDialog_Impl::ShowHdl()
{
    if ( !IsInitialized() || !HasSelectedStyle() )
        return;

    weld::TreeView* pTreeView = mxTreeBox->get_visible() ? mxTreeBox.get() : mxFmtLb.get();
    pTreeView->selected_foreach([this, pTreeView](weld::TreeIter& rEntry){
        OUString aTemplName = pTreeView->get_text(rEntry);

        Execute_Impl( SID_STYLE_SHOW, aTemplName,
                      OUString(), static_cast<sal_uInt16>(GetFamilyItem_Impl()->GetFamily()) );

        return false;
    });
}

void SfxCommonTemplateDialog_Impl::EnableDelete()
{
    bool bEnableDelete(false);
    if(IsInitialized() && HasSelectedStyle())
    {
        OSL_ENSURE(pStyleSheetPool, "No StyleSheetPool");
        const OUString aTemplName(GetSelectedEntry());
        const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
        const SfxStyleFamily eFam = pItem->GetFamily();
        SfxStyleSearchBits nFilter = SfxStyleSearchBits::Auto;
        if (pItem->GetFilterList().size() > nActFilter)
            nFilter = pItem->GetFilterList()[nActFilter].nFlags;
        if(nFilter == SfxStyleSearchBits::Auto)    // automatic
            nFilter = nAppFilter;
        const SfxStyleSheetBase *pStyle =
            pStyleSheetPool->Find(aTemplName,eFam, mxTreeBox->get_visible() ? SfxStyleSearchBits::All : nFilter);

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
                    uno::Reference<frame::XFrame > xFrame = pFrame->GetFrame().GetFrameInterface();
                    if (vcl::CommandInfoProvider::GetModuleIdentifier(xFrame) == "com.sun.star.sheet.SpreadsheetDocument")
                    {
                        bEnableDelete = true;
                    }
                }
            }
        }
    }
    EnableDel(bEnableDelete);
}

IMPL_LINK(SfxCommonTemplateDialog_Impl, MousePressHdl, const MouseEvent&, rMEvt, bool)
{
    m_nModifier = rMEvt.GetModifier();
    return false;
}

// Double-click on a style sheet in the ListBox is applied.
IMPL_LINK_NOARG(SfxCommonTemplateDialog_Impl, TreeListApplyHdl, weld::TreeView&, bool)
{
    // only if that region is allowed
    if ( IsInitialized() && nullptr != pFamilyState[nActFamily-1] &&
         !GetSelectedEntry().isEmpty() )
    {
        Execute_Impl(SID_STYLE_APPLY,
                     GetSelectedEntry(), OUString(),
                     static_cast<sal_uInt16>(GetFamilyItem_Impl()->GetFamily()),
                     SfxStyleSearchBits::Auto, nullptr, &m_nModifier);
    }
    // After selecting a focused item if possible again on the app window
    if ( dynamic_cast< const SfxTemplateDialog_Impl* >(this) !=  nullptr )
    {
        SfxViewFrame *pViewFrame = pBindings->GetDispatcher_Impl()->GetFrame();
        SfxViewShell *pVu = pViewFrame->GetViewShell();
        vcl::Window *pAppWin = pVu ? pVu->GetWindow(): nullptr;
        if(pAppWin)
            pAppWin->GrabFocus();
    }

    return true;
}

IMPL_LINK_NOARG(SfxCommonTemplateDialog_Impl, PreviewHdl, weld::Button&, void)
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch( comphelper::ConfigurationChanges::create() );
    bool bCustomPreview = mxPreviewCheckbox->get_active();
    officecfg::Office::Common::StylesAndFormatting::Preview::set(bCustomPreview, batch );
    batch->commit();

    mxFmtLb->clear();
    mxFmtLb->set_column_custom_renderer(0, bCustomPreview);
    mxTreeBox->clear();
    mxTreeBox->set_column_custom_renderer(0, bCustomPreview);

    FamilySelect(nActFamily, true);
}

// Selection of a template during the Watercan-Status
IMPL_LINK(SfxCommonTemplateDialog_Impl, FmtSelectHdl, weld::TreeView&, rListBox, void)
{
    std::unique_ptr<weld::TreeIter> xHdlEntry = rListBox.make_iterator();
    if (!rListBox.get_cursor(xHdlEntry.get()))
        return;

    if (rListBox.is_selected(*xHdlEntry))
        UpdateStyleDependents();

    SelectStyle(rListBox.get_text(*xHdlEntry), true);
}

void SfxCommonTemplateDialog_Impl::UpdateStyleDependents()
{
    // Trigger Help PI. Only when the watercan is on
    if ( IsInitialized() &&
         IsCheckedItem("watercan") &&
         // only if that region is allowed
         nullptr != pFamilyState[nActFamily-1] && (mxTreeBox || mxFmtLb->count_selected_rows() <= 1) )
    {
        Execute_Impl(SID_STYLE_WATERCAN,
                     "", "", 0);
        Execute_Impl(SID_STYLE_WATERCAN,
                     GetSelectedEntry(), "",
                     static_cast<sal_uInt16>(GetFamilyItem_Impl()->GetFamily()));
    }
    EnableItem("watercan", !bWaterDisabled);
    EnableDelete();
}

void SfxCommonTemplateDialog_Impl::MenuSelect(const OString& rIdent)
{
    sLastItemIdent = rIdent;
    if (sLastItemIdent.isEmpty())
        return;
    Application::PostUserEvent(
        LINK(this, SfxCommonTemplateDialog_Impl, MenuSelectAsyncHdl));
}

IMPL_LINK_NOARG(SfxCommonTemplateDialog_Impl, MenuSelectAsyncHdl, void*, void)
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

SfxStyleFamily SfxCommonTemplateDialog_Impl::GetActualFamily() const
{
    const SfxStyleFamilyItem *pFamilyItem = GetFamilyItem_Impl();
    if( !pFamilyItem || nActFamily == 0xffff )
        return SfxStyleFamily::Para;
    else
        return pFamilyItem->GetFamily();
}

void SfxCommonTemplateDialog_Impl::EnableExample_Impl(sal_uInt16 nId, bool bEnable)
{
    bool bDisable = !bEnable || !IsSafeForWaterCan();
    if (nId == SID_STYLE_NEW_BY_EXAMPLE)
    {
        bNewByExampleDisabled = bDisable;
        EnableItem("new", bEnable);
        EnableItem("newmenu", bEnable);
    }
    else if( nId == SID_STYLE_UPDATE_BY_EXAMPLE )
    {
        bUpdateByExampleDisabled = bDisable;
        EnableItem("update", bEnable);
    }
}

void SfxCommonTemplateDialog_Impl::CreateContextMenu()
{
    if ( bBindingUpdate )
    {
        pBindings->Invalidate( SID_STYLE_NEW, true );
        pBindings->Update( SID_STYLE_NEW );
        bBindingUpdate = false;
    }
    mxMenu.reset();
    mxMenuBuilder.reset(Application::CreateBuilder(nullptr, "sfx/ui/stylecontextmenu.ui"));
    mxMenu = mxMenuBuilder->weld_menu("menu");
    mxMenu->set_sensitive("edit", bCanEdit);
    mxMenu->set_sensitive("delete", bCanDel);
    mxMenu->set_sensitive("new", bCanNew);
    mxMenu->set_sensitive("hide", bCanHide);
    mxMenu->set_sensitive("show", bCanShow);

    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
    if (pItem && pItem->GetFamily() == SfxStyleFamily::Table) //tdf#101648, no ui for this yet
    {
        mxMenu->set_sensitive("edit", false);
        mxMenu->set_sensitive("new", false);
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

void SfxCommonTemplateDialog_Impl::InvalidateBindings()
{
    pBindings->Invalidate(SID_STYLE_NEW_BY_EXAMPLE, true);
    pBindings->Update( SID_STYLE_NEW_BY_EXAMPLE );
    pBindings->Invalidate(SID_STYLE_UPDATE_BY_EXAMPLE, true);
    pBindings->Update( SID_STYLE_UPDATE_BY_EXAMPLE );
    pBindings->Invalidate( SID_STYLE_WATERCAN, true);
    pBindings->Update( SID_STYLE_WATERCAN );
    pBindings->Invalidate( SID_STYLE_NEW, true);
    pBindings->Update( SID_STYLE_NEW );
    pBindings->Invalidate( SID_STYLE_DRAGHIERARCHIE, true);
    pBindings->Update( SID_STYLE_DRAGHIERARCHIE );
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
        Execute_Impl(SID_STYLE_WATERCAN, "", "", 0);
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
    FamilySelect(rEntry.toUInt32());
}

IMPL_LINK(SfxTemplateDialog_Impl, ToolBoxRSelect, const OString&, rEntry, void)
{
    if (rEntry == "newmenu")
        m_xActionTbR->set_menu_item_active(rEntry, !m_xActionTbR->get_menu_item_active(rEntry));
    else
        ActionSelect(rEntry);
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
    ActionSelect(rMenuId);
}

void SfxCommonTemplateDialog_Impl::SetFamily(SfxStyleFamily const nFamily)
{
    sal_uInt16 const nId(SfxTemplate::SfxFamilyIdToNId(nFamily));
    assert((0 < nId && nId <= MAX_FAMILIES) || 0xffff == nId);
    if ( nId != nActFamily )
    {
        if ( nActFamily != 0xFFFF )
            CheckItem(OString::number(nActFamily), false);
        nActFamily = nId;
        if ( nId != 0xFFFF )
            bUpdateFamily = true;
    }
}

void SfxCommonTemplateDialog_Impl::UpdateFamily_Impl()
{
    bUpdateFamily = false;

    SfxDispatcher* pDispat = pBindings->GetDispatcher_Impl();
    SfxViewFrame *pViewFrame = pDispat->GetFrame();
    SfxObjectShell *pDocShell = pViewFrame->GetObjectShell();

    SfxStyleSheetBasePool *pOldStyleSheetPool = pStyleSheetPool;
    pStyleSheetPool = pDocShell? pDocShell->GetStyleSheetPool(): nullptr;
    if ( pOldStyleSheetPool != pStyleSheetPool )
    {
        if ( pOldStyleSheetPool )
            EndListening(*pOldStyleSheetPool);
        if ( pStyleSheetPool )
            StartListening(*pStyleSheetPool);
    }

    bWaterDisabled = false;
    bCanNew = mxTreeBox->get_visible() || mxFmtLb->count_selected_rows() <= 1;
    bTreeDrag = true;
    bUpdateByExampleDisabled = false;

    if (pStyleSheetPool)
    {
        if (!mxTreeBox->get_visible())
            UpdateStyles_Impl(StyleFlags::UpdateFamily | StyleFlags::UpdateFamilyList);
        else
        {
            UpdateStyles_Impl(StyleFlags::UpdateFamily);
            FillTreeBox();
        }
    }

    InvalidateBindings();

    if (IsCheckedItem("watercan") &&
         // only if that area is allowed
         nullptr != pFamilyState[nActFamily - 1])
    {
        Execute_Impl(SID_STYLE_APPLY,
                     GetSelectedEntry(),
                     OUString(),
                     static_cast<sal_uInt16>(GetFamilyItem_Impl()->GetFamily()));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
