/*************************************************************************
 *
 *  $RCSfile: editattr.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mt $ $Date: 2000-11-24 11:30:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <eeng_pch.hxx>

#pragma hdrstop

#include <svxfont.hxx>
#include <flditem.hxx>
#include "fontitem.hxx"
#include "postitem.hxx"
#include "wghtitem.hxx"
#include "udlnitem.hxx"
#include "fwdtitem.hxx"
#include "cntritem.hxx"
#include "shdditem.hxx"
#include "escpitem.hxx"
#include "colritem.hxx"
#include "wrlmitem.hxx"
#include "fhgtitem.hxx"
#include "crsditem.hxx"
#include "cscoitem.hxx"
#include "kernitem.hxx"
#include "akrnitem.hxx"
#include "langitem.hxx"


#include <editattr.hxx>

DBG_NAME( EE_EditAttrib );

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
}

void EditCharAttrib::SetFont( SvxFont& rFont )
{
}


// -------------------------------------------------------------------------
// class EditCharAttribFont
// -------------------------------------------------------------------------
EditCharAttribFont::EditCharAttribFont( const SvxFontItem& rAttr, USHORT nStart, USHORT nEnd )
    : EditCharAttrib( rAttr, nStart, nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_FONTINFO || rAttr.Which() == EE_CHAR_FONTINFO_CJK || rAttr.Which() == EE_CHAR_FONTINFO_CTL, "Kein Fontattribut!" );
}

void EditCharAttribFont::SetFont( SvxFont& rFont )
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
EditCharAttribItalic::EditCharAttribItalic( const SvxPostureItem& rAttr, USHORT nStart, USHORT nEnd )
    : EditCharAttrib( rAttr, nStart, nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_ITALIC || rAttr.Which() == EE_CHAR_ITALIC_CJK || rAttr.Which() == EE_CHAR_ITALIC_CTL, "Kein Italicattribut!" );
}

void EditCharAttribItalic::SetFont( SvxFont& rFont )
{
    rFont.SetItalic( ((const SvxPostureItem*)GetItem())->GetPosture() );
}

// -------------------------------------------------------------------------
// class EditCharAttribWeight
// -------------------------------------------------------------------------
EditCharAttribWeight::EditCharAttribWeight( const SvxWeightItem& rAttr, USHORT nStart, USHORT nEnd )
    : EditCharAttrib( rAttr, nStart, nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_WEIGHT || rAttr.Which() == EE_CHAR_WEIGHT_CJK || rAttr.Which() == EE_CHAR_WEIGHT_CTL, "Kein Weightttribut!" );
}

void EditCharAttribWeight::SetFont( SvxFont& rFont )
{
    rFont.SetWeight( (FontWeight)((const SvxWeightItem*)GetItem())->GetValue() );
}

// -------------------------------------------------------------------------
// class EditCharAttribUnderline
// -------------------------------------------------------------------------
EditCharAttribUnderline::EditCharAttribUnderline( const SvxUnderlineItem& rAttr, USHORT nStart, USHORT nEnd )
    : EditCharAttrib( rAttr, nStart, nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_UNDERLINE, "Kein Underlineattribut!" );
}

void EditCharAttribUnderline::SetFont( SvxFont& rFont )
{
    rFont.SetUnderline( (FontUnderline)((const SvxUnderlineItem*)GetItem())->GetValue() );
}

// -------------------------------------------------------------------------
// class EditCharAttribFontHeight
// -------------------------------------------------------------------------
EditCharAttribFontHeight::EditCharAttribFontHeight( const SvxFontHeightItem& rAttr, USHORT nStart, USHORT nEnd )
    : EditCharAttrib( rAttr, nStart, nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_FONTHEIGHT || rAttr.Which() == EE_CHAR_FONTHEIGHT_CJK || rAttr.Which() == EE_CHAR_FONTHEIGHT_CTL, "Kein Heightattribut!" );
}

void EditCharAttribFontHeight::SetFont( SvxFont& rFont )
{
    // Prop wird ignoriert
    rFont.SetSize( Size( rFont.GetSize().Width(), ((const SvxFontHeightItem*)GetItem())->GetHeight() ) );
}

// -------------------------------------------------------------------------
// class EditCharAttribFontWidth
// -------------------------------------------------------------------------
EditCharAttribFontWidth::EditCharAttribFontWidth( const SvxFontWidthItem& rAttr, USHORT nStart, USHORT nEnd )
    : EditCharAttrib( rAttr, nStart, nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_FONTWIDTH, "Kein Widthattribut!" );
}

void EditCharAttribFontWidth::SetFont( SvxFont& rFont )
{
    // Prop wird nicht verwendet, weil bei def-Breite von 0 unmoeglich.
    // => f( Device, andere Fonteinstellungen )!
    rFont.SetSize( Size( ((const SvxFontWidthItem*)GetItem())->GetWidth(), rFont.GetSize().Height() ) );
}

// -------------------------------------------------------------------------
// class EditCharAttribStrikeout
// -------------------------------------------------------------------------
EditCharAttribStrikeout::EditCharAttribStrikeout( const SvxCrossedOutItem& rAttr, USHORT nStart, USHORT nEnd )
    : EditCharAttrib( rAttr, nStart, nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_STRIKEOUT, "Kein Sizeattribut!" );
}

void EditCharAttribStrikeout::SetFont( SvxFont& rFont )
{
    rFont.SetStrikeout( (FontStrikeout)((const SvxCrossedOutItem*)GetItem())->GetValue() );
}

// -------------------------------------------------------------------------
// class EditCharAttribColor
// -------------------------------------------------------------------------
EditCharAttribColor::EditCharAttribColor( const SvxColorItem& rAttr, USHORT nStart, USHORT nEnd )
    : EditCharAttrib( rAttr, nStart, nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_COLOR, "Kein Colorattribut!" );
}

void EditCharAttribColor::SetFont( SvxFont& rFont )
{
    rFont.SetColor( ((const SvxColorItem*)GetItem())->GetValue() );
}

// -------------------------------------------------------------------------
// class EditCharAttribLanguage
// -------------------------------------------------------------------------
EditCharAttribLanguage::EditCharAttribLanguage( const SvxLanguageItem& rAttr, USHORT nStart, USHORT nEnd )
    : EditCharAttrib( rAttr, nStart, nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_COLOR, "Kein Languageattribut!" );
}

void EditCharAttribLanguage::SetFont( SvxFont& rFont )
{
    rFont.SetLanguage( ((const SvxLanguageItem*)GetItem())->GetLanguage() );
}

// -------------------------------------------------------------------------
// class EditCharAttribShadow
// -------------------------------------------------------------------------
EditCharAttribShadow::EditCharAttribShadow( const SvxShadowedItem& rAttr, USHORT nStart, USHORT nEnd )
    : EditCharAttrib( rAttr, nStart, nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_SHADOW, "Kein Shadowattribut!" );
}

void EditCharAttribShadow::SetFont( SvxFont& rFont )
{
    rFont.SetShadow( (BOOL)((const SvxShadowedItem*)GetItem())->GetValue() );
}

// -------------------------------------------------------------------------
// class EditCharAttribEscapement
// -------------------------------------------------------------------------
EditCharAttribEscapement::EditCharAttribEscapement( const SvxEscapementItem& rAttr, USHORT nStart, USHORT nEnd )
    : EditCharAttrib( rAttr, nStart, nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_ESCAPEMENT, "Kein Escapementattribut!" );
}

#if defined( WIN ) && !defined( WNT )
#pragma optimize ("", off)
#endif

void EditCharAttribEscapement::SetFont( SvxFont& rFont )
{
    USHORT nProp = ((const SvxEscapementItem*)GetItem())->GetProp();
    rFont.SetPropr( (BYTE)nProp );

    short nEsc = ((const SvxEscapementItem*)GetItem())->GetEsc();
    if ( nEsc == DFLT_ESC_AUTO_SUPER )
        nEsc = 100 - nProp;
    else if ( nEsc == DFLT_ESC_AUTO_SUB )
        nEsc = -( 100 - nProp );
    rFont.SetEscapement( nEsc );
}

#if defined( WIN ) && !defined( WNT )
#pragma optimize ("", on)
#endif


// -------------------------------------------------------------------------
// class EditCharAttribOutline
// -------------------------------------------------------------------------
EditCharAttribOutline::EditCharAttribOutline( const SvxContourItem& rAttr, USHORT nStart, USHORT nEnd )
    : EditCharAttrib( rAttr, nStart, nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_OUTLINE, "Kein Outlineattribut!" );
}

void EditCharAttribOutline::SetFont( SvxFont& rFont )
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

void EditCharAttribTab::SetFont( SvxFont& )
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

void EditCharAttribLineBreak::SetFont( SvxFont& )
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

void EditCharAttribField::SetFont( SvxFont& rFont )
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
    // Diesen CCTOR nur fuer temporaeres Object verwenden,
    // Item wird nicht gepoolt.
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
EditCharAttribPairKerning::EditCharAttribPairKerning( const SvxAutoKernItem& rAttr, USHORT nStart, USHORT nEnd )
    : EditCharAttrib( rAttr, nStart, nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_PAIRKERNING, "Kein PairKerning!" );
}

void EditCharAttribPairKerning::SetFont( SvxFont& rFont )
{
    rFont.SetKerning( ((const SvxAutoKernItem*)GetItem())->GetValue() );
}

// -------------------------------------------------------------------------
// class EditCharAttribKerning
// -------------------------------------------------------------------------
EditCharAttribKerning::EditCharAttribKerning( const SvxKerningItem& rAttr, USHORT nStart, USHORT nEnd )
    : EditCharAttrib( rAttr, nStart, nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_KERNING, "Kein Kerning!" );
}

void EditCharAttribKerning::SetFont( SvxFont& rFont )
{
    rFont.SetFixKerning( ((const SvxKerningItem*)GetItem())->GetValue() );
}

// -------------------------------------------------------------------------
// class EditCharAttribWordLineMode
// -------------------------------------------------------------------------
EditCharAttribWordLineMode::EditCharAttribWordLineMode( const SvxWordLineModeItem& rAttr, USHORT nStart, USHORT nEnd )
    : EditCharAttrib( rAttr, nStart, nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_WLM, "Kein Kerning!" );
}

void EditCharAttribWordLineMode::SetFont( SvxFont& rFont )
{
    rFont.SetWordLineMode( ((const SvxWordLineModeItem*)GetItem())->GetValue() );
}

/*

// -------------------------------------------------------------------------
// class EditCharAttribEmphasisMark
// -------------------------------------------------------------------------
EditCharAttribEmphasisMark::EditCharAttribEmphasisMark( const SvxEmphasisMarkItem& rAttr, USHORT nStart, USHORT nEnd )
    : EditCharAttrib( rAttr, nStart, nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_EMPHASISMARK, "Kein Emphasisattribut!" );
}

void EditCharAttribEmphasisMark::SetFont( SvxFont& rFont )
{
//  rFont.SetEmphasis( ( ((const SvxItem*)GetItem())->GetEmphasisMark() )
}

*/
