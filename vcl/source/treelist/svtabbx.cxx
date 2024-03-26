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

#include <comphelper/types.hxx>
#include <vcl/svtaccessiblefactory.hxx>
#include <vcl/accessiblefactory.hxx>
#include <vcl/toolkit/svtabbx.hxx>
#include <vcl/headbar.hxx>
#include <vcl/toolkit/svlbitm.hxx>
#include <vcl/toolkit/treelistentry.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <strings.hrc>
#include <svdata.hxx>
#include <memory>
#include <tools/json_writer.hxx>
#include <comphelper/propertyvalue.hxx>
#include <vcl/filter/PngImageWriter.hxx>
#include <comphelper/base64.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

constexpr SvLBoxTabFlags MYTABMASK =
    SvLBoxTabFlags::ADJUST_RIGHT | SvLBoxTabFlags::ADJUST_LEFT | SvLBoxTabFlags::ADJUST_CENTER | SvLBoxTabFlags::FORCE;

namespace {
    OString lcl_extractPngString(const BitmapEx& rImage)
    {
        SvMemoryStream aOStm(65535, 65535);
        // Use fastest compression "1"
        css::uno::Sequence<css::beans::PropertyValue> aFilterData{
            comphelper::makePropertyValue("Compression", sal_Int32(1)),
        };
        vcl::PngImageWriter aPNGWriter(aOStm);
        aPNGWriter.setParameters(aFilterData);
        if (aPNGWriter.write(rImage))
        {
            css::uno::Sequence<sal_Int8> aSeq(static_cast<sal_Int8 const*>(aOStm.GetData()),
                                            aOStm.Tell());
            OStringBuffer aBuffer("data:image/png;base64,");
            ::comphelper::Base64::encode(aBuffer, aSeq);
            return aBuffer.makeStringAndClear();
        }

        return ""_ostr;
    }
}

static void lcl_DumpEntryAndSiblings(tools::JsonWriter& rJsonWriter,
                                     SvTreeListEntry* pEntry,
                                     SvTabListBox* pTabListBox,
                                     bool bCheckButtons)
{
    while (pEntry)
    {
        auto aNode = rJsonWriter.startStruct();

        // DEPRECATED
        // simple listbox value
        const SvLBoxItem* pIt = pEntry->GetFirstItem(SvLBoxItemType::String);
        if (pIt)
            rJsonWriter.put("text", static_cast<const SvLBoxString*>(pIt)->GetText());

        // column based data
        {
            auto aColumns = rJsonWriter.startArray("columns");

            for (size_t i = 0; i < pEntry->ItemCount(); i++)
            {
                SvLBoxItem& rItem = pEntry->GetItem(i);
                if (rItem.GetType() == SvLBoxItemType::String)
                {
                    const SvLBoxString* pStringItem = dynamic_cast<const SvLBoxString*>(&rItem);
                    if (pStringItem)
                    {
                        auto aColumn = rJsonWriter.startStruct();
                        rJsonWriter.put("text", pStringItem->GetText());
                    }
                }
                else if (rItem.GetType() == SvLBoxItemType::ContextBmp)
                {
                    const SvLBoxContextBmp* pBmpItem = dynamic_cast<const SvLBoxContextBmp*>(&rItem);
                    if (pBmpItem)
                    {
                        const OUString& rCollapsed = pBmpItem->GetBitmap1().GetStock();
                        const OUString& rExpanded = pBmpItem->GetBitmap2().GetStock();

                        // send identifier only, we will use svg icon
                        if (!o3tl::trim(rCollapsed).empty() || !o3tl::trim(rExpanded).empty())
                        {
                            auto aColumn = rJsonWriter.startStruct();
                            if (!o3tl::trim(rCollapsed).empty())
                                rJsonWriter.put("collapsed", rCollapsed);
                            if (!o3tl::trim(rExpanded).empty())
                                rJsonWriter.put("expanded", rExpanded);
                        }
                        // custom bitmap - send png
                        else
                        {
                            BitmapEx aCollapsedImage = pBmpItem->GetBitmap1().GetBitmapEx();
                            BitmapEx aExpandedImage = pBmpItem->GetBitmap2().GetBitmapEx();
                            bool bHasCollapsed = !aCollapsedImage.IsEmpty() && !aCollapsedImage.GetSizePixel().IsEmpty();
                            bool bHasExpanded = !aExpandedImage.IsEmpty() && !aExpandedImage.GetSizePixel().IsEmpty();
                            if (bHasCollapsed || bHasExpanded)
                            {
                                auto aColumn = rJsonWriter.startStruct();
                                if (bHasCollapsed)
                                    rJsonWriter.put("collapsedimage", lcl_extractPngString(aCollapsedImage));
                                if (bHasExpanded)
                                    rJsonWriter.put("collapsedimage", lcl_extractPngString(aExpandedImage));
                            }
                        }
                    }
                }
            }
        }

        // SalInstanceTreeView does not use the flag CHILDREN_ON_DEMAND
        // and it creates a dummy child
        const SvTreeListEntries& rChildren = pEntry->GetChildEntries();
        if (rChildren.size() == 1)
        {
            auto& rChild = rChildren[0];
            if (const SvLBoxItem* pChild = rChild->GetFirstItem(SvLBoxItemType::String))
            {
                if (static_cast<const SvLBoxString*>(pChild)->GetText() == "<dummy>")
                    rJsonWriter.put("ondemand", true);
            }
        }
        if (rChildren.size() > 0 && !pTabListBox->IsExpanded(pEntry))
        {
            rJsonWriter.put("collapsed", true);
        }

        if (bCheckButtons)
        {
            SvButtonState eCheckState = pTabListBox->GetCheckButtonState(pEntry);
            if (eCheckState == SvButtonState::Unchecked)
                rJsonWriter.put("state", false);
            else if (eCheckState == SvButtonState::Checked)
                rJsonWriter.put("state", true);
        }

        if (pTabListBox->IsSelected(pEntry))
            rJsonWriter.put("selected", true);

        rJsonWriter.put("row", pTabListBox->GetModel()->GetAbsPos(pEntry));

        SvTreeListEntry* pChild = pTabListBox->FirstChild(pEntry);
        if (pChild)
        {
            auto childrenNode = rJsonWriter.startArray("children");
            lcl_DumpEntryAndSiblings(rJsonWriter, pChild, pTabListBox, bCheckButtons);
        }

        pEntry = pEntry->NextSibling();
    }
}

void SvTabListBox::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
{
    SvTreeListBox::DumpAsPropertyTree(rJsonWriter);

    rJsonWriter.put("singleclickactivate", GetActivateOnSingleClick());

    bool bCheckButtons = static_cast<int>(nTreeFlags & SvTreeFlags::CHKBTN);

    bool isRadioButton = false;
    if (pCheckButtonData)
    {
        isRadioButton = pCheckButtonData -> IsRadio();
    }

    OUString checkboxtype;
    if (bCheckButtons)
    {
        checkboxtype = "checkbox";
        if(isRadioButton)
        {
            checkboxtype = "radio";
        }
    }

    rJsonWriter.put("checkboxtype", checkboxtype);
    auto entriesNode = rJsonWriter.startArray("entries");
    lcl_DumpEntryAndSiblings(rJsonWriter, First(), this, bCheckButtons);
}

// SvTreeListBox callback

void SvTabListBox::SetTabs()
{
    SvTreeListBox::SetTabs();
    if( mvTabList.empty() )
        return;

    DBG_ASSERT(!mvTabList.empty(),"TabList ?");

    // The tree listbox has now inserted its tabs into the list. Now we
    // fluff up the list with additional tabs and adjust the rightmost tab
    // of the tree listbox.

    // Picking the rightmost tab.
    // HACK for the explorer! If ViewParent != 0, the first tab of the tree
    // listbox is calculated by the tree listbox itself! This behavior is
    // necessary for ButtonsOnRoot, as the explorer does not know in this
    // case, which additional offset it needs to add to the tabs in this mode
    // -- the tree listbox knows that, though!
    /*
    if( !pViewParent )
    {
    SvLBoxTab* pFirstTab = (SvLBoxTab*)aTabs.GetObject( aTabs.Count()-1 );
    pFirstTab->SetPos( pTabList[0].GetPos() );
    pFirstTab->nFlags &= ~MYTABMASK;
    pFirstTab->nFlags |= pTabList[0].nFlags;
    }
    */

    // the 1st column (index 1 or 2 depending on button flags) is always set
    // editable by SvTreeListBox::SetTabs(),
    // which prevents setting a different column to editable as the first
    // one with the flag is picked in SvTreeListBox::ImplEditEntry()
    assert(aTabs.back()->nFlags & SvLBoxTabFlags::EDITABLE);
    if (!(mvTabList[0].nFlags & SvLBoxTabFlags::EDITABLE))
    {
        aTabs.back()->nFlags &= ~SvLBoxTabFlags::EDITABLE;
    }

    // append all other tabs to the list
    for( sal_uInt16 nCurTab = 1; nCurTab < sal_uInt16(mvTabList.size()); nCurTab++ )
    {
        SvLBoxTab& rTab = mvTabList[nCurTab];
        AddTab( rTab.GetPos(), rTab.nFlags );
    }
}

void SvTabListBox::InitEntry(SvTreeListEntry* pEntry, const OUString& rStr,
    const Image& rColl, const Image& rExp)
{
    SvTreeListBox::InitEntry(pEntry, rStr, rColl, rExp);

    sal_Int32 nIndex = 0;
    // TODO: verify if nTabCount is always >0 here!
    const sal_uInt16 nCount = mvTabList.size() - 1;
    for( sal_uInt16 nToken = 0; nToken < nCount; nToken++ )
    {
        const std::u16string_view aToken = GetToken(aCurEntry, nIndex);
        pEntry->AddItem(std::make_unique<SvLBoxString>(OUString(aToken)));
    }
}

SvTabListBox::SvTabListBox( vcl::Window* pParent, WinBits nBits )
    : SvTreeListBox( pParent, nBits )
{
    SetHighlightRange();    // select full width
}

SvTabListBox::~SvTabListBox()
{
    disposeOnce();
}

void SvTabListBox::dispose()
{
    mvTabList.clear();
    SvTreeListBox::dispose();
}

void SvTabListBox::SetTabs(sal_uInt16 nTabs, tools::Long const pTabPositions[], MapUnit eMapUnit)
{
    assert(0 < nTabs);
    mvTabList.resize(nTabs);

    MapMode aMMSource( eMapUnit );
    MapMode aMMDest( MapUnit::MapPixel );

    for( sal_uInt16 nIdx = 0; nIdx < sal_uInt16(mvTabList.size()); nIdx++, pTabPositions++ )
    {
        Size aSize( *pTabPositions, 0 );
        aSize = LogicToLogic( aSize, &aMMSource, &aMMDest );
        tools::Long nNewTab = aSize.Width();
        mvTabList[nIdx].SetPos( nNewTab );
        mvTabList[nIdx].nFlags &= MYTABMASK;
    }
    // by default, 1st one is editable, others not; override with set_column_editables
    mvTabList[0].nFlags |= SvLBoxTabFlags::EDITABLE;
    SvTreeListBox::nTreeFlags |= SvTreeFlags::RECALCTABS;
    if( IsUpdateMode() )
        Invalidate();
}

SvTreeListEntry* SvTabListBox::InsertEntry( const OUString& rText, SvTreeListEntry* pParent,
                                        bool /*bChildrenOnDemand*/,
                                        sal_uInt32 nPos, void* pUserData )
{
    return InsertEntryToColumn( rText, pParent, nPos, 0xffff, pUserData );
}

SvTreeListEntry* SvTabListBox::InsertEntryToColumn(const OUString& rStr,SvTreeListEntry* pParent,sal_uInt32 nPos,sal_uInt16 nCol,
    void* pUser )
{
    OUString aStr;
    if( nCol != 0xffff )
    {
        while( nCol )
        {
            aStr += "\t";
            nCol--;
        }
    }
    aStr += rStr;
    OUString aFirstStr( aStr );
    sal_Int32 nEnd = aFirstStr.indexOf( '\t' );
    if( nEnd != -1 )
    {
        aFirstStr = aFirstStr.copy(0, nEnd);
        aCurEntry = aStr.copy(++nEnd);
    }
    else
        aCurEntry.clear();
    return SvTreeListBox::InsertEntry( aFirstStr, pParent, false, nPos, pUser );
}

OUString SvTabListBox::GetEntryText( SvTreeListEntry* pEntry ) const
{
    return GetEntryText( pEntry, 0xffff );
}

OUString SvTabListBox::GetEntryText( const SvTreeListEntry* pEntry, sal_uInt16 nCol )
{
    DBG_ASSERT(pEntry,"GetEntryText:Invalid Entry");
    OUStringBuffer aResult;
    if( pEntry )
    {
        sal_uInt16 nCount = pEntry->ItemCount();
        sal_uInt16 nCur = 0;
        while( nCur < nCount )
        {
            const SvLBoxItem& rStr = pEntry->GetItem( nCur );
            if (rStr.GetType() == SvLBoxItemType::String)
            {
                if( nCol == 0xffff )
                {
                    if (!aResult.isEmpty())
                        aResult.append("\t");
                    aResult.append(static_cast<const SvLBoxString&>(rStr).GetText());
                }
                else
                {
                    if( nCol == 0 )
                        return static_cast<const SvLBoxString&>(rStr).GetText();
                    nCol--;
                }
            }
            nCur++;
        }
    }
    return aResult.makeStringAndClear();
}

OUString SvTabListBox::GetEntryText( sal_uInt32 nPos, sal_uInt16 nCol ) const
{
    SvTreeListEntry* pEntry = GetEntryOnPos( nPos );
    return GetEntryText( pEntry, nCol );
}

OUString SvTabListBox::GetCellText( sal_uInt32 nPos, sal_uInt16 nCol ) const
{
    SvTreeListEntry* pEntry = GetEntryOnPos( nPos );
    DBG_ASSERT( pEntry, "SvTabListBox::GetCellText(): Invalid Entry" );
    OUString aResult;
    if (pEntry && pEntry->ItemCount() > o3tl::make_unsigned(nCol+1))
    {
        const SvLBoxItem& rStr = pEntry->GetItem( nCol + 1 );
        if (rStr.GetType() == SvLBoxItemType::String)
            aResult = static_cast<const SvLBoxString&>(rStr).GetText();
    }
    return aResult;
}

sal_uInt32 SvTabListBox::GetEntryPos( const SvTreeListEntry* pEntry ) const
{
    sal_uInt32 nPos = 0;
    SvTreeListEntry* pTmpEntry = First();
    while( pTmpEntry )
    {
        if ( pTmpEntry == pEntry )
            return nPos;
        pTmpEntry = Next( pTmpEntry );
        ++nPos;
    }
    return 0xffffffff;
}

// static
std::u16string_view SvTabListBox::GetToken( std::u16string_view sStr, sal_Int32& nIndex )
{
    return o3tl::getToken(sStr, 0, '\t', nIndex);
}

OUString SvTabListBox::GetTabEntryText( sal_uInt32 nPos, sal_uInt16 nCol ) const
{
    SvTreeListEntry* pEntry = GetEntryOnPos( nPos );
    DBG_ASSERT( pEntry, "GetTabEntryText(): Invalid entry " );
    OUStringBuffer aResult;
    if ( pEntry )
    {
        sal_uInt16 nCount = pEntry->ItemCount();
        sal_uInt16 nCur = 0;
        while( nCur < nCount )
        {
            const SvLBoxItem& rBoxItem = pEntry->GetItem( nCur );
            if (rBoxItem.GetType() == SvLBoxItemType::String)
            {
                if ( nCol == 0xffff )
                {
                    if (!aResult.isEmpty())
                        aResult.append("\t");
                    aResult.append(static_cast<const SvLBoxString&>(rBoxItem).GetText());
                }
                else
                {
                    if ( nCol == 0 )
                    {
                        OUString sRet = static_cast<const SvLBoxString&>(rBoxItem).GetText();
                        if ( sRet.isEmpty() )
                            sRet = VclResId( STR_SVT_ACC_EMPTY_FIELD );
                        return sRet;
                    }
                    --nCol;
                }
            }
            ++nCur;
        }
    }
    return aResult.makeStringAndClear();
}

SvTreeListEntry* SvTabListBox::GetEntryOnPos( sal_uInt32 _nEntryPos ) const
{
    SvTreeListEntry* pEntry = nullptr;
    sal_uInt32 i, nPos = 0, nCount = GetLevelChildCount( nullptr );
    for ( i = 0; i < nCount; ++i )
    {
        SvTreeListEntry* pParent = GetEntry(i);
        if ( nPos == _nEntryPos )
        {
            pEntry = pParent;
            break;
        }
        else
        {
            nPos++;
            pEntry = GetChildOnPos( pParent, _nEntryPos, nPos );
            if ( pEntry )
                break;
        }
    }

    return pEntry;
}

SvTreeListEntry* SvTabListBox::GetChildOnPos( SvTreeListEntry* _pParent, sal_uInt32 _nEntryPos, sal_uInt32& _rPos ) const
{
    sal_uInt32 i, nCount = GetLevelChildCount( _pParent );
    for ( i = 0; i < nCount; ++i )
    {
        SvTreeListEntry* pParent = GetEntry( _pParent, i );
        if ( _rPos == _nEntryPos )
            return pParent;
        else
        {
            _rPos++;
            SvTreeListEntry* pEntry = GetChildOnPos( pParent, _nEntryPos, _rPos );
            if ( pEntry )
                return pEntry;
        }
    }

    return nullptr;
}

void SvTabListBox::SetTabJustify( sal_uInt16 nTab, SvTabJustify eJustify)
{
    DBG_ASSERT(nTab<mvTabList.size(),"GetTabPos:Invalid Tab");
    if( nTab >= mvTabList.size() )
        return;
    SvLBoxTab& rTab = mvTabList[ nTab ];
    SvLBoxTabFlags nFlags = rTab.nFlags;
    nFlags &= ~MYTABMASK;
    // see SvLBoxTab::CalcOffset for force, which only matters for centering
    nFlags |= static_cast<SvLBoxTabFlags>(eJustify) | SvLBoxTabFlags::FORCE;
    rTab.nFlags = nFlags;
    SvTreeListBox::nTreeFlags |= SvTreeFlags::RECALCTABS;
    if( IsUpdateMode() )
        Invalidate();
}

void SvTabListBox::SetTabEditable(sal_uInt16 nTab, bool bEditable)
{
    DBG_ASSERT(nTab<mvTabList.size(),"GetTabPos:Invalid Tab");
    if( nTab >= mvTabList.size() )
        return;
    SvLBoxTab& rTab = mvTabList[ nTab ];
    if (bEditable)
        rTab.nFlags |= SvLBoxTabFlags::EDITABLE;
    else
        rTab.nFlags &= ~SvLBoxTabFlags::EDITABLE;
}

tools::Long SvTabListBox::GetLogicTab( sal_uInt16 nTab )
{
    if( SvTreeListBox::nTreeFlags & SvTreeFlags::RECALCTABS )
        SetTabs();

    DBG_ASSERT(nTab<mvTabList.size(),"GetTabPos:Invalid Tab");
    return aTabs[ nTab ]->GetPos();
}

namespace vcl
{
    struct SvHeaderTabListBoxImpl
    {
        VclPtr<HeaderBar>       m_pHeaderBar;
        AccessibleFactoryAccess m_aFactoryAccess;

        SvHeaderTabListBoxImpl() : m_pHeaderBar( nullptr ) { }
    };
}

SvHeaderTabListBox::SvHeaderTabListBox( vcl::Window* pParent, WinBits nWinStyle )
    : SvTabListBox(pParent, nWinStyle)
    , m_bFirstPaint(true)
    , m_pImpl(new ::vcl::SvHeaderTabListBoxImpl)
    , m_pAccessible(nullptr)
{
}

SvHeaderTabListBox::~SvHeaderTabListBox()
{
    disposeOnce();
}

void SvHeaderTabListBox::dispose()
{
    for (css::uno::Reference<css::accessibility::XAccessible>& rxChild : m_aAccessibleChildren)
        comphelper::disposeComponent(rxChild);
    m_aAccessibleChildren.clear();

    m_pImpl.reset();
    SvTabListBox::dispose();
}

void SvHeaderTabListBox::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect )
{
    if (m_bFirstPaint)
    {
        m_bFirstPaint = false;
    }
    SvTabListBox::Paint(rRenderContext, rRect);
}

void SvHeaderTabListBox::InitHeaderBar( HeaderBar* pHeaderBar )
{
    DBG_ASSERT( !m_pImpl->m_pHeaderBar, "header bar already initialized" );
    DBG_ASSERT( pHeaderBar, "invalid header bar initialization" );
    m_pImpl->m_pHeaderBar = pHeaderBar;
    SetScrolledHdl( LINK( this, SvHeaderTabListBox, ScrollHdl_Impl ) );
    m_pImpl->m_pHeaderBar->SetCreateAccessibleHdl( LINK( this, SvHeaderTabListBox, CreateAccessibleHdl_Impl ) );
}

HeaderBar* SvHeaderTabListBox::GetHeaderBar()
{
    return m_pImpl ? m_pImpl->m_pHeaderBar : nullptr;
}

bool SvHeaderTabListBox::IsItemChecked( SvTreeListEntry* pEntry, sal_uInt16 nCol )
{
    SvButtonState eState = SvButtonState::Unchecked;
    SvLBoxButton& rItem = static_cast<SvLBoxButton&>( pEntry->GetItem( nCol + 1 ) );

    if (rItem.GetType() == SvLBoxItemType::Button)
    {
        SvItemStateFlags nButtonFlags = rItem.GetButtonFlags();
        eState = SvLBoxButtonData::ConvertToButtonState( nButtonFlags );
    }

    return ( eState == SvButtonState::Checked );
}

SvTreeListEntry* SvHeaderTabListBox::InsertEntryToColumn(
    const OUString& rStr, SvTreeListEntry* pParent, sal_uInt32 nPos, sal_uInt16 nCol, void* pUserData )
{
    SvTreeListEntry* pEntry = SvTabListBox::InsertEntryToColumn( rStr, pParent, nPos, nCol, pUserData );
    RecalculateAccessibleChildren();
    return pEntry;
}

sal_uInt32 SvHeaderTabListBox::Insert(
    SvTreeListEntry* pEnt, SvTreeListEntry* pPar, sal_uInt32 nPos )
{
    sal_uInt32 n = SvTabListBox::Insert( pEnt, pPar, nPos );
    RecalculateAccessibleChildren();
    return n;
}

sal_uInt32 SvHeaderTabListBox::Insert( SvTreeListEntry* pEntry, sal_uInt32 nRootPos )
{
    sal_uInt32 nPos = SvTabListBox::Insert( pEntry, nRootPos );
    RecalculateAccessibleChildren();
    return nPos;
}

void SvHeaderTabListBox::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
{
    SvTabListBox::DumpAsPropertyTree(rJsonWriter);

    auto aHeaders = rJsonWriter.startArray("headers");

    HeaderBar* pHeaderBar = GetHeaderBar();
    for(sal_uInt16 i = 0; i < pHeaderBar->GetItemCount(); i++)
    {
        auto aNode = rJsonWriter.startStruct();
        rJsonWriter.put("text", pHeaderBar->GetItemText(pHeaderBar->GetItemId(i)));
    }
}

IMPL_LINK_NOARG(SvHeaderTabListBox, ScrollHdl_Impl, SvTreeListBox*, void)
{
    m_pImpl->m_pHeaderBar->SetOffset( -GetXOffset() );
}

IMPL_LINK_NOARG(SvHeaderTabListBox, CreateAccessibleHdl_Impl, HeaderBar*, void)
{
    vcl::Window* pParent = m_pImpl->m_pHeaderBar->GetAccessibleParentWindow();
    DBG_ASSERT( pParent, "SvHeaderTabListBox..CreateAccessibleHdl_Impl - accessible parent not found" );
    if ( pParent )
    {
        css::uno::Reference< XAccessible > xAccParent = pParent->GetAccessible();
        if ( xAccParent.is() )
        {
            Reference< XAccessible > xAccessible = m_pImpl->m_aFactoryAccess.getFactory().createAccessibleBrowseBoxHeaderBar(
                xAccParent, *this, AccessibleBrowseBoxObjType::ColumnHeaderBar );
            m_pImpl->m_pHeaderBar->SetAccessible( xAccessible );
        }
    }
}

void SvHeaderTabListBox::RecalculateAccessibleChildren()
{
    if ( !m_aAccessibleChildren.empty() )
    {
        sal_uInt32 nCount = ( GetRowCount() + 1 ) * GetColumnCount();
        if ( m_aAccessibleChildren.size() < nCount )
            m_aAccessibleChildren.resize( nCount );
        else
        {
            DBG_ASSERT( m_aAccessibleChildren.size() == nCount, "wrong children count" );
        }
    }
}

bool SvHeaderTabListBox::IsCellCheckBox( sal_Int32 _nRow, sal_uInt16 _nColumn, TriState& _rState ) const
{
    bool bRet = false;
    SvTreeListEntry* pEntry = GetEntryOnPos( _nRow );
    if ( pEntry )
    {
        sal_uInt16 nItemCount = pEntry->ItemCount();
        if ( nItemCount > ( _nColumn + 1 ) )
        {
            SvLBoxItem& rItem = pEntry->GetItem( _nColumn + 1 );
            if (rItem.GetType() == SvLBoxItemType::Button)
            {
                bRet = true;
                _rState = ( ( static_cast<SvLBoxButton&>(rItem).GetButtonFlags() & SvItemStateFlags::UNCHECKED ) == SvItemStateFlags::NONE )
                            ? TRISTATE_TRUE : TRISTATE_FALSE;
            }
        }
        else
        {
            SAL_WARN( "svtools.contnr", "SvHeaderTabListBox::IsCellCheckBox(): column out of range" );
        }
    }
    return bRet;
}
sal_Int32 SvHeaderTabListBox::GetRowCount() const
{
    return GetEntryCount();
}

sal_uInt16 SvHeaderTabListBox::GetColumnCount() const
{
    return m_pImpl->m_pHeaderBar->GetItemCount();
}

sal_Int32 SvHeaderTabListBox::GetCurrRow() const
{
    sal_Int32 nRet = -1;
    SvTreeListEntry* pEntry = GetCurEntry();
    if ( pEntry )
    {
        sal_uInt32 nCount = GetEntryCount();
        for ( sal_uInt32 i = 0; i < nCount; ++i )
        {
            if ( pEntry == GetEntryOnPos(i) )
            {
                nRet = i;
                break;
            }
        }
    }

    return nRet;
}

sal_uInt16 SvHeaderTabListBox::GetCurrColumn() const
{
    return 0;
}

OUString SvHeaderTabListBox::GetRowDescription( sal_Int32 _nRow ) const
{
    return GetEntryText( _nRow );
}

OUString SvHeaderTabListBox::GetColumnDescription( sal_uInt16 _nColumn ) const
{
    return m_pImpl->m_pHeaderBar->GetItemText( m_pImpl->m_pHeaderBar->GetItemId( _nColumn ) );
}

bool SvHeaderTabListBox::HasRowHeader() const
{
    return false;
}

bool SvHeaderTabListBox::GoToCell( sal_Int32 /*_nRow*/, sal_uInt16 /*_nColumn*/ )
{
    return false;
}

void SvHeaderTabListBox::SetNoSelection()
{
    SvTreeListBox::SelectAll(false);
}

void SvHeaderTabListBox::SelectAll()
{
    SvTreeListBox::SelectAll(true);
}

void SvHeaderTabListBox::SelectRow( sal_Int32 _nRow, bool _bSelect, bool )
{
    Select( GetEntryOnPos( _nRow ), _bSelect );
}

void SvHeaderTabListBox::SelectColumn( sal_uInt16, bool )
{
}

sal_Int32 SvHeaderTabListBox::GetSelectedRowCount() const
{
    return GetSelectionCount();
}

sal_Int32 SvHeaderTabListBox::GetSelectedColumnCount() const
{
    return 0;
}

bool SvHeaderTabListBox::IsRowSelected( sal_Int32 _nRow ) const
{
    SvTreeListEntry* pEntry = GetEntryOnPos( _nRow );
    return ( pEntry && IsSelected( pEntry ) );
}

bool SvHeaderTabListBox::IsColumnSelected( sal_Int32 ) const
{
    return false;
}

void SvHeaderTabListBox::GetAllSelectedRows(css::uno::Sequence<sal_Int32 >& rRowIndices) const
{
    const sal_Int32 nCount = GetSelectedRowCount();
    rRowIndices.realloc(nCount);
    auto pRows = rRowIndices.getArray();
    SvTreeListEntry* pEntry = FirstSelected();
    sal_Int32 nIndex = 0;
    while (nIndex < nCount && pEntry)
    {
        pRows[nIndex] = GetEntryPos(pEntry);
        pEntry = NextSelected( pEntry );
        ++nIndex;
    }
    assert(nIndex == nCount && "Mismatch between GetSelectedRowCount() and count of selected rows when iterating.");
}

void SvHeaderTabListBox::GetAllSelectedColumns( css::uno::Sequence< sal_Int32 >& ) const
{
}

bool SvHeaderTabListBox::IsCellVisible( sal_Int32, sal_uInt16 ) const
{
    return true;
}

OUString SvHeaderTabListBox::GetAccessibleCellText( sal_Int32 _nRow, sal_uInt16 _nColumnPos ) const
{
    return GetTabEntryText(_nRow, _nColumnPos);
}

tools::Rectangle SvHeaderTabListBox::calcHeaderRect( bool _bIsColumnBar, bool _bOnScreen )
{
    tools::Rectangle aRect;
    if ( _bIsColumnBar )
    {
        vcl::Window* pParent = nullptr;
        if (_bOnScreen)
            aRect = tools::Rectangle(m_pImpl->m_pHeaderBar->GetWindowExtentsAbsolute());
        else
        {
            pParent = m_pImpl->m_pHeaderBar->GetAccessibleParentWindow();
            assert(pParent);
            aRect = m_pImpl->m_pHeaderBar->GetWindowExtentsRelative(*pParent );
        }
    }
    return aRect;
}

tools::Rectangle SvHeaderTabListBox::calcTableRect( bool _bOnScreen )
{
    if ( _bOnScreen )
        return tools::Rectangle(GetWindowExtentsAbsolute());
    else
        return GetWindowExtentsRelative( *GetAccessibleParentWindow() );
}

tools::Rectangle SvHeaderTabListBox::GetFieldRectPixel( sal_Int32 _nRow, sal_uInt16 _nColumn, bool _bIsHeader, bool _bOnScreen )
{
    DBG_ASSERT( !_bIsHeader || 0 == _nRow, "invalid parameters" );
    tools::Rectangle aRect;
    SvTreeListEntry* pEntry = GetEntryOnPos(_nRow );
    if ( pEntry )
    {
        aRect = _bIsHeader ? calcHeaderRect( true, false ) : GetBoundingRect( pEntry );
        Point aTopLeft = aRect.TopLeft();
        DBG_ASSERT( m_pImpl->m_pHeaderBar->GetItemCount() > _nColumn, "invalid column" );
        tools::Rectangle aItemRect = m_pImpl->m_pHeaderBar->GetItemRect( m_pImpl->m_pHeaderBar->GetItemId( _nColumn ) );
        aTopLeft.setX( aItemRect.Left() );
        Size aSize = aItemRect.GetSize();
        aRect = tools::Rectangle( aTopLeft, aSize );
        aTopLeft = aRect.TopLeft();
        if (_bOnScreen)
            aTopLeft += Point(GetWindowExtentsAbsolute().TopLeft());
        else
            aTopLeft += GetWindowExtentsRelative( *GetAccessibleParentWindow() ).TopLeft();
        aRect = tools::Rectangle( aTopLeft, aRect.GetSize() );
    }

    return aRect;
}

Reference< XAccessible > SvHeaderTabListBox::CreateAccessibleCell( sal_Int32 _nRow, sal_uInt16 _nColumnPos )
{
    OSL_ENSURE( m_pAccessible, "Invalid call: Accessible is null" );

    Reference< XAccessible > xChild;

    TriState eState = TRISTATE_INDET;
    bool bIsCheckBox = IsCellCheckBox( _nRow, _nColumnPos, eState );
    if ( bIsCheckBox )
        xChild = m_pImpl->m_aFactoryAccess.getFactory().createAccessibleCheckBoxCell(
                m_pAccessible->getTable(), *this, nullptr, _nRow, _nColumnPos, eState, false );
    else
        xChild = m_pImpl->m_aFactoryAccess.getFactory().createAccessibleBrowseBoxTableCell(
                m_pAccessible->getTable(), *this, nullptr, _nRow, _nColumnPos, OFFSET_NONE );

    return xChild;
}

Reference< XAccessible > SvHeaderTabListBox::CreateAccessibleRowHeader( sal_Int32 )
{
    Reference< XAccessible > xHeader;
    return xHeader;
}

Reference< XAccessible > SvHeaderTabListBox::CreateAccessibleColumnHeader( sal_uInt16 _nColumn )
{
    // first call? -> initial list
    if ( m_aAccessibleChildren.empty() )
    {
        const sal_uInt16 nColumnCount = GetColumnCount();
        m_aAccessibleChildren.assign( nColumnCount, Reference< XAccessible >() );
    }

    // get header
    Reference< XAccessible > xChild = m_aAccessibleChildren[ _nColumn ];
    // already exists?
    if ( !xChild.is() && m_pAccessible )
    {
        // no -> create new header cell
        xChild = m_pImpl->m_aFactoryAccess.getFactory().createAccessibleBrowseBoxHeaderCell(
            _nColumn, m_pAccessible->getHeaderBar(),
            *this, nullptr, AccessibleBrowseBoxObjType::ColumnHeaderCell
        );

        // insert into list
        m_aAccessibleChildren[ _nColumn ] = xChild;
    }
    return xChild;
}

sal_Int32 SvHeaderTabListBox::GetAccessibleControlCount() const
{
    return -1;
}

Reference< XAccessible > SvHeaderTabListBox::CreateAccessibleControl( sal_Int32 )
{
    Reference< XAccessible > xControl;
    return xControl;
}

bool SvHeaderTabListBox::ConvertPointToControlIndex( sal_Int32&, const Point& )
{
    return false;
}

bool SvHeaderTabListBox::ConvertPointToCellAddress( sal_Int32&, sal_uInt16&, const Point& )
{
    return false;
}

bool SvHeaderTabListBox::ConvertPointToRowHeader( sal_Int32&, const Point& )
{
    return false;
}

bool SvHeaderTabListBox::ConvertPointToColumnHeader( sal_uInt16&, const Point& )
{
    return false;
}

OUString SvHeaderTabListBox::GetAccessibleObjectName( AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos ) const
{
    OUString aRetText;
    switch( _eType )
    {
        case AccessibleBrowseBoxObjType::BrowseBox:
        case AccessibleBrowseBoxObjType::Table:
        case AccessibleBrowseBoxObjType::ColumnHeaderBar:
            // should be empty now (see #i63983)
            aRetText.clear();
            break;

        case AccessibleBrowseBoxObjType::TableCell:
        {
            // here we need a valid pos, we can not handle -1
            if ( _nPos >= 0 )
            {
                sal_uInt16 nColumnCount = GetColumnCount();
                if (nColumnCount > 0)
                {
                    sal_Int32 nRow = _nPos / nColumnCount;
                    sal_uInt16 nColumn  = static_cast< sal_uInt16 >( _nPos % nColumnCount );
                    aRetText = GetCellText( nRow, nColumn );
                }
            }
            break;
        }
        case AccessibleBrowseBoxObjType::CheckBoxCell:
        {
            break; // checkbox cells have no name
        }
        case AccessibleBrowseBoxObjType::ColumnHeaderCell:
        {
            aRetText = m_pImpl->m_pHeaderBar->GetItemText( m_pImpl->m_pHeaderBar->GetItemId( static_cast<sal_uInt16>(_nPos) ) );
            break;
        }

        case AccessibleBrowseBoxObjType::RowHeaderBar:
        case AccessibleBrowseBoxObjType::RowHeaderCell:
            aRetText = "error";
            break;

        default:
            OSL_FAIL("BrowseBox::GetAccessibleName: invalid enum!");
    }
    return aRetText;
}

OUString SvHeaderTabListBox::GetAccessibleObjectDescription( AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos ) const
{
    OUString aRetText;

    if( _eType == AccessibleBrowseBoxObjType::TableCell && _nPos != -1 )
    {
        sal_uInt16 nColumnCount = GetColumnCount();
        if (nColumnCount > 0)
        {
            sal_Int32 nRow = _nPos / nColumnCount;
            sal_uInt16 nColumn  = static_cast< sal_uInt16 >( _nPos % nColumnCount );

            OUString aText( VclResId(STR_SVT_ACC_DESC_TABLISTBOX) );
            aText = aText.replaceFirst( "%1", OUString::number( nRow ) );
            OUString sColHeader = m_pImpl->m_pHeaderBar->GetItemText( m_pImpl->m_pHeaderBar->GetItemId( nColumn ) );
            if ( sColHeader.isEmpty() )
                sColHeader = OUString::number( nColumn );
            aText = aText.replaceFirst( "%2", sColHeader );
            aRetText = aText;
        }
    }

    return aRetText;
}

void SvHeaderTabListBox::FillAccessibleStateSet( sal_Int64& _rStateSet, AccessibleBrowseBoxObjType _eType ) const
{
    switch( _eType )
    {
        case AccessibleBrowseBoxObjType::BrowseBox:
        case AccessibleBrowseBoxObjType::Table:
        {
            _rStateSet |= AccessibleStateType::FOCUSABLE;
            if ( HasFocus() )
                _rStateSet |= AccessibleStateType::FOCUSED;
            if ( IsActive() )
                _rStateSet |= AccessibleStateType::ACTIVE;
            if ( IsEnabled() )
            {
                _rStateSet |= AccessibleStateType::ENABLED;
                _rStateSet |= AccessibleStateType::SENSITIVE;
            }
            if ( IsReallyVisible() )
                _rStateSet |= AccessibleStateType::VISIBLE;
            if ( _eType == AccessibleBrowseBoxObjType::Table )
            {

                _rStateSet |= AccessibleStateType::MANAGES_DESCENDANTS;
                _rStateSet |= AccessibleStateType::MULTI_SELECTABLE;
            }
            break;
        }

        case AccessibleBrowseBoxObjType::ColumnHeaderBar:
        {
            sal_Int32 nCurRow = GetCurrRow();
            sal_uInt16 nCurColumn = GetCurrColumn();
            if ( IsCellVisible( nCurRow, nCurColumn ) )
                _rStateSet |= AccessibleStateType::VISIBLE;
            if ( IsEnabled() )
                _rStateSet |= AccessibleStateType::ENABLED;
            _rStateSet |= AccessibleStateType::TRANSIENT;
            break;
        }

        case AccessibleBrowseBoxObjType::RowHeaderCell:
        case AccessibleBrowseBoxObjType::ColumnHeaderCell:
        {
            _rStateSet |= AccessibleStateType::VISIBLE;
            _rStateSet |= AccessibleStateType::FOCUSABLE;
            _rStateSet |= AccessibleStateType::TRANSIENT;
            if ( IsEnabled() )
                _rStateSet |= AccessibleStateType::ENABLED;
            break;
        }
        default:
            break;
    }
}

void SvHeaderTabListBox::FillAccessibleStateSetForCell( sal_Int64& _rStateSet, sal_Int32 _nRow, sal_uInt16 _nColumn ) const
{
    _rStateSet |= AccessibleStateType::FOCUSABLE;
    _rStateSet |= AccessibleStateType::SELECTABLE;
    _rStateSet |= AccessibleStateType::TRANSIENT;

    if ( IsCellVisible( _nRow, _nColumn ) )
    {
        _rStateSet |= AccessibleStateType::VISIBLE;
        _rStateSet |= AccessibleStateType::ENABLED;
    }

    if ( IsRowSelected( _nRow ) )
    {
        _rStateSet |= AccessibleStateType::ACTIVE;
        if (HasChildPathFocus())
            _rStateSet |= AccessibleStateType::FOCUSED;
        _rStateSet |= AccessibleStateType::SELECTED;
    }
    if ( IsEnabled() )
        _rStateSet |= AccessibleStateType::ENABLED;
}

void SvHeaderTabListBox::GrabTableFocus()
{
    GrabFocus();
}

bool SvHeaderTabListBox::GetGlyphBoundRects( const Point& rOrigin, const OUString& rStr, int nIndex, int nLen, std::vector< tools::Rectangle >& rVector )
{
    return GetOutDev()->GetGlyphBoundRects( rOrigin, rStr, nIndex, nLen, rVector );
}

AbsoluteScreenPixelRectangle SvHeaderTabListBox::GetWindowExtentsAbsolute() const
{
    return Control::GetWindowExtentsAbsolute();
}

tools::Rectangle SvHeaderTabListBox::GetWindowExtentsRelative(const vcl::Window& rRelativeWindow) const
{
    return Control::GetWindowExtentsRelative( rRelativeWindow );
}

void SvHeaderTabListBox::GrabFocus()
{
    Control::GrabFocus();
}

Reference< XAccessible > SvHeaderTabListBox::GetAccessible()
{
    return Control::GetAccessible();
}

vcl::Window* SvHeaderTabListBox::GetAccessibleParentWindow() const
{
    return Control::GetAccessibleParentWindow();
}

vcl::Window* SvHeaderTabListBox::GetWindowInstance()
{
    return this;
}

Reference< XAccessible > SvHeaderTabListBox::CreateAccessible()
{
    vcl::Window* pParent = GetAccessibleParentWindow();
    DBG_ASSERT( pParent, "SvHeaderTabListBox::::CreateAccessible - accessible parent not found" );

    Reference< XAccessible > xAccessible;
    if ( m_pAccessible ) xAccessible = m_pAccessible->getMyself();

    if( pParent && !m_pAccessible )
    {
        Reference< XAccessible > xAccParent = pParent->GetAccessible();
        if ( xAccParent.is() )
        {
            m_pAccessible = m_pImpl->m_aFactoryAccess.getFactory().createAccessibleTabListBox( xAccParent, *this );
            if ( m_pAccessible )
                xAccessible = m_pAccessible->getMyself();
        }
    }
    return xAccessible;
}

tools::Rectangle SvHeaderTabListBox::GetFieldCharacterBounds(sal_Int32,sal_Int32,sal_Int32)
{
    return tools::Rectangle();
}

sal_Int32 SvHeaderTabListBox::GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint)
{
    OUString sText = GetAccessibleCellText( _nRow, static_cast< sal_uInt16 >( _nColumnPos ) );
    std::vector< tools::Rectangle > aRects;
    if ( GetGlyphBoundRects(Point(0,0), sText, 0, sText.getLength(), aRects) )
    {
        sal_Int32 nPos = 0;
        for (auto const& rectangle : aRects)
        {
            if( rectangle.Contains(_rPoint) )
                return nPos;
            ++nPos;
        }
    }

    return -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
