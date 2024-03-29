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

#include <config_wasm_strip.h>

#include <vcl/event.hxx>
#include <vcl/virdev.hxx>

#include <svx/ucsubset.hxx>
#include <unordered_map>

#include <svx/searchcharmap.hxx>

#include <charmapacc.hxx>

#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;


SvxSearchCharSet::SvxSearchCharSet(std::unique_ptr<weld::ScrolledWindow> pScrolledWindow, const VclPtr<VirtualDevice>& rVirDev)
    : SvxShowCharSet(std::move(pScrolledWindow), rVirDev)
{
}

int SvxSearchCharSet::LastInView() const
{
    int nIndex = FirstInView();
    nIndex += ROW_COUNT * COLUMN_COUNT - 1;
    return std::min<int>(nIndex, getMaxCharCount() -1);
}

bool SvxSearchCharSet::KeyInput(const KeyEvent& rKEvt)
{
    vcl::KeyCode aCode = rKEvt.GetKeyCode();

    if (aCode.GetModifier())
        return false;

    int tmpSelected = nSelectedIndex;

    bool bRet = true;

    switch (aCode.GetCode())
    {
        case KEY_RETURN:
            return SvxShowCharSet::KeyInput(rKEvt);
        case KEY_SPACE:
            aDoubleClkHdl.Call(this);
            return true;
        case KEY_LEFT:
            --tmpSelected;
            break;
        case KEY_RIGHT:
            ++tmpSelected;
            break;
        case KEY_UP:
            tmpSelected -= COLUMN_COUNT;
            break;
        case KEY_DOWN:
            tmpSelected += COLUMN_COUNT;
            break;
        case KEY_PAGEUP:
            tmpSelected -= ROW_COUNT * COLUMN_COUNT;
            break;
        case KEY_PAGEDOWN:
            tmpSelected += ROW_COUNT * COLUMN_COUNT;
            break;
        case KEY_HOME:
            tmpSelected = 0;
            break;
        case KEY_END:
            tmpSelected = getMaxCharCount() - 1;
            break;
        case KEY_TAB:   // some fonts have a character at these unicode control codes
        case KEY_ESCAPE:
            bRet = false;
            tmpSelected = - 1;  // mark as invalid
            break;
        default:
            tmpSelected = -1;
            bRet = false;
            break;
    }

    if ( tmpSelected >= 0 )
    {
        SelectIndex( tmpSelected, true );
        aPreSelectHdl.Call( this );
    }

    return bRet;
}

void SvxSearchCharSet::SelectCharacter( const Subset* sub )
{
    if (!mxFontCharMap.is())
        RecalculateFont(*mxVirDev);

    // get next available char of current font
    sal_UCS4 cChar = sub->GetRangeMin();
    int nMapIndex = 0;

    while(cChar <= sub->GetRangeMax() && nMapIndex == 0)
    {
        auto it = std::find_if(m_aItemList.begin(), m_aItemList.end(),
            [&cChar](const std::pair<const sal_Int32, sal_UCS4>& rItem) { return rItem.second == cChar; });
        if (it != m_aItemList.end())
            nMapIndex = it->first;
        cChar++;
    }

    if(nMapIndex == 0)
        SelectIndex( 0 );
    else
        SelectIndex( nMapIndex );
    aHighHdl.Call(this);
    // move selected item to top row if not in focus
    //TO.DO aVscrollSB->SetThumbPos( nMapIndex / COLUMN_COUNT );
    Invalidate();
}

sal_UCS4 SvxSearchCharSet::GetCharFromIndex(int index) const
{
    std::unordered_map<sal_Int32, sal_UCS4>::const_iterator got = m_aItemList.find(index);
    return (got != m_aItemList.end()) ? got->second : 0;
}

void SvxSearchCharSet::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    InitSettings(rRenderContext);
    RecalculateFont(rRenderContext);
    DrawChars_Impl(rRenderContext, FirstInView(), LastInView());
}

sal_UCS4 SvxSearchCharSet::GetSelectCharacter() const
{
    if( nSelectedIndex >= 0 )
    {
        std::unordered_map<sal_Int32,sal_UCS4>::const_iterator got = m_aItemList.find (nSelectedIndex);

        if(got == m_aItemList.end())
            return 1;
        else
            return got->second;
    }
    return 1;
}

void SvxSearchCharSet::RecalculateFont(vcl::RenderContext& rRenderContext)
{
    if (!mbRecalculateFont)
        return;

    Size aSize(GetOutputSizePixel());

    vcl::Font aFont = rRenderContext.GetFont();
    aFont.SetWeight(WEIGHT_LIGHT);
    aFont.SetAlignment(ALIGN_TOP);
    int nFontHeight = (aSize.Height() - 5) * 2 / (3 * ROW_COUNT);
    maFontSize = rRenderContext.PixelToLogic(Size(0, nFontHeight));
    aFont.SetFontSize(maFontSize);
    aFont.SetTransparent(true);
    rRenderContext.SetFont(aFont);
    rRenderContext.GetFontCharMap(mxFontCharMap);
    m_aItems.clear();
    getFavCharacterList();

    nX = aSize.Width() / COLUMN_COUNT;
    nY = aSize.Height() / ROW_COUNT;

    UpdateScrollRange();

    // rearrange CharSet element in sync with nX- and nY-multiples
    Size aDrawSize(nX * COLUMN_COUNT, nY * ROW_COUNT);
    m_nXGap = (aSize.Width() - aDrawSize.Width()) / 2;
    m_nYGap = (aSize.Height() - aDrawSize.Height()) / 2;

    mbRecalculateFont = false;
}

void SvxSearchCharSet::UpdateScrollRange()
{
    //scrollbar settings
    int nLastRow = (getMaxCharCount() - 1 + COLUMN_COUNT) / COLUMN_COUNT;
    mxScrollArea->vadjustment_configure(mxScrollArea->vadjustment_get_value(), 0, nLastRow, 1, ROW_COUNT - 1, ROW_COUNT);
}

void SvxSearchCharSet::SelectIndex(int nNewIndex, bool bFocus)
{
    if (!mxFontCharMap.is())
        RecalculateFont(*mxVirDev);

    if( nNewIndex < 0 )
    {
        mxScrollArea->vadjustment_set_value(0);
        nSelectedIndex = bFocus ? 0 : -1;
        Invalidate();
    }
    else if( nNewIndex < FirstInView() )
    {
        // need to scroll up to see selected item
        int nOldPos = mxScrollArea->vadjustment_get_value();
        int nDelta = (FirstInView() - nNewIndex + COLUMN_COUNT-1) / COLUMN_COUNT;
        mxScrollArea->vadjustment_set_value(nOldPos - nDelta);
        nSelectedIndex = nNewIndex;
        Invalidate();
    }
    else if( nNewIndex > LastInView() )
    {
        // need to scroll down to see selected item
        int nOldPos = mxScrollArea->vadjustment_get_value();
        int nDelta = (nNewIndex - LastInView() + COLUMN_COUNT) / COLUMN_COUNT;
        mxScrollArea->vadjustment_set_value(nOldPos + nDelta);

        if (nNewIndex < getMaxCharCount())
        {
            nSelectedIndex = nNewIndex;
            Invalidate();
        }
        else if (nOldPos != mxScrollArea->vadjustment_get_value())
        {
            Invalidate();
        }
    }
    else
    {
        nSelectedIndex = nNewIndex;
        Invalidate();
    }

    if( nSelectedIndex >= 0 )
    {
#if 0
        if( m_xAccessible.is() )
        {
            svx::SvxShowCharSetItem* pItem = ImplGetItem(nSelectedIndex);
            // Don't fire the focus event.
            if ( bFocus )
                m_xAccessible->fireEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, Any(), makeAny(pItem->GetAccessible()) ); // this call assures that m_pItem is set
            else
                m_xAccessible->fireEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED_NOFOCUS, Any(), makeAny(pItem->GetAccessible()) ); // this call assures that m_pItem is set

            assert(pItem->m_xItem.is() && "No accessible created!");
            Any aOldAny, aNewAny;
            aNewAny <<= AccessibleStateType::FOCUSED;
            // Don't fire the focus event.
            if ( bFocus )
                pItem->m_xItem->fireEvent( AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny );

            aNewAny <<= AccessibleStateType::SELECTED;
            pItem->m_xItem->fireEvent( AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny );
        }
#endif
        aSelectHdl.Call(this);
    }
    aHighHdl.Call( this );
}

SvxSearchCharSet::~SvxSearchCharSet()
{
}

svx::SvxShowCharSetItem* SvxSearchCharSet::ImplGetItem( int _nPos )
{
    ItemsMap::iterator aFind = m_aItems.find(_nPos);
    if ( aFind == m_aItems.end() )
    {
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
        OSL_ENSURE(m_xAccessible.is(), "Who wants to create a child of my table without a parent?");
#endif
        auto xItem = std::make_shared<svx::SvxShowCharSetItem>(*this,
            m_xAccessible.get(), sal::static_int_cast< sal_uInt16 >(_nPos));
        aFind = m_aItems.emplace(_nPos, xItem).first;
        OUStringBuffer buf;
        std::unordered_map<sal_Int32,sal_UCS4>::const_iterator got = m_aItemList.find (_nPos);
        if (got != m_aItemList.end())
            buf.appendUtf32(got->second);
        aFind->second->maText = buf.makeStringAndClear();
        Point pix = MapIndexToPixel( _nPos );
        aFind->second->maRect = tools::Rectangle( Point( pix.X() + 1, pix.Y() + 1 ), Size(nX-1,nY-1) );
    }

    return aFind->second.get();
}

sal_Int32 SvxSearchCharSet::getMaxCharCount() const
{
    return m_aItemList.size();
}

void SvxSearchCharSet::ClearPreviousData()
{
    m_aItemList.clear();
    Invalidate();
}

void SvxSearchCharSet::AppendCharToList(sal_UCS4 sChar)
{
    m_aItemList.insert(std::make_pair(m_aItemList.size(), sChar));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
