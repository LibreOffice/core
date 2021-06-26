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


/*

// Item with the status of a Family is copied and noted
// (is updated when all states have also been updated.)
// See also: <SfxBindings::AddDoneHdl(const Link &)>
void weld::StyleList::SetFamilyState( sal_uInt16 nSlotId, const SfxTemplateItem* pItem )
{
    sal_uInt16 nIdx = nSlotId - SID_STYLE_FAMILY_START;
    pFamilyState[nIdx].reset();
    if ( pItem )
        pFamilyState[nIdx].reset( new SfxTemplateItem(*pItem) );
    bUpdate = true;

    // If used templates (how the hell you find this out??)
    bUpdateFamily = true;
}


void weld::StyleList::Notify(SfxBroadcaster& , const SfxHint& rHint)
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
                         (pDocShell && pDocShell->GetStyleSheetPool() != m_aStyleList.pStyleSheetPool)
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

                if( m_aStyleList.pStyleSheetPool )
                {
                    OUString aStr = GetSelectedEntry();
                    if (!aStr.isEmpty())
                    {
                        const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
                        if( !pItem ) break;
                        const SfxStyleFamily eFam = pItem->GetFamily();
                        SfxStyleSheetBase *pStyle = m_aStyleList.pStyleSheetPool->Find( aStr, eFam );
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
                EndListening(*m_aStyleList.pStyleSheetPool);
                m_aStyleList.pStyleSheetPool=nullptr;
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
            pIdle->SetInvokeHandler(LINK(this,weld::StyleList,TimeOut));
        }
        pIdle->Start();

    }
}

// Other filters; can be switched by the users or as a result of new or
// editing, if the current document has been assigned a different filter.
void weld::StyleList::FilterSelect(
                sal_uInt16 nEntry,  // Idx of the new Filters
                bool bForce )   // Force update, even if the new filter is
                                // equal to the current
{
    if( nEntry == nActFilter && !bForce )
        return;

    nActFilter = nEntry;
    SfxObjectShell *const pDocShell = SaveSelection();
    SfxStyleSheetBasePool *pOldStyleSheetPool = m_aStyleList.pStyleSheetPool;
    m_aStyleList.pStyleSheetPool = pDocShell? pDocShell->GetStyleSheetPool(): nullptr;
    if ( pOldStyleSheetPool != m_aStyleList.pStyleSheetPool )
    {
        if ( pOldStyleSheetPool )
            EndListening(*pOldStyleSheetPool);
        if ( m_aStyleList.pStyleSheetPool )
            StartListening(*m_aStyleList.pStyleSheetPool);
    }

    UpdateStyles_Impl(StyleFlags::UpdateFamilyList);
}

// Handler Listbox of Filter
void weld::StyleList::EnableHierarchical(bool const bEnable)
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

IMPL_LINK(weld::StyleList, FilterSelectHdl, weld::ComboBox&, rBox, void)
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
void weld::StyleList::FamilySelect(sal_uInt16 nEntry, bool bPreviewRefresh)
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

sal_Int32 weld::StyleList::LoadFactoryStyleFilter( SfxObjectShell const * i_pObjSh )
{
    OSL_ENSURE( i_pObjSh, "weld::StyleList::LoadFactoryStyleFilter(): no ObjectShell" );

    ::comphelper::SequenceAsHashMap aFactoryProps(
        xModuleManager->getByName( getModuleIdentifier( xModuleManager, i_pObjSh ) ) );
    sal_Int32 nFilter = aFactoryProps.getUnpackedValueOrDefault( "ooSetupFactoryStyleFilter", sal_Int32(-1) );

    m_bWantHierarchical = (nFilter & 0x1000) != 0;
    nFilter &= ~0x1000; // clear it

    return nFilter;
}

void weld::StyleList::SaveFactoryStyleFilter( SfxObjectShell const * i_pObjSh, sal_Int32 i_nFilter )
{
    OSL_ENSURE( i_pObjSh, "weld::StyleList::LoadFactoryStyleFilter(): no ObjectShell" );
    Sequence< PropertyValue > lProps(1);
    lProps[0].Name = "ooSetupFactoryStyleFilter";
    lProps[0].Value <<= i_nFilter | (m_bWantHierarchical ? 0x1000 : 0);
    xModuleManager->replaceByName( getModuleIdentifier( xModuleManager, i_pObjSh ), makeAny( lProps ) );
}

SfxObjectShell* weld::StyleList::SaveSelection()
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

void weld::StyleList::DropHdl(const OUString& rStyle, const OUString& rParent)
{
    bDontUpdate = true;
    const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
    const SfxStyleFamily eFam = pItem->GetFamily();
    m_aStyleList.pStyleSheetPool->SetParent(eFam, rStyle, rParent);
    bDontUpdate = false;
}

// Handler for the New-Buttons
void weld::StyleList::NewHdl()
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
void weld::StyleList::EditHdl()
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
void weld::StyleList::DeleteHdl()
{
    if ( !IsInitialized() || !HasSelectedStyle() )
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

        SfxStyleSheetBase* pStyle = m_aStyleList.pStyleSheetPool->Find( aTemplName, pItem->GetFamily() );

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

void weld::StyleList::HideHdl()
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

void weld::StyleList::ShowHdl()
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

void weld::StyleList::EnableDelete()
{
    bool bEnableDelete(false);
    if(IsInitialized() && HasSelectedStyle())
    {
        OSL_ENSURE(m_aStyleList.pStyleSheetPool, "No StyleSheetPool");
        const OUString aTemplName(GetSelectedEntry());
        const SfxStyleFamilyItem *pItem = GetFamilyItem_Impl();
        const SfxStyleFamily eFam = pItem->GetFamily();
        SfxStyleSearchBits nFilter = SfxStyleSearchBits::Auto;
        if (pItem->GetFilterList().size() > nActFilter)
            nFilter = pItem->GetFilterList()[nActFilter].nFlags;
        if(nFilter == SfxStyleSearchBits::Auto)    // automatic
            nFilter = nAppFilter;
        const SfxStyleSheetBase *pStyle =
            m_aStyleList.pStyleSheetPool->Find(aTemplName,eFam, mxTreeBox->get_visible() ? SfxStyleSearchBits::All : nFilter);

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

IMPL_LINK(weld::StyleList, MousePressHdl, const MouseEvent&, rMEvt, bool)
{
    m_nModifier = rMEvt.GetModifier();
    return false;
}

// Double-click on a style sheet in the ListBox is applied.
IMPL_LINK_NOARG(weld::StyleList, TreeListApplyHdl, weld::TreeView&, bool)
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

IMPL_LINK_NOARG(weld::StyleList, PreviewHdl, weld::Toggleable&, void)
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
IMPL_LINK(weld::StyleList, FmtSelectHdl, weld::TreeView&, rListBox, void)
{
    std::unique_ptr<weld::TreeIter> xHdlEntry = rListBox.make_iterator();
    if (!rListBox.get_cursor(xHdlEntry.get()))
        return;

    if (rListBox.is_selected(*xHdlEntry))
        UpdateStyleDependents();

    SelectStyle(rListBox.get_text(*xHdlEntry), true);
}

void weld::StyleList::UpdateStyleDependents()
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

void weld::StyleList::MenuSelect(const OString& rIdent)
{
    sLastItemIdent = rIdent;
    if (sLastItemIdent.isEmpty())
        return;
    Application::PostUserEvent(
        LINK(this, weld::StyleList, MenuSelectAsyncHdl));
}

IMPL_LINK_NOARG(weld::StyleList, MenuSelectAsyncHdl, void*, void)
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

SfxStyleFamily weld::StyleList::GetActualFamily() const
{
    const SfxStyleFamilyItem *pFamilyItem = GetFamilyItem_Impl();
    if( !pFamilyItem || nActFamily == 0xffff )
        return SfxStyleFamily::Para;
    else
        return pFamilyItem->GetFamily();
}

void weld::StyleList::EnableExample_Impl(sal_uInt16 nId, bool bEnable)
{
    bool bDisable = !bEnable || !IsSafeForWaterCan();
    if (nId == SID_STYLE_NEW_BY_EXAMPLE)
    {
        m_aStyleList.bNewByExampleDisabled = bDisable;
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
    : weld::StyleList(pB, pDlgWindow->get_container(), pDlgWindow->get_builder())
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
    weld::StyleList::Initialize();

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

void weld::StyleList::InvalidateBindings()
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

void weld::StyleList::SetFamily(SfxStyleFamily const nFamily)
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

void weld::StyleList::UpdateFamily_Impl()
{
    bUpdateFamily = false;

    SfxDispatcher* pDispat = pBindings->GetDispatcher_Impl();
    SfxViewFrame *pViewFrame = pDispat->GetFrame();
    SfxObjectShell *pDocShell = pViewFrame->GetObjectShell();

    SfxStyleSheetBasePool *pOldStyleSheetPool = m_aStyleList.pStyleSheetPool;
    m_aStyleList.pStyleSheetPool = pDocShell? pDocShell->GetStyleSheetPool(): nullptr;
    if ( pOldStyleSheetPool != m_aStyleList.pStyleSheetPool )
    {
        if ( pOldStyleSheetPool )
            EndListening(*pOldStyleSheetPool);
        if ( m_aStyleList.pStyleSheetPool )
            StartListening(*m_aStyleList.pStyleSheetPool);
    }

    bWaterDisabled = false;
    bCanNew = mxTreeBox->get_visible() || mxFmtLb->count_selected_rows() <= 1;
    bTreeDrag = true;
    bUpdateByExampleDisabled = false;

    if (m_aStyleList.pStyleSheetPool)
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

void weld::StyleList::ReplaceUpdateButtonByMenu()
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
        !m_aStyleList.bNewByExampleDisabled)
    {
        nReturn = DND_ACTION_COPY;
    }
    return nReturn;
}
*/

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
