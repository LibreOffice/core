/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"

#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include <editeng/svxfont.hxx>
#include <editeng/flditem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/cscoitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/akrnitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/emphitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charreliefitem.hxx>

#include "editattr.hxx"

DBG_NAME( EE_EditAttrib )

// -------------------------------------------------------------------------
// class EditAttrib
// -------------------------------------------------------------------------
EditAttrib::EditAttrib( const SfxPoolItem& rAttr )
{
    DBG_CTOR( EE_EditAttrib, 0 );
    pItem = &rAttr;
}

EditAttrib::~EditAttrib()
{
    DBG_DTOR( EE_EditAttrib, 0 );
}

// -------------------------------------------------------------------------
// class EditCharAttrib
// -------------------------------------------------------------------------
EditCharAttrib::EditCharAttrib( const SfxPoolItem& rAttr, USHORT nS, USHORT nE )
                    : EditAttrib( rAttr )
{
    nStart      = nS;
    nEnd        = nE;
    bFeature    = FALSE;
    bEdge       = FALSE;

    DBG_ASSERT( ( rAttr.Which() >= EE_ITEMS_START ) && ( rAttr.Which() <= EE_ITEMS_END ), "EditCharAttrib CTOR: Invalid id!" );
    DBG_ASSERT( ( rAttr.Which() < EE_FEATURE_START ) || ( rAttr.Which() > EE_FEATURE_END ) || ( nE == (nS+1) ), "EditCharAttrib CTOR: Invalid feature!" );
}

void EditCharAttrib::SetFont( SvxFont&, OutputDevice* )
{
}


// -------------------------------------------------------------------------
// class EditCharAttribFont
// -------------------------------------------------------------------------
EditCharAttribFont::EditCharAttribFont( const SvxFontItem& rAttr, USHORT _nStart, USHORT _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_FONTINFO || rAttr.Which() == EE_CHAR_FONTINFO_CJK || rAttr.Which() == EE_CHAR_FONTINFO_CTL, "Not a Font attribute!" );
}

void EditCharAttribFont::SetFont( SvxFont& rFont, OutputDevice* )
{
    const SvxFontItem& rAttr = (const SvxFontItem&)(*GetItem());

    rFont.SetName( rAttr.GetFamilyName() );
    rFont.SetFamily( rAttr.GetFamily() );
    rFont.SetPitch( rAttr.GetPitch() );
    rFont.SetCharSet( rAttr.GetCharSet() );
}

// -------------------------------------------------------------------------
// class EditCharAttribItalic
// -------------------------------------------------------------------------
EditCharAttribItalic::EditCharAttribItalic( const SvxPostureItem& rAttr, USHORT _nStart, USHORT _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_ITALIC || rAttr.Which() == EE_CHAR_ITALIC_CJK || rAttr.Which() == EE_CHAR_ITALIC_CTL, "Not a Italic attribute!" );
}

void EditCharAttribItalic::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetItalic( ((const SvxPostureItem*)GetItem())->GetPosture() );
}

// -------------------------------------------------------------------------
// class EditCharAttribWeight
// -------------------------------------------------------------------------
EditCharAttribWeight::EditCharAttribWeight( const SvxWeightItem& rAttr, USHORT _nStart, USHORT _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_WEIGHT || rAttr.Which() == EE_CHAR_WEIGHT_CJK || rAttr.Which() == EE_CHAR_WEIGHT_CTL, "Not a Weight attribute!" );
}

void EditCharAttribWeight::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetWeight( (FontWeight)((const SvxWeightItem*)GetItem())->GetValue() );
}

// -------------------------------------------------------------------------
// class EditCharAttribUnderline
// -------------------------------------------------------------------------
EditCharAttribUnderline::EditCharAttribUnderline( const SvxUnderlineItem& rAttr, USHORT _nStart, USHORT _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_UNDERLINE, "Not a Underline attribute!" );
}

void EditCharAttribUnderline::SetFont( SvxFont& rFont, OutputDevice* pOutDev )
{
    rFont.SetUnderline( (FontUnderline)((const SvxUnderlineItem*)GetItem())->GetValue() );
    if ( pOutDev )
        pOutDev->SetTextLineColor( ((const SvxUnderlineItem*)GetItem())->GetColor() );
}

// -------------------------------------------------------------------------
// class EditCharAttribOverline
// -------------------------------------------------------------------------
EditCharAttribOverline::EditCharAttribOverline( const SvxOverlineItem& rAttr, USHORT _nStart, USHORT _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_OVERLINE, "Not a overline attribute!" );
}

void EditCharAttribOverline::SetFont( SvxFont& rFont, OutputDevice* pOutDev )
{
    rFont.SetOverline( (FontUnderline)((const SvxOverlineItem*)GetItem())->GetValue() );
    if ( pOutDev )
        pOutDev->SetOverlineColor( ((const SvxOverlineItem*)GetItem())->GetColor() );
}

// -------------------------------------------------------------------------
// class EditCharAttribFontHeight
// -------------------------------------------------------------------------
EditCharAttribFontHeight::EditCharAttribFontHeight( const SvxFontHeightItem& rAttr, USHORT _nStart, USHORT _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_FONTHEIGHT || rAttr.Which() == EE_CHAR_FONTHEIGHT_CJK || rAttr.Which() == EE_CHAR_FONTHEIGHT_CTL, "Not a Height attribute!" );
}

void EditCharAttribFontHeight::SetFont( SvxFont& rFont, OutputDevice* )
{
    // Property is ignored
    rFont.SetSize( Size( rFont.GetSize().Width(), ((const SvxFontHeightItem*)GetItem())->GetHeight() ) );
}

// -------------------------------------------------------------------------
// class EditCharAttribFontWidth
// -------------------------------------------------------------------------
EditCharAttribFontWidth::EditCharAttribFontWidth( const SvxCharScaleWidthItem& rAttr, USHORT _nStart, USHORT _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_FONTWIDTH, "Not a Width attribute!" );
}

void EditCharAttribFontWidth::SetFont( SvxFont& /*rFont*/, OutputDevice* )
{
    // must be calculated outside, because f(device)...
}

// -------------------------------------------------------------------------
// class EditCharAttribStrikeout
// -------------------------------------------------------------------------
EditCharAttribStrikeout::EditCharAttribStrikeout( const SvxCrossedOutItem& rAttr, USHORT _nStart, USHORT _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_STRIKEOUT, "Not a Size attribute!" );
}

void EditCharAttribStrikeout::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetStrikeout( (FontStrikeout)((const SvxCrossedOutItem*)GetItem())->GetValue() );
}

// -------------------------------------------------------------------------
// class EditCharAttribColor
// -------------------------------------------------------------------------
EditCharAttribColor::EditCharAttribColor( const SvxColorItem& rAttr, USHORT _nStart, USHORT _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_COLOR, "Not a Color attribute!" );
}

void EditCharAttribColor::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetColor( ((const SvxColorItem*)GetItem())->GetValue() );
}

// -------------------------------------------------------------------------
// class EditCharAttribLanguage
// -------------------------------------------------------------------------
EditCharAttribLanguage::EditCharAttribLanguage( const SvxLanguageItem& rAttr, USHORT _nStart, USHORT _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( ( rAttr.Which() == EE_CHAR_LANGUAGE ) || ( rAttr.Which() == EE_CHAR_LANGUAGE_CJK ) || ( rAttr.Which() == EE_CHAR_LANGUAGE_CTL ), "Not a Language attribute!" );
}

void EditCharAttribLanguage::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetLanguage( ((const SvxLanguageItem*)GetItem())->GetLanguage() );
}

// -------------------------------------------------------------------------
// class EditCharAttribShadow
// -------------------------------------------------------------------------
EditCharAttribShadow::EditCharAttribShadow( const SvxShadowedItem& rAttr, USHORT _nStart, USHORT _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_SHADOW, "Not a Shadow attribute!" );
}

void EditCharAttribShadow::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetShadow( (BOOL)((const SvxShadowedItem*)GetItem())->GetValue() );
}

// -------------------------------------------------------------------------
// class EditCharAttribEscapement
// -------------------------------------------------------------------------
EditCharAttribEscapement::EditCharAttribEscapement( const SvxEscapementItem& rAttr, USHORT _nStart, USHORT _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_ESCAPEMENT, "Not a escapement attribute!" );
}

#if defined( WIN ) && !defined( WNT )
#pragma optimize ("", off)
#endif

void EditCharAttribEscapement::SetFont( SvxFont& rFont, OutputDevice* )
{
    USHORT nProp = ((const SvxEscapementItem*)GetItem())->GetProp();
    rFont.SetPropr( (BYTE)nProp );

    short nEsc = ((const SvxEscapementItem*)GetItem())->GetEsc();
    if ( nEsc == DFLT_ESC_AUTO_SUPER )
        nEsc = 100 - nProp;
    else if ( nEsc == DFLT_ESC_AUTO_SUB )
        nEsc = sal::static_int_cast< short >( -( 100 - nProp ) );
    rFont.SetEscapement( nEsc );
}

#if defined( WIN ) && !defined( WNT )
#pragma optimize ("", on)
#endif


// -------------------------------------------------------------------------
// class EditCharAttribOutline
// -------------------------------------------------------------------------
EditCharAttribOutline::EditCharAttribOutline( const SvxContourItem& rAttr, USHORT _nStart, USHORT _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_OUTLINE, "Not a Outline attribute!" );
}

void EditCharAttribOutline::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetOutline( (BOOL)((const SvxContourItem*)GetItem())->GetValue() );
}

// -------------------------------------------------------------------------
// class EditCharAttribTab
// -------------------------------------------------------------------------
EditCharAttribTab::EditCharAttribTab( const SfxVoidItem& rAttr, USHORT nPos )
    : EditCharAttrib( rAttr, nPos, nPos+1 )
{
    SetFeature( TRUE );
}

void EditCharAttribTab::SetFont( SvxFont&, OutputDevice* )
{
}

// -------------------------------------------------------------------------
// class EditCharAttribLineBreak
// -------------------------------------------------------------------------
EditCharAttribLineBreak::EditCharAttribLineBreak( const SfxVoidItem& rAttr, USHORT nPos )
    : EditCharAttrib( rAttr, nPos, nPos+1 )
{
    SetFeature( TRUE );
}

void EditCharAttribLineBreak::SetFont( SvxFont&, OutputDevice* )
{
}

// -------------------------------------------------------------------------
// class EditCharAttribField
// -------------------------------------------------------------------------
EditCharAttribField::EditCharAttribField( const SvxFieldItem& rAttr, USHORT nPos )
    : EditCharAttrib( rAttr, nPos, nPos+1 )
{
    SetFeature( TRUE ); // !!!
    pTxtColor = 0;
    pFldColor = 0;
}

void EditCharAttribField::SetFont( SvxFont& rFont, OutputDevice* )
{
    if ( pFldColor )
    {
        rFont.SetFillColor( *pFldColor );
        rFont.SetTransparent( FALSE );
    }
    if ( pTxtColor )
        rFont.SetColor( *pTxtColor );
}

EditCharAttribField::EditCharAttribField( const EditCharAttribField& rAttr )
    : EditCharAttrib( *rAttr.GetItem(), rAttr.GetStart(), rAttr.GetEnd() ),
        aFieldValue( rAttr.aFieldValue )
{
    // Use this constructor only for temporary Objects, Item is not pooled.
    pTxtColor = rAttr.pTxtColor ? new Color( *rAttr.pTxtColor ) : 0;
    pFldColor = rAttr.pFldColor ? new Color( *rAttr.pFldColor ) : 0;
}

EditCharAttribField::~EditCharAttribField()
{
    Reset();
}

BOOL EditCharAttribField::operator == ( const EditCharAttribField& rAttr ) const
{
    if ( aFieldValue != rAttr.aFieldValue )
        return FALSE;

    if ( ( pTxtColor && !rAttr.pTxtColor ) || ( !pTxtColor && rAttr.pTxtColor ) )
        return FALSE;
    if ( ( pTxtColor && rAttr.pTxtColor ) && ( *pTxtColor != *rAttr.pTxtColor ) )
        return FALSE;

    if ( ( pFldColor && !rAttr.pFldColor ) || ( !pFldColor && rAttr.pFldColor ) )
        return FALSE;
    if ( ( pFldColor && rAttr.pFldColor ) && ( *pFldColor != *rAttr.pFldColor ) )
        return FALSE;

    return TRUE;
}

// -------------------------------------------------------------------------
// class EditCharAttribPairKerning
// -------------------------------------------------------------------------
EditCharAttribPairKerning::EditCharAttribPairKerning( const SvxAutoKernItem& rAttr, USHORT _nStart, USHORT _nEnd )
: EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_PAIRKERNING, "Not a Pair Kerning!" );
}

void EditCharAttribPairKerning::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetKerning( ((const SvxAutoKernItem*)GetItem())->GetValue() );
}

// -------------------------------------------------------------------------
// class EditCharAttribKerning
// -------------------------------------------------------------------------
EditCharAttribKerning::EditCharAttribKerning( const SvxKerningItem& rAttr, USHORT _nStart, USHORT _nEnd )
: EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_KERNING, "Not a Kerning!" );
}

void EditCharAttribKerning::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetFixKerning( ((const SvxKerningItem*)GetItem())->GetValue() );
}

// -------------------------------------------------------------------------
// class EditCharAttribWordLineMode
// -------------------------------------------------------------------------
EditCharAttribWordLineMode::EditCharAttribWordLineMode( const SvxWordLineModeItem& rAttr, USHORT _nStart, USHORT _nEnd )
: EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_WLM, "Not a Kerning!" );
}

void EditCharAttribWordLineMode::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetWordLineMode( ((const SvxWordLineModeItem*)GetItem())->GetValue() );
}

// -------------------------------------------------------------------------
// class EditCharAttribEmphasisMark
// -------------------------------------------------------------------------
EditCharAttribEmphasisMark::EditCharAttribEmphasisMark( const SvxEmphasisMarkItem& rAttr, USHORT _nStart, USHORT _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_EMPHASISMARK, "Not a Emphasis attribute!" );
}

void EditCharAttribEmphasisMark::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetEmphasisMark( ((const SvxEmphasisMarkItem*)GetItem())->GetEmphasisMark() );
}

// -------------------------------------------------------------------------
// class EditCharAttribRelief
// -------------------------------------------------------------------------
EditCharAttribRelief::EditCharAttribRelief( const SvxCharReliefItem& rAttr, USHORT _nStart, USHORT _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_RELIEF, "Not a relief attribute!" );
}

void EditCharAttribRelief::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetRelief( (FontRelief)((const SvxCharReliefItem*)GetItem())->GetValue() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
