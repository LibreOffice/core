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

StyleList::StyleList(weld::Builder* pBuilder, const OString& flatviewname,
                     const OString& hierViewName, bool bAllow, SfxStyleSheetBasePool* pPool,
                     bool exampleDisabled, bool dontUpdate, sal_uInt16 nActfam, bool btreeDrag,
                     std::optional<SfxStyleFamilies> mxFamilies, bool bCanEdit, bool bCanHide,
                     bool bCanShow, bool bUpdateDisabled, SfxStyleSearchBits sfxStyleSearchBits,
                     sal_uInt16 nActfilter, bool bHeirarchical, bool m_bWantHierarchical,
                     const OString& filtername)
{
    mxFmtLb = pBuilder->weld_tree_view(flatviewname);
    mxTreeBox = pBuilder->weld_tree_view(hierViewName);
    bAllowReParentDrop = bAllow;
    pPool = pStyleSheetPool;
    bNewByExampleDisabled = exampleDisabled;
    bDontUpdate = dontUpdate;
    nActFamily = nActfam;
    btreeDrag = bTreeDrag;
    mxStyleFamilies = mxFamilies;
    this->bCanEdit = bCanEdit;
    this->bCanHide = bCanHide;
    this->bCanShow = bCanShow;
    this->bUpdateByExampleDisabled = bUpdateDisabled;
    this->nAppFilter = sfxStyleSearchBits;
    this->nActFilter = nActFilter;
    this->bHierarchical = bHierarchical;
    this->m_bWantHierarchical = m_bWantHierarchical;
    mxFilterLb = pBuilder->weld_combo_box(filtername);
}

//Destructor

StyleList::~StyleList()
{}

/** Drop is enabled as long as it is allowed to create a new style by example, i.e. to
    create a style out of the current selection.
*/
sal_Int8 StyleList::AcceptDrop(const AcceptDropEvent& rEvt, const DropTargetHelper& rHelper)
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

        for ( sal_uInt32 i = 0; i < nFormatCount; ++i )
        {
            SotClipboardFormatId nId = aHelper.GetFormat(i);
            TransferableObjectDescriptor aDesc;

            if ( aHelper.GetTransferableObjectDescriptor( nId, aDesc ) )
            {
                if ( aDesc.maClassName == pDocShell->GetFactory().GetClassId() )
                {
                    Application::PostUserEvent(LINK(pParentDialog, SfxCommonTemplateDialog_Impl, OnAsyncExecuteDrop));

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
    FillTreeBox(GetActualFamily());
    SelectStyle(aTargetStyle, false, GetActualFamily());
    return DND_ACTION_NONE;
}

void StyleList::DropHdl(const OUString& rStyle, const OUString& rParent)
{
    bDontUpdate = true;
    const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
    const SfxStyleFamily eFam = pItem->GetFamily();
    pStyleSheetPool->SetParent(eFam, rStyle, rParent);
    bDontUpdate = false;
}

weld::TreeView* StyleList::PrepareMenu(const Point& rPos)
{
    weld::TreeView* pTreeView = mxTreeBox->get_visible() ? mxTreeBox.get() : mxFmtLb.get();
    std::unique_ptr<weld::TreeIter> xIter(pTreeView->make_iterator());
    if (pTreeView->get_dest_row_at_pos(rPos, xIter.get(), false) && !pTreeView->is_selected(*xIter))
    {
        pTreeView->unselect_all();
        pTreeView->set_cursor(*xIter);
        pTreeView->select(*xIter);
    }
    return pTreeView;
}

weld::TreeView* StyleList::ShowMenu(const CommandEvent& rCEvt)
{
    weld::TreeView* pTreeView = mxTreeBox->get_visible() ? mxTreeBox.get() : mxFmtLb.get();
    return pTreeView;
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
                             std::u16string_view rStr)
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

sal_uInt16 StyleList::StyleNrToInfoOffset(sal_uInt16 nId)
{
    const SfxStyleFamilyItem& rItem = mxStyleFamilies->at( nId );
    return SfxTemplate::SfxFamilyIdToNId(rItem.GetFamily())-1;
}

// Helper function: Access to the current family item

const SfxStyleFamilyItem *StyleList::GetFamilyItem_Impl() const
{
    const size_t nCount = mxStyleFamilies->size();
    for(size_t i = 0; i < nCount; ++i)
    {
        const SfxStyleFamilyItem &rItem = mxStyleFamilies->at( i );
        sal_uInt16 nId = SfxTemplate::SfxFamilyIdToNId(rItem.GetFamily());
        if(nId == nActFamily)
            return &rItem;
    }
    return nullptr;
}

void StyleList::GetSelectedStyle() const
{
    const OUString aTemplName( GetSelectedEntry() );
    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
    pStyleSheetPool->Find( aTemplName, pItem->GetFamily() );
}

OUString StyleList::GetSelectedEntry() const
{
    OUString aRet;
    if (mxTreeBox->get_visible())
        aRet = mxTreeBox->get_selected_text();
    else
        aRet = mxFmtLb->get_selected_text();
    return aRet;
}

/**
 * Is it safe to show the water-can / fill icon. If we've a
 * hierarchical widget - we have only single select, otherwise
 * we need to check if we have a multi-selection. We either have
 * a mxTreeBox showing or an mxFmtLb (which we hide when not shown)
 */
bool StyleList::IsSafeForWaterCan() const
{
    if (mxTreeBox->get_visible())
        return mxTreeBox->get_selected_index() != -1;
    else
        return mxFmtLb->count_selected_rows() == 1;
}

void StyleList::SelectStyle(const OUString &rStr, bool bIsCallback, SfxStyleFamily eFam)
{

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
            else if (eFam == SfxStyleFamily::Pseudo)
            {
                std::unique_ptr<weld::TreeIter> xEntry = mxTreeBox->make_iterator();
                if (mxTreeBox->get_iter_first(*xEntry))
                {
                    mxTreeBox->scroll_to_row(*xEntry);
                    mxTreeBox->select(*xEntry);
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
                pParentDialog->EnableEdit(false);
                pParentDialog->EnableHide(false);
                pParentDialog->EnableShow(false);
            }
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

void StyleList::FillTreeBox(SfxStyleFamily eFam)
{
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

    pParentDialog->EnableItem("watercan", false);

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
    SelectStyle(aStyle, false, eFam);
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
    if (nFamily == SfxStyleFamily::Pseudo)
        return "NumberingStyles";
    return OUString();
}

OUString StyleList::getDefaultStyleName( const SfxStyleFamily eFam )
{
    OUString sDefaultStyle;
    OUString aFamilyName = lcl_GetStyleFamilyName(eFam);
    if( aFamilyName == "TableStyles" )
        sDefaultStyle = "Default Style";
    else if(aFamilyName == "NumberingStyles")
        sDefaultStyle = "No List";
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

SfxStyleFamily StyleList::GetActualFamily() const
{
    const SfxStyleFamilyItem *pFamilyItem = GetFamilyItem_Impl();
    if( !pFamilyItem || nActFamily == 0xffff )
        return SfxStyleFamily::Para;
    else
        return pFamilyItem->GetFamily();
}

bool StyleList::HasSelectedStyle() const
{
    return mxTreeBox->get_visible() ? mxTreeBox->get_selected_index() != -1
                                    : mxFmtLb->count_selected_rows() != 0;
}

void StyleList::UpdateStyles_Impl(StyleFlags nFlags)
{
    OSL_ENSURE(nFlags != StyleFlags::NONE, "nothing to do");
    const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
    if (!pItem)
    {
        // Is the case for the template catalog
        const size_t nFamilyCount = mxStyleFamilies->size();
        size_t n;
        for( n = 0; n < nFamilyCount; n++ )
            if( pFamilyState[ StyleNrToInfoOffset(n) ] ) break;
        if ( n == nFamilyCount )
            // It happens sometimes, God knows why
            return;
        nAppFilter = pFamilyState[StyleNrToInfoOffset(n)]->GetValue();
        pParentDialog->FamilySelect(  StyleNrToInfoOffset(n)+1 );
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
        pParentDialog->CheckItem(OString::number(nActFamily));    // check Button in Toolbox

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

    pParentDialog->EnableItem("watercan", false);

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
    SelectStyle(aStyle, false, GetActualFamily());
    pParentDialog->EnableDelete();
}

// Handler Listbox of Filter
void StyleList::EnableHierarchical(bool const bEnable)
{
    if (bEnable)
    {
        if (!bHierarchical)
        {
            // Turn on treeView
            bHierarchical=true;
            m_bWantHierarchical = true;
            pParentDialog->SaveSelection(); // fdo#61429 store "hierarchical"
            const OUString aSelectEntry(GetSelectedEntry());
            mxFmtLb->hide();
            FillTreeBox(GetActualFamily());
            SelectStyle(aSelectEntry, false, GetActualFamily());
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

// Other filters; can be switched by the users or as a result of new or
// editing, if the current document has been assigned a different filter.
void StyleList::FilterSelect(
                sal_uInt16 nEntry,  // Idx of the new Filters
                bool bForce )   // Force update, even if the new filter is
                                // equal to the current
{
    if( nEntry == nActFilter && !bForce )
        return;

    nActFilter = nEntry;
    SfxObjectShell *const pDocShell = pParentDialog->SaveSelection();
    SfxStyleSheetBasePool *pOldStyleSheetPool = pStyleSheetPool;
    pStyleSheetPool = pDocShell? pDocShell->GetStyleSheetPool(): nullptr;
    if ( pOldStyleSheetPool != pStyleSheetPool )
    {
        if ( pOldStyleSheetPool )
            pParentDialog->EndListening(*pOldStyleSheetPool);
        if ( pStyleSheetPool )
            pParentDialog->StartListening(*pStyleSheetPool);
    }

    UpdateStyles_Impl(StyleFlags::UpdateFamilyList);
}


// Handler for the New-Buttons
void StyleList::NewHdl()
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

    pParentDialog->Execute_Impl(SID_STYLE_NEW,
                 "", GetSelectedEntry(),
                 static_cast<sal_uInt16>(eFam),
                 nMask);
}

// Handler for the edit-Buttons
void StyleList::EditHdl()
{
    if(pParentDialog->IsInitialized() && HasSelectedStyle())
    {
        sal_uInt16 nFilter = nActFilter;
        OUString aTemplName(GetSelectedEntry());
        GetSelectedStyle(); // -Wall required??
        pParentDialog->Execute_Impl( SID_STYLE_EDIT, aTemplName, OUString(),
                          static_cast<sal_uInt16>(GetFamilyItem_Impl()->GetFamily()), SfxStyleSearchBits::Auto, &nFilter );
    }
}

// Handler for the Delete-Buttons
void StyleList::DeleteHdl()
{
    if ( !pParentDialog->IsInitialized() || !HasSelectedStyle() )
        return;

    bool bUsedStyle = false;     // one of the selected styles are used in the document?

    std::vector<std::unique_ptr<weld::TreeIter>> aList;
    weld::TreeView* pTreeView = mxTreeBox->get_visible() ? mxTreeBox.get() : mxFmtLb.get();
    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();

    OUStringBuffer aMsg;
    aMsg.append(SfxResId(STR_DELETE_STYLE_USED) + SfxResId(STR_DELETE_STYLE));

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
        pParentDialog->Execute_Impl( SID_STYLE_DELETE, aTemplName,
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

void StyleList::HideHdl()
{
    if ( !pParentDialog->IsInitialized() || !HasSelectedStyle() )
        return;

    weld::TreeView* pTreeView = mxTreeBox->get_visible() ? mxTreeBox.get() : mxFmtLb.get();
    pTreeView->selected_foreach([this, pTreeView](weld::TreeIter& rEntry){
        OUString aTemplName = pTreeView->get_text(rEntry);

        pParentDialog->Execute_Impl( SID_STYLE_HIDE, aTemplName,
                      OUString(), static_cast<sal_uInt16>(GetFamilyItem_Impl()->GetFamily()) );

        return false;
    });
}

void StyleList::ShowHdl()
{
    if ( !pParentDialog->IsInitialized() || !HasSelectedStyle() )
        return;

    weld::TreeView* pTreeView = mxTreeBox->get_visible() ? mxTreeBox.get() : mxFmtLb.get();
    pTreeView->selected_foreach([this, pTreeView](weld::TreeIter& rEntry){
        OUString aTemplName = pTreeView->get_text(rEntry);

        pParentDialog->Execute_Impl( SID_STYLE_SHOW, aTemplName,
                      OUString(), static_cast<sal_uInt16>(GetFamilyItem_Impl()->GetFamily()) );

        return false;
    });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
