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

#include <tools/shl.hxx>
#include <vcl/svapp.hxx>
#include <svtools/colorcfg.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/itempool.hxx>

#include <rtl/textenc.h>
#include <svx/ucsubset.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/msgbox.hxx>
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

SvxCharacterMap::SvxCharacterMap( Window* pParent, sal_Bool bOne_, const SfxItemSet* pSet )
    : SfxModalDialog(pParent, "SpecialCharactersDialog", "cui/ui/specialcharacters.ui")
    , bOne( bOne_ )
    , pSubsetMap( NULL )
{
    get(m_pShowSet, "showcharset");
    get(m_pShowChar, "showchar");
    m_pShowChar->SetCentered(true);
    get(m_pShowText, "showtext");
    get(m_pOKBtn, "ok");
    get(m_pDeleteBtn, "delete");
    get(m_pFontText, "fontft");
    get(m_pFontLB, "fontlb");
    get(m_pSubsetText, "subsetft");
    get(m_pSubsetLB, "subsetlb");
    get(m_pCharCodeText, "charcodeft");
    get(m_pSymbolText, "symboltext");

    SFX_ITEMSET_ARG( pSet, pItem, SfxBoolItem, FN_PARAM_1, sal_False );
    if ( pItem )
        bOne = pItem->GetValue();

    init();

    SFX_ITEMSET_ARG( pSet, pCharItem, SfxInt32Item, SID_ATTR_CHAR, sal_False );
    if ( pCharItem )
        SetChar( pCharItem->GetValue() );

    SFX_ITEMSET_ARG( pSet, pDisableItem, SfxBoolItem, FN_PARAM_2, sal_False );
    if ( pDisableItem && pDisableItem->GetValue() )
        DisableFontSelection();

    SFX_ITEMSET_ARG( pSet, pFontItem, SvxFontItem, SID_ATTR_CHAR_FONT, sal_False );
    SFX_ITEMSET_ARG( pSet, pFontNameItem, SfxStringItem, SID_FONT_NAME, sal_False );
    if ( pFontItem )
    {
        Font aTmpFont( pFontItem->GetFamilyName(), pFontItem->GetStyleName(), GetCharFont().GetSize() );
        aTmpFont.SetCharSet( pFontItem->GetCharSet() );
        aTmpFont.SetPitch( pFontItem->GetPitch() );
        SetCharFont( aTmpFont );
    }
    else if ( pFontNameItem )
    {
        Font aTmpFont( GetCharFont() );
        aTmpFont.SetName( pFontNameItem->GetValue() );
        SetCharFont( aTmpFont );
    }

    CreateOutputItemSet( pSet ? *pSet->GetPool() : SFX_APP()->GetPool() );
}

// -----------------------------------------------------------------------

SvxCharacterMap::~SvxCharacterMap()
{
}

// -----------------------------------------------------------------------

const Font& SvxCharacterMap::GetCharFont() const
{
    return aFont;
}

// -----------------------------------------------------------------------

void SvxCharacterMap::SetChar( sal_UCS4 c )
{
    m_pShowSet->SelectCharacter( c );
}

// -----------------------------------------------------------------------

sal_UCS4 SvxCharacterMap::GetChar() const
{
    return m_pShowSet->GetSelectCharacter();
}

// -----------------------------------------------------------------------

String SvxCharacterMap::GetCharacters() const
{
    return m_pShowText->GetText();
}


// -----------------------------------------------------------------------

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
            const Font& rFont( GetCharFont() );
            pSet->Put( SfxStringItem( pPool->GetWhich(SID_CHARMAP), GetCharacters() ) );
            pSet->Put( SvxFontItem( rFont.GetFamily(), rFont.GetName(),
                rFont.GetStyleName(), rFont.GetPitch(), rFont.GetCharSet(), pPool->GetWhich(SID_ATTR_CHAR_FONT) ) );
            pSet->Put( SfxStringItem( pPool->GetWhich(SID_FONT_NAME), rFont.GetName() ) );
            pSet->Put( SfxInt32Item( pPool->GetWhich(SID_ATTR_CHAR), GetChar() ) );
        }
    }

    return nResult;
}


// class SvxShowText =====================================================

SvxShowText::SvxShowText(Window* pParent, sal_Bool bCenter)
:   Control( pParent ),
    mbCenter( bCenter)
{}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvxShowText(Window *pParent, VclBuilder::stringmap &)
{
    return new SvxShowText(pParent);
}

// -----------------------------------------------------------------------

void SvxShowText::Paint( const Rectangle& )
{
    Color aTextCol = GetTextColor();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Color aWindowTextColor( rStyleSettings.GetDialogTextColor() );
    SetTextColor( aWindowTextColor );

    const String aText = GetText();
    const Size aSize = GetOutputSizePixel();
    Point aPoint( 2, mnY );

    // adjust position using ink boundary if possible
    Rectangle aBoundRect;
    if( !GetTextBoundRect( aBoundRect, aText ) || aBoundRect.IsEmpty() )
        aPoint.X() = (aSize.Width() - GetTextWidth( aText )) / 2;
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

        if( mbCenter )
        {
            // move glyph to middle of cell
            aPoint.X() = -aBoundRect.Left()
                       + (aSize.Width() - aBoundRect.GetWidth()) / 2;
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

    DrawText( aPoint, aText );
    SetTextColor( aTextCol );
}

// -----------------------------------------------------------------------

void SvxShowText::SetFont( const Font& rFont )
{
    long nWinHeight = GetOutputSizePixel().Height();
    Font aFont = rFont;
    aFont.SetWeight( WEIGHT_NORMAL );
    aFont.SetAlign( ALIGN_TOP );
    aFont.SetSize( PixelToLogic( Size( 0, nWinHeight/2 ) ) );
    aFont.SetTransparent( sal_True );
    Control::SetFont( aFont );
    mnY = ( nWinHeight - GetTextHeight() ) / 2;

    Invalidate();
}

void SvxShowText::Resize()
{
    Control::Resize();
    SetFont(GetFont()); //force recalculation of size
}

// -----------------------------------------------------------------------

void SvxShowText::SetText( const String& rText )
{
    Control::SetText( rText );
    Invalidate();
}

// -----------------------------------------------------------------------

SvxShowText::~SvxShowText()
{}

// class SvxCharacterMap =================================================

void SvxCharacterMap::init()
{
    aFont = GetFont();
    aFont.SetTransparent( sal_True );
    aFont.SetFamily( FAMILY_DONTKNOW );
    aFont.SetPitch( PITCH_DONTKNOW );
    aFont.SetCharSet( RTL_TEXTENCODING_DONTKNOW );

    if (bOne)
    {
        m_pSymbolText->Hide();
        m_pShowText->Hide();
        m_pDeleteBtn->Hide();
    }

    rtl::OUString aDefStr( aFont.GetName() );
    String aLastName;
    int nCount = GetDevFontCount();
    for ( int i = 0; i < nCount; i++ )
    {
        String aFontName( GetDevFont( i ).GetName() );
        if ( aFontName != aLastName )
        {
            aLastName = aFontName;
            sal_uInt16 nPos = m_pFontLB->InsertEntry( aFontName );
            m_pFontLB->SetEntryData( nPos, (void*)(sal_uLong)i );
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
            rtl::OUString aToken = aDefStr.getToken(0, ';', nIndex);
            if ( m_pFontLB->GetEntryPos( aToken ) != LISTBOX_ENTRY_NOTFOUND )
            {
                aDefStr = aToken;
                bFound = sal_True;
                break;
            }
        }
        while ( nIndex >= 0 );
    }

    if ( bFound )
        m_pFontLB->SelectEntry( aDefStr );
    else if ( m_pFontLB->GetEntryCount() )
        m_pFontLB->SelectEntryPos(0);
    FontSelectHdl(m_pFontLB);

    m_pOKBtn->SetClickHdl( LINK( this, SvxCharacterMap, OKHdl ) );
    m_pFontLB->SetSelectHdl( LINK( this, SvxCharacterMap, FontSelectHdl ) );
    m_pSubsetLB->SetSelectHdl( LINK( this, SvxCharacterMap, SubsetSelectHdl ) );
    m_pShowSet->SetDoubleClickHdl( LINK( this, SvxCharacterMap, CharDoubleClickHdl ) );
    m_pShowSet->SetSelectHdl( LINK( this, SvxCharacterMap, CharSelectHdl ) );
    m_pShowSet->SetHighlightHdl( LINK( this, SvxCharacterMap, CharHighlightHdl ) );
    m_pShowSet->SetPreSelectHdl( LINK( this, SvxCharacterMap, CharPreSelectHdl ) );
    m_pDeleteBtn->SetClickHdl( LINK( this, SvxCharacterMap, DeleteHdl ) );

    if( SvxShowCharSet::getSelectedChar() == ' ')
        m_pOKBtn->Disable();
    else
        m_pOKBtn->Enable();
}

// -----------------------------------------------------------------------

void SvxCharacterMap::SetCharFont( const Font& rFont )
{
    // first get the underlying info in order to get font names
    // like "Times New Roman;Times" resolved
    Font aTmp( GetFontMetric( rFont ) );

    if ( m_pFontLB->GetEntryPos( aTmp.GetName() ) == LISTBOX_ENTRY_NOTFOUND )
        return;

    m_pFontLB->SelectEntry( aTmp.GetName() );
    aFont = aTmp;
    FontSelectHdl(m_pFontLB);

    // for compatibility reasons
    ModalDialog::SetFont( aFont );
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxCharacterMap, OKHdl)
{
    String aStr = m_pShowText->GetText();

    if ( !aStr.Len() )
    {
        sal_UCS4 cChar = m_pShowSet->GetSelectCharacter();
        // using the new UCS4 constructor
    rtl::OUString aOUStr( &cChar, 1 );
        m_pShowText->SetText( aOUStr );
    }
    EndDialog( sal_True );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxCharacterMap, FontSelectHdl)
{
    sal_uInt16 nPos = m_pFontLB->GetSelectEntryPos(),
        nFont = (sal_uInt16)(sal_uLong)m_pFontLB->GetEntryData( nPos );
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
    pSubsetMap = NULL;

    sal_Bool bNeedSubset = (aFont.GetCharSet() != RTL_TEXTENCODING_SYMBOL);
    if( bNeedSubset )
    {
        FontCharMap aFontCharMap;
        m_pShowSet->GetFontCharMap( aFontCharMap );
        pSubsetMap = new SubsetMap( &aFontCharMap );

        // update subset listbox for new font's unicode subsets
        m_pSubsetLB->Clear();
        // TODO: is it worth to improve the stupid linear search?
        bool bFirst = true;
        const Subset* s;
        while( NULL != (s = pSubsetMap->GetNextSubset( bFirst ))  )
        {
            sal_uInt16 nPos_ = m_pSubsetLB->InsertEntry( s->GetName() );
            m_pSubsetLB->SetEntryData( nPos_, (void*)s );
            // NOTE: subset must live at least as long as the selected font
            if( bFirst )
                m_pSubsetLB->SelectEntryPos( nPos_ );
            bFirst = false;
        }
        if( m_pSubsetLB->GetEntryCount() <= 1 )
            bNeedSubset = sal_False;
    }

    m_pSubsetText->Show( bNeedSubset);
    m_pSubsetLB->Show( bNeedSubset);

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxCharacterMap, SubsetSelectHdl)
{
    sal_uInt16 nPos = m_pSubsetLB->GetSelectEntryPos();
    const Subset* pSubset = reinterpret_cast<const Subset*> (m_pSubsetLB->GetEntryData(nPos));
    if( pSubset )
    {
        sal_UCS4 cFirst = pSubset->GetRangeMin();
        m_pShowSet->SelectCharacter( cFirst );
    }
    m_pSubsetLB->SelectEntryPos( nPos );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxCharacterMap, CharDoubleClickHdl)
{
    EndDialog( sal_True );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxCharacterMap, CharSelectHdl)
{
    if ( !bOne )
    {
        String aText = m_pShowText->GetText();

        if ( aText.Len() != CHARMAP_MAXLEN )
        {
            sal_UCS4 cChar = m_pShowSet->GetSelectCharacter();
            // using the new UCS4 constructor
            rtl::OUString aOUStr( &cChar, 1 );
            m_pShowText->SetText( aText + aOUStr );
        }

    }
    m_pOKBtn->Enable();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxCharacterMap, CharHighlightHdl)
{
    String aText;
    sal_UCS4 cChar = m_pShowSet->GetSelectCharacter();
    sal_Bool bSelect = (cChar > 0);

    // show char sample
    if ( bSelect )
    {
        // using the new UCS4 constructor
        aText = rtl::OUString( &cChar, 1 );

        const Subset* pSubset = NULL;
        if( pSubsetMap )
            pSubset = pSubsetMap->GetSubsetByUnicode( cChar );
        if( pSubset )
            m_pSubsetLB->SelectEntry( pSubset->GetName() );
        else
            m_pSubsetLB->SetNoSelection();
    }
    m_pShowChar->SetText( aText );
    m_pShowChar->Update();

    // show char code
    if ( bSelect )
    {
        char aBuf[32];
        snprintf( aBuf, sizeof(aBuf), "U+%04X", static_cast<unsigned>(cChar) );
        if( cChar < 0x0100 )
            snprintf( aBuf+6, sizeof(aBuf)-6, " (%u)", static_cast<unsigned>(cChar) );
        aText = rtl::OUString::createFromAscii(aBuf);
    }
    m_pCharCodeText->SetText( aText );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxCharacterMap, CharPreSelectHdl)
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
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxCharacterMap, DeleteHdl)
{
    m_pShowText->SetText( String() );
    m_pOKBtn->Disable();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
