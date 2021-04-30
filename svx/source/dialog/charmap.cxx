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
#include <vcl/fontcharmap.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>

#include <svx/ucsubset.hxx>


#include <svx/strings.hrc>

#include <svx/charmap.hxx>
#include <svx/dialmgr.hxx>

#include <charmapacc.hxx>
#include <uiobject.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/SystemClipboard.hpp>
#include <officecfg/Office/Common.hxx>
#include <comphelper/processfactory.hxx>
#include <unicode/uchar.h>
#include <vcl/textview.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

sal_uInt32& SvxShowCharSet::getSelectedChar()
{
    static sal_uInt32 cSelectedChar = ' '; // keeps selected character over app lifetime
    return cSelectedChar;
}

FactoryFunction SvxShowCharSet::GetUITestFactory() const
{
    return SvxShowCharSetUIObject::create;
}

SvxShowCharSet::SvxShowCharSet(std::unique_ptr<weld::ScrolledWindow> pScrolledWindow, const VclPtr<VirtualDevice>& rVirDev)
    : mxVirDev(rVirDev)
    , mxScrollArea(std::move(pScrolledWindow))
    , mxContext(comphelper::getProcessComponentContext())
    , nX(0)
    , nY(0)
    , maFontSize(0, 0)
    , maPosition(0,0)
    , mbRecalculateFont(true)
    , mbUpdateForeground(true)
    , mbUpdateBackground(true)
{
    init();
}

void SvxShowCharSet::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);

    Size aSize(COLUMN_COUNT * pDrawingArea->get_approximate_digit_width() * 5.25,
               ROW_COUNT * pDrawingArea->get_text_height() * 2);

    nX = aSize.Width() / COLUMN_COUNT;
    nY = aSize.Height() / ROW_COUNT;

    // tdf#121232 set a size request that will result in a 0 m_nXGap by default
    mxScrollArea->set_size_request(COLUMN_COUNT * nX + mxScrollArea->get_scroll_thickness() + 2,
                                   ROW_COUNT * nY);
}

void SvxShowCharSet::init()
{
    nSelectedIndex = -1;    // TODO: move into init list when it is no longer static
    m_nXGap = 0;
    m_nYGap = 0;

    mxScrollArea->connect_vadjustment_changed(LINK(this, SvxShowCharSet, VscrollHdl));
    getFavCharacterList();
    // other settings depend on selected font => see RecalculateFont

    bDrag = false;
}

void SvxShowCharSet::Resize()
{
    mbRecalculateFont = true;
}

void SvxShowCharSet::GetFocus()
{
    SelectIndex(nSelectedIndex, true);
}

void SvxShowCharSet::LoseFocus()
{
    SelectIndex(nSelectedIndex);
}

bool SvxShowCharSet::MouseButtonDown(const MouseEvent& rMEvt)
{
    if ( rMEvt.IsLeft() )
    {
        if ( rMEvt.GetClicks() == 1 )
        {
            GrabFocus();
            bDrag = true;
            CaptureMouse();

            int nIndex = PixelToMapIndex( rMEvt.GetPosPixel() );
            // Fire the focus event
            SelectIndex( nIndex, true);
        }

        if ( !(rMEvt.GetClicks() % 2) )
            aDoubleClkHdl.Call( this );
    }

    if (rMEvt.IsRight())
    {
        Point aPosition (rMEvt.GetPosPixel());
        maPosition = aPosition;
        int nIndex = PixelToMapIndex( rMEvt.GetPosPixel() );
        // Fire the focus event
        SelectIndex( nIndex, true);
        createContextMenu();
    }

    return true;
}

bool SvxShowCharSet::MouseButtonUp(const MouseEvent& rMEvt)
{
    if ( bDrag && rMEvt.IsLeft() )
    {
        // released mouse over character map
        if ( tools::Rectangle(Point(), GetOutputSizePixel()).Contains(rMEvt.GetPosPixel()))
            aSelectHdl.Call( this );
        ReleaseMouse();
        bDrag = false;
    }

    return true;
}

bool SvxShowCharSet::MouseMove(const MouseEvent& rMEvt)
{
    if ( rMEvt.IsLeft() && bDrag )
    {
        Point aPos  = rMEvt.GetPosPixel();
        Size  aSize = GetOutputSizePixel();

        if ( aPos.X() < 0 )
            aPos.setX( 0 );
        else if ( aPos.X() > aSize.Width()-5 )
            aPos.setX( aSize.Width()-5 );
        if ( aPos.Y() < 0 )
            aPos.setY( 0 );
        else if ( aPos.Y() > aSize.Height()-5 )
            aPos.setY( aSize.Height()-5 );

        int nIndex = PixelToMapIndex( aPos );
    // Fire the focus event.
        SelectIndex( nIndex, true );
    }

    return true;
}

sal_uInt16 SvxShowCharSet::GetRowPos(sal_uInt16 _nPos)
{
    return _nPos / COLUMN_COUNT ;
}

void SvxShowCharSet::getFavCharacterList()
{
    maFavCharList.clear();
    maFavCharFontList.clear();
    //retrieve recent character list
    css::uno::Sequence< OUString > rFavCharList( officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterList::get() );
    comphelper::sequenceToContainer(maFavCharList, rFavCharList);

    //retrieve recent character font list
    css::uno::Sequence< OUString > rFavCharFontList( officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterFontList::get() );
    comphelper::sequenceToContainer(maFavCharFontList, rFavCharFontList);
}

bool SvxShowCharSet::isFavChar(const OUString& sTitle, const OUString& rFont)
{
    auto isFavCharTitleExists = std::any_of(maFavCharList.begin(),
         maFavCharList.end(),
         [sTitle] (const OUString & a) { return a == sTitle; });

    auto isFavCharFontExists = std::any_of(maFavCharFontList.begin(),
         maFavCharFontList.end(),
         [rFont] (const OUString & a) { return a == rFont; });

    // if Fav char to be added is already in list, return true
    return isFavCharTitleExists && isFavCharFontExists;
}

void SvxShowCharSet::createContextMenu()
{
    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetDrawingArea(), "svx/ui/charsetmenu.ui"));
    std::unique_ptr<weld::Menu> xItemMenu(xBuilder->weld_menu("charsetmenu"));

    sal_UCS4 cChar = GetSelectCharacter();
    OUString aOUStr( &cChar, 1 );
    if (isFavChar(aOUStr, mxVirDev->GetFont().GetFamilyName()) || maFavCharList.size() >= 16)
        xItemMenu->set_visible("add", false);
    else
        xItemMenu->set_visible("remove", false);

    ContextMenuSelect(xItemMenu->popup_at_rect(GetDrawingArea(), tools::Rectangle(maPosition, Size(1,1))));
    GrabFocus();
    Invalidate();
}

void SvxShowCharSet::ContextMenuSelect(std::string_view rIdent)
{
    sal_UCS4 cChar = GetSelectCharacter();
    OUString aOUStr(&cChar, 1);

    if (rIdent == "insert")
        aDoubleClkHdl.Call(this);
    else if (rIdent == "add" || rIdent == "remove")
    {
        updateFavCharacterList(aOUStr, mxVirDev->GetFont().GetFamilyName());
        aFavClickHdl.Call(this);
    }
    else if (rIdent == "copy")
        CopyToClipboard(aOUStr);
}

void SvxShowCharSet::CopyToClipboard(const OUString& rOUStr)
{
    css::uno::Reference<css::datatransfer::clipboard::XClipboard> xClipboard =
        css::datatransfer::clipboard::SystemClipboard::create(comphelper::getProcessComponentContext());

    if (!xClipboard.is())
        return;

    rtl::Reference<TETextDataObject> pDataObj = new TETextDataObject(rOUStr);

    try
    {
        xClipboard->setContents( pDataObj, nullptr );

        css::uno::Reference<css::datatransfer::clipboard::XFlushableClipboard> xFlushableClipboard(xClipboard, css::uno::UNO_QUERY);
        if( xFlushableClipboard.is() )
            xFlushableClipboard->flushClipboard();
    }
    catch( const css::uno::Exception& )
    {
    }
}

void SvxShowCharSet::updateFavCharacterList(const OUString& sTitle, const OUString& rFont)
{
    if(isFavChar(sTitle, rFont))
    {
        auto itChar = std::find(maFavCharList.begin(), maFavCharList.end(), sTitle);
        auto itChar2 = std::find(maFavCharFontList.begin(), maFavCharFontList.end(), rFont);

        // if Fav char to be added is already in list, remove it
        if( itChar != maFavCharList.end() &&  itChar2 != maFavCharFontList.end() )
        {
            maFavCharList.erase( itChar );
            maFavCharFontList.erase( itChar2);
        }

        css::uno::Sequence< OUString > aFavCharList(maFavCharList.size());
        auto aFavCharListRange = asNonConstRange(aFavCharList);
        css::uno::Sequence< OUString > aFavCharFontList(maFavCharFontList.size());
        auto aFavCharFontListRange = asNonConstRange(aFavCharFontList);

        for (size_t i = 0; i < maFavCharList.size(); ++i)
        {
            aFavCharListRange[i] = maFavCharList[i];
            aFavCharFontListRange[i] = maFavCharFontList[i];
        }

        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(mxContext));
        officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterList::set(aFavCharList, batch);
        officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterFontList::set(aFavCharFontList, batch);
        batch->commit();
        return;
    }

    auto itChar = std::find(maFavCharList.begin(), maFavCharList.end(), sTitle);
    auto itChar2 = std::find(maFavCharFontList.begin(), maFavCharFontList.end(), rFont);

    // if Fav char to be added is already in list, remove it
    if( itChar != maFavCharList.end() &&  itChar2 != maFavCharFontList.end() )
    {
        maFavCharList.erase( itChar );
        maFavCharFontList.erase( itChar2);
    }

    if (maFavCharList.size() == 16)
    {
        maFavCharList.pop_back();
        maFavCharFontList.pop_back();
    }

    maFavCharList.push_back(sTitle);
    maFavCharFontList.push_back(rFont);

    css::uno::Sequence< OUString > aFavCharList(maFavCharList.size());
    auto aFavCharListRange = asNonConstRange(aFavCharList);
    css::uno::Sequence< OUString > aFavCharFontList(maFavCharFontList.size());
    auto aFavCharFontListRange = asNonConstRange(aFavCharFontList);

    for (size_t i = 0; i < maFavCharList.size(); ++i)
    {
        aFavCharListRange[i] = maFavCharList[i];
        aFavCharFontListRange[i] = maFavCharFontList[i];
    }

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(mxContext));
    officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterList::set(aFavCharList, batch);
    officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterFontList::set(aFavCharFontList, batch);
    batch->commit();
}

sal_uInt16 SvxShowCharSet::GetColumnPos(sal_uInt16 _nPos)
{
    return _nPos % COLUMN_COUNT ;
}

int SvxShowCharSet::FirstInView() const
{
    return mxScrollArea->vadjustment_get_value() * COLUMN_COUNT;
}

int SvxShowCharSet::LastInView() const
{
    sal_uInt32 nIndex = FirstInView();
    nIndex += ROW_COUNT * COLUMN_COUNT - 1;
    sal_uInt32 nCompare = mxFontCharMap->GetCharCount() - 1;
    if (nIndex > nCompare)
        nIndex = nCompare;
    return nIndex;
}

Point SvxShowCharSet::MapIndexToPixel( int nIndex ) const
{
    const int nBase = FirstInView();
    int x = ((nIndex - nBase) % COLUMN_COUNT) * nX;
    int y = ((nIndex - nBase) / COLUMN_COUNT) * nY;
    return Point( x + m_nXGap, y + m_nYGap );
}


int SvxShowCharSet::PixelToMapIndex( const Point& point) const
{
    int nBase = FirstInView();
    return (nBase + ((point.X() - m_nXGap)/nX) + ((point.Y() - m_nYGap)/nY) * COLUMN_COUNT);
}

bool SvxShowCharSet::KeyInput(const KeyEvent& rKEvt)
{
    vcl::KeyCode aCode = rKEvt.GetKeyCode();

    if (aCode.GetModifier())
        return false;

    bool bRet = true;

    int tmpSelected = nSelectedIndex;

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
            tmpSelected = mxFontCharMap->GetCharCount() - 1;
            break;
        case KEY_TAB:   // some fonts have a character at these unicode control codes
        case KEY_ESCAPE:
        case KEY_RETURN:
            tmpSelected = - 1;  // mark as invalid
            bRet = false;
            break;
        default:
        {
            sal_UCS4 cChar = rKEvt.GetCharCode();
            sal_UCS4 cNext = mxFontCharMap->GetNextChar(cChar - 1);
            tmpSelected = mxFontCharMap->GetIndexFromChar(cNext);
            if (tmpSelected < 0 || (cChar != cNext))
            {
                tmpSelected = - 1;  // mark as invalid
                bRet = false;
            }
            break;
        }
    }

    if ( tmpSelected >= 0 )
    {
        SelectIndex( tmpSelected, true );
        aPreSelectHdl.Call( this );
    }

    return bRet;
}

void SvxShowCharSet::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    InitSettings(rRenderContext);
    RecalculateFont(rRenderContext);
    DrawChars_Impl(rRenderContext, FirstInView(), LastInView());
}

void SvxShowCharSet::SetFont( const vcl::Font& rFont )
{
    maFont = rFont;
    mbRecalculateFont = true;
    Invalidate();
}

void SvxShowCharSet::DeSelect()
{
    Invalidate();
}

// stretch a grid rectangle if its at the edge to fill unused space
tools::Rectangle SvxShowCharSet::getGridRectangle(const Point &rPointUL, const Size &rOutputSize) const
{
    tools::Long x = rPointUL.X() - 1;
    tools::Long y = rPointUL.Y() - 1;
    Point aPointUL(x+1, y+1);
    Size aGridSize(nX-1, nY-1);

    tools::Long nXDistFromLeft = x - m_nXGap;
    if (nXDistFromLeft <= 1)
    {
        aPointUL.setX( 1 );
        aGridSize.AdjustWidth(m_nXGap + nXDistFromLeft );
    }
    tools::Long nXDistFromRight = rOutputSize.Width() - m_nXGap - nX - x;
    if (nXDistFromRight <= 1)
        aGridSize.AdjustWidth(m_nXGap + nXDistFromRight );

    tools::Long nXDistFromTop = y - m_nYGap;
    if (nXDistFromTop <= 1)
    {
        aPointUL.setY( 1 );
        aGridSize.AdjustHeight(m_nYGap + nXDistFromTop );
    }
    tools::Long nXDistFromBottom = rOutputSize.Height() - m_nYGap - nY - y;
    if (nXDistFromBottom <= 1)
        aGridSize.AdjustHeight(m_nYGap + nXDistFromBottom );

    return tools::Rectangle(aPointUL, aGridSize);
}

void SvxShowCharSet::DrawChars_Impl(vcl::RenderContext& rRenderContext, int n1, int n2)
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

        sal_UCS4 nChar = mxFontCharMap->GetCharFromIndex(i);
        OUString aCharStr(&nChar, 1);
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


void SvxShowCharSet::InitSettings(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    if (mbUpdateForeground)
    {
        rRenderContext.SetTextColor(rStyleSettings.GetDialogTextColor());
        mbUpdateForeground = false;
    }

    if (mbUpdateBackground)
    {
        rRenderContext.SetBackground(rStyleSettings.GetWindowColor());
        rRenderContext.Erase();
        mbUpdateBackground = false;
    }

    vcl::Font aFont(maFont);
    aFont.SetWeight(WEIGHT_LIGHT);
    aFont.SetAlignment(ALIGN_TOP);
    aFont.SetFontSize(maFontSize);
    aFont.SetTransparent(true);
    rRenderContext.SetFont(aFont);
}

sal_UCS4 SvxShowCharSet::GetSelectCharacter() const
{
    if( nSelectedIndex >= 0 )
        getSelectedChar() = mxFontCharMap->GetCharFromIndex( nSelectedIndex );
    return getSelectedChar();
}


void SvxShowCharSet::RecalculateFont(vcl::RenderContext& rRenderContext)
{
    if (!mbRecalculateFont)
        return;

    // save last selected unicode
    if (nSelectedIndex >= 0)
        getSelectedChar() = mxFontCharMap->GetCharFromIndex(nSelectedIndex);

    Size aSize(GetOutputSizePixel());

    vcl::Font aFont = maFont;
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

    const int nLastRow = (mxFontCharMap->GetCharCount() - 1 + COLUMN_COUNT) / COLUMN_COUNT;
    mxScrollArea->vadjustment_configure(mxScrollArea->vadjustment_get_value(), 0, nLastRow, 1, ROW_COUNT - 1, ROW_COUNT);

    // restore last selected unicode
    int nMapIndex = mxFontCharMap->GetIndexFromChar(getSelectedChar());
    if (nMapIndex != nSelectedIndex)
        SelectIndex(nMapIndex);

    // rearrange CharSet element in sync with nX- and nY-multiples
    Size aDrawSize(nX * COLUMN_COUNT, nY * ROW_COUNT);
    m_nXGap = (aSize.Width() - aDrawSize.Width()) / 2;
    m_nYGap = (aSize.Height() - aDrawSize.Height()) / 2;

    mbRecalculateFont = false;
}

void SvxShowCharSet::SelectIndex(int nNewIndex, bool bFocus)
{
    if (!mxFontCharMap.is())
        RecalculateFont(*mxVirDev);

    if( nNewIndex < 0 )
    {
        // need to scroll see closest unicode
        sal_uInt32 cPrev = mxFontCharMap->GetPrevChar( getSelectedChar() );
        int nMapIndex = mxFontCharMap->GetIndexFromChar( cPrev );
        int nNewPos = nMapIndex / COLUMN_COUNT;
        mxScrollArea->vadjustment_set_value(nNewPos);
        nSelectedIndex = bFocus ? nMapIndex+1 : -1;
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
        if( nNewIndex < mxFontCharMap->GetCharCount() )
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
        getSelectedChar() = mxFontCharMap->GetCharFromIndex( nSelectedIndex );
#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
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

void SvxShowCharSet::OutputIndex( int nNewIndex )
{
    SelectIndex( nNewIndex, true );
    aSelectHdl.Call( this );
}


void SvxShowCharSet::SelectCharacter( sal_UCS4 cNew )
{
    if ( !mxFontCharMap.is() )
        RecalculateFont(*mxVirDev);

    // get next available char of current font
    sal_UCS4 cNext = mxFontCharMap->GetNextChar( (cNew > 0) ? cNew - 1 : cNew );

    int nMapIndex = mxFontCharMap->GetIndexFromChar( cNext );
    SelectIndex( nMapIndex );
    // move selected item to top row if not in focus
    mxScrollArea->vadjustment_set_value(nMapIndex / COLUMN_COUNT);
    Invalidate();
}

IMPL_LINK_NOARG(SvxShowCharSet, VscrollHdl, weld::ScrolledWindow&, void)
{
    if( nSelectedIndex < FirstInView() )
    {
        SelectIndex( FirstInView() + (nSelectedIndex % COLUMN_COUNT) );
    }
    else if( nSelectedIndex > LastInView() )
    {
#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
        if( m_xAccessible.is() )
        {
            css::uno::Any aOldAny, aNewAny;
            int nLast = LastInView();
            for ( ; nLast != nSelectedIndex; ++nLast)
            {
                aOldAny <<= ImplGetItem(nLast)->GetAccessible();
                m_xAccessible ->fireEvent( AccessibleEventId::CHILD, aOldAny, aNewAny );
            }
        }
#endif
        SelectIndex( (LastInView() - COLUMN_COUNT + 1) + (nSelectedIndex % COLUMN_COUNT) );
    }

    Invalidate();
}

SvxShowCharSet::~SvxShowCharSet()
{
#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
    if (m_xAccessible.is())
    {
        m_aItems.clear();
        m_xAccessible->clearCharSetControl();
        m_xAccessible.clear();
    }
#endif
}

css::uno::Reference< XAccessible > SvxShowCharSet::CreateAccessible()
{
#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
    OSL_ENSURE(!m_xAccessible.is(),"Accessible already created!");
    m_xAccessible = new svx::SvxShowCharSetAcc(this);
#endif
    return m_xAccessible;
}

svx::SvxShowCharSetItem* SvxShowCharSet::ImplGetItem( int _nPos )
{
    ItemsMap::iterator aFind = m_aItems.find(_nPos);
    if ( aFind == m_aItems.end() )
    {
#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
        OSL_ENSURE(m_xAccessible.is(), "Who wants to create a child of my table without a parent?");
#endif
        auto xItem = std::make_shared<svx::SvxShowCharSetItem>(*this,
            m_xAccessible.get(), sal::static_int_cast< sal_uInt16 >(_nPos));
        aFind = m_aItems.emplace(_nPos, xItem).first;
        OUStringBuffer buf;
        buf.appendUtf32( mxFontCharMap->GetCharFromIndex( _nPos ) );
        aFind->second->maText = buf.makeStringAndClear();
        Point pix = MapIndexToPixel( _nPos );
        aFind->second->maRect = tools::Rectangle( Point( pix.X() + 1, pix.Y() + 1 ), Size(nX-1,nY-1) );
    }

    return aFind->second.get();
}

sal_Int32 SvxShowCharSet::getMaxCharCount() const
{
    return mxFontCharMap->GetCharCount();
}

FontCharMapRef const & SvxShowCharSet::GetFontCharMap()
{
    RecalculateFont(*mxVirDev);
    return mxFontCharMap;
}

// TODO: should be moved into Font Attributes stuff
// we let it mature here though because it is currently the only use

SubsetMap::SubsetMap( const FontCharMapRef& rxFontCharMap )
{
    InitList();
    ApplyCharMap(rxFontCharMap);
}

const SubsetVec& SubsetMap::GetSubsetMap() const
{
    return maSubsets;
}

const Subset* SubsetMap::GetSubsetByUnicode( sal_UCS4 cChar ) const
{
    for (auto const& subset : maSubsets)
        if( (subset.GetRangeMin() <= cChar) && (cChar <= subset.GetRangeMax()) )
            return &subset;
    return nullptr;
}

inline Subset::Subset(sal_UCS4 nMin, sal_UCS4 nMax, const OUString& rName)
:   mnRangeMin(nMin), mnRangeMax(nMax), maRangeName(rName)
{
}

void SubsetMap::InitList()
{
    static SubsetVec s_aAllSubsets = []()
    {
        SubsetVec aAllSubsets;
        //I wish icu had a way to give me the block ranges
        for (int i = UBLOCK_BASIC_LATIN; i < UBLOCK_COUNT; ++i)
        {
            UBlockCode eBlock = static_cast<UBlockCode>(i);
            switch (eBlock)
            {
                case UBLOCK_NO_BLOCK:
                case UBLOCK_INVALID_CODE:
                case UBLOCK_COUNT:
                case UBLOCK_HIGH_SURROGATES:
                case UBLOCK_HIGH_PRIVATE_USE_SURROGATES:
                case UBLOCK_LOW_SURROGATES:
                    break;
                case UBLOCK_BASIC_LATIN:
                    aAllSubsets.emplace_back( 0x0000, 0x007F, SvxResId(RID_SUBSETSTR_BASIC_LATIN) );
                    break;
                case UBLOCK_LATIN_1_SUPPLEMENT:
                    aAllSubsets.emplace_back( 0x0080, 0x00FF, SvxResId(RID_SUBSETSTR_LATIN_1) );
                    break;
                case UBLOCK_LATIN_EXTENDED_A:
                    aAllSubsets.emplace_back( 0x0100, 0x017F, SvxResId(RID_SUBSETSTR_LATIN_EXTENDED_A) );
                    break;
                case UBLOCK_LATIN_EXTENDED_B:
                    aAllSubsets.emplace_back( 0x0180, 0x024F, SvxResId(RID_SUBSETSTR_LATIN_EXTENDED_B) );
                    break;
                case UBLOCK_IPA_EXTENSIONS:
                    aAllSubsets.emplace_back( 0x0250, 0x02AF, SvxResId(RID_SUBSETSTR_IPA_EXTENSIONS) );
                    break;
                case UBLOCK_SPACING_MODIFIER_LETTERS:
                    aAllSubsets.emplace_back( 0x02B0, 0x02FF, SvxResId(RID_SUBSETSTR_SPACING_MODIFIERS) );
                    break;
                case UBLOCK_COMBINING_DIACRITICAL_MARKS:
                    aAllSubsets.emplace_back( 0x0300, 0x036F, SvxResId(RID_SUBSETSTR_COMB_DIACRITICAL) );
                    break;
                case UBLOCK_GREEK:
                    aAllSubsets.emplace_back( 0x0370, 0x03FF, SvxResId(RID_SUBSETSTR_BASIC_GREEK) );
                    break;
                case UBLOCK_CYRILLIC:
                    aAllSubsets.emplace_back( 0x0400, 0x04FF, SvxResId(RID_SUBSETSTR_CYRILLIC) );
                    break;
                case UBLOCK_ARMENIAN:
                    aAllSubsets.emplace_back( 0x0530, 0x058F, SvxResId(RID_SUBSETSTR_ARMENIAN) );
                    break;
                case UBLOCK_HEBREW:
                    aAllSubsets.emplace_back( 0x0590, 0x05FF, SvxResId(RID_SUBSETSTR_BASIC_HEBREW) );
                    break;
                case UBLOCK_ARABIC:
                    aAllSubsets.emplace_back( 0x0600, 0x065F, SvxResId(RID_SUBSETSTR_BASIC_ARABIC) );
                    break;
                case UBLOCK_SYRIAC:
                    aAllSubsets.emplace_back( 0x0700, 0x074F, SvxResId(RID_SUBSETSTR_SYRIAC) );
                    break;
                case UBLOCK_THAANA:
                    aAllSubsets.emplace_back( 0x0780, 0x07BF, SvxResId(RID_SUBSETSTR_THAANA) );
                    break;
                case UBLOCK_DEVANAGARI:
                    aAllSubsets.emplace_back( 0x0900, 0x097F, SvxResId(RID_SUBSETSTR_DEVANAGARI) );
                    break;
                case UBLOCK_BENGALI:
                    aAllSubsets.emplace_back( 0x0980, 0x09FF, SvxResId(RID_SUBSETSTR_BENGALI) );
                    break;
                case UBLOCK_GURMUKHI:
                    aAllSubsets.emplace_back( 0x0A00, 0x0A7F, SvxResId(RID_SUBSETSTR_GURMUKHI) );
                    break;
                case UBLOCK_GUJARATI:
                    aAllSubsets.emplace_back( 0x0A80, 0x0AFF, SvxResId(RID_SUBSETSTR_GUJARATI) );
                    break;
                case UBLOCK_ORIYA:
                    aAllSubsets.emplace_back( 0x0B00, 0x0B7F, SvxResId(RID_SUBSETSTR_ODIA) );
                    break;
                case UBLOCK_TAMIL:
                    aAllSubsets.emplace_back( 0x0B80, 0x0BFF, SvxResId(RID_SUBSETSTR_TAMIL) );
                    break;
                case UBLOCK_TELUGU:
                    aAllSubsets.emplace_back( 0x0C00, 0x0C7F, SvxResId(RID_SUBSETSTR_TELUGU) );
                    break;
                case UBLOCK_KANNADA:
                    aAllSubsets.emplace_back( 0x0C80, 0x0CFF, SvxResId(RID_SUBSETSTR_KANNADA) );
                    break;
                case UBLOCK_MALAYALAM:
                    aAllSubsets.emplace_back( 0x0D00, 0x0D7F, SvxResId(RID_SUBSETSTR_MALAYALAM) );
                    break;
                case UBLOCK_SINHALA:
                    aAllSubsets.emplace_back( 0x0D80, 0x0DFF, SvxResId(RID_SUBSETSTR_SINHALA) );
                    break;
                case UBLOCK_THAI:
                    aAllSubsets.emplace_back( 0x0E00, 0x0E7F, SvxResId(RID_SUBSETSTR_THAI) );
                    break;
                case UBLOCK_LAO:
                    aAllSubsets.emplace_back( 0x0E80, 0x0EFF, SvxResId(RID_SUBSETSTR_LAO) );
                    break;
                case UBLOCK_TIBETAN:
                    aAllSubsets.emplace_back( 0x0F00, 0x0FBF, SvxResId(RID_SUBSETSTR_TIBETAN) );
                    break;
                case UBLOCK_MYANMAR:
                    aAllSubsets.emplace_back( 0x1000, 0x109F, SvxResId(RID_SUBSETSTR_MYANMAR) );
                    break;
                case UBLOCK_GEORGIAN:
                    aAllSubsets.emplace_back( 0x10A0, 0x10FF, SvxResId(RID_SUBSETSTR_BASIC_GEORGIAN) );
                    break;
                case UBLOCK_HANGUL_JAMO:
                    aAllSubsets.emplace_back( 0x1100, 0x11FF, SvxResId(RID_SUBSETSTR_HANGUL_JAMO) );
                    break;
                case UBLOCK_ETHIOPIC:
                    aAllSubsets.emplace_back( 0x1200, 0x137F, SvxResId(RID_SUBSETSTR_ETHIOPIC) );
                    break;
                case UBLOCK_CHEROKEE:
                    aAllSubsets.emplace_back( 0x13A0, 0x13FF, SvxResId(RID_SUBSETSTR_CHEROKEE) );
                    break;
                case UBLOCK_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS:
                    aAllSubsets.emplace_back( 0x1400, 0x167F, SvxResId(RID_SUBSETSTR_CANADIAN_ABORIGINAL) );
                    break;
                case UBLOCK_OGHAM:
                    aAllSubsets.emplace_back( 0x1680, 0x169F, SvxResId(RID_SUBSETSTR_OGHAM) );
                    break;
                case UBLOCK_RUNIC:
                    aAllSubsets.emplace_back( 0x16A0, 0x16F0, SvxResId(RID_SUBSETSTR_RUNIC) );
                    break;
                case UBLOCK_KHMER:
                    aAllSubsets.emplace_back( 0x1780, 0x17FF, SvxResId(RID_SUBSETSTR_KHMER) );
                    break;
                case UBLOCK_MONGOLIAN:
                    aAllSubsets.emplace_back( 0x1800, 0x18AF, SvxResId(RID_SUBSETSTR_MONGOLIAN) );
                    break;
                case UBLOCK_LATIN_EXTENDED_ADDITIONAL:
                    aAllSubsets.emplace_back( 0x1E00, 0x1EFF, SvxResId(RID_SUBSETSTR_LATIN_EXTENDED_ADDS) );
                    break;
                case UBLOCK_GREEK_EXTENDED:
                    aAllSubsets.emplace_back( 0x1F00, 0x1FFF, SvxResId(RID_SUBSETSTR_GREEK_EXTENDED) );
                    break;
                case UBLOCK_GENERAL_PUNCTUATION:
                    aAllSubsets.emplace_back( 0x2000, 0x206F, SvxResId(RID_SUBSETSTR_GENERAL_PUNCTUATION) );
                    break;
                case UBLOCK_SUPERSCRIPTS_AND_SUBSCRIPTS:
                    aAllSubsets.emplace_back( 0x2070, 0x209F, SvxResId(RID_SUBSETSTR_SUB_SUPER_SCRIPTS) );
                    break;
                case UBLOCK_CURRENCY_SYMBOLS:
                    aAllSubsets.emplace_back( 0x20A0, 0x20CF, SvxResId(RID_SUBSETSTR_CURRENCY_SYMBOLS) );
                    break;
                case UBLOCK_COMBINING_MARKS_FOR_SYMBOLS:
                    aAllSubsets.emplace_back( 0x20D0, 0x20FF, SvxResId(RID_SUBSETSTR_COMB_DIACRITIC_SYMS) );
                    break;
                case UBLOCK_LETTERLIKE_SYMBOLS:
                    aAllSubsets.emplace_back( 0x2100, 0x214F, SvxResId(RID_SUBSETSTR_LETTERLIKE_SYMBOLS) );
                    break;
                case UBLOCK_NUMBER_FORMS:
                    aAllSubsets.emplace_back( 0x2150, 0x218F, SvxResId(RID_SUBSETSTR_NUMBER_FORMS) );
                    break;
                case UBLOCK_ARROWS:
                    aAllSubsets.emplace_back( 0x2190, 0x21FF, SvxResId(RID_SUBSETSTR_ARROWS) );
                    break;
                case UBLOCK_MATHEMATICAL_OPERATORS:
                    aAllSubsets.emplace_back( 0x2200, 0x22FF, SvxResId(RID_SUBSETSTR_MATH_OPERATORS) );
                    break;
                case UBLOCK_MISCELLANEOUS_TECHNICAL:
                    aAllSubsets.emplace_back( 0x2300, 0x23FF, SvxResId(RID_SUBSETSTR_MISC_TECHNICAL) );
                    break;
                case UBLOCK_CONTROL_PICTURES:
                    aAllSubsets.emplace_back( 0x2400, 0x243F, SvxResId(RID_SUBSETSTR_CONTROL_PICTURES) );
                    break;
                case UBLOCK_OPTICAL_CHARACTER_RECOGNITION:
                    aAllSubsets.emplace_back( 0x2440, 0x245F, SvxResId(RID_SUBSETSTR_OPTICAL_CHAR_REC) );
                    break;
                case UBLOCK_ENCLOSED_ALPHANUMERICS:
                    aAllSubsets.emplace_back( 0x2460, 0x24FF, SvxResId(RID_SUBSETSTR_ENCLOSED_ALPHANUM) );
                    break;
                case UBLOCK_BOX_DRAWING:
                    aAllSubsets.emplace_back( 0x2500, 0x257F, SvxResId(RID_SUBSETSTR_BOX_DRAWING) );
                    break;
                case UBLOCK_BLOCK_ELEMENTS:
                    aAllSubsets.emplace_back( 0x2580, 0x259F, SvxResId(RID_SUBSETSTR_BLOCK_ELEMENTS) );
                    break;
                case UBLOCK_GEOMETRIC_SHAPES:
                    aAllSubsets.emplace_back( 0x25A0, 0x25FF, SvxResId(RID_SUBSETSTR_GEOMETRIC_SHAPES) );
                    break;
                case UBLOCK_MISCELLANEOUS_SYMBOLS:
                    aAllSubsets.emplace_back( 0x2600, 0x26FF, SvxResId(RID_SUBSETSTR_MISC_DINGBATS) );
                    break;
                case UBLOCK_DINGBATS:
                    aAllSubsets.emplace_back( 0x2700, 0x27BF, SvxResId(RID_SUBSETSTR_DINGBATS) );
                    break;
                case UBLOCK_BRAILLE_PATTERNS:
                    aAllSubsets.emplace_back( 0x2800, 0x28FF, SvxResId(RID_SUBSETSTR_BRAILLE_PATTERNS) );
                    break;
                case UBLOCK_CJK_RADICALS_SUPPLEMENT:
                    aAllSubsets.emplace_back( 0x2E80, 0x2EFF, SvxResId(RID_SUBSETSTR_CJK_RADICAL_SUPPL) );
                    break;
                case UBLOCK_KANGXI_RADICALS:
                    aAllSubsets.emplace_back( 0x2F00, 0x2FDF, SvxResId(RID_SUBSETSTR_KANGXI_RADICALS) );
                    break;
                case UBLOCK_IDEOGRAPHIC_DESCRIPTION_CHARACTERS:
                    aAllSubsets.emplace_back( 0x2FF0, 0x2FFF, SvxResId(RID_SUBSETSTR_IDEO_DESC_CHARS) );
                    break;
                case UBLOCK_CJK_SYMBOLS_AND_PUNCTUATION:
                    aAllSubsets.emplace_back( 0x3000, 0x303F, SvxResId(RID_SUBSETSTR_CJK_SYMS_PUNCTUATION) );
                    break;
                case UBLOCK_HIRAGANA:
                    aAllSubsets.emplace_back( 0x3040, 0x309F, SvxResId(RID_SUBSETSTR_HIRAGANA) );
                    break;
                case UBLOCK_KATAKANA:
                    aAllSubsets.emplace_back( 0x30A0, 0x30FF, SvxResId(RID_SUBSETSTR_KATAKANA) );
                    break;
                case UBLOCK_BOPOMOFO:
                    aAllSubsets.emplace_back( 0x3100, 0x312F, SvxResId(RID_SUBSETSTR_BOPOMOFO) );
                    break;
                case UBLOCK_HANGUL_COMPATIBILITY_JAMO:
                    aAllSubsets.emplace_back( 0x3130, 0x318F, SvxResId(RID_SUBSETSTR_HANGUL_COMPAT_JAMO) );
                    break;
                case UBLOCK_KANBUN:
                    aAllSubsets.emplace_back( 0x3190, 0x319F, SvxResId(RID_SUBSETSTR_KANBUN) );
                    break;
                case UBLOCK_BOPOMOFO_EXTENDED:
                    aAllSubsets.emplace_back( 0x31A0, 0x31BF, SvxResId(RID_SUBSETSTR_BOPOMOFO_EXTENDED) );
                    break;
                case UBLOCK_ENCLOSED_CJK_LETTERS_AND_MONTHS:
                    aAllSubsets.emplace_back( 0x3200, 0x32FF, SvxResId(RID_SUBSETSTR_ENCLOSED_CJK_LETTERS) );
                    break;
                case UBLOCK_CJK_COMPATIBILITY:
                    aAllSubsets.emplace_back( 0x3300, 0x33FF, SvxResId(RID_SUBSETSTR_CJK_COMPATIBILITY) );
                    break;
                case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A:
                    aAllSubsets.emplace_back( 0x3400, 0x4DBF, SvxResId(RID_SUBSETSTR_CJK_EXT_A_UNIFIED_IDGRAPH) );
                    break;
                case UBLOCK_CJK_UNIFIED_IDEOGRAPHS:
                    aAllSubsets.emplace_back( 0x4E00, 0x9FA5, SvxResId(RID_SUBSETSTR_CJK_UNIFIED_IDGRAPH) );
                    break;
                case UBLOCK_YI_SYLLABLES:
                    aAllSubsets.emplace_back( 0xA000, 0xA48F, SvxResId(RID_SUBSETSTR_YI_SYLLABLES) );
                    break;
                case UBLOCK_YI_RADICALS:
                    aAllSubsets.emplace_back( 0xA490, 0xA4CF, SvxResId(RID_SUBSETSTR_YI_RADICALS) );
                    break;
                case UBLOCK_HANGUL_SYLLABLES:
                    aAllSubsets.emplace_back( 0xAC00, 0xD7AF, SvxResId(RID_SUBSETSTR_HANGUL) );
                    break;
                case UBLOCK_PRIVATE_USE_AREA:
                    aAllSubsets.emplace_back( 0xE000, 0xF8FF, SvxResId(RID_SUBSETSTR_PRIVATE_USE_AREA) );
                    break;
                case UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS:
                    aAllSubsets.emplace_back( 0xF900, 0xFAFF, SvxResId(RID_SUBSETSTR_CJK_COMPAT_IDGRAPHS) );
                    break;
                case UBLOCK_ALPHABETIC_PRESENTATION_FORMS:
                    aAllSubsets.emplace_back( 0xFB00, 0xFB4F, SvxResId(RID_SUBSETSTR_ALPHA_PRESENTATION) );
                    break;
                case UBLOCK_ARABIC_PRESENTATION_FORMS_A:
                    aAllSubsets.emplace_back( 0xFB50, 0xFDFF, SvxResId(RID_SUBSETSTR_ARABIC_PRESENT_A) );
                    break;
                case UBLOCK_COMBINING_HALF_MARKS:
                    aAllSubsets.emplace_back( 0xFE20, 0xFE2F, SvxResId(RID_SUBSETSTR_COMBINING_HALF_MARKS) );
                    break;
                case UBLOCK_CJK_COMPATIBILITY_FORMS:
                    aAllSubsets.emplace_back( 0xFE30, 0xFE4F, SvxResId(RID_SUBSETSTR_CJK_COMPAT_FORMS) );
                    break;
                case UBLOCK_SMALL_FORM_VARIANTS:
                    aAllSubsets.emplace_back( 0xFE50, 0xFE6F, SvxResId(RID_SUBSETSTR_SMALL_FORM_VARIANTS) );
                    break;
                case UBLOCK_ARABIC_PRESENTATION_FORMS_B:
                    aAllSubsets.emplace_back( 0xFE70, 0xFEFF, SvxResId(RID_SUBSETSTR_ARABIC_PRESENT_B) );
                    break;
                case UBLOCK_SPECIALS:
                    aAllSubsets.emplace_back( 0xFFF0, 0xFFFF, SvxResId(RID_SUBSETSTR_SPECIALS) );
                    break;
                case UBLOCK_HALFWIDTH_AND_FULLWIDTH_FORMS:
                    aAllSubsets.emplace_back( 0xFF00, 0xFFEF, SvxResId(RID_SUBSETSTR_HALFW_FULLW_FORMS) );
                    break;
                case UBLOCK_OLD_ITALIC:
                    aAllSubsets.emplace_back( 0x10300, 0x1032F, SvxResId(RID_SUBSETSTR_OLD_ITALIC) );
                    break;
                case UBLOCK_GOTHIC:
                    aAllSubsets.emplace_back( 0x10330, 0x1034F, SvxResId(RID_SUBSETSTR_GOTHIC) );
                    break;
                case UBLOCK_DESERET:
                    aAllSubsets.emplace_back( 0x10400, 0x1044F, SvxResId(RID_SUBSETSTR_DESERET) );
                    break;
                case UBLOCK_BYZANTINE_MUSICAL_SYMBOLS:
                    aAllSubsets.emplace_back( 0x1D000, 0x1D0FF, SvxResId(RID_SUBSETSTR_BYZANTINE_MUSICAL_SYMBOLS) );
                    break;
                case UBLOCK_MUSICAL_SYMBOLS:
                    aAllSubsets.emplace_back( 0x1D100, 0x1D1FF, SvxResId(RID_SUBSETSTR_MUSICAL_SYMBOLS) );
                    break;
                case UBLOCK_MATHEMATICAL_ALPHANUMERIC_SYMBOLS:
                    aAllSubsets.emplace_back( 0x1D400, 0x1D7FF, SvxResId(RID_SUBSETSTR_MATHEMATICAL_ALPHANUMERIC_SYMBOLS) );
                    break;
                case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B:
                    aAllSubsets.emplace_back( 0x20000, 0x2A6DF, SvxResId(RID_SUBSETSTR_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B) );
                    break;
                case UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT:
                    aAllSubsets.emplace_back( 0x2F800, 0x2FA1F, SvxResId(RID_SUBSETSTR_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT) );
                    break;
                case UBLOCK_TAGS:
                    aAllSubsets.emplace_back( 0xE0000, 0xE007F, SvxResId(RID_SUBSETSTR_TAGS) );
                    break;
                case UBLOCK_CYRILLIC_SUPPLEMENTARY:
                    aAllSubsets.emplace_back( 0x0500, 0x052F, SvxResId(RID_SUBSETSTR_CYRILLIC_SUPPLEMENTARY) );
                    break;
                case UBLOCK_TAGALOG:
                    aAllSubsets.emplace_back( 0x1700, 0x171F, SvxResId(RID_SUBSETSTR_TAGALOG) );
                    break;
                case UBLOCK_HANUNOO:
                    aAllSubsets.emplace_back( 0x1720, 0x173F, SvxResId(RID_SUBSETSTR_HANUNOO) );
                    break;
                case UBLOCK_BUHID:
                    aAllSubsets.emplace_back( 0x1740, 0x175F, SvxResId(RID_SUBSETSTR_BUHID) );
                    break;
                case UBLOCK_TAGBANWA:
                    aAllSubsets.emplace_back( 0x1760, 0x177F, SvxResId(RID_SUBSETSTR_TAGBANWA) );
                    break;
                case UBLOCK_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A:
                    aAllSubsets.emplace_back( 0x27C0, 0x27EF, SvxResId(RID_SUBSETSTR_MISC_MATH_SYMS_A) );
                    break;
                case UBLOCK_SUPPLEMENTAL_ARROWS_A:
                    aAllSubsets.emplace_back( 0x27F0, 0x27FF, SvxResId(RID_SUBSETSTR_SUPPL_ARROWS_A) );
                    break;
                case UBLOCK_SUPPLEMENTAL_ARROWS_B:
                    aAllSubsets.emplace_back( 0x2900, 0x297F, SvxResId(RID_SUBSETSTR_SUPPL_ARROWS_B) );
                    break;
                case UBLOCK_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B:
                    aAllSubsets.emplace_back( 0x2980, 0x29FF, SvxResId(RID_SUBSETSTR_MISC_MATH_SYMS_B) );
                    break;
                case UBLOCK_SUPPLEMENTAL_MATHEMATICAL_OPERATORS:
                    aAllSubsets.emplace_back( 0x2A00, 0x2AFF, SvxResId(RID_SUBSETSTR_MISC_MATH_SYMS_B) );
                    break;
                case UBLOCK_KATAKANA_PHONETIC_EXTENSIONS:
                    aAllSubsets.emplace_back( 0x31F0, 0x31FF, SvxResId(RID_SUBSETSTR_KATAKANA_PHONETIC) );
                    break;
                case UBLOCK_VARIATION_SELECTORS:
                    aAllSubsets.emplace_back( 0xFE00, 0xFE0F, SvxResId(RID_SUBSETSTR_VARIATION_SELECTORS) );
                    break;
                case UBLOCK_SUPPLEMENTARY_PRIVATE_USE_AREA_A:
                    aAllSubsets.emplace_back( 0xF0000, 0xFFFFF, SvxResId(RID_SUBSETSTR_SUPPLEMENTARY_PRIVATE_USE_AREA_A) );
                    break;
                case UBLOCK_SUPPLEMENTARY_PRIVATE_USE_AREA_B:
                    aAllSubsets.emplace_back( 0x100000, 0x10FFFF, SvxResId(RID_SUBSETSTR_SUPPLEMENTARY_PRIVATE_USE_AREA_B) );
                    break;
                case UBLOCK_LIMBU:
                    aAllSubsets.emplace_back( 0x1900, 0x194F, SvxResId(RID_SUBSETSTR_LIMBU) );
                    break;
                case UBLOCK_TAI_LE:
                    aAllSubsets.emplace_back( 0x1950, 0x197F, SvxResId(RID_SUBSETSTR_TAI_LE) );
                    break;
                case UBLOCK_KHMER_SYMBOLS:
                    aAllSubsets.emplace_back( 0x19E0, 0x19FF, SvxResId(RID_SUBSETSTR_KHMER_SYMBOLS) );
                    break;
                case UBLOCK_PHONETIC_EXTENSIONS:
                    aAllSubsets.emplace_back( 0x1D00, 0x1D7F, SvxResId(RID_SUBSETSTR_PHONETIC_EXTENSIONS) );
                    break;
                case UBLOCK_MISCELLANEOUS_SYMBOLS_AND_ARROWS:
                    aAllSubsets.emplace_back( 0x2B00, 0x2BFF, SvxResId(RID_SUBSETSTR_MISCELLANEOUS_SYMBOLS_AND_ARROWS) );
                    break;
                case UBLOCK_YIJING_HEXAGRAM_SYMBOLS:
                    aAllSubsets.emplace_back( 0x4DC0, 0x4DFF, SvxResId(RID_SUBSETSTR_YIJING_HEXAGRAM_SYMBOLS) );
                    break;
                case UBLOCK_LINEAR_B_SYLLABARY:
                    aAllSubsets.emplace_back( 0x10000, 0x1007F, SvxResId(RID_SUBSETSTR_LINEAR_B_SYLLABARY) );
                    break;
                case UBLOCK_LINEAR_B_IDEOGRAMS:
                    aAllSubsets.emplace_back( 0x10080, 0x100FF, SvxResId(RID_SUBSETSTR_LINEAR_B_IDEOGRAMS) );
                    break;
                case UBLOCK_AEGEAN_NUMBERS:
                    aAllSubsets.emplace_back( 0x10100, 0x1013F, SvxResId(RID_SUBSETSTR_AEGEAN_NUMBERS) );
                    break;
                case UBLOCK_UGARITIC:
                    aAllSubsets.emplace_back( 0x10380, 0x1039F, SvxResId(RID_SUBSETSTR_UGARITIC) );
                    break;
                case UBLOCK_SHAVIAN:
                    aAllSubsets.emplace_back( 0x10450, 0x1047F, SvxResId(RID_SUBSETSTR_SHAVIAN) );
                    break;
                case UBLOCK_OSMANYA:
                    aAllSubsets.emplace_back( 0x10480, 0x104AF, SvxResId(RID_SUBSETSTR_OSMANYA) );
                    break;
                case UBLOCK_CYPRIOT_SYLLABARY:
                    aAllSubsets.emplace_back( 0x10800, 0x1083F, SvxResId(RID_SUBSETSTR_CYPRIOT_SYLLABARY) );
                    break;
                case UBLOCK_TAI_XUAN_JING_SYMBOLS:
                    aAllSubsets.emplace_back( 0x1D300, 0x1D35F, SvxResId(RID_SUBSETSTR_TAI_XUAN_JING_SYMBOLS) );
                    break;
                case UBLOCK_VARIATION_SELECTORS_SUPPLEMENT:
                    aAllSubsets.emplace_back( 0xE0100, 0xE01EF, SvxResId(RID_SUBSETSTR_VARIATION_SELECTORS_SUPPLEMENT) );
                    break;
                case UBLOCK_ANCIENT_GREEK_MUSICAL_NOTATION:
                    aAllSubsets.emplace_back(0x1D200, 0x1D24F, SvxResId(RID_SUBSETSTR_ANCIENT_GREEK_MUSICAL_NOTATION) );
                    break;
                case UBLOCK_ANCIENT_GREEK_NUMBERS:
                    aAllSubsets.emplace_back(0x10140, 0x1018F , SvxResId(RID_SUBSETSTR_ANCIENT_GREEK_NUMBERS) );
                    break;
                case UBLOCK_ARABIC_SUPPLEMENT:
                    aAllSubsets.emplace_back(0x0750, 0x077F , SvxResId(RID_SUBSETSTR_ARABIC_SUPPLEMENT) );
                    break;
                case UBLOCK_BUGINESE:
                    aAllSubsets.emplace_back(0x1A00, 0x1A1F , SvxResId(RID_SUBSETSTR_BUGINESE) );
                    break;
                case UBLOCK_CJK_STROKES:
                    aAllSubsets.emplace_back( 0x31C0, 0x31EF, SvxResId(RID_SUBSETSTR_CJK_STROKES) );
                    break;
                case UBLOCK_COMBINING_DIACRITICAL_MARKS_SUPPLEMENT:
                    aAllSubsets.emplace_back( 0x1DC0, 0x1DFF , SvxResId(RID_SUBSETSTR_COMBINING_DIACRITICAL_MARKS_SUPPLEMENT) );
                    break;
                case UBLOCK_COPTIC:
                    aAllSubsets.emplace_back( 0x2C80, 0x2CFF , SvxResId(RID_SUBSETSTR_COPTIC) );
                    break;
                case UBLOCK_ETHIOPIC_EXTENDED:
                    aAllSubsets.emplace_back( 0x2D80, 0x2DDF , SvxResId(RID_SUBSETSTR_ETHIOPIC_EXTENDED) );
                    break;
                case UBLOCK_ETHIOPIC_SUPPLEMENT:
                    aAllSubsets.emplace_back( 0x1380, 0x139F, SvxResId(RID_SUBSETSTR_ETHIOPIC_SUPPLEMENT) );
                    break;
                case UBLOCK_GEORGIAN_SUPPLEMENT:
                    aAllSubsets.emplace_back( 0x2D00, 0x2D2F, SvxResId(RID_SUBSETSTR_GEORGIAN_SUPPLEMENT) );
                    break;
                case UBLOCK_GLAGOLITIC:
                    aAllSubsets.emplace_back( 0x2C00, 0x2C5F, SvxResId(RID_SUBSETSTR_GLAGOLITIC) );
                    break;
                case UBLOCK_KHAROSHTHI:
                    aAllSubsets.emplace_back( 0x10A00, 0x10A5F, SvxResId(RID_SUBSETSTR_KHAROSHTHI) );
                    break;
                case UBLOCK_MODIFIER_TONE_LETTERS:
                    aAllSubsets.emplace_back( 0xA700, 0xA71F, SvxResId(RID_SUBSETSTR_MODIFIER_TONE_LETTERS) );
                    break;
                case UBLOCK_NEW_TAI_LUE:
                    aAllSubsets.emplace_back( 0x1980, 0x19DF, SvxResId(RID_SUBSETSTR_NEW_TAI_LUE) );
                    break;
                case UBLOCK_OLD_PERSIAN:
                    aAllSubsets.emplace_back( 0x103A0, 0x103DF, SvxResId(RID_SUBSETSTR_OLD_PERSIAN) );
                    break;
                case UBLOCK_PHONETIC_EXTENSIONS_SUPPLEMENT:
                    aAllSubsets.emplace_back( 0x1D80, 0x1DBF, SvxResId(RID_SUBSETSTR_PHONETIC_EXTENSIONS_SUPPLEMENT) );
                    break;
                case UBLOCK_SUPPLEMENTAL_PUNCTUATION:
                    aAllSubsets.emplace_back( 0x2E00, 0x2E7F, SvxResId(RID_SUBSETSTR_SUPPLEMENTAL_PUNCTUATION) );
                    break;
                case UBLOCK_SYLOTI_NAGRI:
                    aAllSubsets.emplace_back( 0xA800, 0xA82F, SvxResId(RID_SUBSETSTR_SYLOTI_NAGRI) );
                    break;
                case UBLOCK_TIFINAGH:
                    aAllSubsets.emplace_back( 0x2D30, 0x2D7F, SvxResId(RID_SUBSETSTR_TIFINAGH) );
                    break;
                case UBLOCK_VERTICAL_FORMS:
                    aAllSubsets.emplace_back( 0xFE10, 0xFE1F, SvxResId(RID_SUBSETSTR_VERTICAL_FORMS) );
                    break;
                case UBLOCK_NKO:
                    aAllSubsets.emplace_back( 0x07C0, 0x07FF, SvxResId(RID_SUBSETSTR_NKO) );
                    break;
                case UBLOCK_BALINESE:
                    aAllSubsets.emplace_back( 0x1B00, 0x1B7F, SvxResId(RID_SUBSETSTR_BALINESE) );
                    break;
                case UBLOCK_LATIN_EXTENDED_C:
                    aAllSubsets.emplace_back( 0x2C60, 0x2C7F, SvxResId(RID_SUBSETSTR_LATIN_EXTENDED_C) );
                    break;
                case UBLOCK_LATIN_EXTENDED_D:
                    aAllSubsets.emplace_back( 0xA720, 0xA7FF, SvxResId(RID_SUBSETSTR_LATIN_EXTENDED_D) );
                    break;
                case UBLOCK_PHAGS_PA:
                    aAllSubsets.emplace_back( 0xA840, 0xA87F, SvxResId(RID_SUBSETSTR_PHAGS_PA) );
                    break;
                case UBLOCK_PHOENICIAN:
                    aAllSubsets.emplace_back( 0x10900, 0x1091F, SvxResId(RID_SUBSETSTR_PHOENICIAN) );
                    break;
                case UBLOCK_CUNEIFORM:
                    aAllSubsets.emplace_back( 0x12000, 0x123FF, SvxResId(RID_SUBSETSTR_CUNEIFORM) );
                    break;
                case UBLOCK_CUNEIFORM_NUMBERS_AND_PUNCTUATION:
                    aAllSubsets.emplace_back( 0x12400, 0x1247F, SvxResId(RID_SUBSETSTR_CUNEIFORM_NUMBERS_AND_PUNCTUATION) );
                    break;
                case UBLOCK_COUNTING_ROD_NUMERALS:
                    aAllSubsets.emplace_back( 0x1D360, 0x1D37F, SvxResId(RID_SUBSETSTR_COUNTING_ROD_NUMERALS) );
                    break;
                case UBLOCK_SUNDANESE:
                    aAllSubsets.emplace_back( 0x1B80, 0x1BBF, SvxResId(RID_SUBSETSTR_SUNDANESE) );
                    break;
                case UBLOCK_LEPCHA:
                    aAllSubsets.emplace_back( 0x1C00, 0x1C4F, SvxResId(RID_SUBSETSTR_LEPCHA) );
                    break;
                case UBLOCK_OL_CHIKI:
                    aAllSubsets.emplace_back( 0x1C50, 0x1C7F, SvxResId(RID_SUBSETSTR_OL_CHIKI) );
                    break;
                case UBLOCK_CYRILLIC_EXTENDED_A:
                    aAllSubsets.emplace_back( 0x2DE0, 0x2DFF, SvxResId(RID_SUBSETSTR_CYRILLIC_EXTENDED_A) );
                    break;
                case UBLOCK_VAI:
                    aAllSubsets.emplace_back( 0xA500, 0xA63F, SvxResId(RID_SUBSETSTR_VAI) );
                    break;
                case UBLOCK_CYRILLIC_EXTENDED_B:
                    aAllSubsets.emplace_back( 0xA640, 0xA69F, SvxResId(RID_SUBSETSTR_CYRILLIC_EXTENDED_B) );
                    break;
                case UBLOCK_SAURASHTRA:
                    aAllSubsets.emplace_back( 0xA880, 0xA8DF, SvxResId(RID_SUBSETSTR_SAURASHTRA) );
                    break;
                case UBLOCK_KAYAH_LI:
                    aAllSubsets.emplace_back( 0xA900, 0xA92F, SvxResId(RID_SUBSETSTR_KAYAH_LI) );
                    break;
                case UBLOCK_REJANG:
                    aAllSubsets.emplace_back( 0xA930, 0xA95F, SvxResId(RID_SUBSETSTR_REJANG) );
                    break;
                case UBLOCK_CHAM:
                    aAllSubsets.emplace_back( 0xAA00, 0xAA5F, SvxResId(RID_SUBSETSTR_CHAM) );
                    break;
                case UBLOCK_ANCIENT_SYMBOLS:
                    aAllSubsets.emplace_back( 0x10190, 0x101CF, SvxResId(RID_SUBSETSTR_ANCIENT_SYMBOLS) );
                    break;
                case UBLOCK_PHAISTOS_DISC:
                    aAllSubsets.emplace_back( 0x101D0, 0x101FF, SvxResId(RID_SUBSETSTR_PHAISTOS_DISC) );
                    break;
                case UBLOCK_LYCIAN:
                    aAllSubsets.emplace_back( 0x10280, 0x1029F, SvxResId(RID_SUBSETSTR_LYCIAN) );
                    break;
                case UBLOCK_CARIAN:
                    aAllSubsets.emplace_back( 0x102A0, 0x102DF, SvxResId(RID_SUBSETSTR_CARIAN) );
                    break;
                case UBLOCK_LYDIAN:
                    aAllSubsets.emplace_back( 0x10920, 0x1093F, SvxResId(RID_SUBSETSTR_LYDIAN) );
                    break;
                case UBLOCK_MAHJONG_TILES:
                    aAllSubsets.emplace_back( 0x1F000, 0x1F02F, SvxResId(RID_SUBSETSTR_MAHJONG_TILES) );
                    break;
                case UBLOCK_DOMINO_TILES:
                    aAllSubsets.emplace_back( 0x1F030, 0x1F09F, SvxResId(RID_SUBSETSTR_DOMINO_TILES) );
                    break;
                case UBLOCK_SAMARITAN:
                    aAllSubsets.emplace_back( 0x0800, 0x083F, SvxResId(RID_SUBSETSTR_SAMARITAN) );
                    break;
                case UBLOCK_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED:
                    aAllSubsets.emplace_back( 0x18B0, 0x18FF, SvxResId(RID_SUBSETSTR_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED) );
                    break;
                case UBLOCK_TAI_THAM:
                    aAllSubsets.emplace_back( 0x1A20, 0x1AAF, SvxResId(RID_SUBSETSTR_TAI_THAM) );
                    break;
                case UBLOCK_VEDIC_EXTENSIONS:
                    aAllSubsets.emplace_back( 0x1CD0, 0x1CFF, SvxResId(RID_SUBSETSTR_VEDIC_EXTENSIONS) );
                    break;
                case UBLOCK_LISU:
                    aAllSubsets.emplace_back( 0xA4D0, 0xA4FF, SvxResId(RID_SUBSETSTR_LISU) );
                    break;
                case UBLOCK_BAMUM:
                    aAllSubsets.emplace_back( 0xA6A0, 0xA6FF, SvxResId(RID_SUBSETSTR_BAMUM) );
                    break;
                case UBLOCK_COMMON_INDIC_NUMBER_FORMS:
                    aAllSubsets.emplace_back( 0xA830, 0xA83F, SvxResId(RID_SUBSETSTR_COMMON_INDIC_NUMBER_FORMS) );
                    break;
                case UBLOCK_DEVANAGARI_EXTENDED:
                    aAllSubsets.emplace_back( 0xA8E0, 0xA8FF, SvxResId(RID_SUBSETSTR_DEVANAGARI_EXTENDED) );
                    break;
                case UBLOCK_HANGUL_JAMO_EXTENDED_A:
                    aAllSubsets.emplace_back( 0xA960, 0xA97F, SvxResId(RID_SUBSETSTR_HANGUL_JAMO_EXTENDED_A) );
                    break;
                case UBLOCK_JAVANESE:
                    aAllSubsets.emplace_back( 0xA980, 0xA9DF, SvxResId(RID_SUBSETSTR_JAVANESE) );
                    break;
                case UBLOCK_MYANMAR_EXTENDED_A:
                    aAllSubsets.emplace_back( 0xAA60, 0xAA7F, SvxResId(RID_SUBSETSTR_MYANMAR_EXTENDED_A) );
                    break;
                case UBLOCK_TAI_VIET:
                    aAllSubsets.emplace_back( 0xAA80, 0xAADF, SvxResId(RID_SUBSETSTR_TAI_VIET) );
                    break;
                case UBLOCK_MEETEI_MAYEK:
                    aAllSubsets.emplace_back( 0xABC0, 0xABFF, SvxResId(RID_SUBSETSTR_MEETEI_MAYEK) );
                    break;
                case UBLOCK_HANGUL_JAMO_EXTENDED_B:
                    aAllSubsets.emplace_back( 0xD7B0, 0xD7FF, SvxResId(RID_SUBSETSTR_HANGUL_JAMO_EXTENDED_B) );
                    break;
                case UBLOCK_IMPERIAL_ARAMAIC:
                    aAllSubsets.emplace_back( 0x10840, 0x1085F, SvxResId(RID_SUBSETSTR_IMPERIAL_ARAMAIC) );
                    break;
                case UBLOCK_OLD_SOUTH_ARABIAN:
                    aAllSubsets.emplace_back( 0x10A60, 0x10A7F, SvxResId(RID_SUBSETSTR_OLD_SOUTH_ARABIAN) );
                    break;
                case UBLOCK_AVESTAN:
                    aAllSubsets.emplace_back( 0x10B00, 0x10B3F, SvxResId(RID_SUBSETSTR_AVESTAN) );
                    break;
                case UBLOCK_INSCRIPTIONAL_PARTHIAN:
                    aAllSubsets.emplace_back( 0x10B40, 0x10B5F, SvxResId(RID_SUBSETSTR_INSCRIPTIONAL_PARTHIAN) );
                    break;
                case UBLOCK_INSCRIPTIONAL_PAHLAVI:
                    aAllSubsets.emplace_back( 0x10B60, 0x10B7F, SvxResId(RID_SUBSETSTR_INSCRIPTIONAL_PAHLAVI) );
                    break;
                case UBLOCK_OLD_TURKIC:
                    aAllSubsets.emplace_back( 0x10C00, 0x10C4F, SvxResId(RID_SUBSETSTR_OLD_TURKIC) );
                    break;
                case UBLOCK_RUMI_NUMERAL_SYMBOLS:
                    aAllSubsets.emplace_back( 0x10E60, 0x10E7F, SvxResId(RID_SUBSETSTR_RUMI_NUMERAL_SYMBOLS) );
                    break;
                case UBLOCK_KAITHI:
                    aAllSubsets.emplace_back( 0x11080, 0x110CF, SvxResId(RID_SUBSETSTR_KAITHI) );
                    break;
                case UBLOCK_EGYPTIAN_HIEROGLYPHS:
                    aAllSubsets.emplace_back( 0x13000, 0x1342F, SvxResId(RID_SUBSETSTR_EGYPTIAN_HIEROGLYPHS) );
                    break;
                case UBLOCK_ENCLOSED_ALPHANUMERIC_SUPPLEMENT:
                    aAllSubsets.emplace_back( 0x1F100, 0x1F1FF, SvxResId(RID_SUBSETSTR_ENCLOSED_ALPHANUMERIC_SUPPLEMENT) );
                    break;
                case UBLOCK_ENCLOSED_IDEOGRAPHIC_SUPPLEMENT:
                    aAllSubsets.emplace_back( 0x1F200, 0x1F2FF, SvxResId(RID_SUBSETSTR_ENCLOSED_IDEOGRAPHIC_SUPPLEMENT) );
                    break;
                case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C:
                    aAllSubsets.emplace_back( 0x2A700, 0x2B73F, SvxResId(RID_SUBSETSTR_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C) );
                    break;
                case UBLOCK_MANDAIC:
                    aAllSubsets.emplace_back( 0x0840, 0x085F, SvxResId(RID_SUBSETSTR_MANDAIC) );
                    break;
                case UBLOCK_BATAK:
                    aAllSubsets.emplace_back( 0x1BC0, 0x1BFF, SvxResId(RID_SUBSETSTR_BATAK) );
                    break;
                case UBLOCK_ETHIOPIC_EXTENDED_A:
                    aAllSubsets.emplace_back( 0xAB00, 0xAB2F, SvxResId(RID_SUBSETSTR_ETHIOPIC_EXTENDED_A) );
                    break;
                case UBLOCK_BRAHMI:
                    aAllSubsets.emplace_back( 0x11000, 0x1107F, SvxResId(RID_SUBSETSTR_BRAHMI) );
                    break;
                case UBLOCK_BAMUM_SUPPLEMENT:
                    aAllSubsets.emplace_back( 0x16800, 0x16A3F, SvxResId(RID_SUBSETSTR_BAMUM_SUPPLEMENT) );
                    break;
                case UBLOCK_KANA_SUPPLEMENT:
                    aAllSubsets.emplace_back( 0x1B000, 0x1B0FF, SvxResId(RID_SUBSETSTR_KANA_SUPPLEMENT) );
                    break;
                case UBLOCK_PLAYING_CARDS:
                    aAllSubsets.emplace_back( 0x1F0A0, 0x1F0FF, SvxResId(RID_SUBSETSTR_PLAYING_CARDS) );
                    break;
                case UBLOCK_MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS:
                    aAllSubsets.emplace_back( 0x1F300, 0x1F5FF, SvxResId(RID_SUBSETSTR_MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS) );
                    break;
                case UBLOCK_EMOTICONS:
                    aAllSubsets.emplace_back( 0x1F600, 0x1F64F, SvxResId(RID_SUBSETSTR_EMOTICONS) );
                    break;
                case UBLOCK_TRANSPORT_AND_MAP_SYMBOLS:
                    aAllSubsets.emplace_back( 0x1F680, 0x1F6FF, SvxResId(RID_SUBSETSTR_TRANSPORT_AND_MAP_SYMBOLS) );
                    break;
                case UBLOCK_ALCHEMICAL_SYMBOLS:
                    aAllSubsets.emplace_back( 0x1F700, 0x1F77F, SvxResId(RID_SUBSETSTR_ALCHEMICAL_SYMBOLS) );
                    break;
                case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D:
                    aAllSubsets.emplace_back( 0x2B740, 0x2B81F, SvxResId(RID_SUBSETSTR_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D) );
                    break;
// Note ICU version 49 (NOT 4.9), so the MAJOR_NUM is two digits.
#if U_ICU_VERSION_MAJOR_NUM >= 49
                case UBLOCK_ARABIC_EXTENDED_A:
                    aAllSubsets.emplace_back( 0x08A0, 0x08FF, SvxResId(RID_SUBSETSTR_ARABIC_EXTENDED_A) );
                    break;
                case UBLOCK_ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS:
                    aAllSubsets.emplace_back( 0x1EE00, 0x1EEFF, SvxResId(RID_SUBSETSTR_ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS) );
                    break;
                case UBLOCK_CHAKMA:
                    aAllSubsets.emplace_back( 0x11100, 0x1114F, SvxResId(RID_SUBSETSTR_CHAKMA) );
                    break;
                case UBLOCK_MEETEI_MAYEK_EXTENSIONS:
                    aAllSubsets.emplace_back( 0xAAE0, 0xAAFF, SvxResId(RID_SUBSETSTR_MEETEI_MAYEK_EXTENSIONS) );
                    break;
                case UBLOCK_MEROITIC_CURSIVE:
                    aAllSubsets.emplace_back( 0x109A0, 0x109FF, SvxResId(RID_SUBSETSTR_MEROITIC_CURSIVE) );
                    break;
                case UBLOCK_MEROITIC_HIEROGLYPHS:
                    aAllSubsets.emplace_back( 0x10980, 0x1099F, SvxResId(RID_SUBSETSTR_MEROITIC_HIEROGLYPHS) );
                    break;
                case UBLOCK_MIAO:
                    aAllSubsets.emplace_back( 0x16F00, 0x16F9F, SvxResId(RID_SUBSETSTR_MIAO) );
                    break;
                case UBLOCK_SHARADA:
                    aAllSubsets.emplace_back( 0x11180, 0x111DF, SvxResId(RID_SUBSETSTR_SHARADA) );
                    break;
                case UBLOCK_SORA_SOMPENG:
                    aAllSubsets.emplace_back( 0x110D0, 0x110FF, SvxResId(RID_SUBSETSTR_SORA_SOMPENG) );
                    break;
                case UBLOCK_SUNDANESE_SUPPLEMENT:
                    aAllSubsets.emplace_back( 0x1CC0, 0x1CCF, SvxResId(RID_SUBSETSTR_SUNDANESE_SUPPLEMENT) );
                    break;
                case UBLOCK_TAKRI:
                    aAllSubsets.emplace_back( 0x11680, 0x116CF, SvxResId(RID_SUBSETSTR_TAKRI) );
                    break;
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 54
                case UBLOCK_BASSA_VAH:
                    aAllSubsets.emplace_back( 0x16AD0, 0x16AFF, SvxResId(RID_SUBSETSTR_BASSA_VAH) );
                    break;
                case UBLOCK_CAUCASIAN_ALBANIAN:
                    aAllSubsets.emplace_back( 0x10530, 0x1056F, SvxResId(RID_SUBSETSTR_CAUCASIAN_ALBANIAN) );
                    break;
                case UBLOCK_COPTIC_EPACT_NUMBERS:
                    aAllSubsets.emplace_back( 0x102E0, 0x102FF, SvxResId(RID_SUBSETSTR_COPTIC_EPACT_NUMBERS) );
                    break;
                case UBLOCK_COMBINING_DIACRITICAL_MARKS_EXTENDED:
                    aAllSubsets.emplace_back( 0x1AB0, 0x1AFF, SvxResId(RID_SUBSETSTR_COMBINING_DIACRITICAL_MARKS_EXTENDED) );
                    break;
                case UBLOCK_DUPLOYAN:
                    aAllSubsets.emplace_back( 0x1BC00, 0x1BC9F, SvxResId(RID_SUBSETSTR_DUPLOYAN) );
                    break;
                case UBLOCK_ELBASAN:
                    aAllSubsets.emplace_back( 0x10500, 0x1052F, SvxResId(RID_SUBSETSTR_ELBASAN) );
                    break;
                case UBLOCK_GEOMETRIC_SHAPES_EXTENDED:
                    aAllSubsets.emplace_back( 0x1F780, 0x1F7FF, SvxResId(RID_SUBSETSTR_GEOMETRIC_SHAPES_EXTENDED) );
                    break;
                case UBLOCK_GRANTHA:
                    aAllSubsets.emplace_back( 0x11300, 0x1137F, SvxResId(RID_SUBSETSTR_GRANTHA) );
                    break;
                case UBLOCK_KHOJKI:
                    aAllSubsets.emplace_back( 0x11200, 0x1124F, SvxResId(RID_SUBSETSTR_KHOJKI) );
                    break;
                case UBLOCK_KHUDAWADI:
                    aAllSubsets.emplace_back( 0x112B0, 0x112FF, SvxResId(RID_SUBSETSTR_KHUDAWADI) );
                    break;
                case UBLOCK_LATIN_EXTENDED_E:
                    aAllSubsets.emplace_back( 0xAB30, 0xAB6F, SvxResId(RID_SUBSETSTR_LATIN_EXTENDED_E) );
                    break;
                case UBLOCK_LINEAR_A:
                    aAllSubsets.emplace_back( 0x10600, 0x1077F, SvxResId(RID_SUBSETSTR_LINEAR_A) );
                    break;
                case UBLOCK_MAHAJANI:
                    aAllSubsets.emplace_back( 0x11150, 0x1117F, SvxResId(RID_SUBSETSTR_MAHAJANI) );
                    break;
                case UBLOCK_MANICHAEAN:
                    aAllSubsets.emplace_back( 0x10AC0, 0x10AFF, SvxResId(RID_SUBSETSTR_MANICHAEAN) );
                    break;
                case UBLOCK_MENDE_KIKAKUI:
                    aAllSubsets.emplace_back( 0x1E800, 0x1E8DF, SvxResId(RID_SUBSETSTR_MENDE_KIKAKUI) );
                    break;
                case UBLOCK_MODI:
                    aAllSubsets.emplace_back( 0x11600, 0x1165F, SvxResId(RID_SUBSETSTR_MODI) );
                    break;
                case UBLOCK_MRO:
                    aAllSubsets.emplace_back( 0x16A40, 0x16A6F, SvxResId(RID_SUBSETSTR_MRO) );
                    break;
                case UBLOCK_MYANMAR_EXTENDED_B:
                    aAllSubsets.emplace_back( 0xA9E0, 0xA9FF, SvxResId(RID_SUBSETSTR_MYANMAR_EXTENDED_B) );
                    break;
                case UBLOCK_NABATAEAN:
                    aAllSubsets.emplace_back( 0x10880, 0x108AF, SvxResId(RID_SUBSETSTR_NABATAEAN) );
                    break;
                case UBLOCK_OLD_NORTH_ARABIAN:
                    aAllSubsets.emplace_back( 0x10A80, 0x10A9F, SvxResId(RID_SUBSETSTR_OLD_NORTH_ARABIAN) );
                    break;
                case UBLOCK_OLD_PERMIC:
                    aAllSubsets.emplace_back( 0x10350, 0x1037F, SvxResId(RID_SUBSETSTR_OLD_PERMIC) );
                    break;
                case UBLOCK_ORNAMENTAL_DINGBATS:
                    aAllSubsets.emplace_back( 0x1F650, 0x1F67F, SvxResId(RID_SUBSETSTR_ORNAMENTAL_DINGBATS) );
                    break;
                case UBLOCK_PAHAWH_HMONG:
                    aAllSubsets.emplace_back( 0x16B00, 0x16B8F, SvxResId(RID_SUBSETSTR_PAHAWH_HMONG) );
                    break;
                case UBLOCK_PALMYRENE:
                    aAllSubsets.emplace_back( 0x10860, 0x1087F, SvxResId(RID_SUBSETSTR_PALMYRENE) );
                    break;
                case UBLOCK_PAU_CIN_HAU:
                    aAllSubsets.emplace_back( 0x11AC0, 0x11AFF, SvxResId(RID_SUBSETSTR_PAU_CIN_HAU) );
                    break;
                case UBLOCK_PSALTER_PAHLAVI:
                    aAllSubsets.emplace_back( 0x10B80, 0x10BAF, SvxResId(RID_SUBSETSTR_PSALTER_PAHLAVI) );
                    break;
                case UBLOCK_SHORTHAND_FORMAT_CONTROLS:
                    aAllSubsets.emplace_back( 0x1BCA0, 0x1BCAF, SvxResId(RID_SUBSETSTR_SHORTHAND_FORMAT_CONTROLS) );
                    break;
                case UBLOCK_SIDDHAM:
                    aAllSubsets.emplace_back( 0x11580, 0x115FF, SvxResId(RID_SUBSETSTR_SIDDHAM) );
                    break;
                case UBLOCK_SINHALA_ARCHAIC_NUMBERS:
                    aAllSubsets.emplace_back( 0x111E0, 0x111FF, SvxResId(RID_SUBSETSTR_SINHALA_ARCHAIC_NUMBERS) );
                    break;
                case UBLOCK_SUPPLEMENTAL_ARROWS_C:
                    aAllSubsets.emplace_back( 0x1F800, 0x1F8FF, SvxResId(RID_SUBSETSTR_SUPPLEMENTAL_ARROWS_C) );
                    break;
                case UBLOCK_TIRHUTA:
                    aAllSubsets.emplace_back( 0x11480, 0x114DF, SvxResId(RID_SUBSETSTR_TIRHUTA) );
                    break;
                case UBLOCK_WARANG_CITI:
                    aAllSubsets.emplace_back( 0x118A0, 0x118FF, SvxResId(RID_SUBSETSTR_WARANG_CITI) );
                    break;
#endif
#if U_ICU_VERSION_MAJOR_NUM >= 56
                case UBLOCK_AHOM:
                    aAllSubsets.emplace_back( 0x11700, 0x1173F, SvxResId(RID_SUBSETSTR_AHOM) );
                    break;
                case UBLOCK_ANATOLIAN_HIEROGLYPHS:
                    aAllSubsets.emplace_back( 0x14400, 0x1467F, SvxResId(RID_SUBSETSTR_ANATOLIAN_HIEROGLYPHS) );
                    break;
                case UBLOCK_CHEROKEE_SUPPLEMENT:
                    aAllSubsets.emplace_back( 0xAB70, 0xABBF, SvxResId(RID_SUBSETSTR_CHEROKEE_SUPPLEMENT) );
                    break;
                case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_E:
                    aAllSubsets.emplace_back( 0x2B820, 0x2CEAF, SvxResId(RID_SUBSETSTR_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_E) );
                    break;
                case UBLOCK_EARLY_DYNASTIC_CUNEIFORM:
                    aAllSubsets.emplace_back( 0x12480, 0x1254F, SvxResId(RID_SUBSETSTR_EARLY_DYNASTIC_CUNEIFORM) );
                    break;
                case UBLOCK_HATRAN:
                    aAllSubsets.emplace_back( 0x108E0, 0x108FF, SvxResId(RID_SUBSETSTR_HATRAN) );
                    break;
                case UBLOCK_MULTANI:
                    aAllSubsets.emplace_back( 0x11280, 0x112AF, SvxResId(RID_SUBSETSTR_MULTANI) );
                    break;
                case UBLOCK_OLD_HUNGARIAN:
                    aAllSubsets.emplace_back( 0x10C80, 0x10CFF, SvxResId(RID_SUBSETSTR_OLD_HUNGARIAN) );
                    break;
                case UBLOCK_SUPPLEMENTAL_SYMBOLS_AND_PICTOGRAPHS:
                    aAllSubsets.emplace_back( 0x1F900, 0x1F9FF, SvxResId(RID_SUBSETSTR_SUPPLEMENTAL_SYMBOLS_AND_PICTOGRAPHS) );
                    break;
                case UBLOCK_SUTTON_SIGNWRITING:
                    aAllSubsets.emplace_back( 0x1D800, 0x1DAAF, SvxResId(RID_SUBSETSTR_SUTTON_SIGNWRITING) );
                    break;
#endif
#if (U_ICU_VERSION_MAJOR_NUM >= 58)
                case UBLOCK_ADLAM:
                    aAllSubsets.emplace_back( 0x1E900, 0x1E95F, SvxResId(RID_SUBSETSTR_ADLAM) );
                    break;
                case UBLOCK_BHAIKSUKI:
                    aAllSubsets.emplace_back( 0x11C00, 0x11C6F, SvxResId(RID_SUBSETSTR_BHAIKSUKI) );
                    break;
                case UBLOCK_CYRILLIC_EXTENDED_C:
                    aAllSubsets.emplace_back( 0x1C80, 0x1C8F, SvxResId(RID_SUBSETSTR_CYRILLIC_EXTENDED_C) );
                    break;
                case UBLOCK_GLAGOLITIC_SUPPLEMENT:
                    aAllSubsets.emplace_back( 0x1E000, 0x1E02F, SvxResId(RID_SUBSETSTR_GLAGOLITIC_SUPPLEMENT) );
                    break;
                case UBLOCK_IDEOGRAPHIC_SYMBOLS_AND_PUNCTUATION:
                    aAllSubsets.emplace_back( 0x16FE0, 0x16FFF, SvxResId(RID_SUBSETSTR_IDEOGRAPHIC_SYMBOLS_AND_PUNCTUATION) );
                    break;
                case UBLOCK_MARCHEN:
                    aAllSubsets.emplace_back( 0x11C70, 0x11CBF, SvxResId(RID_SUBSETSTR_MARCHEN) );
                    break;
                case UBLOCK_MONGOLIAN_SUPPLEMENT:
                    aAllSubsets.emplace_back( 0x11660, 0x1167F, SvxResId(RID_SUBSETSTR_MONGOLIAN_SUPPLEMENT) );
                    break;
                case UBLOCK_NEWA:
                    aAllSubsets.emplace_back( 0x11400, 0x1147F, SvxResId(RID_SUBSETSTR_NEWA) );
                    break;
                case UBLOCK_OSAGE:
                    aAllSubsets.emplace_back( 0x104B0, 0x104FF, SvxResId(RID_SUBSETSTR_OSAGE) );
                    break;
                case UBLOCK_TANGUT:
                    aAllSubsets.emplace_back( 0x17000, 0x187FF, SvxResId(RID_SUBSETSTR_TANGUT) );
                    break;
                case UBLOCK_TANGUT_COMPONENTS:
                    aAllSubsets.emplace_back( 0x18800, 0x18AFF, SvxResId(RID_SUBSETSTR_TANGUT_COMPONENTS) );
                    break;
#endif
#if (U_ICU_VERSION_MAJOR_NUM >= 60)
                case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_F:
                    aAllSubsets.emplace_back( 0x2CEB0, 0x2EBE0, SvxResId(RID_SUBSETSTR_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_F) );
                    break;
                case UBLOCK_KANA_EXTENDED_A:
                    aAllSubsets.emplace_back( 0x1B100, 0x1B12F, SvxResId(RID_SUBSETSTR_KANA_EXTENDED_A) );
                    break;
                case UBLOCK_MASARAM_GONDI:
                    aAllSubsets.emplace_back( 0x11D00, 0x11D5F, SvxResId(RID_SUBSETSTR_MASARAM_GONDI) );
                    break;
                case UBLOCK_NUSHU:
                    aAllSubsets.emplace_back( 0x1B170, 0x1B2FF, SvxResId(RID_SUBSETSTR_NUSHU) );
                    break;
                case UBLOCK_SOYOMBO:
                    aAllSubsets.emplace_back( 0x11A50, 0x11AAF, SvxResId(RID_SUBSETSTR_SOYOMBO) );
                    break;
                case UBLOCK_SYRIAC_SUPPLEMENT:
                    aAllSubsets.emplace_back( 0x0860, 0x086f, SvxResId(RID_SUBSETSTR_SYRIAC_SUPPLEMENT) );
                    break;
                case UBLOCK_ZANABAZAR_SQUARE:
                    aAllSubsets.emplace_back( 0x11A00, 0x11A4F, SvxResId(RID_SUBSETSTR_ZANABAZAR_SQUARE) );
                    break;
#endif
#if (U_ICU_VERSION_MAJOR_NUM >= 62)
                case UBLOCK_CHESS_SYMBOLS:
                    aAllSubsets.emplace_back( 0x1FA00, 0x1FA6F, SvxResId(RID_SUBSETSTR_CHESS_SYMBOLS) );
                    break;
                case UBLOCK_DOGRA:
                    aAllSubsets.emplace_back( 0x11800, 0x1184F, SvxResId(RID_SUBSETSTR_DOGRA) );
                    break;
                case UBLOCK_GEORGIAN_EXTENDED:
                    aAllSubsets.emplace_back( 0x1C90, 0x1CBF, SvxResId(RID_SUBSETSTR_GEORGIAN_EXTENDED) );
                    break;
                case UBLOCK_GUNJALA_GONDI:
                    aAllSubsets.emplace_back( 0x11D60, 0x11DAF, SvxResId(RID_SUBSETSTR_GUNJALA_GONDI) );
                    break;
                case UBLOCK_HANIFI_ROHINGYA:
                    aAllSubsets.emplace_back( 0x10D00, 0x10D3F, SvxResId(RID_SUBSETSTR_HANIFI_ROHINGYA) );
                    break;
                case UBLOCK_INDIC_SIYAQ_NUMBERS:
                    aAllSubsets.emplace_back( 0x1EC70, 0x1ECBF, SvxResId(RID_SUBSETSTR_INDIC_SIYAQ_NUMBERS) );
                    break;
                case UBLOCK_MAKASAR:
                    aAllSubsets.emplace_back( 0x11EE0, 0x11EFF, SvxResId(RID_SUBSETSTR_MAKASAR) );
                    break;
                case UBLOCK_MAYAN_NUMERALS:
                    aAllSubsets.emplace_back( 0x1D2E0, 0x1D2FF, SvxResId(RID_SUBSETSTR_MAYAN_NUMERALS) );
                    break;
                case UBLOCK_MEDEFAIDRIN:
                    aAllSubsets.emplace_back( 0x16E40, 0x16E9F, SvxResId(RID_SUBSETSTR_MEDEFAIDRIN) );
                    break;
                case UBLOCK_OLD_SOGDIAN:
                    aAllSubsets.emplace_back( 0x10F00, 0x10F2F, SvxResId(RID_SUBSETSTR_OLD_SOGDIAN) );
                    break;
                case UBLOCK_SOGDIAN:
                    aAllSubsets.emplace_back( 0x10F30, 0x10F6F, SvxResId(RID_SUBSETSTR_SOGDIAN) );
                    break;
#endif
#if (U_ICU_VERSION_MAJOR_NUM >= 64)
                case UBLOCK_EGYPTIAN_HIEROGLYPH_FORMAT_CONTROLS:
                    aAllSubsets.emplace_back( 0x13430, 0x1343F, SvxResId(RID_SUBSETSTR_EGYPTIAN_HIEROGLYPH_FORMAT_CONTROLS) );
                    break;
                case UBLOCK_ELYMAIC:
                    aAllSubsets.emplace_back( 0x10FE0, 0x10FFF, SvxResId(RID_SUBSETSTR_ELYMAIC) );
                    break;
                case UBLOCK_NANDINAGARI:
                    aAllSubsets.emplace_back( 0x119A0, 0x119FF, SvxResId(RID_SUBSETSTR_NANDINAGARI) );
                    break;
                case UBLOCK_NYIAKENG_PUACHUE_HMONG:
                    aAllSubsets.emplace_back( 0x1E100, 0x1E14F, SvxResId(RID_SUBSETSTR_NYIAKENG_PUACHUE_HMONG) );
                    break;
                case UBLOCK_OTTOMAN_SIYAQ_NUMBERS:
                    aAllSubsets.emplace_back( 0x1ED00, 0x1ED4F, SvxResId(RID_SUBSETSTR_OTTOMAN_SIYAQ_NUMBERS) );
                    break;
                case UBLOCK_SMALL_KANA_EXTENSION:
                    aAllSubsets.emplace_back( 0x1B130, 0x1B16F, SvxResId(RID_SUBSETSTR_SMALL_KANA_EXTENSION) );
                    break;
                case UBLOCK_SYMBOLS_AND_PICTOGRAPHS_EXTENDED_A:
                    aAllSubsets.emplace_back( 0x1FA70, 0x1FAFF, SvxResId(RID_SUBSETSTR_SYMBOLS_AND_PICTOGRAPHS_EXTENDED_A) );
                    break;
                case UBLOCK_TAMIL_SUPPLEMENT:
                    aAllSubsets.emplace_back( 0x11FC0, 0x11FFF, SvxResId(RID_SUBSETSTR_TAMIL_SUPPLEMENT) );
                    break;
                case UBLOCK_WANCHO:
                    aAllSubsets.emplace_back( 0x1E2C0, 0x1E2FF, SvxResId(RID_SUBSETSTR_WANCHO) );
                    break;
#endif
#if (U_ICU_VERSION_MAJOR_NUM >= 66)
                case UBLOCK_CHORASMIAN:
                    aAllSubsets.emplace_back( 0x10FB0, 0x10FDF, SvxResId(RID_SUBSETSTR_CHORASMIAN) );
                    break;
                case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_G:
                    aAllSubsets.emplace_back( 0x30000, 0x3134F, SvxResId(RID_SUBSETSTR_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_G) );
                    break;
                case UBLOCK_DIVES_AKURU:
                    aAllSubsets.emplace_back( 0x11900, 0x1195F, SvxResId(RID_SUBSETSTR_DIVES_AKURU) );
                    break;
                case UBLOCK_KHITAN_SMALL_SCRIPT:
                    aAllSubsets.emplace_back( 0x18B00, 0x18CFF, SvxResId(RID_SUBSETSTR_KHITAN_SMALL_SCRIPT) );
                    break;
                case UBLOCK_LISU_SUPPLEMENT:
                    aAllSubsets.emplace_back( 0x11FB0, 0x11FBF, SvxResId(RID_SUBSETSTR_LISU_SUPPLEMENT) );
                    break;
                case UBLOCK_SYMBOLS_FOR_LEGACY_COMPUTING:
                    aAllSubsets.emplace_back( 0x1FB00, 0x1FBFF, SvxResId(RID_SUBSETSTR_SYMBOLS_FOR_LEGACY_COMPUTING) );
                    break;
                case UBLOCK_TANGUT_SUPPLEMENT:
                    aAllSubsets.emplace_back( 0x18D00, 0x18D7F, SvxResId(RID_SUBSETSTR_TANGUT_SUPPLEMENT) );
                    break;
                case UBLOCK_YEZIDI:
                    aAllSubsets.emplace_back( 0x10E80, 0x10EBF, SvxResId(RID_SUBSETSTR_YEZIDI) );
                    break;
#endif
#if (U_ICU_VERSION_MAJOR_NUM >= 70)
                case UBLOCK_ARABIC_EXTENDED_B:
                    aAllSubsets.emplace_back( 0x0870, 0x089F, SvxResId(RID_SUBSETSTR_ARABIC_EXTENDED_B) );
                    break;
                case UBLOCK_CYPRO_MINOAN:
                    aAllSubsets.emplace_back( 0x12F90, 0x12FFF, SvxResId(RID_SUBSETSTR_CYPRO_MINOAN) );
                    break;
                case UBLOCK_ETHIOPIC_EXTENDED_B:
                    aAllSubsets.emplace_back( 0x1E7E0, 0x1E7FF, SvxResId(RID_SUBSETSTR_ETHIOPIC_EXTENDED_B) );
                    break;
                case UBLOCK_KANA_EXTENDED_B:
                    aAllSubsets.emplace_back( 0x1AFF0, 0x1AFFF, SvxResId(RID_SUBSETSTR_KANA_EXTENDED_B) );
                    break;
                case UBLOCK_LATIN_EXTENDED_F:
                    aAllSubsets.emplace_back( 0x10780, 0x107BF, SvxResId(RID_SUBSETSTR_LATIN_EXTENDED_F) );
                    break;
                case UBLOCK_LATIN_EXTENDED_G:
                    aAllSubsets.emplace_back( 0x1DF00, 0x1DFFF, SvxResId(RID_SUBSETSTR_LATIN_EXTENDED_G) );
                    break;
                case UBLOCK_OLD_UYGHUR:
                    aAllSubsets.emplace_back( 0x10F70, 0x10FAF, SvxResId(RID_SUBSETSTR_OLD_UYGHUR) );
                    break;
                case UBLOCK_TANGSA:
                    aAllSubsets.emplace_back( 0x16A70, 0x16ACF, SvxResId(RID_SUBSETSTR_TANGSA) );
                    break;
                case UBLOCK_TOTO:
                    aAllSubsets.emplace_back( 0x1E290, 0x1E2BF, SvxResId(RID_SUBSETSTR_TOTO) );
                    break;
                case UBLOCK_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED_A:
                    aAllSubsets.emplace_back( 0x11AB0, 0x11ABF, SvxResId(RID_SUBSETSTR_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED_A) );
                    break;
                case UBLOCK_VITHKUQI:
                    aAllSubsets.emplace_back( 0x10570, 0x105BF, SvxResId(RID_SUBSETSTR_VITHKUQI) );
                    break;
                case UBLOCK_ZNAMENNY_MUSICAL_NOTATION:
                    aAllSubsets.emplace_back( 0x1CF00, 0x1CFCF, SvxResId(RID_SUBSETSTR_ZNAMENNY_MUSICAL_NOTATION) );
                    break;
#endif

            }

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
            if (eBlock != UBLOCK_NO_BLOCK &&
                eBlock != UBLOCK_INVALID_CODE &&
                eBlock != UBLOCK_COUNT &&
                eBlock != UBLOCK_HIGH_SURROGATES &&
                eBlock != UBLOCK_HIGH_PRIVATE_USE_SURROGATES &&
                eBlock != UBLOCK_LOW_SURROGATES)

            {
                UBlockCode eBlockStart = ublock_getCode(aAllSubsets.back().GetRangeMin());
                UBlockCode eBlockEnd = ublock_getCode(aAllSubsets.back().GetRangeMax());
                assert(eBlockStart == eBlockEnd && eBlockStart == eBlock);
            }
#endif
        }

        std::stable_sort(aAllSubsets.begin(), aAllSubsets.end());
        return aAllSubsets;
    }();

    maSubsets = s_aAllSubsets;
}

void SubsetMap::ApplyCharMap( const FontCharMapRef& rxFontCharMap )
{
    if( !rxFontCharMap.is() )
        return;

    // remove subsets that are not matched in any range
    maSubsets.erase(std::remove_if(maSubsets.begin(), maSubsets.end(),
        [&rxFontCharMap](const Subset& rSubset) {
            sal_uInt32 cMin = rSubset.GetRangeMin();
            sal_uInt32 cMax = rSubset.GetRangeMax();
            int nCount = rxFontCharMap->CountCharsInRange( cMin, cMax );
            return nCount <= 0;
        }),
        maSubsets.end());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
