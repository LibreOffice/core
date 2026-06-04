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

#include <SvHeaderTabListBox.hxx>
#include <accessibility/accessibletablistbox.hxx>

#include <comphelper/types.hxx>
#include <vcl/headbar.hxx>
#include <vcl/toolkit/svlbitm.hxx>
#include <vcl/toolkit/treelistbox.hxx>
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

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

// SvTreeListBox callback

void SvTabListBox::SetTabs()
{
    SvTreeListBox::SetTabs();
    if( mvTabList.empty() )
        return;

    // The tree listbox has now inserted its tabs into the list. Now we
    // fluff up the list with additional tabs and adjust the rightmost tab
    // of the tree listbox.

    // the 1st column (index 1 or 2 depending on button flags) is always set
    // editable by SvTreeListBox::SetTabs(),
    // which prevents setting a different column to editable as the first
    // one with the flag is picked in SvTreeListBox::ImplEditEntry()
    assert(m_aTabs.back()->nFlags & SvLBoxTabFlags::EDITABLE);
    if (!(mvTabList[0].nFlags & SvLBoxTabFlags::EDITABLE))
    {
        m_aTabs.back()->nFlags &= ~SvLBoxTabFlags::EDITABLE;
    }

    // append all other tabs to the list
    for( sal_uInt16 nCurTab = 1; nCurTab < sal_uInt16(mvTabList.size()); nCurTab++ )
    {
        SvLBoxTab& rTab = mvTabList[nCurTab];
        AddTab( rTab.GetPos(), rTab.nFlags );
    }
}

void SvTabListBox::InitEntry(SvTreeListEntry& rEntry, const OUString& rStr, const Image& rColl,
                             const Image& rExp)
{
    SvTreeListBox::InitEntry(rEntry, rStr, rColl, rExp);

    sal_Int32 nIndex = 0;
    // TODO: verify if nTabCount is always >0 here!
    const sal_uInt16 nCount = mvTabList.size() - 1;
    for( sal_uInt16 nToken = 0; nToken < nCount; nToken++ )
    {
        const std::u16string_view aToken = GetToken(aCurEntry, nIndex);
        rEntry.AddItem(std::make_unique<SvLBoxString>(OUString(aToken)));
    }
}

SvTabListBox::SvTabListBox( vcl::Window* pParent, WinBits nBits )
    : SvTreeListBox( pParent, nBits )
    , m_eRole(SvTabListBoxRole::Unknown)
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

void SvTabListBox::SetTabWidth(sal_uInt16 nTab, tools::Long tabWidth)
{
    // Ensure that mvTabList[nTab + 1] exists because it is required to calculate diff
    if (nTab + 2 > tools::Long(mvTabList.size()))
        mvTabList.resize(nTab + 2);

    tools::Long diff = tabWidth -
            (mvTabList[nTab + 1].GetPos() - mvTabList[nTab].GetPos()); // Width change

    // Shift all tab positions after nTab by diff
    for( sal_uInt16 nIdx = nTab + 1; nIdx < sal_uInt16(mvTabList.size()); nIdx++)
        mvTabList[nIdx].SetPos(mvTabList[nIdx].GetPos() + diff);
    SvTreeListBox::m_nTreeFlags |= SvTreeFlags::RECALCTABS;
    if( IsUpdateMode() )
        Invalidate();
}

void SvTabListBox::SetTabs(const std::vector<tools::Long>& rTabPositions)
{
    assert(!rTabPositions.empty());
    mvTabList.resize(rTabPositions.size());

    for( sal_uInt16 nIdx = 0; nIdx < sal_uInt16(mvTabList.size()); nIdx++)
        mvTabList[nIdx].SetPos(rTabPositions.at(nIdx));

    SvTreeListBox::m_nTreeFlags |= SvTreeFlags::RECALCTABS;
    if( IsUpdateMode() )
        Invalidate();
}

SvTreeListEntry* SvTabListBox::InsertEntry( const OUString& rText, SvTreeListEntry* pParent,
                                        bool /*bChildrenOnDemand*/,
                                        sal_uInt32 nPos, OUString* pUserData )
{
    return InsertEntryToColumn( rText, pParent, nPos, 0xffff, pUserData );
}

SvTreeListEntry* SvTabListBox::InsertEntryToColumn(const OUString& rStr,SvTreeListEntry* pParent,sal_uInt32 nPos,sal_uInt16 nCol,
    OUString* pUser )
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

void SvTabListBox::SetTabAlignCenter(sal_uInt16 nTab)
{
    DBG_ASSERT(nTab<mvTabList.size(),"GetTabPos:Invalid Tab");
    if( nTab >= mvTabList.size() )
        return;
    SvLBoxTab& rTab = mvTabList[ nTab ];
    SvLBoxTabFlags nFlags = rTab.nFlags;
    nFlags &= ~SvLBoxTabFlags::ADJUST_FLAGS;
    // see SvLBoxTab::CalcOffset for force, which only matters for centering
    nFlags |= SvLBoxTabFlags::ADJUST_CENTER | SvLBoxTabFlags::FORCE;
    rTab.nFlags = nFlags;
    SvTreeListBox::m_nTreeFlags |= SvTreeFlags::RECALCTABS;
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

void SvTabListBox::SetTabVisible(sal_uInt16 nTab, bool bVisible)
{
    DBG_ASSERT(nTab<mvTabList.size(),"SetTabVisible:Invalid Tab");
    if( nTab >= mvTabList.size() )
        return;

    if( SvTreeListBox::m_nTreeFlags & SvTreeFlags::RECALCTABS )
        SetTabs();

    // Find index in aTabs
    nTab += m_aTabs.size() - mvTabList.size();
    SvLBoxTab* pTab = m_aTabs[ nTab ].get();

    if (!bVisible)
        pTab->nFlags |= SvLBoxTabFlags::HIDDEN;
    else
        pTab->nFlags &= ~SvLBoxTabFlags::HIDDEN;
}

bool SvTabListBox::GetTabVisible(sal_uInt16 nTab)
{
    DBG_ASSERT(nTab<mvTabList.size(),"GetTabVisible:Invalid Tab");
    if( nTab >= mvTabList.size() )
        return true;

    if( SvTreeListBox::m_nTreeFlags & SvTreeFlags::RECALCTABS )
        SetTabs();

    nTab += m_aTabs.size() - mvTabList.size();
    SvLBoxTab* pTab = m_aTabs[ nTab ].get();
    return !pTab->IsHidden();
}

tools::Long SvTabListBox::GetLogicTab( sal_uInt16 nTab )
{
    if (SvTreeListBox::m_nTreeFlags & SvTreeFlags::RECALCTABS)
        SetTabs();

    DBG_ASSERT(nTab<mvTabList.size(),"GetTabPos:Invalid Tab");
    return m_aTabs[nTab]->GetPos();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
