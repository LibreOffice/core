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

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/virdev.hxx>
#include <svtools/colorcfg.hxx>

#include <rtl/textenc.h>
#include <svx/ucsubset.hxx>
#include <unordered_map>

#include <svx/searchcharmap.hxx>
#include <svx/svxdlg.hxx>

#include <charmapacc.hxx>
#include <uiobject.hxx>

#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <svl/itemset.hxx>
#include <unicode/uchar.h>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;


SvxSearchCharSet::SvxSearchCharSet(std::unique_ptr<weld::ScrolledWindow> pScrolledWindow, const VclPtr<VirtualDevice>& rVirDev)
    : SvxShowCharSet(std::move(pScrolledWindow), rVirDev)
    , nCount(0)
{
}

int SvxSearchCharSet::LastInView() const
{
    sal_uIntPtr nIndex = FirstInView();
    nIndex += ROW_COUNT * COLUMN_COUNT - 1;
    sal_uIntPtr nCompare = sal::static_int_cast<sal_uIntPtr>(nCount - 1);
    if (nIndex > nCompare)
        nIndex = nCompare;
    return nIndex;
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
        case KEY_SPACE:
            aSelectHdl.Call( this );
            break;
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
            tmpSelected = nCount - 1;
            break;
        case KEY_TAB:   // some fonts have a character at these unicode control codes
        case KEY_ESCAPE:
        case KEY_RETURN:
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

void SvxSearchCharSet::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    InitSettings(rRenderContext);
    RecalculateFont(rRenderContext);
    DrawChars_Impl(rRenderContext, FirstInView(), LastInView());
}

void SvxSearchCharSet::DrawChars_Impl(vcl::RenderContext& rRenderContext, int n1, int n2)
{
    if (n1 > LastInView() || n2 < FirstInView())
        return;

    Size aOutputSize(GetOutputSizePixel());

    int i;
    for (i = 1; i < COLUMN_COUNT; ++i)
    {
        rRenderContext.DrawLine(Point(nX * i + m_nXGap, 0),
                          Point(nX * i + m_nXGap, aOutputSize.Height()));
    }
    for (i = 1; i < ROW_COUNT; ++i)
    {
        rRenderContext.DrawLine(Point(0, nY * i + m_nYGap),
                                Point(aOutputSize.Width(), nY * i + m_nYGap));
    }
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Color aWindowTextColor(rStyleSettings.GetFieldTextColor());
    Color aHighlightColor(rStyleSettings.GetHighlightColor());
    Color aHighlightTextColor(rStyleSettings.GetHighlightTextColor());
    Color aFaceColor(rStyleSettings.GetFaceColor());
    Color aLightColor(rStyleSettings.GetLightColor());
    Color aShadowColor(rStyleSettings.GetShadowColor());

    int nTextHeight = rRenderContext.GetTextHeight();
    tools::Rectangle aBoundRect;
    for (i = n1; i <= n2; ++i)
    {
        Point pix = MapIndexToPixel(i);
        int x = pix.X();
        int y = pix.Y();

        OUStringBuffer buf;
        std::unordered_map<sal_Int32, sal_UCS4>::const_iterator got = m_aItemList.find (i);
        sal_UCS4 sName;

        if(got == m_aItemList.end())
            continue;
        else
            sName = got->second;

        buf.appendUtf32(sName);
        OUString aCharStr(buf.makeStringAndClear());
        int nTextWidth = rRenderContext.GetTextWidth(aCharStr);
        int tx = x + (nX - nTextWidth + 1) / 2;
        int ty = y + (nY - nTextHeight + 1) / 2;
        Point aPointTxTy(tx, ty);

        // adjust position before it gets out of bounds
        if (rRenderContext.GetTextBoundRect(aBoundRect, aCharStr) && !aBoundRect.IsEmpty())
        {
            // zero advance width => use ink width to center glyph
            if (!nTextWidth)
            {
                aPointTxTy.setX( x - aBoundRect.Left() + (nX - aBoundRect.GetWidth() + 1) / 2 );
            }

            aBoundRect += aPointTxTy;

            // shift back vertically if needed
            int nYLDelta = aBoundRect.Top() - y;
            int nYHDelta = (y + nY) - aBoundRect.Bottom();
            if (nYLDelta <= 0)
                aPointTxTy.AdjustY( -(nYLDelta - 1) );
            else if (nYHDelta <= 0)
                aPointTxTy.AdjustY(nYHDelta - 1 );

            // shift back horizontally if needed
            int nXLDelta = aBoundRect.Left() - x;
            int nXHDelta = (x + nX) - aBoundRect.Right();
            if (nXLDelta <= 0)
                aPointTxTy.AdjustX( -(nXLDelta - 1) );
            else if (nXHDelta <= 0)
                aPointTxTy.AdjustX(nXHDelta - 1 );
        }

        Color aTextCol = rRenderContext.GetTextColor();
        if (i != nSelectedIndex)
        {
            rRenderContext.SetTextColor(aWindowTextColor);
            rRenderContext.DrawText(aPointTxTy, aCharStr);
        }
        else
        {
            Color aLineCol = rRenderContext.GetLineColor();
            Color aFillCol = rRenderContext.GetFillColor();
            rRenderContext.SetLineColor();
            Point aPointUL(x + 1, y + 1);
            if (HasFocus())
            {
                rRenderContext.SetFillColor(aHighlightColor);
                rRenderContext.DrawRect(getGridRectangle(aPointUL, aOutputSize));

                rRenderContext.SetTextColor(aHighlightTextColor);
                rRenderContext.DrawText(aPointTxTy, aCharStr);
            }
            else
            {
                rRenderContext.SetFillColor(aFaceColor);
                rRenderContext.DrawRect(getGridRectangle(aPointUL, aOutputSize));

                rRenderContext.SetLineColor(aLightColor);
                rRenderContext.DrawLine(aPointUL, Point(x + nX - 1, y + 1));
                rRenderContext.DrawLine(aPointUL, Point(x + 1, y + nY - 1));

                rRenderContext.SetLineColor(aShadowColor);
                rRenderContext.DrawLine(Point(x + 1, y + nY - 1), Point(x + nX - 1, y + nY - 1));
                rRenderContext.DrawLine(Point(x + nX - 1, y + nY - 1), Point(x + nX - 1, y + 1));

                rRenderContext.DrawText(aPointTxTy, aCharStr);
            }
            rRenderContext.SetLineColor(aLineCol);
            rRenderContext.SetFillColor(aFillCol);
        }
        rRenderContext.SetTextColor(aTextCol);
    }
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
    getFavCharacterList();

    nX = aSize.Width() / COLUMN_COUNT;
    nY = aSize.Height() / ROW_COUNT;

    //scrollbar settings -- error
    int nLastRow = (nCount - 1 + COLUMN_COUNT) / COLUMN_COUNT;
    mxScrollArea->vadjustment_configure(mxScrollArea->vadjustment_get_value(), 0, nLastRow, 1, ROW_COUNT - 1, ROW_COUNT);

    // rearrange CharSet element in sync with nX- and nY-multiples
    Size aDrawSize(nX * COLUMN_COUNT, nY * ROW_COUNT);
    m_nXGap = (aSize.Width() - aDrawSize.Width()) / 2;
    m_nYGap = (aSize.Height() - aDrawSize.Height()) / 2;

    mbRecalculateFont = false;
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

        if( nNewIndex < nCount )
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
        OSL_ENSURE(m_xAccessible.is(), "Who wants to create a child of my table without a parent?");
        std::shared_ptr<svx::SvxShowCharSetItem> xItem(new svx::SvxShowCharSetItem(*this,
            m_xAccessible.get(), sal::static_int_cast< sal_uInt16 >(_nPos)));
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
    return nCount;
}

void SvxSearchCharSet::ClearPreviousData()
{
    m_aItemList.clear();
    nCount = 0;
    Invalidate();
}

void SvxSearchCharSet::AppendCharToList(sal_UCS4 sChar)
{
    m_aItemList.insert(std::make_pair(nCount++, sChar));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
