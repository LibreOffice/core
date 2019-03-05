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

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/event.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/help.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/i18nhelp.hxx>

#include <listbox.hxx>
#include <controldata.hxx>
#include <svdata.hxx>
#include <window.h>
#include <impglyphitem.hxx>

#include <com/sun/star/i18n/XCollator.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>

#include <rtl/instance.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <comphelper/string.hxx>
#include <comphelper/processfactory.hxx>

#include <limits>

#define MULTILINE_ENTRY_DRAW_FLAGS ( DrawTextFlags::WordBreak | DrawTextFlags::MultiLine | DrawTextFlags::VCenter )

using namespace ::com::sun::star;

static constexpr long gnBorder = 1;

void ImplInitDropDownButton( PushButton* pButton )
{
    pButton->SetSymbol( SymbolType::SPIN_DOWN );

    if ( pButton->IsNativeControlSupported(ControlType::Listbox, ControlPart::Entire)
            && ! pButton->IsNativeControlSupported(ControlType::Listbox, ControlPart::ButtonDown) )
        pButton->SetBackground();
}

ImplEntryList::ImplEntryList( vcl::Window* pWindow )
{
    mpWindow = pWindow;
    mnLastSelected = LISTBOX_ENTRY_NOTFOUND;
    mnSelectionAnchor = LISTBOX_ENTRY_NOTFOUND;
    mnImages = 0;
    mbCallSelectionChangedHdl = true;

    mnMRUCount = 0;
    mnMaxMRUCount = 0;
}

ImplEntryList::~ImplEntryList()
{
    Clear();
}

void ImplEntryList::Clear()
{
    mnImages = 0;
    maEntries.clear();
}

void ImplEntryList::SelectEntry( sal_Int32 nPos, bool bSelect )
{
    if (0 <= nPos && static_cast<size_t>(nPos) < maEntries.size())
    {
        std::vector<std::unique_ptr<ImplEntryType> >::iterator iter = maEntries.begin()+nPos;

        if ( ( (*iter)->mbIsSelected != bSelect ) &&
           ( ( (*iter)->mnFlags & ListBoxEntryFlags::DisableSelection) == ListBoxEntryFlags::NONE  ) )
        {
            (*iter)->mbIsSelected = bSelect;
            if ( mbCallSelectionChangedHdl )
                maSelectionChangedHdl.Call( nPos );
        }
    }
}

namespace
{
    struct theSorter
        : public rtl::StaticWithInit< comphelper::string::NaturalStringSorter, theSorter >
    {
        comphelper::string::NaturalStringSorter operator () ()
        {
            return comphelper::string::NaturalStringSorter(
                ::comphelper::getProcessComponentContext(),
                Application::GetSettings().GetLanguageTag().getLocale());
        }
    };
}

sal_Int32 ListBox::NaturalSortCompare(const OUString &rA, const OUString &rB)
{
    const comphelper::string::NaturalStringSorter &rSorter = theSorter::get();
    return rSorter.compare(rA, rB);
}

sal_Int32 ImplEntryList::InsertEntry( sal_Int32 nPos, ImplEntryType* pNewEntry, bool bSort )
{
    assert(nPos >= 0);
    assert(maEntries.size() < LISTBOX_MAX_ENTRIES);

    if ( !!pNewEntry->maImage )
        mnImages++;

    sal_Int32 insPos = 0;
    const sal_Int32 nEntriesSize = static_cast<sal_Int32>(maEntries.size());

    if ( !bSort || maEntries.empty())
    {
        if (0 <= nPos && nPos < nEntriesSize)
        {
            insPos = nPos;
            maEntries.insert( maEntries.begin() + nPos, std::unique_ptr<ImplEntryType>(pNewEntry) );
        }
        else
        {
            insPos = nEntriesSize;
            maEntries.push_back(std::unique_ptr<ImplEntryType>(pNewEntry));
        }
    }
    else
    {
        const comphelper::string::NaturalStringSorter &rSorter = theSorter::get();

        const OUString& rStr = pNewEntry->maStr;

        ImplEntryType* pTemp = GetEntry( nEntriesSize-1 );

        try
        {
            sal_Int32 nComp = rSorter.compare(rStr, pTemp->maStr);

            // fast insert for sorted data
            if ( nComp >= 0 )
            {
                insPos = nEntriesSize;
                maEntries.push_back(std::unique_ptr<ImplEntryType>(pNewEntry));
            }
            else
            {
                pTemp = GetEntry( mnMRUCount );

                nComp = rSorter.compare(rStr, pTemp->maStr);
                if ( nComp <= 0 )
                {
                    insPos = 0;
                    maEntries.insert(maEntries.begin(), std::unique_ptr<ImplEntryType>(pNewEntry));
                }
                else
                {
                    sal_uLong nLow = mnMRUCount;
                    sal_uLong nHigh = maEntries.size()-1;
                    sal_Int32 nMid;

                    // binary search
                    do
                    {
                        nMid = static_cast<sal_Int32>((nLow + nHigh) / 2);
                        pTemp = GetEntry( nMid );

                        nComp = rSorter.compare(rStr, pTemp->maStr);

                        if ( nComp < 0 )
                            nHigh = nMid-1;
                        else
                        {
                            if ( nComp > 0 )
                                nLow = nMid + 1;
                            else
                                break;
                        }
                    }
                    while ( nLow <= nHigh );

                    if ( nComp >= 0 )
                        nMid++;

                    insPos = nMid;
                    maEntries.insert(maEntries.begin()+nMid, std::unique_ptr<ImplEntryType>(pNewEntry));
                }
            }
        }
        catch (uno::RuntimeException& )
        {
            // XXX this is arguable, if the exception occurred because pNewEntry is
            // garbage you wouldn't insert it. If the exception occurred because the
            // Collator implementation is garbage then give the user a chance to see
            // his stuff
            insPos = 0;
            maEntries.insert(maEntries.begin(), std::unique_ptr<ImplEntryType>(pNewEntry));
        }

    }

    return insPos;
}

void ImplEntryList::RemoveEntry( sal_Int32 nPos )
{
    if (0 <= nPos && static_cast<size_t>(nPos) < maEntries.size())
    {
        std::vector<std::unique_ptr<ImplEntryType> >::iterator iter = maEntries.begin()+ nPos;

        if ( !!(*iter)->maImage )
            mnImages--;

        maEntries.erase(iter);
    }
}

sal_Int32 ImplEntryList::FindEntry( const OUString& rString, bool bSearchMRUArea ) const
{
    const sal_Int32 nEntries = static_cast<sal_Int32>(maEntries.size());
    for ( sal_Int32 n = bSearchMRUArea ? 0 : GetMRUCount(); n < nEntries; n++ )
    {
        OUString aComp( vcl::I18nHelper::filterFormattingChars( maEntries[n]->maStr ) );
        if ( aComp == rString )
            return n;
    }
    return LISTBOX_ENTRY_NOTFOUND;
}

sal_Int32 ImplEntryList::FindMatchingEntry( const OUString& rStr, sal_Int32 nStart, bool bLazy ) const
{
    sal_Int32  nPos = LISTBOX_ENTRY_NOTFOUND;
    sal_Int32  nEntryCount = GetEntryCount();

    const vcl::I18nHelper& rI18nHelper = mpWindow->GetSettings().GetLocaleI18nHelper();
    for ( sal_Int32 n = nStart; n < nEntryCount; )
    {
        ImplEntryType* pImplEntry = GetEntry( n );
        bool bMatch;
        if ( bLazy  )
        {
            bMatch = rI18nHelper.MatchString( rStr, pImplEntry->maStr );
        }
        else
        {
            bMatch = pImplEntry->maStr.startsWith(rStr);
        }
        if ( bMatch )
        {
            nPos = n;
            break;
        }

        n++;
    }

    return nPos;
}

sal_Int32 ImplEntryList::FindEntry( const void* pData ) const
{
    sal_Int32 nPos = LISTBOX_ENTRY_NOTFOUND;
    for ( sal_Int32 n = GetEntryCount(); n; )
    {
        ImplEntryType* pImplEntry = GetEntry( --n );
        if ( pImplEntry->mpUserData == pData )
        {
            nPos = n;
            break;
        }
    }
    return nPos;
}

long ImplEntryList::GetAddedHeight( sal_Int32 i_nEndIndex, sal_Int32 i_nBeginIndex ) const
{
    long nHeight = 0;
    sal_Int32 nStart = std::min(i_nEndIndex, i_nBeginIndex);
    sal_Int32 nStop  = std::max(i_nEndIndex, i_nBeginIndex);
    sal_Int32 nEntryCount = GetEntryCount();
    if( 0 <= nStop && nStop != LISTBOX_ENTRY_NOTFOUND && nEntryCount != 0 )
    {
        // sanity check
        if( nStop > nEntryCount-1 )
            nStop = nEntryCount-1;
        if (nStart < 0)
            nStart = 0;
        else if( nStart > nEntryCount-1 )
            nStart = nEntryCount-1;

        sal_Int32 nIndex = nStart;
        while( nIndex != LISTBOX_ENTRY_NOTFOUND && nIndex < nStop )
        {
            long nPosHeight = GetEntryPtr( nIndex )->mnHeight;
            if (nHeight > ::std::numeric_limits<long>::max() - nPosHeight)
            {
                SAL_WARN( "vcl", "ImplEntryList::GetAddedHeight: truncated");
                break;
            }
            nHeight += nPosHeight;
            nIndex++;
        }
    }
    else
        nHeight = 0;
    return i_nEndIndex > i_nBeginIndex ? nHeight : -nHeight;
}

long ImplEntryList::GetEntryHeight( sal_Int32 nPos ) const
{
    ImplEntryType* pImplEntry = GetEntry( nPos );
    return pImplEntry ? pImplEntry->mnHeight : 0;
}

OUString ImplEntryList::GetEntryText( sal_Int32 nPos ) const
{
    OUString aEntryText;
    ImplEntryType* pImplEntry = GetEntry( nPos );
    if ( pImplEntry )
        aEntryText = pImplEntry->maStr;
    return aEntryText;
}

bool ImplEntryList::HasEntryImage( sal_Int32 nPos ) const
{
    bool bImage = false;
    ImplEntryType* pImplEntry = GetEntry( nPos );
    if ( pImplEntry )
        bImage = !!pImplEntry->maImage;
    return bImage;
}

Image ImplEntryList::GetEntryImage( sal_Int32 nPos ) const
{
    Image aImage;
    ImplEntryType* pImplEntry = GetEntry( nPos );
    if ( pImplEntry )
        aImage = pImplEntry->maImage;
    return aImage;
}

void ImplEntryList::SetEntryData( sal_Int32 nPos, void* pNewData )
{
    ImplEntryType* pImplEntry = GetEntry( nPos );
    if ( pImplEntry )
        pImplEntry->mpUserData = pNewData;
}

void* ImplEntryList::GetEntryData( sal_Int32 nPos ) const
{
    ImplEntryType* pImplEntry = GetEntry( nPos );
    return pImplEntry ? pImplEntry->mpUserData : nullptr;
}

void ImplEntryList::SetEntryFlags( sal_Int32 nPos, ListBoxEntryFlags nFlags )
{
    ImplEntryType* pImplEntry = GetEntry( nPos );
    if ( pImplEntry )
        pImplEntry->mnFlags = nFlags;
}

ListBoxEntryFlags ImplEntryList::GetEntryFlags( sal_Int32 nPos ) const
{
    ImplEntryType* pImplEntry = GetEntry( nPos );
    return pImplEntry ? pImplEntry->mnFlags : ListBoxEntryFlags::NONE;
}

sal_Int32 ImplEntryList::GetSelectedEntryCount() const
{
    sal_Int32 nSelCount = 0;
    for ( sal_Int32 n = GetEntryCount(); n; )
    {
        ImplEntryType* pImplEntry = GetEntry( --n );
        if ( pImplEntry->mbIsSelected )
            nSelCount++;
    }
    return nSelCount;
}

OUString ImplEntryList::GetSelectedEntry( sal_Int32 nIndex ) const
{
    return GetEntryText( GetSelectedEntryPos( nIndex ) );
}

sal_Int32 ImplEntryList::GetSelectedEntryPos( sal_Int32 nIndex ) const
{
    sal_Int32 nSelEntryPos = LISTBOX_ENTRY_NOTFOUND;
    sal_Int32 nSel = 0;
    sal_Int32 nEntryCount = GetEntryCount();

    for ( sal_Int32 n = 0; n < nEntryCount; n++ )
    {
        ImplEntryType* pImplEntry = GetEntry( n );
        if ( pImplEntry->mbIsSelected )
        {
            if ( nSel == nIndex )
            {
                nSelEntryPos = n;
                break;
            }
            nSel++;
        }
    }

    return nSelEntryPos;
}

bool ImplEntryList::IsEntryPosSelected( sal_Int32 nIndex ) const
{
    ImplEntryType* pImplEntry = GetEntry( nIndex );
    return pImplEntry && pImplEntry->mbIsSelected;
}

bool ImplEntryList::IsEntrySelectable( sal_Int32 nPos ) const
{
    ImplEntryType* pImplEntry = GetEntry( nPos );
    return pImplEntry == nullptr || ((pImplEntry->mnFlags & ListBoxEntryFlags::DisableSelection) == ListBoxEntryFlags::NONE);
}

sal_Int32 ImplEntryList::FindFirstSelectable( sal_Int32 nPos, bool bForward /* = true */ )
{
    if( IsEntrySelectable( nPos ) )
        return nPos;

    if( bForward )
    {
        for( nPos = nPos + 1; nPos < GetEntryCount(); nPos++ )
        {
            if( IsEntrySelectable( nPos ) )
                return nPos;
        }
    }
    else
    {
        while( nPos )
        {
            nPos--;
            if( IsEntrySelectable( nPos ) )
                return nPos;
        }
    }

    return LISTBOX_ENTRY_NOTFOUND;
}

ImplListBoxWindow::ImplListBoxWindow( vcl::Window* pParent, WinBits nWinStyle ) :
    Control( pParent, 0 ),
    maQuickSelectionEngine( *this )
{
    mpEntryList.reset(new ImplEntryList( this ));

    mnTop               = 0;
    mnLeft              = 0;
    mnSelectModifier    = 0;
    mnUserDrawEntry     = LISTBOX_ENTRY_NOTFOUND;
    mbTrack             = false;
    mbTravelSelect      = false;
    mbTrackingSelect    = false;
    mbSelectionChanged  = false;
    mbMouseMoveSelect   = false;
    mbMulti             = false;
    mbStackMode         = false;
    mbGrabFocus         = false;
    mbUserDrawEnabled   = false;
    mbInUserDraw        = false;
    mbReadOnly          = false;
    mbHasFocusRect      = false;
    mbRight             = ( nWinStyle & WB_RIGHT );
    mbCenter            = ( nWinStyle & WB_CENTER );
    mbSimpleMode        = ( nWinStyle & WB_SIMPLEMODE );
    mbSort              = ( nWinStyle & WB_SORT );
    mbEdgeBlending      = false;

    // pb: #106948# explicit mirroring for calc
    mbMirroring         = false;

    mnCurrentPos            = LISTBOX_ENTRY_NOTFOUND;
    mnTrackingSaveSelection = LISTBOX_ENTRY_NOTFOUND;
    meProminentType         = ProminentEntry::TOP;

    SetLineColor();
    SetTextFillColor();
    SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetFieldColor() ) );

    ApplySettings(*this);
    ImplCalcMetrics();
}

ImplListBoxWindow::~ImplListBoxWindow()
{
    disposeOnce();
}

void ImplListBoxWindow::dispose()
{
    mpEntryList.reset();
    Control::dispose();
}

void ImplListBoxWindow::ApplySettings(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    vcl::Font aFont = rStyleSettings.GetFieldFont();
    if (IsControlFont())
        aFont.Merge(GetControlFont());
    SetZoomedPointFont(rRenderContext, aFont);

    Color aTextColor = rStyleSettings.GetFieldTextColor();
    if (IsControlForeground())
        aTextColor = GetControlForeground();
    rRenderContext.SetTextColor(aTextColor);

    if (IsControlBackground())
        rRenderContext.SetBackground(GetControlBackground());
    else
        rRenderContext.SetBackground(rStyleSettings.GetFieldColor());
}

void ImplListBoxWindow::ImplCalcMetrics()
{
    mnMaxWidth      = 0;
    mnMaxTxtWidth   = 0;
    mnMaxImgWidth   = 0;
    mnMaxImgTxtWidth= 0;
    mnMaxImgHeight  = 0;

    mnTextHeight = static_cast<sal_uInt16>(GetTextHeight());
    mnMaxTxtHeight = mnTextHeight + gnBorder;
    mnMaxHeight = mnMaxTxtHeight;

    if ( maUserItemSize.Height() > mnMaxHeight )
        mnMaxHeight = static_cast<sal_uInt16>(maUserItemSize.Height());
    if ( maUserItemSize.Width() > mnMaxWidth )
        mnMaxWidth= static_cast<sal_uInt16>(maUserItemSize.Width());

    for ( sal_Int32 n = mpEntryList->GetEntryCount(); n; )
    {
        ImplEntryType* pEntry = mpEntryList->GetMutableEntryPtr( --n );
        ImplUpdateEntryMetrics( *pEntry );
    }

    if( mnCurrentPos != LISTBOX_ENTRY_NOTFOUND )
    {
        Size aSz( GetOutputSizePixel().Width(), mpEntryList->GetEntryPtr( mnCurrentPos )->mnHeight );
        maFocusRect.SetSize( aSz );
    }
}

void ImplListBoxWindow::Clear()
{
    mpEntryList->Clear();

    mnMaxHeight     = mnMaxTxtHeight;
    mnMaxWidth      = 0;
    mnMaxTxtWidth   = 0;
    mnMaxImgTxtWidth= 0;
    mnMaxImgWidth   = 0;
    mnMaxImgHeight  = 0;
    mnTop           = 0;
    mnLeft          = 0;
    ImplClearLayoutData();

    mnCurrentPos = LISTBOX_ENTRY_NOTFOUND;
    maQuickSelectionEngine.Reset();

    Invalidate();
}

void ImplListBoxWindow::SetUserItemSize( const Size& rSz )
{
    ImplClearLayoutData();
    maUserItemSize = rSz;
    ImplCalcMetrics();
}

struct ImplEntryMetrics
{
    bool    bText;
    bool    bImage;
    long    nEntryWidth;
    long    nEntryHeight;
    long    nTextWidth;
    long    nImgWidth;
    long    nImgHeight;
};

SalLayoutGlyphs* ImplEntryType::GetTextGlyphs(const OutputDevice* pOutputDevice)
{
    if (maStrGlyphs.IsValid())
        // Use pre-calculated result.
        return &maStrGlyphs;

    std::unique_ptr<SalLayout> pLayout = pOutputDevice->ImplLayout(
        maStr, 0, maStr.getLength(), Point(0, 0), 0, nullptr, SalLayoutFlags::GlyphItemsOnly);
    if (!pLayout)
        return nullptr;

    const SalLayoutGlyphs* pGlyphs = pLayout->GetGlyphs();
    if (!pGlyphs)
        return nullptr;

    // Remember the calculation result.
    maStrGlyphs = *pGlyphs;

    return &maStrGlyphs;
}

void ImplListBoxWindow::EnableQuickSelection( bool b )
{
    maQuickSelectionEngine.SetEnabled( b );
}

void ImplListBoxWindow::ImplUpdateEntryMetrics( ImplEntryType& rEntry )
{
    ImplEntryMetrics aMetrics;
    aMetrics.bText = !rEntry.maStr.isEmpty();
    aMetrics.bImage = !!rEntry.maImage;
    aMetrics.nEntryWidth = 0;
    aMetrics.nEntryHeight = 0;
    aMetrics.nTextWidth = 0;
    aMetrics.nImgWidth = 0;
    aMetrics.nImgHeight = 0;

    if ( aMetrics.bText )
    {
        if( rEntry.mnFlags & ListBoxEntryFlags::MultiLine )
        {
            // multiline case
            Size aCurSize( PixelToLogic( GetSizePixel() ) );
            // set the current size to a large number
            // GetTextRect should shrink it to the actual size
            aCurSize.setHeight( 0x7fffff );
            tools::Rectangle aTextRect( Point( 0, 0 ), aCurSize );
            aTextRect = GetTextRect( aTextRect, rEntry.maStr, DrawTextFlags::WordBreak | DrawTextFlags::MultiLine );
            aMetrics.nTextWidth = aTextRect.GetWidth();
            if( aMetrics.nTextWidth > mnMaxTxtWidth )
                mnMaxTxtWidth = aMetrics.nTextWidth;
            aMetrics.nEntryWidth = mnMaxTxtWidth;
            aMetrics.nEntryHeight = aTextRect.GetHeight() + gnBorder;
        }
        else
        {
            // normal single line case
            const SalLayoutGlyphs* pGlyphs = rEntry.GetTextGlyphs(this);
            aMetrics.nTextWidth
                = static_cast<sal_uInt16>(GetTextWidth(rEntry.maStr, 0, -1, nullptr, pGlyphs));
            if( aMetrics.nTextWidth > mnMaxTxtWidth )
                mnMaxTxtWidth = aMetrics.nTextWidth;
            aMetrics.nEntryWidth = mnMaxTxtWidth;
            aMetrics.nEntryHeight = mnTextHeight + gnBorder;
        }
    }
    if ( aMetrics.bImage )
    {
        Size aImgSz = rEntry.maImage.GetSizePixel();
        aMetrics.nImgWidth  = static_cast<sal_uInt16>(CalcZoom( aImgSz.Width() ));
        aMetrics.nImgHeight = static_cast<sal_uInt16>(CalcZoom( aImgSz.Height() ));

        if( aMetrics.nImgWidth > mnMaxImgWidth )
            mnMaxImgWidth = aMetrics.nImgWidth;
        if( aMetrics.nImgHeight > mnMaxImgHeight )
            mnMaxImgHeight = aMetrics.nImgHeight;

        mnMaxImgTxtWidth = std::max( mnMaxImgTxtWidth, aMetrics.nTextWidth );
        aMetrics.nEntryHeight = std::max( aMetrics.nImgHeight, aMetrics.nEntryHeight );

    }
    if ( IsUserDrawEnabled() || aMetrics.bImage )
    {
        aMetrics.nEntryWidth = std::max( aMetrics.nImgWidth, maUserItemSize.Width() );
        if ( aMetrics.bText )
            aMetrics.nEntryWidth += aMetrics.nTextWidth + IMG_TXT_DISTANCE;
        aMetrics.nEntryHeight = std::max( std::max( mnMaxImgHeight, maUserItemSize.Height() ) + 2,
                                     aMetrics.nEntryHeight );
    }

    if ( !aMetrics.bText && !aMetrics.bImage && !IsUserDrawEnabled() )
    {
        // entries which have no (aka an empty) text, and no image,
        // and are not user-drawn, should be shown nonetheless
        aMetrics.nEntryHeight = mnTextHeight + gnBorder;
    }

    if ( aMetrics.nEntryWidth > mnMaxWidth )
        mnMaxWidth = aMetrics.nEntryWidth;
    if ( aMetrics.nEntryHeight > mnMaxHeight )
        mnMaxHeight = aMetrics.nEntryHeight;

    rEntry.mnHeight = aMetrics.nEntryHeight;
}

void ImplListBoxWindow::ImplCallSelect()
{
    if ( !IsTravelSelect() && GetEntryList()->GetMaxMRUCount() )
    {
        // Insert the selected entry as MRU, if not already first MRU
        sal_Int32 nSelected = GetEntryList()->GetSelectedEntryPos( 0 );
        sal_Int32 nMRUCount = GetEntryList()->GetMRUCount();
        OUString aSelected = GetEntryList()->GetEntryText( nSelected );
        sal_Int32 nFirstMatchingEntryPos = GetEntryList()->FindEntry( aSelected, true );
        if ( nFirstMatchingEntryPos || !nMRUCount )
        {
            bool bSelectNewEntry = false;
            if ( nFirstMatchingEntryPos < nMRUCount )
            {
                RemoveEntry( nFirstMatchingEntryPos );
                nMRUCount--;
                if ( nFirstMatchingEntryPos == nSelected )
                    bSelectNewEntry = true;
            }
            else if ( nMRUCount == GetEntryList()->GetMaxMRUCount() )
            {
                RemoveEntry( nMRUCount - 1 );
                nMRUCount--;
            }

            ImplClearLayoutData();

            ImplEntryType* pNewEntry = new ImplEntryType( aSelected );
            pNewEntry->mbIsSelected = bSelectNewEntry;
            GetEntryList()->InsertEntry( 0, pNewEntry, false );
            ImplUpdateEntryMetrics( *pNewEntry );
            GetEntryList()->SetMRUCount( ++nMRUCount );
            SetSeparatorPos( nMRUCount ? nMRUCount-1 : 0 );
            maMRUChangedHdl.Call( nullptr );
        }
    }

    maSelectHdl.Call( nullptr );
    mbSelectionChanged = false;
}

sal_Int32 ImplListBoxWindow::InsertEntry( sal_Int32 nPos, ImplEntryType* pNewEntry )
{
    assert(nPos >= 0);
    assert(mpEntryList->GetEntryCount() < LISTBOX_MAX_ENTRIES);

    ImplClearLayoutData();
    sal_Int32 nNewPos = mpEntryList->InsertEntry( nPos, pNewEntry, mbSort );

    if( GetStyle() & WB_WORDBREAK )
        pNewEntry->mnFlags |= ListBoxEntryFlags::MultiLine;

    ImplUpdateEntryMetrics( *pNewEntry );
    return nNewPos;
}

void ImplListBoxWindow::RemoveEntry( sal_Int32 nPos )
{
    ImplClearLayoutData();
    mpEntryList->RemoveEntry( nPos );
    if( mnCurrentPos >= mpEntryList->GetEntryCount() )
        mnCurrentPos = LISTBOX_ENTRY_NOTFOUND;
    ImplCalcMetrics();
}

void ImplListBoxWindow::SetEntryFlags( sal_Int32 nPos, ListBoxEntryFlags nFlags )
{
    mpEntryList->SetEntryFlags( nPos, nFlags );
    ImplEntryType* pEntry = mpEntryList->GetMutableEntryPtr( nPos );
    if( pEntry )
        ImplUpdateEntryMetrics( *pEntry );
}

void ImplListBoxWindow::ImplShowFocusRect()
{
    if ( mbHasFocusRect )
        HideFocus();
    ShowFocus( maFocusRect );
    mbHasFocusRect = true;
}

void ImplListBoxWindow::ImplHideFocusRect()
{
    if ( mbHasFocusRect )
    {
        HideFocus();
        mbHasFocusRect = false;
    }
}

sal_Int32 ImplListBoxWindow::GetEntryPosForPoint( const Point& rPoint ) const
{
    long nY = gnBorder;

    sal_Int32 nSelect = mnTop;
    const ImplEntryType* pEntry = mpEntryList->GetEntryPtr( nSelect );
    while( pEntry && rPoint.Y() > pEntry->mnHeight + nY )
    {
        nY += pEntry->mnHeight;
        pEntry = mpEntryList->GetEntryPtr( ++nSelect );
    }
    if( pEntry == nullptr )
        nSelect = LISTBOX_ENTRY_NOTFOUND;

    return nSelect;
}

bool ImplListBoxWindow::IsVisible( sal_Int32 i_nEntry ) const
{
    bool bRet = false;

    if( i_nEntry >= mnTop )
    {
        if( mpEntryList->GetAddedHeight( i_nEntry, mnTop ) <
            PixelToLogic( GetSizePixel() ).Height() )
        {
            bRet = true;
        }
    }

    return bRet;
}

sal_Int32 ImplListBoxWindow::GetLastVisibleEntry() const
{
    sal_Int32 nPos = mnTop;
    long nWindowHeight = GetSizePixel().Height();
    sal_Int32 nCount = mpEntryList->GetEntryCount();
    long nDiff;
    for( nDiff = 0; nDiff < nWindowHeight && nPos < nCount; nDiff = mpEntryList->GetAddedHeight( nPos, mnTop ) )
        nPos++;

    if( nDiff > nWindowHeight && nPos > mnTop )
        nPos--;

    if( nPos >= nCount )
        nPos = nCount-1;

    return nPos;
}

void ImplListBoxWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    mbMouseMoveSelect = false;  // only till the first MouseButtonDown
    maQuickSelectionEngine.Reset();

    if ( !IsReadOnly() )
    {
        if( rMEvt.GetClicks() == 1 )
        {
            sal_Int32 nSelect = GetEntryPosForPoint( rMEvt.GetPosPixel() );
            if( nSelect != LISTBOX_ENTRY_NOTFOUND )
            {
                if ( !mbMulti && GetEntryList()->GetSelectedEntryCount() )
                    mnTrackingSaveSelection = GetEntryList()->GetSelectedEntryPos( 0 );
                else
                    mnTrackingSaveSelection = LISTBOX_ENTRY_NOTFOUND;

                mnCurrentPos = nSelect;
                mbTrackingSelect = true;
                bool bCurPosChange = (mnCurrentPos != nSelect);
                (void)SelectEntries( nSelect, LET_MBDOWN, rMEvt.IsShift(), rMEvt.IsMod1() ,bCurPosChange);
                mbTrackingSelect = false;
                if ( mbGrabFocus )
                    GrabFocus();

                StartTracking( StartTrackingFlags::ScrollRepeat );
            }
        }
        if( rMEvt.GetClicks() == 2 )
        {
            maDoubleClickHdl.Call( this );
        }
    }
    else // if ( mbGrabFocus )
    {
        GrabFocus();
    }
}

void ImplListBoxWindow::MouseMove( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeaveWindow() )
    {
        if ( mbStackMode && IsMouseMoveSelect() && IsReallyVisible() )
        {
            if ( rMEvt.GetPosPixel().Y() < 0 )
            {
                DeselectAll();
                mnCurrentPos = LISTBOX_ENTRY_NOTFOUND;
                SetTopEntry( 0 );
                if ( mbStackMode )
                {
                    mbTravelSelect = true;
                    mnSelectModifier = rMEvt.GetModifier();
                    ImplCallSelect();
                    mbTravelSelect = false;
                }

            }
        }
    }
    else if ( ( ( !mbMulti && IsMouseMoveSelect() ) || mbStackMode ) && mpEntryList->GetEntryCount() )
    {
        tools::Rectangle aRect( Point(), GetOutputSizePixel() );
        if( aRect.IsInside( rMEvt.GetPosPixel() ) )
        {
            if ( IsMouseMoveSelect() )
            {
                sal_Int32 nSelect = GetEntryPosForPoint( rMEvt.GetPosPixel() );
                if( nSelect == LISTBOX_ENTRY_NOTFOUND )
                    nSelect = mpEntryList->GetEntryCount() - 1;
                nSelect = std::min( nSelect, GetLastVisibleEntry() );
                nSelect = std::min( nSelect, static_cast<sal_Int32>( mpEntryList->GetEntryCount() - 1 ) );
                // Select only visible Entries with MouseMove, otherwise Tracking...
                if ( IsVisible( nSelect ) &&
                    mpEntryList->IsEntrySelectable( nSelect ) &&
                    ( ( nSelect != mnCurrentPos ) || !GetEntryList()->GetSelectedEntryCount() || ( nSelect != GetEntryList()->GetSelectedEntryPos( 0 ) ) ) )
                {
                    mbTrackingSelect = true;
                    if ( SelectEntries( nSelect, LET_TRACKING ) )
                    {
                        if ( mbStackMode )
                        {
                            mbTravelSelect = true;
                            mnSelectModifier = rMEvt.GetModifier();
                            ImplCallSelect();
                            mbTravelSelect = false;
                        }
                        // When list box selection change by mouse move, notify
                        // VclEventId::ListboxSelect vcl event.
                        else
                        {
                            maListItemSelectHdl.Call(nullptr);
                        }
                    }
                    mbTrackingSelect = false;
                }
            }

            // if the DD button was pressed and someone moved into the ListBox
            // with the mouse button pressed...
            if ( rMEvt.IsLeft() && !rMEvt.IsSynthetic() )
            {
                if ( !mbMulti && GetEntryList()->GetSelectedEntryCount() )
                    mnTrackingSaveSelection = GetEntryList()->GetSelectedEntryPos( 0 );
                else
                    mnTrackingSaveSelection = LISTBOX_ENTRY_NOTFOUND;

                if ( mbStackMode && ( mpEntryList->GetSelectionAnchor() == LISTBOX_ENTRY_NOTFOUND ) )
                    mpEntryList->SetSelectionAnchor( 0 );

                StartTracking( StartTrackingFlags::ScrollRepeat );
            }
        }
    }
}

void ImplListBoxWindow::DeselectAll()
{
    while ( GetEntryList()->GetSelectedEntryCount() )
    {
        sal_Int32 nS = GetEntryList()->GetSelectedEntryPos( 0 );
        SelectEntry( nS, false );
    }
}

void ImplListBoxWindow::SelectEntry( sal_Int32 nPos, bool bSelect )
{
    if( (mpEntryList->IsEntryPosSelected( nPos ) != bSelect) && mpEntryList->IsEntrySelectable( nPos ) )
    {
        ImplHideFocusRect();
        if( bSelect )
        {
            if( !mbMulti )
            {
                // deselect the selected entry
                sal_Int32 nDeselect = GetEntryList()->GetSelectedEntryPos( 0 );
                if( nDeselect != LISTBOX_ENTRY_NOTFOUND )
                {
                    //SelectEntryPos( nDeselect, false );
                    GetEntryList()->SelectEntry( nDeselect, false );
                    if (IsUpdateMode() && IsReallyVisible())
                        Invalidate();
                }
            }
            mpEntryList->SelectEntry( nPos, true );
            mnCurrentPos = nPos;
            if ( ( nPos != LISTBOX_ENTRY_NOTFOUND ) && IsUpdateMode() )
            {
                Invalidate();
                if ( !IsVisible( nPos ) )
                {
                    ImplClearLayoutData();
                    sal_Int32 nVisibleEntries = GetLastVisibleEntry()-mnTop;
                    if ( !nVisibleEntries || !IsReallyVisible() || ( nPos < GetTopEntry() ) )
                    {
                        Resize();
                        ShowProminentEntry( nPos );
                    }
                    else
                    {
                        ShowProminentEntry( nPos );
                    }
                }
            }
        }
        else
        {
            mpEntryList->SelectEntry( nPos, false );
            Invalidate();
        }
        mbSelectionChanged = true;
    }
}

bool ImplListBoxWindow::SelectEntries( sal_Int32 nSelect, LB_EVENT_TYPE eLET, bool bShift, bool bCtrl, bool bSelectPosChange /*=FALSE*/ )
{
    bool bSelectionChanged = false;

    if( IsEnabled() && mpEntryList->IsEntrySelectable( nSelect ) )
    {
        bool bFocusChanged = false;

        // here (Single-ListBox) only one entry can be deselected
        if( !mbMulti )
        {
            sal_Int32 nDeselect = mpEntryList->GetSelectedEntryPos( 0 );
            if( nSelect != nDeselect )
            {
                SelectEntry( nSelect, true );
                mpEntryList->SetLastSelected( nSelect );
                bFocusChanged = true;
                bSelectionChanged = true;
            }
        }
        // MultiListBox without Modifier
        else if( mbSimpleMode && !bCtrl && !bShift )
        {
            sal_Int32 nEntryCount = mpEntryList->GetEntryCount();
            for ( sal_Int32 nPos = 0; nPos < nEntryCount; nPos++ )
            {
                bool bSelect = nPos == nSelect;
                if ( mpEntryList->IsEntryPosSelected( nPos ) != bSelect )
                {
                    SelectEntry( nPos, bSelect );
                    bFocusChanged = true;
                    bSelectionChanged = true;
                }
            }
            mpEntryList->SetLastSelected( nSelect );
            mpEntryList->SetSelectionAnchor( nSelect );
        }
        // MultiListBox only with CTRL/SHIFT or not in SimpleMode
        else if( ( !mbSimpleMode /* && !bShift */ ) || ( (mbSimpleMode && ( bCtrl || bShift )) || mbStackMode ) )
        {
            // Space for selection change
            if( !bShift && ( ( eLET == LET_KEYSPACE ) || ( eLET == LET_MBDOWN ) ) )
            {
                bool bSelect = ( mbStackMode && IsMouseMoveSelect() ) || !mpEntryList->IsEntryPosSelected( nSelect );
                if ( mbStackMode )
                {
                    sal_Int32 n;
                    if ( bSelect )
                    {
                        // All entries before nSelect must be selected...
                        for ( n = 0; n < nSelect; n++ )
                            SelectEntry( n, true );
                    }
                    if ( !bSelect )
                    {
                        for ( n = nSelect+1; n < mpEntryList->GetEntryCount(); n++ )
                            SelectEntry( n, false );
                    }
                }
                SelectEntry( nSelect, bSelect );
                mpEntryList->SetLastSelected( nSelect );
                mpEntryList->SetSelectionAnchor( mbStackMode ? 0 : nSelect );
                if ( !mpEntryList->IsEntryPosSelected( nSelect ) )
                    mpEntryList->SetSelectionAnchor( LISTBOX_ENTRY_NOTFOUND );
                bFocusChanged = true;
                bSelectionChanged = true;
            }
            else if( ( ( eLET == LET_TRACKING ) && ( nSelect != mnCurrentPos ) ) ||
                     ( (bShift||mbStackMode) && ( ( eLET == LET_KEYMOVE ) || ( eLET == LET_MBDOWN ) ) ) )
            {
                mnCurrentPos = nSelect;
                bFocusChanged = true;

                sal_Int32 nAnchor = mpEntryList->GetSelectionAnchor();
                if( ( nAnchor == LISTBOX_ENTRY_NOTFOUND ) && ( mpEntryList->GetSelectedEntryCount() || mbStackMode ) )
                {
                    nAnchor = mbStackMode ? 0 : mpEntryList->GetSelectedEntryPos( mpEntryList->GetSelectedEntryCount() - 1 );
                }
                if( nAnchor != LISTBOX_ENTRY_NOTFOUND )
                {
                    // All entries from Anchor to nSelect have to be selected
                    sal_Int32 nStart = std::min( nSelect, nAnchor );
                    sal_Int32 nEnd = std::max( nSelect, nAnchor );
                    for ( sal_Int32 n = nStart; n <= nEnd; n++ )
                    {
                        if ( !mpEntryList->IsEntryPosSelected( n ) )
                        {
                            SelectEntry( n, true );
                            bSelectionChanged = true;
                        }
                    }

                    // if appropriate some more has to be deselected...
                    sal_Int32 nLast = mpEntryList->GetLastSelected();
                    if ( nLast != LISTBOX_ENTRY_NOTFOUND )
                    {
                        if ( ( nLast > nSelect ) && ( nLast > nAnchor ) )
                        {
                            for ( sal_Int32 n = nSelect+1; n <= nLast; n++ )
                            {
                                if ( mpEntryList->IsEntryPosSelected( n ) )
                                {
                                    SelectEntry( n, false );
                                    bSelectionChanged = true;
                                }
                            }
                        }
                        else if ( ( nLast < nSelect ) && ( nLast < nAnchor ) )
                        {
                            for ( sal_Int32 n = nLast; n < nSelect; n++ )
                            {
                                if ( mpEntryList->IsEntryPosSelected( n ) )
                                {
                                    SelectEntry( n, false );
                                    bSelectionChanged = true;
                                }
                            }
                        }
                    }
                    mpEntryList->SetLastSelected( nSelect );
                }
            }
            else if( eLET != LET_TRACKING )
            {
                ImplHideFocusRect();
                Invalidate();
                bFocusChanged = true;
            }
        }
        else if( bShift )
        {
            bFocusChanged = true;
        }

        if( bSelectionChanged )
            mbSelectionChanged = true;

        if( bFocusChanged )
        {
            long nHeightDiff = mpEntryList->GetAddedHeight( nSelect, mnTop );
            maFocusRect.SetPos( Point( 0, nHeightDiff ) );
            Size aSz( maFocusRect.GetWidth(),
                      mpEntryList->GetEntryHeight( nSelect ) );
            maFocusRect.SetSize( aSz );
            if( HasFocus() )
                ImplShowFocusRect();
            if (bSelectPosChange)
            {
                maFocusHdl.Call(nSelect);
            }
        }
        ImplClearLayoutData();
    }
    return bSelectionChanged;
}

void ImplListBoxWindow::Tracking( const TrackingEvent& rTEvt )
{
    tools::Rectangle aRect( Point(), GetOutputSizePixel() );
    bool bInside = aRect.IsInside( rTEvt.GetMouseEvent().GetPosPixel() );

    if( rTEvt.IsTrackingCanceled() || rTEvt.IsTrackingEnded() ) // MouseButtonUp
    {
        if ( bInside && !rTEvt.IsTrackingCanceled() )
        {
            mnSelectModifier = rTEvt.GetMouseEvent().GetModifier();
            ImplCallSelect();
        }
        else
        {
            maCancelHdl.Call( nullptr );
            if ( !mbMulti )
            {
                mbTrackingSelect = true;
                SelectEntry( mnTrackingSaveSelection, true );
                mbTrackingSelect = false;
                if ( mnTrackingSaveSelection != LISTBOX_ENTRY_NOTFOUND )
                {
                    long nHeightDiff = mpEntryList->GetAddedHeight( mnCurrentPos, mnTop );
                    maFocusRect.SetPos( Point( 0, nHeightDiff ) );
                    Size aSz( maFocusRect.GetWidth(),
                              mpEntryList->GetEntryHeight( mnCurrentPos ) );
                    maFocusRect.SetSize( aSz );
                    ImplShowFocusRect();
                }
            }
        }

        mbTrack = false;
    }
    else
    {
        bool bTrackOrQuickClick = mbTrack;
        if( !mbTrack )
        {
            if ( bInside )
            {
                mbTrack = true;
            }

            // this case only happens, if the mouse button is pressed very briefly
            if( rTEvt.IsTrackingEnded() && mbTrack )
            {
                bTrackOrQuickClick = true;
                mbTrack = false;
            }
        }

        if( bTrackOrQuickClick )
        {
            MouseEvent aMEvt = rTEvt.GetMouseEvent();
            Point aPt( aMEvt.GetPosPixel() );
            bool bShift = aMEvt.IsShift();
            bool bCtrl  = aMEvt.IsMod1();

            sal_Int32 nSelect = LISTBOX_ENTRY_NOTFOUND;
            if( aPt.Y() < 0 )
            {
                if ( mnCurrentPos != LISTBOX_ENTRY_NOTFOUND )
                {
                    nSelect = mnCurrentPos ? ( mnCurrentPos - 1 ) : 0;
                    if( nSelect < mnTop )
                        SetTopEntry( mnTop-1 );
                }
            }
            else if( aPt.Y() > GetOutputSizePixel().Height() )
            {
                if ( mnCurrentPos != LISTBOX_ENTRY_NOTFOUND )
                {
                    nSelect = std::min(  static_cast<sal_Int32>(mnCurrentPos+1), static_cast<sal_Int32>(mpEntryList->GetEntryCount()-1) );
                    if( nSelect >= GetLastVisibleEntry() )
                        SetTopEntry( mnTop+1 );
                }
            }
            else
            {
                nSelect = static_cast<sal_Int32>( ( aPt.Y() + gnBorder ) / mnMaxHeight ) + mnTop;
                nSelect = std::min( nSelect, GetLastVisibleEntry() );
                nSelect = std::min( nSelect, static_cast<sal_Int32>( mpEntryList->GetEntryCount() - 1 ) );
            }

            if ( bInside )
            {
                if ( ( nSelect != mnCurrentPos ) || !GetEntryList()->GetSelectedEntryCount() )
                {
                    mbTrackingSelect = true;
                    if ( SelectEntries( nSelect, LET_TRACKING, bShift, bCtrl ) )
                    {
                        if ( mbStackMode )
                        {
                            mbTravelSelect = true;
                            mnSelectModifier = rTEvt.GetMouseEvent().GetModifier();
                            ImplCallSelect();
                            mbTravelSelect = false;
                        }
                    }
                    mbTrackingSelect = false;
                }
            }
            else
            {
                if ( !mbMulti && GetEntryList()->GetSelectedEntryCount() )
                {
                    mbTrackingSelect = true;
                    SelectEntry( GetEntryList()->GetSelectedEntryPos( 0 ), false );
                    mbTrackingSelect = false;
                }
                else if ( mbStackMode )
                {
                    if ( ( rTEvt.GetMouseEvent().GetPosPixel().X() > 0 )  && ( rTEvt.GetMouseEvent().GetPosPixel().X() < aRect.Right() ) )
                    {
                        if ( ( rTEvt.GetMouseEvent().GetPosPixel().Y() < 0 ) || ( rTEvt.GetMouseEvent().GetPosPixel().Y() > GetOutputSizePixel().Height() ) )
                        {
                            bool bSelectionChanged = false;
                            if ( ( rTEvt.GetMouseEvent().GetPosPixel().Y() < 0 )
                                   && !mnCurrentPos )
                            {
                                if ( mpEntryList->IsEntryPosSelected( 0 ) )
                                {
                                    SelectEntry( 0, false );
                                    bSelectionChanged = true;
                                    nSelect = LISTBOX_ENTRY_NOTFOUND;

                                }
                            }
                            else
                            {
                                mbTrackingSelect = true;
                                bSelectionChanged = SelectEntries( nSelect, LET_TRACKING, bShift, bCtrl );
                                mbTrackingSelect = false;
                            }

                            if ( bSelectionChanged )
                            {
                                mbSelectionChanged = true;
                                mbTravelSelect = true;
                                mnSelectModifier = rTEvt.GetMouseEvent().GetModifier();
                                ImplCallSelect();
                                mbTravelSelect = false;
                            }
                        }
                    }
                }
            }
            mnCurrentPos = nSelect;
            if ( mnCurrentPos == LISTBOX_ENTRY_NOTFOUND )
            {
                ImplHideFocusRect();
            }
            else
            {
                long nHeightDiff = mpEntryList->GetAddedHeight( mnCurrentPos, mnTop );
                maFocusRect.SetPos( Point( 0, nHeightDiff ) );
                Size aSz( maFocusRect.GetWidth(), mpEntryList->GetEntryHeight( mnCurrentPos ) );
                maFocusRect.SetSize( aSz );
                ImplShowFocusRect();
            }
        }
    }
}

void ImplListBoxWindow::KeyInput( const KeyEvent& rKEvt )
{
    if( !ProcessKeyInput( rKEvt ) )
        Control::KeyInput( rKEvt );
}

bool ImplListBoxWindow::ProcessKeyInput( const KeyEvent& rKEvt )
{
    // entry to be selected
    sal_Int32 nSelect = LISTBOX_ENTRY_NOTFOUND;
    LB_EVENT_TYPE eLET = LET_KEYMOVE;

    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

    bool bShift = aKeyCode.IsShift();
    bool bCtrl  = aKeyCode.IsMod1() || aKeyCode.IsMod3();
    bool bMod2 = aKeyCode.IsMod2();
    bool bDone = false;
    bool bHandleKey = false;

    switch( aKeyCode.GetCode() )
    {
        case KEY_UP:
        {
            if ( IsReadOnly() )
            {
                if ( GetTopEntry() )
                    SetTopEntry( GetTopEntry()-1 );
            }
            else if ( !bMod2 )
            {
                if( mnCurrentPos == LISTBOX_ENTRY_NOTFOUND )
                {
                    nSelect = mpEntryList->FindFirstSelectable( 0 );
                }
                else if ( mnCurrentPos )
                {
                    // search first selectable above the current position
                    nSelect = mpEntryList->FindFirstSelectable( mnCurrentPos - 1, false );
                }

                if( ( nSelect != LISTBOX_ENTRY_NOTFOUND ) && ( nSelect < mnTop ) )
                    SetTopEntry( mnTop-1 );

                bDone = true;
            }
            maQuickSelectionEngine.Reset();
        }
        break;

        case KEY_DOWN:
        {
            if ( IsReadOnly() )
            {
                SetTopEntry( GetTopEntry()+1 );
            }
            else if ( !bMod2 )
            {
                if( mnCurrentPos == LISTBOX_ENTRY_NOTFOUND )
                {
                    nSelect = mpEntryList->FindFirstSelectable( 0 );
                }
                else if ( (mnCurrentPos+1) < mpEntryList->GetEntryCount() )
                {
                    // search first selectable below the current position
                    nSelect = mpEntryList->FindFirstSelectable( mnCurrentPos + 1 );
                }

                if( ( nSelect != LISTBOX_ENTRY_NOTFOUND ) && ( nSelect >= GetLastVisibleEntry() ) )
                    SetTopEntry( mnTop+1 );

                bDone = true;
            }
            maQuickSelectionEngine.Reset();
        }
        break;

        case KEY_PAGEUP:
        {
            if ( IsReadOnly() )
            {
                sal_Int32 nCurVis = GetLastVisibleEntry() - mnTop +1;
                SetTopEntry( ( mnTop > nCurVis ) ?
                                (mnTop-nCurVis) : 0 );
            }
            else if ( !bCtrl && !bMod2 )
            {
                if( mnCurrentPos == LISTBOX_ENTRY_NOTFOUND )
                {
                    nSelect = mpEntryList->FindFirstSelectable( 0 );
                }
                else if ( mnCurrentPos )
                {
                    if( mnCurrentPos == mnTop )
                    {
                        sal_Int32 nCurVis = GetLastVisibleEntry() - mnTop +1;
                        SetTopEntry( ( mnTop > nCurVis ) ? ( mnTop-nCurVis+1 ) : 0 );
                    }

                    // find first selectable starting from mnTop looking forward
                    nSelect = mpEntryList->FindFirstSelectable( mnTop );
                }
                bDone = true;
            }
            maQuickSelectionEngine.Reset();
        }
        break;

        case KEY_PAGEDOWN:
        {
            if ( IsReadOnly() )
            {
                SetTopEntry( GetLastVisibleEntry() );
            }
            else if ( !bCtrl && !bMod2 )
            {
                if( mnCurrentPos == LISTBOX_ENTRY_NOTFOUND )
                {
                    nSelect = mpEntryList->FindFirstSelectable( 0 );
                }
                else if ( (mnCurrentPos+1) < mpEntryList->GetEntryCount() )
                {
                    sal_Int32 nCount = mpEntryList->GetEntryCount();
                    sal_Int32 nCurVis = GetLastVisibleEntry() - mnTop;
                    sal_Int32 nTmp = std::min( nCurVis, nCount );
                    nTmp += mnTop - 1;
                    if( mnCurrentPos == nTmp && mnCurrentPos != nCount - 1 )
                    {
                        long nTmp2 = std::min( static_cast<long>(nCount-nCurVis), static_cast<long>(static_cast<long>(mnTop)+static_cast<long>(nCurVis)-1) );
                        nTmp2 = std::max( long(0) , nTmp2 );
                        nTmp = static_cast<sal_Int32>(nTmp2+(nCurVis-1) );
                        SetTopEntry( static_cast<sal_Int32>(nTmp2) );
                    }
                    // find first selectable starting from nTmp looking backwards
                    nSelect = mpEntryList->FindFirstSelectable( nTmp, false );
                }
                bDone = true;
            }
            maQuickSelectionEngine.Reset();
        }
        break;

        case KEY_HOME:
        {
            if ( IsReadOnly() )
            {
                SetTopEntry( 0 );
            }
            else if ( !bCtrl && !bMod2 &&  mnCurrentPos )
            {
                nSelect = mpEntryList->FindFirstSelectable( mpEntryList->GetEntryCount() ? 0 : LISTBOX_ENTRY_NOTFOUND );
                if( mnTop != 0 )
                    SetTopEntry( 0 );

                bDone = true;
            }
            maQuickSelectionEngine.Reset();
        }
        break;

        case KEY_END:
        {
            if ( IsReadOnly() )
            {
                SetTopEntry( 0xFFFF );
            }
            else if ( !bCtrl && !bMod2 )
            {
                if( mnCurrentPos == LISTBOX_ENTRY_NOTFOUND )
                {
                    nSelect = mpEntryList->FindFirstSelectable( 0 );
                }
                else if ( (mnCurrentPos+1) < mpEntryList->GetEntryCount() )
                {
                    sal_Int32 nCount = mpEntryList->GetEntryCount();
                    nSelect = mpEntryList->FindFirstSelectable( nCount - 1, false );
                    sal_Int32 nCurVis = GetLastVisibleEntry() - mnTop + 1;
                    if( nCount > nCurVis )
                        SetTopEntry( nCount - nCurVis );
                }
                bDone = true;
            }
            maQuickSelectionEngine.Reset();
        }
        break;

        case KEY_LEFT:
        {
            if ( !bCtrl && !bMod2 )
            {
                ScrollHorz( -HORZ_SCROLL );
                bDone = true;
            }
            maQuickSelectionEngine.Reset();
        }
        break;

        case KEY_RIGHT:
        {
            if ( !bCtrl && !bMod2 )
            {
                ScrollHorz( HORZ_SCROLL );
                bDone = true;
            }
            maQuickSelectionEngine.Reset();
        }
        break;

        case KEY_RETURN:
        {
            if ( !bMod2 && !IsReadOnly() )
            {
                mnSelectModifier = rKEvt.GetKeyCode().GetModifier();
                ImplCallSelect();
                bDone = false;  // do not catch RETURN
            }
            maQuickSelectionEngine.Reset();
        }
        break;

        case KEY_SPACE:
        {
            if ( !bMod2 && !IsReadOnly() )
            {
                if( mbMulti && ( !mbSimpleMode || ( mbSimpleMode && bCtrl && !bShift ) || mbStackMode ) )
                {
                    nSelect = mnCurrentPos;
                    eLET = LET_KEYSPACE;
                }
                bDone = true;
            }
            bHandleKey = true;
        }
        break;

        case KEY_A:
        {
            if( bCtrl && mbMulti )
            {
                // paint only once
                bool bUpdates = IsUpdateMode();
                SetUpdateMode( false );

                sal_Int32 nEntryCount = mpEntryList->GetEntryCount();
                for( sal_Int32 i = 0; i < nEntryCount; i++ )
                    SelectEntry( i, true );

                // restore update mode
                SetUpdateMode( bUpdates );
                Invalidate();

                maQuickSelectionEngine.Reset();

                bDone = true;
            }
            else
            {
                bHandleKey = true;
            }
        }
        break;

        default:
            bHandleKey = true;
            break;
    }
    if (bHandleKey && !IsReadOnly())
    {
        bDone = maQuickSelectionEngine.HandleKeyEvent( rKEvt );
    }

    if  (   ( nSelect != LISTBOX_ENTRY_NOTFOUND )
        &&  (   ( !mpEntryList->IsEntryPosSelected( nSelect ) )
            ||  ( eLET == LET_KEYSPACE )
            )
        )
    {
        SAL_WARN_IF( mpEntryList->IsEntryPosSelected( nSelect ) && !mbMulti, "vcl", "ImplListBox: Selecting same Entry" );
        sal_Int32 nCount = mpEntryList->GetEntryCount();
        if (nSelect >= nCount)
            nSelect = nCount ? nCount-1 : LISTBOX_ENTRY_NOTFOUND;
        bool bCurPosChange = (mnCurrentPos != nSelect);
        mnCurrentPos = nSelect;
        if(SelectEntries( nSelect, eLET, bShift, bCtrl, bCurPosChange))
        {
            mbTravelSelect = true;
            mnSelectModifier = rKEvt.GetKeyCode().GetModifier();
            ImplCallSelect();
            mbTravelSelect = false;
        }
    }

    return bDone;
}

namespace
{
    vcl::StringEntryIdentifier lcl_getEntry( const ImplEntryList& _rList, sal_Int32 _nPos, OUString& _out_entryText )
    {
        OSL_PRECOND( ( _nPos != LISTBOX_ENTRY_NOTFOUND ), "lcl_getEntry: invalid position!" );
        sal_Int32 nEntryCount( _rList.GetEntryCount() );
        if ( _nPos >= nEntryCount )
            _nPos = 0;
        _out_entryText = _rList.GetEntryText( _nPos );

        // vcl::StringEntryIdentifier does not allow for 0 values, but our position is 0-based
        // => normalize
        return reinterpret_cast< vcl::StringEntryIdentifier >( _nPos + 1 );
    }

    sal_Int32 lcl_getEntryPos( vcl::StringEntryIdentifier _entry )
    {
        // our pos is 0-based, but StringEntryIdentifier does not allow for a NULL
        return static_cast< sal_Int32 >( reinterpret_cast< sal_Int64 >( _entry ) ) - 1;
    }
}

vcl::StringEntryIdentifier ImplListBoxWindow::CurrentEntry( OUString& _out_entryText ) const
{
    return lcl_getEntry( *GetEntryList(), ( mnCurrentPos == LISTBOX_ENTRY_NOTFOUND ) ? 0 : mnCurrentPos, _out_entryText );
}

vcl::StringEntryIdentifier ImplListBoxWindow::NextEntry( vcl::StringEntryIdentifier _currentEntry, OUString& _out_entryText ) const
{
    sal_Int32 nNextPos = lcl_getEntryPos( _currentEntry ) + 1;
    return lcl_getEntry( *GetEntryList(), nNextPos, _out_entryText );
}

void ImplListBoxWindow::SelectEntry( vcl::StringEntryIdentifier _entry )
{
    sal_Int32 nSelect = lcl_getEntryPos( _entry );
    if ( mpEntryList->IsEntryPosSelected( nSelect ) )
    {
        // ignore that. This method is a callback from the QuickSelectionEngine, which means the user attempted
        // to select the given entry by typing its starting letters. No need to act.
        return;
    }

    // normalize
    OSL_ENSURE( nSelect < mpEntryList->GetEntryCount(), "ImplListBoxWindow::SelectEntry: how that?" );
    sal_Int32 nCount = mpEntryList->GetEntryCount();
    if (nSelect >= nCount)
        nSelect = nCount ? nCount-1 : LISTBOX_ENTRY_NOTFOUND;

    // make visible
    ShowProminentEntry( nSelect );

    // actually select
    mnCurrentPos = nSelect;
    if ( SelectEntries( nSelect, LET_KEYMOVE ) )
    {
        mbTravelSelect = true;
        mnSelectModifier = 0;
        ImplCallSelect();
        mbTravelSelect = false;
    }
}

void ImplListBoxWindow::ImplPaint(vcl::RenderContext& rRenderContext, sal_Int32 nPos)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    const ImplEntryType* pEntry = mpEntryList->GetEntryPtr( nPos );
    if (!pEntry)
        return;

    long nWidth = GetOutputSizePixel().Width();
    long nY = mpEntryList->GetAddedHeight(nPos, mnTop);
    tools::Rectangle aRect(Point(0, nY), Size(nWidth, pEntry->mnHeight));

    if (mpEntryList->IsEntryPosSelected(nPos))
    {
        rRenderContext.SetTextColor(!IsEnabled() ? rStyleSettings.GetDisableColor() : rStyleSettings.GetHighlightTextColor());
        rRenderContext.SetFillColor(rStyleSettings.GetHighlightColor());
        rRenderContext.SetLineColor();
        rRenderContext.DrawRect(aRect);
    }
    else
    {
        ApplySettings(rRenderContext);
        if (!IsEnabled())
            rRenderContext.SetTextColor(rStyleSettings.GetDisableColor());
    }
    rRenderContext.SetTextFillColor();

    if (IsUserDrawEnabled())
    {
        mbInUserDraw = true;
        mnUserDrawEntry = nPos;
        aRect.AdjustLeft( -mnLeft );
        if (nPos < GetEntryList()->GetMRUCount())
            nPos = GetEntryList()->FindEntry(GetEntryList()->GetEntryText(nPos));
        nPos = nPos - GetEntryList()->GetMRUCount();
        sal_Int32 nCurr = mnCurrentPos;
        if (mnCurrentPos < GetEntryList()->GetMRUCount())
            nCurr = GetEntryList()->FindEntry(GetEntryList()->GetEntryText(nCurr));
        nCurr = sal::static_int_cast<sal_Int32>(nCurr - GetEntryList()->GetMRUCount());

        UserDrawEvent aUDEvt(this, &rRenderContext, aRect, nPos, nCurr);
        maUserDrawHdl.Call( &aUDEvt );
        mbInUserDraw = false;
    }
    else
    {
        DrawEntry(rRenderContext, nPos, true, true);
    }
}

void ImplListBoxWindow::DrawEntry(vcl::RenderContext& rRenderContext, sal_Int32 nPos, bool bDrawImage, bool bDrawText, bool bDrawTextAtImagePos)
{
    const ImplEntryType* pEntry = mpEntryList->GetEntryPtr(nPos);
    if (!pEntry)
        return;

    // when changing this function don't forget to adjust ImplWin::DrawEntry()

    if (mbInUserDraw)
        nPos = mnUserDrawEntry; // real entry, not the matching entry from MRU

    long nY = mpEntryList->GetAddedHeight(nPos, mnTop);
    Size aImgSz;

    if (bDrawImage && mpEntryList->HasImages())
    {
        Image aImage = mpEntryList->GetEntryImage(nPos);
        if (!!aImage)
        {
            aImgSz = aImage.GetSizePixel();
            Point aPtImg(gnBorder - mnLeft, nY + ((pEntry->mnHeight - aImgSz.Height()) / 2));

            // pb: #106948# explicit mirroring for calc
            if (mbMirroring)
                // right aligned
                aPtImg.setX( mnMaxWidth + gnBorder - aImgSz.Width() - mnLeft );

            if (!IsZoom())
            {
                rRenderContext.DrawImage(aPtImg, aImage);
            }
            else
            {
                aImgSz.setWidth( CalcZoom(aImgSz.Width()) );
                aImgSz.setHeight( CalcZoom(aImgSz.Height()) );
                rRenderContext.DrawImage(aPtImg, aImgSz, aImage);
            }

            const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
            const sal_uInt16 nEdgeBlendingPercent(GetEdgeBlending() ? rStyleSettings.GetEdgeBlending() : 0);

            if (nEdgeBlendingPercent && aImgSz.Width() && aImgSz.Height())
            {
                const Color& rTopLeft(rStyleSettings.GetEdgeBlendingTopLeftColor());
                const Color& rBottomRight(rStyleSettings.GetEdgeBlendingBottomRightColor());
                const sal_uInt8 nAlpha((nEdgeBlendingPercent * 255) / 100);
                const BitmapEx aBlendFrame(createBlendFrame(aImgSz, nAlpha, rTopLeft, rBottomRight));

                if (!aBlendFrame.IsEmpty())
                {
                    rRenderContext.DrawBitmapEx(aPtImg, aBlendFrame);
                }
            }
        }
    }

    if (bDrawText)
    {
        OUString aStr(mpEntryList->GetEntryText(nPos));
        if (!aStr.isEmpty())
        {
            long nMaxWidth = std::max(mnMaxWidth, GetOutputSizePixel().Width() - 2 * gnBorder);
            // a multiline entry should only be as wide a the window
            if (pEntry->mnFlags & ListBoxEntryFlags::MultiLine)
                nMaxWidth = GetOutputSizePixel().Width() - 2 * gnBorder;

            tools::Rectangle aTextRect(Point(gnBorder - mnLeft, nY),
                                Size(nMaxWidth, pEntry->mnHeight));

            if (!bDrawTextAtImagePos && (mpEntryList->HasEntryImage(nPos) || IsUserDrawEnabled()))
            {
                long nImageWidth = std::max(mnMaxImgWidth, maUserItemSize.Width());
                aTextRect.AdjustLeft(nImageWidth + IMG_TXT_DISTANCE );
            }

            // pb: #106948# explicit mirroring for calc
            if (mbMirroring)
            {
                // right aligned
                aTextRect.SetLeft( nMaxWidth + gnBorder - rRenderContext.GetTextWidth(aStr) - mnLeft );
                if (aImgSz.Width() > 0)
                    aTextRect.AdjustLeft( -(aImgSz.Width() + IMG_TXT_DISTANCE) );
            }

            DrawTextFlags nDrawStyle = ImplGetTextStyle();
            if (pEntry->mnFlags & ListBoxEntryFlags::MultiLine)
                nDrawStyle |= MULTILINE_ENTRY_DRAW_FLAGS;
            if (pEntry->mnFlags & ListBoxEntryFlags::DrawDisabled)
                nDrawStyle |= DrawTextFlags::Disable;

            rRenderContext.DrawText(aTextRect, aStr, nDrawStyle);
        }
    }

    if ( !maSeparators.empty() && ( isSeparator(nPos) || isSeparator(nPos-1) ) )
    {
        Color aOldLineColor(rRenderContext.GetLineColor());
        rRenderContext.SetLineColor((GetBackground().GetColor() != COL_LIGHTGRAY) ? COL_LIGHTGRAY : COL_GRAY);
        Point aStartPos(0, nY);
        if (isSeparator(nPos))
            aStartPos.AdjustY(pEntry->mnHeight - 1 );
        Point aEndPos(aStartPos);
        aEndPos.setX( GetOutputSizePixel().Width() );
        rRenderContext.DrawLine(aStartPos, aEndPos);
        rRenderContext.SetLineColor(aOldLineColor);
    }
}

void ImplListBoxWindow::FillLayoutData() const
{
    mpControlData->mpLayoutData.reset( new vcl::ControlLayoutData );
    const_cast<ImplListBoxWindow*>(this)->Invalidate(tools::Rectangle(Point(0, 0), GetOutputSize()));
}

void ImplListBoxWindow::ImplDoPaint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    sal_Int32 nCount = mpEntryList->GetEntryCount();

    bool bShowFocusRect = mbHasFocusRect;
    if (mbHasFocusRect)
        ImplHideFocusRect();

    long nY = 0; // + gnBorder;
    long nHeight = GetOutputSizePixel().Height();// - mnMaxHeight + gnBorder;

    for (sal_Int32 i = mnTop; i < nCount && nY < nHeight + mnMaxHeight; i++)
    {
        const ImplEntryType* pEntry = mpEntryList->GetEntryPtr(i);
        if (nY + pEntry->mnHeight >= rRect.Top() &&
            nY <= rRect.Bottom() + mnMaxHeight)
        {
            ImplPaint(rRenderContext, i);
        }
        nY += pEntry->mnHeight;
    }

    long nHeightDiff = mpEntryList->GetAddedHeight(mnCurrentPos, mnTop);
    maFocusRect.SetPos(Point(0, nHeightDiff));
    Size aSz(maFocusRect.GetWidth(), mpEntryList->GetEntryHeight(mnCurrentPos));
    maFocusRect.SetSize(aSz);
    if (HasFocus() && bShowFocusRect)
        ImplShowFocusRect();
}

void ImplListBoxWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    ImplDoPaint(rRenderContext, rRect);
}

sal_uInt16 ImplListBoxWindow::GetDisplayLineCount() const
{
    // FIXME: ListBoxEntryFlags::MultiLine

    const sal_Int32 nCount = mpEntryList->GetEntryCount()-mnTop;
    long nHeight = GetOutputSizePixel().Height();// - mnMaxHeight + gnBorder;
    sal_uInt16 nEntries = static_cast< sal_uInt16 >( ( nHeight + mnMaxHeight - 1 ) / mnMaxHeight );
    if( nEntries > nCount )
        nEntries = static_cast<sal_uInt16>(nCount);

    return nEntries;
}

void ImplListBoxWindow::Resize()
{
    Control::Resize();

    bool bShowFocusRect = mbHasFocusRect;
    if ( bShowFocusRect )
        ImplHideFocusRect();

    if( mnCurrentPos != LISTBOX_ENTRY_NOTFOUND )
    {
        Size aSz( GetOutputSizePixel().Width(), mpEntryList->GetEntryHeight( mnCurrentPos ) );
        maFocusRect.SetSize( aSz );
    }

    if ( bShowFocusRect )
        ImplShowFocusRect();

    ImplClearLayoutData();
}

void ImplListBoxWindow::GetFocus()
{
    sal_Int32 nPos = mnCurrentPos;
    if ( nPos == LISTBOX_ENTRY_NOTFOUND )
        nPos = 0;
    long nHeightDiff = mpEntryList->GetAddedHeight( nPos, mnTop );
    maFocusRect.SetPos( Point( 0, nHeightDiff ) );
    Size aSz( maFocusRect.GetWidth(), mpEntryList->GetEntryHeight( nPos ) );
    maFocusRect.SetSize( aSz );
    ImplShowFocusRect();
    Control::GetFocus();
}

void ImplListBoxWindow::LoseFocus()
{
    ImplHideFocusRect();
    Control::LoseFocus();
}

void ImplListBoxWindow::SetTopEntry( sal_Int32 nTop )
{
    if( mpEntryList->GetEntryCount() == 0 )
        return;

    long nWHeight = PixelToLogic( GetSizePixel() ).Height();

    sal_Int32 nLastEntry = mpEntryList->GetEntryCount()-1;
    if( nTop > nLastEntry )
        nTop = nLastEntry;
    const ImplEntryType* pLast = mpEntryList->GetEntryPtr( nLastEntry );
    while( nTop > 0 && mpEntryList->GetAddedHeight( nLastEntry, nTop-1 ) + pLast->mnHeight <= nWHeight )
        nTop--;

    if ( nTop != mnTop )
    {
        ImplClearLayoutData();
        long nDiff = mpEntryList->GetAddedHeight( mnTop, nTop );
        Update();
        ImplHideFocusRect();
        mnTop = nTop;
        Scroll( 0, nDiff );
        Update();
        if( HasFocus() )
            ImplShowFocusRect();
        maScrollHdl.Call( this );
    }
}

void ImplListBoxWindow::ShowProminentEntry( sal_Int32 nEntryPos )
{
    if( meProminentType == ProminentEntry::MIDDLE )
    {
        sal_Int32 nPos = nEntryPos;
        long nWHeight = PixelToLogic( GetSizePixel() ).Height();
        while( nEntryPos > 0 && mpEntryList->GetAddedHeight( nPos+1, nEntryPos ) < nWHeight/2 )
            nEntryPos--;
    }
    SetTopEntry( nEntryPos );
}

void ImplListBoxWindow::SetLeftIndent( long n )
{
    ScrollHorz( n - mnLeft );
}

void ImplListBoxWindow::ScrollHorz( long n )
{
    long nDiff = 0;
    if ( n > 0 )
    {
        long nWidth = GetOutputSizePixel().Width();
        if( ( mnMaxWidth - mnLeft + n ) > nWidth )
            nDiff = n;
    }
    else if ( n < 0 )
    {
        if( mnLeft )
        {
            long nAbs = -n;
            nDiff = - std::min( mnLeft, nAbs );
        }
    }

    if ( nDiff )
    {
        ImplClearLayoutData();
        mnLeft = sal::static_int_cast<sal_uInt16>(mnLeft + nDiff);
        Update();
        ImplHideFocusRect();
        Scroll( -nDiff, 0 );
        Update();
        if( HasFocus() )
            ImplShowFocusRect();
        maScrollHdl.Call( this );
    }
}

void ImplListBoxWindow::SetSeparatorPos( sal_Int32 n )
{
    maSeparators.clear();

    if ( n != LISTBOX_ENTRY_NOTFOUND )
    {
        maSeparators.insert( n );
    }
}

sal_Int32 ImplListBoxWindow::GetSeparatorPos() const
{
    if (!maSeparators.empty())
        return *(maSeparators.begin());
    else
        return LISTBOX_ENTRY_NOTFOUND;
}

bool ImplListBoxWindow::isSeparator( const sal_Int32 &n) const
{
    return maSeparators.find(n) != maSeparators.end();
}

Size ImplListBoxWindow::CalcSize(sal_Int32 nMaxLines) const
{
    // FIXME: ListBoxEntryFlags::MultiLine

    Size aSz;
    aSz.setHeight(  nMaxLines * mnMaxHeight );
    aSz.setWidth( mnMaxWidth + 2*gnBorder );
    return aSz;
}

tools::Rectangle ImplListBoxWindow::GetBoundingRectangle( sal_Int32 nItem ) const
{
    const ImplEntryType* pEntry = mpEntryList->GetEntryPtr( nItem );
    Size aSz( GetSizePixel().Width(), pEntry ? pEntry->mnHeight : GetEntryHeight() );
    long nY = mpEntryList->GetAddedHeight( nItem, GetTopEntry() ) + GetEntryList()->GetMRUCount()*GetEntryHeight();
    tools::Rectangle aRect( Point( 0, nY ), aSz );
    return aRect;
}

void ImplListBoxWindow::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( nType == StateChangedType::Zoom )
    {
        ApplySettings(*this);
        ImplCalcMetrics();
        Invalidate();
    }
    else if ( nType == StateChangedType::UpdateMode )
    {
        if ( IsUpdateMode() && IsReallyVisible() )
            Invalidate();
    }
    else if ( nType == StateChangedType::ControlFont )
    {
        ApplySettings(*this);
        ImplCalcMetrics();
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlForeground )
    {
        ApplySettings(*this);
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        ApplySettings(*this);
        Invalidate();
    }
    else if( nType == StateChangedType::Enable )
    {
        Invalidate();
    }

    ImplClearLayoutData();
}

void ImplListBoxWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        ImplClearLayoutData();
        ApplySettings(*this);
        ImplCalcMetrics();
        Invalidate();
    }
}

DrawTextFlags ImplListBoxWindow::ImplGetTextStyle() const
{
    DrawTextFlags nTextStyle = DrawTextFlags::VCenter;

    if (mpEntryList->HasImages())
        nTextStyle |= DrawTextFlags::Left;
    else if (mbCenter)
        nTextStyle |= DrawTextFlags::Center;
    else if (mbRight)
        nTextStyle |= DrawTextFlags::Right;
    else
        nTextStyle |= DrawTextFlags::Left;

    return nTextStyle;
}

ImplListBox::ImplListBox( vcl::Window* pParent, WinBits nWinStyle ) :
    Control( pParent, nWinStyle ),
    maLBWindow(VclPtr<ImplListBoxWindow>::Create( this, nWinStyle&(~WB_BORDER) ))
{
    // for native widget rendering we must be able to detect this window type
    SetType( WindowType::LISTBOXWINDOW );

    mpVScrollBar    = VclPtr<ScrollBar>::Create( this, WB_VSCROLL | WB_DRAG );
    mpHScrollBar    = VclPtr<ScrollBar>::Create( this, WB_HSCROLL | WB_DRAG );
    mpScrollBarBox  = VclPtr<ScrollBarBox>::Create( this );

    Link<ScrollBar*,void> aLink( LINK( this, ImplListBox, ScrollBarHdl ) );
    mpVScrollBar->SetScrollHdl( aLink );
    mpHScrollBar->SetScrollHdl( aLink );

    mbVScroll       = false;
    mbHScroll       = false;
    mbAutoHScroll   = ( nWinStyle & WB_AUTOHSCROLL );
    mbEdgeBlending  = false;

    maLBWindow->SetScrollHdl( LINK( this, ImplListBox, LBWindowScrolled ) );
    maLBWindow->SetMRUChangedHdl( LINK( this, ImplListBox, MRUChanged ) );
    maLBWindow->SetEdgeBlending(GetEdgeBlending());
    maLBWindow->Show();
}

ImplListBox::~ImplListBox()
{
    disposeOnce();
}

void ImplListBox::dispose()
{
    mpHScrollBar.disposeAndClear();
    mpVScrollBar.disposeAndClear();
    mpScrollBarBox.disposeAndClear();
    maLBWindow.disposeAndClear();
    Control::dispose();
}

void ImplListBox::Clear()
{
    maLBWindow->Clear();
    if ( GetEntryList()->GetMRUCount() )
    {
        maLBWindow->GetEntryList()->SetMRUCount( 0 );
        maLBWindow->SetSeparatorPos( LISTBOX_ENTRY_NOTFOUND );
    }
    mpVScrollBar->SetThumbPos( 0 );
    mpHScrollBar->SetThumbPos( 0 );
    CompatStateChanged( StateChangedType::Data );
}

sal_Int32 ImplListBox::InsertEntry( sal_Int32 nPos, const OUString& rStr )
{
    ImplEntryType* pNewEntry = new ImplEntryType( rStr );
    sal_Int32 nNewPos = maLBWindow->InsertEntry( nPos, pNewEntry );
    CompatStateChanged( StateChangedType::Data );
    return nNewPos;
}

sal_Int32 ImplListBox::InsertEntry( sal_Int32 nPos, const OUString& rStr, const Image& rImage )
{
    ImplEntryType* pNewEntry = new ImplEntryType( rStr, rImage );
    sal_Int32 nNewPos = maLBWindow->InsertEntry( nPos, pNewEntry );
    CompatStateChanged( StateChangedType::Data );
    return nNewPos;
}

void ImplListBox::RemoveEntry( sal_Int32 nPos )
{
    maLBWindow->RemoveEntry( nPos );
    CompatStateChanged( StateChangedType::Data );
}

void ImplListBox::SetEntryFlags( sal_Int32 nPos, ListBoxEntryFlags nFlags )
{
    maLBWindow->SetEntryFlags( nPos, nFlags );
}

void ImplListBox::SelectEntry( sal_Int32 nPos, bool bSelect )
{
    maLBWindow->SelectEntry( nPos, bSelect );
}

void ImplListBox::SetNoSelection()
{
    maLBWindow->DeselectAll();
}

void ImplListBox::GetFocus()
{
    if (maLBWindow)
        maLBWindow->GrabFocus();
    else
        Control::GetFocus();
}

void ImplListBox::Resize()
{
    Control::Resize();
    ImplResizeControls();
    ImplCheckScrollBars();
}

IMPL_LINK_NOARG(ImplListBox, MRUChanged, LinkParamNone*, void)
{
    CompatStateChanged( StateChangedType::Data );
}

IMPL_LINK_NOARG(ImplListBox, LBWindowScrolled, ImplListBoxWindow*, void)
{
    long nSet = GetTopEntry();
    if( nSet > mpVScrollBar->GetRangeMax() )
        mpVScrollBar->SetRangeMax( GetEntryList()->GetEntryCount() );
    mpVScrollBar->SetThumbPos( GetTopEntry() );

    mpHScrollBar->SetThumbPos( GetLeftIndent() );

    maScrollHdl.Call( this );
}

IMPL_LINK( ImplListBox, ScrollBarHdl, ScrollBar*, pSB, void )
{
    sal_uInt16 nPos = static_cast<sal_uInt16>(pSB->GetThumbPos());
    if( pSB == mpVScrollBar )
        SetTopEntry( nPos );
    else if( pSB == mpHScrollBar )
        SetLeftIndent( nPos );
}

void ImplListBox::ImplCheckScrollBars()
{
    bool bArrange = false;

    Size aOutSz = GetOutputSizePixel();
    sal_Int32 nEntries = GetEntryList()->GetEntryCount();
    sal_uInt16 nMaxVisEntries = static_cast<sal_uInt16>(aOutSz.Height() / GetEntryHeight());

    // vertical ScrollBar
    if( nEntries > nMaxVisEntries )
    {
        if( !mbVScroll )
            bArrange = true;
        mbVScroll = true;

        // check of the scrolled-out region
        if( GetEntryList()->GetSelectedEntryCount() == 1 &&
            GetEntryList()->GetSelectedEntryPos( 0 ) != LISTBOX_ENTRY_NOTFOUND )
            ShowProminentEntry( GetEntryList()->GetSelectedEntryPos( 0 ) );
        else
            SetTopEntry( GetTopEntry() );   // MaxTop is being checked...
    }
    else
    {
        if( mbVScroll )
            bArrange = true;
        mbVScroll = false;
        SetTopEntry( 0 );
    }

    // horizontal ScrollBar
    if( mbAutoHScroll )
    {
        long nWidth = static_cast<sal_uInt16>(aOutSz.Width());
        if ( mbVScroll )
            nWidth -= mpVScrollBar->GetSizePixel().Width();

        long nMaxWidth = GetMaxEntryWidth();
        if( nWidth < nMaxWidth )
        {
            if( !mbHScroll )
                bArrange = true;
            mbHScroll = true;

            if ( !mbVScroll )   // maybe we do need one now
            {
                nMaxVisEntries = static_cast<sal_uInt16>( ( aOutSz.Height() - mpHScrollBar->GetSizePixel().Height() ) / GetEntryHeight() );
                if( nEntries > nMaxVisEntries )
                {
                    bArrange = true;
                    mbVScroll = true;

                    // check of the scrolled-out region
                    if( GetEntryList()->GetSelectedEntryCount() == 1 &&
                        GetEntryList()->GetSelectedEntryPos( 0 ) != LISTBOX_ENTRY_NOTFOUND )
                        ShowProminentEntry( GetEntryList()->GetSelectedEntryPos( 0 ) );
                    else
                        SetTopEntry( GetTopEntry() );   // MaxTop is being checked...
                }
            }

            // check of the scrolled-out region
            sal_uInt16 nMaxLI = static_cast<sal_uInt16>(nMaxWidth - nWidth);
            if ( nMaxLI < GetLeftIndent() )
                SetLeftIndent( nMaxLI );
        }
        else
        {
            if( mbHScroll )
                bArrange = true;
            mbHScroll = false;
            SetLeftIndent( 0 );
        }
    }

    if( bArrange )
        ImplResizeControls();

    ImplInitScrollBars();
}

void ImplListBox::ImplInitScrollBars()
{
    Size aOutSz = maLBWindow->GetOutputSizePixel();

    if ( mbVScroll )
    {
        sal_Int32 nEntries = GetEntryList()->GetEntryCount();
        sal_uInt16 nVisEntries = static_cast<sal_uInt16>(aOutSz.Height() / GetEntryHeight());
        mpVScrollBar->SetRangeMax( nEntries );
        mpVScrollBar->SetVisibleSize( nVisEntries );
        mpVScrollBar->SetPageSize( nVisEntries - 1 );
    }

    if ( mbHScroll )
    {
        mpHScrollBar->SetRangeMax( GetMaxEntryWidth() + HORZ_SCROLL );
        mpHScrollBar->SetVisibleSize( static_cast<sal_uInt16>(aOutSz.Width()) );
        mpHScrollBar->SetLineSize( HORZ_SCROLL );
        mpHScrollBar->SetPageSize( aOutSz.Width() - HORZ_SCROLL );
    }
}

void ImplListBox::ImplResizeControls()
{
    // Here we only position the Controls; if the Scrollbars are to be
    // visible is already determined in ImplCheckScrollBars

    Size aOutSz = GetOutputSizePixel();
    long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
    nSBWidth = CalcZoom( nSBWidth );

    Size aInnerSz( aOutSz );
    if ( mbVScroll )
        aInnerSz.AdjustWidth( -nSBWidth );
    if ( mbHScroll )
        aInnerSz.AdjustHeight( -nSBWidth );

    // pb: #106948# explicit mirroring for calc
    // Scrollbar on left or right side?
    bool bMirroring = maLBWindow->IsMirroring();
    Point aWinPos( bMirroring && mbVScroll ? nSBWidth : 0, 0 );
    maLBWindow->SetPosSizePixel( aWinPos, aInnerSz );

    // ScrollBarBox
    if( mbVScroll && mbHScroll )
    {
        Point aBoxPos( bMirroring ? 0 : aInnerSz.Width(), aInnerSz.Height() );
        mpScrollBarBox->SetPosSizePixel( aBoxPos, Size( nSBWidth, nSBWidth ) );
        mpScrollBarBox->Show();
    }
    else
    {
        mpScrollBarBox->Hide();
    }

    // vertical ScrollBar
    if( mbVScroll )
    {
        // Scrollbar on left or right side?
        Point aVPos( bMirroring ? 0 : aOutSz.Width() - nSBWidth, 0 );
        mpVScrollBar->SetPosSizePixel( aVPos, Size( nSBWidth, aInnerSz.Height() ) );
        mpVScrollBar->Show();
    }
    else
    {
        mpVScrollBar->Hide();
        // #107254# Don't reset top entry after resize, but check for max top entry
        SetTopEntry( GetTopEntry() );
    }

    // horizontal ScrollBar
    if( mbHScroll )
    {
        Point aHPos( ( bMirroring && mbVScroll ) ? nSBWidth : 0, aOutSz.Height() - nSBWidth );
        mpHScrollBar->SetPosSizePixel( aHPos, Size( aInnerSz.Width(), nSBWidth ) );
        mpHScrollBar->Show();
    }
    else
    {
        mpHScrollBar->Hide();
        SetLeftIndent( 0 );
    }
}

void ImplListBox::StateChanged( StateChangedType nType )
{
    if ( nType == StateChangedType::InitShow )
    {
        ImplCheckScrollBars();
    }
    else if ( ( nType == StateChangedType::UpdateMode ) || ( nType == StateChangedType::Data ) )
    {
        bool bUpdate = IsUpdateMode();
        maLBWindow->SetUpdateMode( bUpdate );
        if ( bUpdate && IsReallyVisible() )
            ImplCheckScrollBars();
    }
    else if( nType == StateChangedType::Enable )
    {
        mpHScrollBar->Enable( IsEnabled() );
        mpVScrollBar->Enable( IsEnabled() );
        mpScrollBarBox->Enable( IsEnabled() );
        maLBWindow->Enable( IsEnabled() );

        Invalidate();
    }
    else if ( nType == StateChangedType::Zoom )
    {
        maLBWindow->SetZoom( GetZoom() );
        Resize();
    }
    else if ( nType == StateChangedType::ControlFont )
    {
        maLBWindow->SetControlFont( GetControlFont() );
    }
    else if ( nType == StateChangedType::ControlForeground )
    {
        maLBWindow->SetControlForeground( GetControlForeground() );
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        maLBWindow->SetControlBackground( GetControlBackground() );
    }
    else if( nType == StateChangedType::Mirroring )
    {
        maLBWindow->EnableRTL( IsRTLEnabled() );
        mpHScrollBar->EnableRTL( IsRTLEnabled() );
        mpVScrollBar->EnableRTL( IsRTLEnabled() );
        ImplResizeControls();
    }

    Control::StateChanged( nType );
}

bool ImplListBox::EventNotify( NotifyEvent& rNEvt )
{
    bool bDone = false;
    if ( rNEvt.GetType() == MouseNotifyEvent::COMMAND )
    {
        const CommandEvent& rCEvt = *rNEvt.GetCommandEvent();
        if ( rCEvt.GetCommand() == CommandEventId::Wheel )
        {
            const CommandWheelData* pData = rCEvt.GetWheelData();
            if( !pData->GetModifier() && ( pData->GetMode() == CommandWheelMode::SCROLL ) )
            {
                bDone = HandleScrollCommand( rCEvt, mpHScrollBar, mpVScrollBar );
            }
        }
    }

    return bDone || Window::EventNotify( rNEvt );
}

const Wallpaper& ImplListBox::GetDisplayBackground() const
{
    return maLBWindow->GetDisplayBackground();
}

bool ImplListBox::HandleWheelAsCursorTravel( const CommandEvent& rCEvt )
{
    bool bDone = false;
    if ( rCEvt.GetCommand() == CommandEventId::Wheel )
    {
        const CommandWheelData* pData = rCEvt.GetWheelData();
        if( !pData->GetModifier() && ( pData->GetMode() == CommandWheelMode::SCROLL ) )
        {
            sal_uInt16 nKey = ( pData->GetDelta() < 0 ) ? KEY_DOWN : KEY_UP;
            KeyEvent aKeyEvent( 0, vcl::KeyCode( nKey ) );
            bDone = ProcessKeyInput( aKeyEvent );
        }
    }
    return bDone;
}

void ImplListBox::SetMRUEntries( const OUString& rEntries, sal_Unicode cSep )
{
    bool bChanges = GetEntryList()->GetMRUCount() != 0;

    // Remove old MRU entries
    for ( sal_Int32 n = GetEntryList()->GetMRUCount();n; )
        maLBWindow->RemoveEntry( --n );

    sal_Int32 nMRUCount = 0;
    sal_Int32 nIndex = 0;
    do
    {
        OUString aEntry = rEntries.getToken( 0, cSep, nIndex );
        // Accept only existing entries
        if ( GetEntryList()->FindEntry( aEntry ) != LISTBOX_ENTRY_NOTFOUND )
        {
            ImplEntryType* pNewEntry = new ImplEntryType( aEntry );
            maLBWindow->GetEntryList()->InsertEntry( nMRUCount++, pNewEntry, false );
            bChanges = true;
        }
    }
    while ( nIndex >= 0 );

    if ( bChanges )
    {
        maLBWindow->GetEntryList()->SetMRUCount( nMRUCount );
        SetSeparatorPos( nMRUCount ? nMRUCount-1 : 0 );
        CompatStateChanged( StateChangedType::Data );
    }
}

OUString ImplListBox::GetMRUEntries( sal_Unicode cSep ) const
{
    OUStringBuffer aEntries;
    for ( sal_Int32 n = 0; n < GetEntryList()->GetMRUCount(); n++ )
    {
        aEntries.append(GetEntryList()->GetEntryText( n ));
        if( n < ( GetEntryList()->GetMRUCount() - 1 ) )
            aEntries.append(cSep);
    }
    return aEntries.makeStringAndClear();
}

void ImplListBox::SetEdgeBlending(bool bNew)
{
    if(mbEdgeBlending != bNew)
    {
        mbEdgeBlending = bNew;
        maLBWindow->SetEdgeBlending(GetEdgeBlending());
    }
}

ImplWin::ImplWin( vcl::Window* pParent, WinBits nWinStyle ) :
    Control ( pParent, nWinStyle )
{
    if ( IsNativeControlSupported(ControlType::Listbox, ControlPart::Entire)
            && ! IsNativeControlSupported(ControlType::Listbox, ControlPart::ButtonDown) )
        SetBackground();
    else
        SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetFieldColor() ) );

    ImplGetWindowImpl()->mbUseNativeFocus = ImplGetSVData()->maNWFData.mbNoFocusRects;

    mbUserDrawEnabled = false;
    mbEdgeBlending = false;
    mnItemPos = LISTBOX_ENTRY_NOTFOUND;
}

void ImplWin::MouseButtonDown( const MouseEvent& )
{
    if( IsEnabled() )
    {
        maMBDownHdl.Call(this);
    }
}

void ImplWin::FillLayoutData() const
{
    mpControlData->mpLayoutData.reset( new vcl::ControlLayoutData );
    ImplWin* pThis = const_cast<ImplWin*>(this);
    pThis->ImplDraw(*pThis, true);
}

bool ImplWin::PreNotify( NotifyEvent& rNEvt )
{
    const MouseEvent* pMouseEvt = nullptr;

    if( (rNEvt.GetType() == MouseNotifyEvent::MOUSEMOVE) && (pMouseEvt = rNEvt.GetMouseEvent()) != nullptr )
    {
        if( pMouseEvt->IsEnterWindow() || pMouseEvt->IsLeaveWindow() )
        {
            // trigger redraw as mouse over state has changed
            if ( IsNativeControlSupported(ControlType::Listbox, ControlPart::Entire)
            && ! IsNativeControlSupported(ControlType::Listbox, ControlPart::ButtonDown) )
            {
                GetParent()->GetWindow( GetWindowType::Border )->Invalidate( InvalidateFlags::NoErase );
                GetParent()->GetWindow( GetWindowType::Border )->Update();
            }
        }
    }

    return Control::PreNotify(rNEvt);
}

void ImplWin::ImplDraw(vcl::RenderContext& rRenderContext, bool bLayout)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    if (!bLayout)
    {
        bool bNativeOK = false;
        bool bHasFocus = HasFocus();
        bool bIsEnabled = IsEnabled();

        ControlState nState = ControlState::ENABLED;
        if (rRenderContext.IsNativeControlSupported(ControlType::Listbox, ControlPart::Entire)
            && rRenderContext.IsNativeControlSupported(ControlType::Listbox, ControlPart::HasBackgroundTexture) )
        {
            // Repaint the (focused) area similarly to
            // ImplSmallBorderWindowView::DrawWindow() in
            // vcl/source/window/brdwin.cxx
            vcl::Window *pWin = GetParent();

            ImplControlValue aControlValue;
            bIsEnabled &= pWin->IsEnabled();
            if ( !bIsEnabled )
                nState &= ~ControlState::ENABLED;
            bHasFocus |= pWin->HasFocus();
            if ( bHasFocus )
                nState |= ControlState::FOCUSED;

            // The listbox is painted over the entire control including the
            // border, but ImplWin does not contain the border => correction
            // needed.
            sal_Int32 nLeft, nTop, nRight, nBottom;
            pWin->GetBorder( nLeft, nTop, nRight, nBottom );
            Point aPoint( -nLeft, -nTop );
            tools::Rectangle aCtrlRegion( aPoint - GetPosPixel(), pWin->GetSizePixel() );

            bool bMouseOver = false;
            vcl::Window *pChild = pWin->GetWindow( GetWindowType::FirstChild );
            while( pChild && !(bMouseOver = pChild->IsMouseOver()) )
                pChild = pChild->GetWindow( GetWindowType::Next );
            if( bMouseOver )
                nState |= ControlState::ROLLOVER;

            // if parent has no border, then nobody has drawn the background
            // since no border window exists. so draw it here.
            WinBits nParentStyle = pWin->GetStyle();
            if( ! (nParentStyle & WB_BORDER) || (nParentStyle & WB_NOBORDER) )
            {
                tools::Rectangle aParentRect( Point( 0, 0 ), pWin->GetSizePixel() );
                pWin->DrawNativeControl( ControlType::Listbox, ControlPart::Entire, aParentRect,
                                         nState, aControlValue, OUString() );
            }

            bNativeOK = rRenderContext.DrawNativeControl(ControlType::Listbox, ControlPart::Entire, aCtrlRegion,
                                                         nState, aControlValue, OUString());
        }

        if (bIsEnabled)
        {
            if (bHasFocus && !ImplGetSVData()->maNWFData.mbDDListBoxNoTextArea)
            {
                if ( !ImplGetSVData()->maNWFData.mbNoFocusRects )
                {
                    rRenderContext.SetFillColor( rStyleSettings.GetHighlightColor() );
                    rRenderContext.SetTextColor( rStyleSettings.GetHighlightTextColor() );
                }
                else
                {
                    rRenderContext.SetLineColor();
                    rRenderContext.SetFillColor();
                    rRenderContext.SetTextColor( rStyleSettings.GetFieldTextColor() );
                }
                rRenderContext.DrawRect( maFocusRect );
            }
            else
            {
                Color aColor;
                if( ImplGetSVData()->maNWFData.mbDDListBoxNoTextArea )
                {
                    if( bNativeOK && (nState & ControlState::ROLLOVER) )
                        aColor = rStyleSettings.GetButtonRolloverTextColor();
                    else
                        aColor = rStyleSettings.GetButtonTextColor();
                }
                else
                {
                    if( bNativeOK && (nState & ControlState::ROLLOVER) )
                        aColor = rStyleSettings.GetFieldRolloverTextColor();
                    else
                        aColor = rStyleSettings.GetFieldTextColor();
                }
                if (IsControlForeground())
                    aColor = GetControlForeground();
                rRenderContext.SetTextColor(aColor);
                if (!bNativeOK)
                    rRenderContext.Erase(maFocusRect);
            }
        }
        else // Disabled
        {
            rRenderContext.SetTextColor(rStyleSettings.GetDisableColor());
            if (!bNativeOK)
                rRenderContext.Erase(maFocusRect);
        }
    }

    if ( IsUserDrawEnabled() )
    {
        UserDrawEvent aUDEvt(this, &rRenderContext, maFocusRect, mnItemPos, 0);
        maUserDrawHdl.Call( &aUDEvt );
    }
    else
    {
        DrawEntry(rRenderContext, bLayout);
    }
}

void ImplWin::ApplySettings(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    vcl::Font aFont = rStyleSettings.GetFieldFont();
    if (IsControlFont())
        aFont.Merge(GetControlFont());
    SetZoomedPointFont(rRenderContext, aFont);

    Color aTextColor = rStyleSettings.GetFieldTextColor();
    if (IsControlForeground())
        aTextColor = GetControlForeground();
    rRenderContext.SetTextColor(aTextColor);

    if (IsControlBackground())
        rRenderContext.SetBackground(GetControlBackground());
    else
        rRenderContext.SetBackground(rStyleSettings.GetFieldColor());
}

void ImplWin::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& )
{
    ImplDraw(rRenderContext);
}

void ImplWin::DrawEntry(vcl::RenderContext& rRenderContext, bool bLayout)
{
    long nBorder = 1;
    Size aOutSz(GetOutputSizePixel());

    bool bImage = !!maImage;
    if (bImage && !bLayout)
    {
        DrawImageFlags nStyle = DrawImageFlags::NONE;
        Size aImgSz = maImage.GetSizePixel();
        Point aPtImg( nBorder, ( ( aOutSz.Height() - aImgSz.Height() ) / 2 ) );
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

        // check for HC mode
        Image *pImage = &maImage;

        if ( !IsZoom() )
        {
            rRenderContext.DrawImage( aPtImg, *pImage, nStyle );
        }
        else
        {
            aImgSz.setWidth( CalcZoom( aImgSz.Width() ) );
            aImgSz.setHeight( CalcZoom( aImgSz.Height() ) );
            rRenderContext.DrawImage( aPtImg, aImgSz, *pImage, nStyle );
        }

        const sal_uInt16 nEdgeBlendingPercent(GetEdgeBlending() ? rStyleSettings.GetEdgeBlending() : 0);

        if(nEdgeBlendingPercent)
        {
            const Color& rTopLeft(rStyleSettings.GetEdgeBlendingTopLeftColor());
            const Color& rBottomRight(rStyleSettings.GetEdgeBlendingBottomRightColor());
            const sal_uInt8 nAlpha((nEdgeBlendingPercent * 255) / 100);
            const BitmapEx aBlendFrame(createBlendFrame(aImgSz, nAlpha, rTopLeft, rBottomRight));

            if(!aBlendFrame.IsEmpty())
            {
                rRenderContext.DrawBitmapEx(aPtImg, aBlendFrame);
            }
        }
    }

    if( !maString.isEmpty() )
    {
        DrawTextFlags nTextStyle = DrawTextFlags::VCenter;

        if ( bImage && !bLayout )
            nTextStyle |= DrawTextFlags::Left;
        else if ( GetStyle() & WB_CENTER )
            nTextStyle |= DrawTextFlags::Center;
        else if ( GetStyle() & WB_RIGHT )
            nTextStyle |= DrawTextFlags::Right;
        else
            nTextStyle |= DrawTextFlags::Left;

        tools::Rectangle aTextRect( Point( nBorder, 0 ), Size( aOutSz.Width()-2*nBorder, aOutSz.Height() ) );

        if ( bImage || IsUserDrawEnabled() )
        {
            aTextRect.AdjustLeft(maImage.GetSizePixel().Width() + IMG_TXT_DISTANCE );
        }

        MetricVector* pVector = bLayout ? &mpControlData->mpLayoutData->m_aUnicodeBoundRects : nullptr;
        OUString* pDisplayText = bLayout ? &mpControlData->mpLayoutData->m_aDisplayText : nullptr;
        rRenderContext.DrawText( aTextRect, maString, nTextStyle, pVector, pDisplayText );
    }

    if( HasFocus() && !bLayout )
        ShowFocus( maFocusRect );
}

void ImplWin::Resize()
{
    Control::Resize();
    maFocusRect.SetSize( GetOutputSizePixel() );
    Invalidate();
}

void ImplWin::GetFocus()
{
    ShowFocus( maFocusRect );
    if( ImplGetSVData()->maNWFData.mbNoFocusRects &&
        IsNativeWidgetEnabled() &&
        IsNativeControlSupported( ControlType::Listbox, ControlPart::Entire ) )
    {
        vcl::Window* pWin = GetParent()->GetWindow( GetWindowType::Border );
        if( ! pWin )
            pWin = GetParent();
        pWin->Invalidate();
    }
    else
        Invalidate();
    Control::GetFocus();
}

void ImplWin::LoseFocus()
{
    HideFocus();
    if( ImplGetSVData()->maNWFData.mbNoFocusRects &&
        IsNativeWidgetEnabled() &&
        IsNativeControlSupported( ControlType::Listbox, ControlPart::Entire ) )
    {
        vcl::Window* pWin = GetParent()->GetWindow( GetWindowType::Border );
        if( ! pWin )
            pWin = GetParent();
        pWin->Invalidate();
    }
    else
        Invalidate();
    Control::LoseFocus();
}

void ImplWin::ShowFocus(const tools::Rectangle& rRect)
{
    if (IsNativeControlSupported(ControlType::Listbox, ControlPart::Focus))
    {
        ImplControlValue aControlValue;

        vcl::Window *pWin = GetParent();
        tools::Rectangle aParentRect(Point(0, 0), pWin->GetSizePixel());
        pWin->DrawNativeControl(ControlType::Listbox, ControlPart::Focus, aParentRect,
                                ControlState::FOCUSED, aControlValue, OUString());
    }
    Control::ShowFocus(rRect);
}

ImplBtn::ImplBtn( vcl::Window* pParent, WinBits nWinStyle ) :
    PushButton(  pParent, nWinStyle )
{
}

void ImplBtn::MouseButtonDown( const MouseEvent& )
{
    if( IsEnabled() )
        maMBDownHdl.Call(this);
}

ImplListBoxFloatingWindow::ImplListBoxFloatingWindow( vcl::Window* pParent ) :
    FloatingWindow( pParent, WB_BORDER | WB_SYSTEMWINDOW | WB_NOSHADOW )    // no drop shadow for list boxes
{
    mpImplLB = nullptr;
    mnDDLineCount = 0;
    mbAutoWidth = false;

    mnPopupModeStartSaveSelection = LISTBOX_ENTRY_NOTFOUND;

    vcl::Window * pBorderWindow = ImplGetBorderWindow();
    if( pBorderWindow )
    {
        SetAccessibleRole(accessibility::AccessibleRole::PANEL);
        pBorderWindow->SetAccessibleRole(accessibility::AccessibleRole::WINDOW);
    }
    else
    {
        SetAccessibleRole(accessibility::AccessibleRole::WINDOW);
    }

}

ImplListBoxFloatingWindow::~ImplListBoxFloatingWindow()
{
    disposeOnce();
}

void ImplListBoxFloatingWindow::dispose()
{
    mpImplLB.clear();
    FloatingWindow::dispose();
}


bool ImplListBoxFloatingWindow::PreNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
    {
        if( !GetParent()->HasChildPathFocus( true ) )
            EndPopupMode();
    }

    return FloatingWindow::PreNotify( rNEvt );
}

void ImplListBoxFloatingWindow::setPosSizePixel( long nX, long nY, long nWidth, long nHeight, PosSizeFlags nFlags )
{
    FloatingWindow::setPosSizePixel( nX, nY, nWidth, nHeight, nFlags );

    // Fix #60890# ( MBA ): to be able to resize the Listbox even in its open state
    // after a call to Resize(), we adjust its position if necessary
    if ( IsReallyVisible() && ( nFlags & PosSizeFlags::Height ) )
    {
        Point aPos = GetParent()->GetPosPixel();
        aPos = GetParent()->GetParent()->OutputToScreenPixel( aPos );

        if ( nFlags & PosSizeFlags::X )
            aPos.setX( nX );

        if ( nFlags & PosSizeFlags::Y )
            aPos.setY( nY );

        sal_uInt16 nIndex;
        SetPosPixel( ImplCalcPos( this, tools::Rectangle( aPos, GetParent()->GetSizePixel() ), FloatWinPopupFlags::Down, nIndex ) );
    }

//  if( !IsReallyVisible() )
    {
        // The ImplListBox does not get a Resize() as not visible.
        // But the windows must get a Resize(), so that the number of
        // visible entries is correct for PgUp/PgDown.
        // The number also cannot be calculated by List/Combobox, as for
        // this the presence of the vertical Scrollbar has to be known.
        mpImplLB->SetSizePixel( GetOutputSizePixel() );
        static_cast<vcl::Window*>(mpImplLB)->Resize();
        static_cast<vcl::Window*>(mpImplLB->GetMainWindow())->Resize();
    }
}

void ImplListBoxFloatingWindow::Resize()
{
    mpImplLB->GetMainWindow()->ImplClearLayoutData();
    FloatingWindow::Resize();
}

Size ImplListBoxFloatingWindow::CalcFloatSize()
{
    Size aFloatSz( maPrefSz );

    sal_Int32 nLeft, nTop, nRight, nBottom;
    GetBorder( nLeft, nTop, nRight, nBottom );

    sal_Int32 nLines = mpImplLB->GetEntryList()->GetEntryCount();
    if ( mnDDLineCount && ( nLines > mnDDLineCount ) )
        nLines = mnDDLineCount;

    Size aSz = mpImplLB->CalcSize( nLines );
    long nMaxHeight = aSz.Height() + nTop + nBottom;

    if ( mnDDLineCount )
        aFloatSz.setHeight( nMaxHeight );

    if( mbAutoWidth )
    {
        // AutoSize first only for width...

        aFloatSz.setWidth( aSz.Width() + nLeft + nRight );
        aFloatSz.AdjustWidth(nRight ); // adding some space looks better...

        if ( ( aFloatSz.Height() < nMaxHeight ) || ( mnDDLineCount && ( mnDDLineCount < mpImplLB->GetEntryList()->GetEntryCount() ) ) )
        {
            // then we also need the vertical Scrollbar
            long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
            aFloatSz.AdjustWidth(nSBWidth );
        }

        long nDesktopWidth = GetDesktopRectPixel().getWidth();
        if (aFloatSz.Width() > nDesktopWidth)
            // Don't exceed the desktop width.
            aFloatSz.setWidth( nDesktopWidth );
    }

    if ( aFloatSz.Height() > nMaxHeight )
        aFloatSz.setHeight( nMaxHeight );

    // Minimal height, in case height is not set to Float height.
    // The parent of FloatWin must be DropDown-Combo/Listbox.
    Size aParentSz = GetParent()->GetSizePixel();
    if( (!mnDDLineCount || !nLines) && ( aFloatSz.Height() < aParentSz.Height() ) )
        aFloatSz.setHeight( aParentSz.Height() );

    // do not get narrower than the parent...
    if( aFloatSz.Width() < aParentSz.Width() )
        aFloatSz.setWidth( aParentSz.Width() );

    // align height to entries...
    long nInnerHeight = aFloatSz.Height() - nTop - nBottom;
    long nEntryHeight = mpImplLB->GetEntryHeight();
    if ( nInnerHeight % nEntryHeight )
    {
        nInnerHeight /= nEntryHeight;
        nInnerHeight++;
        nInnerHeight *= nEntryHeight;
        aFloatSz.setHeight( nInnerHeight + nTop + nBottom );
    }

    if (aFloatSz.Width() < aSz.Width())
    {
        // The max width of list box entries exceeds the window width.
        // Account for the scroll bar height.
        long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
        aFloatSz.AdjustHeight(nSBWidth );
    }

    return aFloatSz;
}

void ImplListBoxFloatingWindow::StartFloat( bool bStartTracking )
{
    if( IsInPopupMode() )
        return;

    Size aFloatSz = CalcFloatSize();

    SetSizePixel( aFloatSz );
    mpImplLB->SetSizePixel( GetOutputSizePixel() );

    sal_Int32 nPos = mpImplLB->GetEntryList()->GetSelectedEntryPos( 0 );
    mnPopupModeStartSaveSelection = nPos;

    Size aSz = GetParent()->GetSizePixel();
    Point aPos = GetParent()->GetPosPixel();
    aPos = GetParent()->GetParent()->OutputToScreenPixel( aPos );
    // FIXME: this ugly hack is for Mac/Aqua
    // should be replaced by a real mechanism to place the float rectangle
    if( ImplGetSVData()->maNWFData.mbNoFocusRects &&
        GetParent()->IsNativeWidgetEnabled() )
    {
        const sal_Int32 nLeft = 4, nTop = 4, nRight = 4, nBottom = 4;
        aPos.AdjustX(nLeft );
        aPos.AdjustY(nTop );
        aSz.AdjustWidth( -(nLeft + nRight) );
        aSz.AdjustHeight( -(nTop + nBottom) );
    }
    tools::Rectangle aRect( aPos, aSz );

    // check if the control's parent is un-mirrored which is the case for form controls in a mirrored UI
    // where the document is unmirrored
    // because StartPopupMode() expects a rectangle in mirrored coordinates we have to re-mirror
    vcl::Window *pGrandparent = GetParent()->GetParent();
    const OutputDevice *pGrandparentOutDev = pGrandparent->GetOutDev();

    if( pGrandparent->ImplIsAntiparallel() )
        pGrandparentOutDev->ReMirror( aRect );

    // mouse-button right: close the List-Box-Float-win and don't stop the handling fdo#84795
    StartPopupMode( aRect, FloatWinPopupFlags::Down | FloatWinPopupFlags::AllMouseButtonClose );

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        mpImplLB->ShowProminentEntry( nPos );

    if( bStartTracking )
        mpImplLB->GetMainWindow()->EnableMouseMoveSelect( true );

    if ( mpImplLB->GetMainWindow()->IsGrabFocusAllowed() )
        mpImplLB->GetMainWindow()->GrabFocus();

    mpImplLB->GetMainWindow()->ImplClearLayoutData();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
