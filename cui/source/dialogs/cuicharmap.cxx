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

#include <stdio.h>

#include <vcl/svapp.hxx>
#include <svtools/colorcfg.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/itempool.hxx>

#include <rtl/textenc.h>
#include <svx/ucsubset.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <svl/stritem.hxx>

#include <cuires.hrc>
#include <dialmgr.hxx>
#include "cuicharmap.hxx"
#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/app.hxx>
#include <editeng/fontitem.hxx>
#include "macroass.hxx"

// class SvxCharacterMap =================================================

SvxCharacterMap::SvxCharacterMap( vcl::Window* pParent, bool bOne_, const SfxItemSet* pSet )
    : SfxModalDialog(pParent, "SpecialCharactersDialog", "cui/ui/specialcharacters.ui")
    , bOne( bOne_ )
    , pSubsetMap( nullptr )
{
    get(m_pShowSet, "showcharset");
    get(m_pShowChar, "showchar");
    m_pShowChar->SetCentered(true);
    get(m_pShowText, "showtext");
    m_pShowText->SetMaxTextLen(CHARMAP_MAXLEN);
    get(m_pOKBtn, "ok");
    get(m_pFontText, "fontft");
    get(m_pFontLB, "fontlb");
    m_pFontLB->SetStyle(m_pFontLB->GetStyle() | WB_SORT);
    get(m_pSubsetText, "subsetft");
    get(m_pSubsetLB, "subsetlb");
    //lock the size request of this widget to the width of all possible entries
    fillAllSubsets(*m_pSubsetLB);
    m_pSubsetLB->set_width_request(m_pSubsetLB->get_preferred_size().Width());
    get(m_pHexCodeText, "hexvalue");
    get(m_pDecimalCodeText, "decimalvalue");
    //lock the size request of this widget to the width of the original .ui string
    m_pHexCodeText->set_width_request(m_pHexCodeText->get_preferred_size().Width());
    get(m_pSymbolText, "symboltext");

    const SfxBoolItem* pItem = SfxItemSet::GetItem<SfxBoolItem>(pSet, FN_PARAM_1, false);
    if ( pItem )
        bOne = pItem->GetValue();

    init();

    const SfxInt32Item* pCharItem = SfxItemSet::GetItem<SfxInt32Item>(pSet, SID_ATTR_CHAR, false);
    if ( pCharItem )
        SetChar( pCharItem->GetValue() );

    const SfxBoolItem* pDisableItem = SfxItemSet::GetItem<SfxBoolItem>(pSet, FN_PARAM_2, false);
    if ( pDisableItem && pDisableItem->GetValue() )
        DisableFontSelection();

    const SvxFontItem* pFontItem = SfxItemSet::GetItem<SvxFontItem>(pSet, SID_ATTR_CHAR_FONT, false);
    const SfxStringItem* pFontNameItem = SfxItemSet::GetItem<SfxStringItem>(pSet, SID_FONT_NAME, false);
    if ( pFontItem )
    {
        vcl::Font aTmpFont( pFontItem->GetFamilyName(), pFontItem->GetStyleName(), GetCharFont().GetSize() );
        aTmpFont.SetCharSet( pFontItem->GetCharSet() );
        aTmpFont.SetPitch( pFontItem->GetPitch() );
        SetCharFont( aTmpFont );
    }
    else if ( pFontNameItem )
    {
        vcl::Font aTmpFont( GetCharFont() );
        aTmpFont.SetName( pFontNameItem->GetValue() );
        SetCharFont( aTmpFont );
    }

    CreateOutputItemSet( pSet ? *pSet->GetPool() : SfxGetpApp()->GetPool() );
}

SvxCharacterMap::~SvxCharacterMap()
{
    disposeOnce();
}

void SvxCharacterMap::dispose()
{
    m_pShowSet.clear();
    m_pShowText.clear();
    m_pOKBtn.clear();
    m_pFontText.clear();
    m_pFontLB.clear();
    m_pSubsetText.clear();
    m_pSubsetLB.clear();
    m_pSymbolText.clear();
    m_pShowChar.clear();
    m_pHexCodeText.clear();
    m_pDecimalCodeText.clear();
    SfxModalDialog::dispose();
}


void SvxCharacterMap::SetChar( sal_UCS4 c )
{
    m_pShowSet->SelectCharacter( c );
}


sal_UCS4 SvxCharacterMap::GetChar() const
{
    return m_pShowSet->GetSelectCharacter();
}


OUString SvxCharacterMap::GetCharacters() const
{
    return m_pShowText->GetText();
}


void SvxCharacterMap::DisableFontSelection()
{
    m_pFontText->Disable();
    m_pFontLB->Disable();
}

short SvxCharacterMap::Execute()
{
    short nResult = SfxModalDialog::Execute();
    if ( nResult == RET_OK )
    {
        SfxItemSet* pSet = GetItemSet();
        if ( pSet )
        {
            const SfxItemPool* pPool = pSet->GetPool();
            const vcl::Font& rFont( GetCharFont() );
            pSet->Put( SfxStringItem( pPool->GetWhich(SID_CHARMAP), GetCharacters() ) );
            pSet->Put( SvxFontItem( rFont.GetFamily(), rFont.GetFamilyName(),
                rFont.GetStyleName(), rFont.GetPitch(), rFont.GetCharSet(), pPool->GetWhich(SID_ATTR_CHAR_FONT) ) );
            pSet->Put( SfxStringItem( pPool->GetWhich(SID_FONT_NAME), rFont.GetFamilyName() ) );
            pSet->Put( SfxInt32Item( pPool->GetWhich(SID_ATTR_CHAR), GetChar() ) );
        }
    }

    return nResult;
}


// class SvxShowText =====================================================

SvxShowText::SvxShowText(vcl::Window* pParent, bool bCenter)
    : Control(pParent)
    , mnY(0)
    , mbCenter(bCenter)
{}

VCL_BUILDER_FACTORY(SvxShowText)

void SvxShowText::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    rRenderContext.SetFont(maFont);

    Color aTextCol = rRenderContext.GetTextColor();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Color aWindowTextColor(rStyleSettings.GetDialogTextColor());
    rRenderContext.SetTextColor(aWindowTextColor);

    const OUString aText = GetText();
    const Size aSize(GetOutputSizePixel());

    long nAvailWidth = aSize.Width();
    long nWinHeight = GetOutputSizePixel().Height();

    bool bGotBoundary = true;
    bool bShrankFont = false;
    vcl::Font aOrigFont(rRenderContext.GetFont());
    Size aFontSize(aOrigFont.GetSize());
    Rectangle aBoundRect;

    for (long nFontHeight = aFontSize.Height(); nFontHeight > 0; nFontHeight -= 5)
    {
        if (!rRenderContext.GetTextBoundRect( aBoundRect, aText ) || aBoundRect.IsEmpty())
        {
            bGotBoundary = false;
            break;
        }
        if (!mbCenter)
            break;
        //only shrink in the single glyph large view mode
        long nTextWidth = aBoundRect.GetWidth();
        if (nAvailWidth > nTextWidth)
            break;
        vcl::Font aFont(aOrigFont);
        aFontSize.Height() = nFontHeight;
        aFont.SetSize(aFontSize);
        rRenderContext.SetFont(aFont);
        mnY = (nWinHeight - GetTextHeight()) / 2;
        bShrankFont = true;
    }

    Point aPoint(2, mnY);
    // adjust position using ink boundary if possible
    if (!bGotBoundary)
        aPoint.X() = (aSize.Width() - rRenderContext.GetTextWidth(aText)) / 2;
    else
    {
        // adjust position before it gets out of bounds
        aBoundRect += aPoint;

        // shift back vertically if needed
        int nYLDelta = aBoundRect.Top();
        int nYHDelta = aSize.Height() - aBoundRect.Bottom();
        if( nYLDelta <= 0 )
            aPoint.Y() -= nYLDelta - 1;
        else if( nYHDelta <= 0 )
            aPoint.Y() += nYHDelta - 1;

        if (mbCenter)
        {
            // move glyph to middle of cell
            aPoint.X() = -aBoundRect.Left() + (aSize.Width() - aBoundRect.GetWidth()) / 2;
        }
        else
        {
            // shift back horizontally if needed
            int nXLDelta = aBoundRect.Left();
            int nXHDelta = aSize.Width() - aBoundRect.Right();
            if( nXLDelta <= 0 )
                aPoint.X() -= nXLDelta - 1;
            else if( nXHDelta <= 0 )
                aPoint.X() += nXHDelta - 1;
        }
    }

    rRenderContext.DrawText(aPoint, aText);
    rRenderContext.SetTextColor(aTextCol);
    if (bShrankFont)
        rRenderContext.SetFont(aOrigFont);
}


void SvxShowText::SetFont( const vcl::Font& rFont )
{
    long nWinHeight = GetOutputSizePixel().Height();
    maFont = vcl::Font(rFont);
    maFont.SetWeight(WEIGHT_NORMAL);
    maFont.SetAlign(ALIGN_TOP);
    maFont.SetSize(PixelToLogic(Size(0, nWinHeight / 2)));
    maFont.SetTransparent(true);
    Control::SetFont(maFont);

    mnY = (nWinHeight - GetTextHeight()) / 2;

    Invalidate();
}

Size SvxShowText::GetOptimalSize() const
{
    const vcl::Font &rFont = GetFont();
    const Size rFontSize = rFont.GetSize();
    long nWinHeight = LogicToPixel(rFontSize).Height() * 2;
    return Size( GetTextWidth( GetText() ) + 2 * 12, nWinHeight );
}

void SvxShowText::Resize()
{
    Control::Resize();
    SetFont(GetFont()); //force recalculation of size
}


void SvxShowText::SetText( const OUString& rText )
{
    Control::SetText( rText );
    Invalidate();
}


// class SvxCharacterMap =================================================

void SvxCharacterMap::init()
{
    aFont = GetFont();
    aFont.SetTransparent( true );
    aFont.SetFamily( FAMILY_DONTKNOW );
    aFont.SetPitch( PITCH_DONTKNOW );
    aFont.SetCharSet( RTL_TEXTENCODING_DONTKNOW );

    if (bOne)
    {
        m_pSymbolText->Hide();
        m_pShowText->Hide();
    }

    OUString aDefStr( aFont.GetFamilyName() );
    OUString aLastName;
    int nCount = GetDevFontCount();
    for ( int i = 0; i < nCount; i++ )
    {
        OUString aFontName( GetDevFont( i ).GetFamilyName() );
        if ( aFontName != aLastName )
        {
            aLastName = aFontName;
            const sal_Int32 nPos = m_pFontLB->InsertEntry( aFontName );
            m_pFontLB->SetEntryData( nPos, reinterpret_cast<void*>(i) );
        }
    }
    // the font may not be in the list =>
    // try to find a font name token in list and select found font,
    // else select topmost entry
    bool bFound = (m_pFontLB->GetEntryPos( aDefStr ) == LISTBOX_ENTRY_NOTFOUND );
    if( !bFound )
    {
        sal_Int32 nIndex = 0;
        do
        {
            OUString aToken = aDefStr.getToken(0, ';', nIndex);
            if ( m_pFontLB->GetEntryPos( aToken ) != LISTBOX_ENTRY_NOTFOUND )
            {
                aDefStr = aToken;
                bFound = true;
                break;
            }
        }
        while ( nIndex >= 0 );
    }

    if ( bFound )
        m_pFontLB->SelectEntry( aDefStr );
    else if ( m_pFontLB->GetEntryCount() )
        m_pFontLB->SelectEntryPos(0);
    FontSelectHdl(*m_pFontLB);

    m_pOKBtn->SetClickHdl( LINK( this, SvxCharacterMap, OKHdl ) );
    m_pFontLB->SetSelectHdl( LINK( this, SvxCharacterMap, FontSelectHdl ) );
    m_pSubsetLB->SetSelectHdl( LINK( this, SvxCharacterMap, SubsetSelectHdl ) );
    m_pShowSet->SetDoubleClickHdl( LINK( this, SvxCharacterMap, CharDoubleClickHdl ) );
    m_pShowSet->SetSelectHdl( LINK( this, SvxCharacterMap, CharSelectHdl ) );
    m_pShowSet->SetHighlightHdl( LINK( this, SvxCharacterMap, CharHighlightHdl ) );
    m_pShowSet->SetPreSelectHdl( LINK( this, SvxCharacterMap, CharPreSelectHdl ) );
    m_pDecimalCodeText->SetModifyHdl( LINK( this, SvxCharacterMap, DecimalCodeChangeHdl ) );
    m_pHexCodeText->SetModifyHdl( LINK( this, SvxCharacterMap, HexCodeChangeHdl ) );

    if( SvxShowCharSet::getSelectedChar() == ' ')
        m_pOKBtn->Disable();
    else
        m_pOKBtn->Enable();
}


void SvxCharacterMap::SetCharFont( const vcl::Font& rFont )
{
    // first get the underlying info in order to get font names
    // like "Times New Roman;Times" resolved
    vcl::Font aTmp( GetFontMetric( rFont ) );

    if ( m_pFontLB->GetEntryPos( aTmp.GetFamilyName() ) == LISTBOX_ENTRY_NOTFOUND )
        return;

    m_pFontLB->SelectEntry( aTmp.GetFamilyName() );
    aFont = aTmp;
    FontSelectHdl(*m_pFontLB);

    // for compatibility reasons
    ModalDialog::SetFont( aFont );
}


IMPL_LINK_NOARG_TYPED(SvxCharacterMap, OKHdl, Button*, void)
{
    OUString aStr = m_pShowText->GetText();

    if ( aStr.isEmpty() )
    {
        sal_UCS4 cChar = m_pShowSet->GetSelectCharacter();
        // using the new UCS4 constructor
        OUString aOUStr( &cChar, 1 );
        m_pShowText->SetText( aOUStr );
    }
    EndDialog( RET_OK );
}

void SvxCharacterMap::fillAllSubsets(ListBox &rListBox)
{
    SubsetMap aAll(nullptr);
    rListBox.Clear();
    bool bFirst = true;
    while (const Subset *s = aAll.GetNextSubset(bFirst))
    {
        rListBox.InsertEntry( s->GetName() );
        bFirst = false;
    }
}


IMPL_LINK_NOARG_TYPED(SvxCharacterMap, FontSelectHdl, ListBox&, void)
{
    const sal_Int32 nPos = m_pFontLB->GetSelectEntryPos();
    const sal_uInt16 nFont = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pFontLB->GetEntryData( nPos ));
    aFont = GetDevFont( nFont );
    aFont.SetWeight( WEIGHT_DONTKNOW );
    aFont.SetItalic( ITALIC_NONE );
    aFont.SetWidthType( WIDTH_DONTKNOW );
    aFont.SetPitch( PITCH_DONTKNOW );
    aFont.SetFamily( FAMILY_DONTKNOW );

    // notify children using this font
    m_pShowSet->SetFont( aFont );
    m_pShowChar->SetFont( aFont );
    m_pShowText->SetFont( aFont );

    // setup unicode subset listbar with font specific subsets,
    // hide unicode subset listbar for symbol fonts
    // TODO: get info from the Font once it provides it
    delete pSubsetMap;
    pSubsetMap = nullptr;
    m_pSubsetLB->Clear();

    bool bNeedSubset = (aFont.GetCharSet() != RTL_TEXTENCODING_SYMBOL);
    if( bNeedSubset )
    {
        FontCharMapPtr pFontCharMap( new FontCharMap() );
        m_pShowSet->GetFontCharMap( pFontCharMap );
        pSubsetMap = new SubsetMap( pFontCharMap );

        // update subset listbox for new font's unicode subsets
        // TODO: is it worth to improve the stupid linear search?
        bool bFirst = true;
        const Subset* s;
        while( nullptr != (s = pSubsetMap->GetNextSubset( bFirst ))  )
        {
            const sal_Int32 nPos_ = m_pSubsetLB->InsertEntry( s->GetName() );
            m_pSubsetLB->SetEntryData( nPos_, const_cast<Subset *>(s) );
            // NOTE: subset must live at least as long as the selected font
            if( bFirst )
                m_pSubsetLB->SelectEntryPos( nPos_ );
            bFirst = false;
        }
        if( m_pSubsetLB->GetEntryCount() <= 1 )
            bNeedSubset = false;
    }

    m_pSubsetText->Enable(bNeedSubset);
    m_pSubsetLB->Enable(bNeedSubset);
}


IMPL_LINK_NOARG_TYPED(SvxCharacterMap, SubsetSelectHdl, ListBox&, void)
{
    const sal_Int32 nPos = m_pSubsetLB->GetSelectEntryPos();
    const Subset* pSubset = static_cast<const Subset*> (m_pSubsetLB->GetEntryData(nPos));
    if( pSubset )
    {
        sal_UCS4 cFirst = pSubset->GetRangeMin();
        m_pShowSet->SelectCharacter( cFirst );
    }
    m_pSubsetLB->SelectEntryPos( nPos );
}


IMPL_LINK_NOARG_TYPED(SvxCharacterMap, CharDoubleClickHdl, SvxShowCharSet*, void)
{
    if (bOne)
    {
        sal_UCS4 cChar = m_pShowSet->GetSelectCharacter();
        m_pShowText->SetText(OUString(&cChar, 1));
    }
    EndDialog( RET_OK );
}


IMPL_LINK_NOARG_TYPED(SvxCharacterMap, CharSelectHdl, SvxShowCharSet*, void)
{
    if ( !bOne )
    {
        OUString aText = m_pShowText->GetText();
        Selection aSelection = m_pShowText->GetSelection();
        aSelection.Justify();
        long nLen = aSelection.Len();

        if ( aText.getLength() != CHARMAP_MAXLEN || nLen > 0 )
        {
            sal_UCS4 cChar = m_pShowSet->GetSelectCharacter();
            // using the new UCS4 constructor
            OUString aOUStr( &cChar, 1 );

            long nPos = aSelection.Min();
            if( aText.getLength() )
            {
                m_pShowText->SetText( aText.copy( 0, nPos ) + aOUStr + aText.copy( nPos + nLen ) );
            }
            else
                m_pShowText->SetText( aOUStr );

            m_pShowText->SetSelection(Selection(nPos + aOUStr.getLength()));
        }

    }
    m_pOKBtn->Enable();
}


IMPL_LINK_NOARG_TYPED(SvxCharacterMap, CharHighlightHdl, SvxShowCharSet*, void)
{
    OUString aText;
    OUString aHexText;
    OUString aDecimalText;
    sal_UCS4 cChar = m_pShowSet->GetSelectCharacter();
    bool bSelect = (cChar > 0);

    // show char sample
    if ( bSelect )
    {
        // using the new UCS4 constructor
        aText = OUString( &cChar, 1 );

        const Subset* pSubset = nullptr;
        if( pSubsetMap )
            pSubset = pSubsetMap->GetSubsetByUnicode( cChar );
        if( pSubset )
            m_pSubsetLB->SelectEntry( pSubset->GetName() );
        else
            m_pSubsetLB->SetNoSelection();
    }
    m_pShowChar->SetText( aText );
    m_pShowChar->Update();

    // show char codes
    if ( bSelect )
    {
        // Get the hexadecimal code
        char aBuf[32];
        snprintf( aBuf, sizeof(aBuf), "%X", static_cast<unsigned>(cChar) );
        aHexText = OUString::createFromAscii(aBuf);
        // Get the decimal code
        char aDecBuf[32];
        snprintf( aDecBuf, sizeof(aDecBuf), "%u", static_cast<unsigned>(cChar) );
        aDecimalText = OUString::createFromAscii(aDecBuf);
    }

    // Update the hex and decimal codes only if necessary
    if (m_pHexCodeText->GetText() != aHexText)
        m_pHexCodeText->SetText( aHexText );
    if (m_pDecimalCodeText->GetText() != aDecimalText)
        m_pDecimalCodeText->SetText( aDecimalText );
}

void SvxCharacterMap::selectCharByCode(Radix radix)
{
    OUString aCodeString;
    switch(radix)
    {
        case Radix::decimal:
            aCodeString = m_pDecimalCodeText->GetText();
            break;
        case Radix::hexadecimal:
            aCodeString = m_pHexCodeText->GetText();
            break;
    }
    // Convert the code back to a character using the appropriate radix
    sal_UCS4 cChar = aCodeString.toUInt32(static_cast<sal_Int16> (radix));
    // Use FontCharMap::HasChar(sal_UCS4 cChar) to see if the desired character is in the font
    FontCharMapPtr pFontCharMap(new FontCharMap());
    m_pShowSet->GetFontCharMap(pFontCharMap);
    if (pFontCharMap->HasChar(cChar))
        // Select the corresponding character
        SetChar(cChar);
}

IMPL_LINK_NOARG_TYPED(SvxCharacterMap, DecimalCodeChangeHdl, Edit&, void)
{
    selectCharByCode(Radix::decimal);
}

IMPL_LINK_NOARG_TYPED(SvxCharacterMap, HexCodeChangeHdl, Edit&, void)
{
    selectCharByCode(Radix::hexadecimal);
}

IMPL_LINK_NOARG_TYPED(SvxCharacterMap, CharPreSelectHdl, SvxShowCharSet*, void)
{
    // adjust subset selection
    if( pSubsetMap )
    {
        sal_UCS4 cChar = m_pShowSet->GetSelectCharacter();
        const Subset* pSubset = pSubsetMap->GetSubsetByUnicode( cChar );
        if( pSubset )
            m_pSubsetLB->SelectEntry( pSubset->GetName() );
    }

    m_pOKBtn->Enable();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
