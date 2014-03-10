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


#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include <svl/grabbagitem.hxx>
#include <editeng/svxfont.hxx>
#include <editeng/flditem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/charsetcoloritem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/cmapitem.hxx>

#include "editattr.hxx"



// class EditCharAttrib

EditCharAttrib::EditCharAttrib( const SfxPoolItem& rAttr, sal_uInt16 nS, sal_uInt16 nE ) :
    nStart(nS), nEnd(nE), bFeature(false), bEdge(false)
{
    pItem = &rAttr;

    DBG_ASSERT( ( rAttr.Which() >= EE_ITEMS_START ) && ( rAttr.Which() <= EE_ITEMS_END ), "EditCharAttrib CTOR: Invalid id!" );
    DBG_ASSERT( ( rAttr.Which() < EE_FEATURE_START ) || ( rAttr.Which() > EE_FEATURE_END ) || ( nE == (nS+1) ), "EditCharAttrib CTOR: Invalid feature!" );
}

EditCharAttrib::~EditCharAttrib()
{
}

void EditCharAttrib::SetFont( SvxFont&, OutputDevice* )
{
}



// class EditCharAttribFont

EditCharAttribFont::EditCharAttribFont( const SvxFontItem& rAttr, sal_uInt16 _nStart, sal_uInt16 _nEnd )
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


// class EditCharAttribItalic

EditCharAttribItalic::EditCharAttribItalic( const SvxPostureItem& rAttr, sal_uInt16 _nStart, sal_uInt16 _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_ITALIC || rAttr.Which() == EE_CHAR_ITALIC_CJK || rAttr.Which() == EE_CHAR_ITALIC_CTL, "Not a Italic attribute!" );
}

void EditCharAttribItalic::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetItalic( ((const SvxPostureItem*)GetItem())->GetPosture() );
}


// class EditCharAttribWeight

EditCharAttribWeight::EditCharAttribWeight( const SvxWeightItem& rAttr, sal_uInt16 _nStart, sal_uInt16 _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_WEIGHT || rAttr.Which() == EE_CHAR_WEIGHT_CJK || rAttr.Which() == EE_CHAR_WEIGHT_CTL, "Not a Weight attribute!" );
}

void EditCharAttribWeight::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetWeight( (FontWeight)((const SvxWeightItem*)GetItem())->GetValue() );
}


// class EditCharAttribUnderline

EditCharAttribUnderline::EditCharAttribUnderline( const SvxUnderlineItem& rAttr, sal_uInt16 _nStart, sal_uInt16 _nEnd )
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


// class EditCharAttribOverline

EditCharAttribOverline::EditCharAttribOverline( const SvxOverlineItem& rAttr, sal_uInt16 _nStart, sal_uInt16 _nEnd )
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


// class EditCharAttribFontHeight

EditCharAttribFontHeight::EditCharAttribFontHeight( const SvxFontHeightItem& rAttr, sal_uInt16 _nStart, sal_uInt16 _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_FONTHEIGHT || rAttr.Which() == EE_CHAR_FONTHEIGHT_CJK || rAttr.Which() == EE_CHAR_FONTHEIGHT_CTL, "Not a Height attribute!" );
}

void EditCharAttribFontHeight::SetFont( SvxFont& rFont, OutputDevice* )
{
    // Property is ignored
    rFont.SetSize( Size( rFont.GetSize().Width(), ((const SvxFontHeightItem*)GetItem())->GetHeight() ) );
}


// class EditCharAttribFontWidth

EditCharAttribFontWidth::EditCharAttribFontWidth( const SvxCharScaleWidthItem& rAttr, sal_uInt16 _nStart, sal_uInt16 _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_FONTWIDTH, "Not a Width attribute!" );
}

void EditCharAttribFontWidth::SetFont( SvxFont& /*rFont*/, OutputDevice* )
{
    // must be calculated outside, because f(device)...
}


// class EditCharAttribStrikeout

EditCharAttribStrikeout::EditCharAttribStrikeout( const SvxCrossedOutItem& rAttr, sal_uInt16 _nStart, sal_uInt16 _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_STRIKEOUT, "Not a Strikeout attribute!" );
}

void EditCharAttribStrikeout::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetStrikeout( (FontStrikeout)((const SvxCrossedOutItem*)GetItem())->GetValue() );
}


// class EditCharAttribCaseMap

EditCharAttribCaseMap::EditCharAttribCaseMap( const SvxCaseMapItem& rAttr, sal_uInt16 _nStart, sal_uInt16 _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_CASEMAP, "Not a CaseMap Item!" );
}

void EditCharAttribCaseMap::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetCaseMap( ((const SvxCaseMapItem*)GetItem())->GetCaseMap() );
}


// class EditCharAttribColor

EditCharAttribColor::EditCharAttribColor( const SvxColorItem& rAttr, sal_uInt16 _nStart, sal_uInt16 _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_COLOR, "Not a Color attribute!" );
}

void EditCharAttribColor::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetColor( ((const SvxColorItem*)GetItem())->GetValue() );
}


// class EditCharAttribLanguage

EditCharAttribLanguage::EditCharAttribLanguage( const SvxLanguageItem& rAttr, sal_uInt16 _nStart, sal_uInt16 _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( ( rAttr.Which() == EE_CHAR_LANGUAGE ) || ( rAttr.Which() == EE_CHAR_LANGUAGE_CJK ) || ( rAttr.Which() == EE_CHAR_LANGUAGE_CTL ), "Not a Language attribute!" );
}

void EditCharAttribLanguage::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetLanguage( ((const SvxLanguageItem*)GetItem())->GetLanguage() );
}


// class EditCharAttribShadow

EditCharAttribShadow::EditCharAttribShadow( const SvxShadowedItem& rAttr, sal_uInt16 _nStart, sal_uInt16 _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_SHADOW, "Not a Shadow attribute!" );
}

void EditCharAttribShadow::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetShadow( ((const SvxShadowedItem*)GetItem())->GetValue() );
}


// class EditCharAttribEscapement

EditCharAttribEscapement::EditCharAttribEscapement( const SvxEscapementItem& rAttr, sal_uInt16 _nStart, sal_uInt16 _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_ESCAPEMENT, "Not a escapement attribute!" );
}

void EditCharAttribEscapement::SetFont( SvxFont& rFont, OutputDevice* )
{
    sal_uInt16 nProp = ((const SvxEscapementItem*)GetItem())->GetProp();
    rFont.SetPropr( (sal_uInt8)nProp );

    short nEsc = ((const SvxEscapementItem*)GetItem())->GetEsc();
    if ( nEsc == DFLT_ESC_AUTO_SUPER )
        nEsc = 100 - nProp;
    else if ( nEsc == DFLT_ESC_AUTO_SUB )
        nEsc = sal::static_int_cast< short >( -( 100 - nProp ) );
    rFont.SetEscapement( nEsc );
}


// class EditCharAttribOutline

EditCharAttribOutline::EditCharAttribOutline( const SvxContourItem& rAttr, sal_uInt16 _nStart, sal_uInt16 _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_OUTLINE, "Not a Outline attribute!" );
}

void EditCharAttribOutline::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetOutline( ((const SvxContourItem*)GetItem())->GetValue() );
}


// class EditCharAttribTab

EditCharAttribTab::EditCharAttribTab( const SfxVoidItem& rAttr, sal_uInt16 nPos )
    : EditCharAttrib( rAttr, nPos, nPos+1 )
{
    SetFeature( sal_True );
}

void EditCharAttribTab::SetFont( SvxFont&, OutputDevice* )
{
}


// class EditCharAttribLineBreak

EditCharAttribLineBreak::EditCharAttribLineBreak( const SfxVoidItem& rAttr, sal_uInt16 nPos )
    : EditCharAttrib( rAttr, nPos, nPos+1 )
{
    SetFeature( sal_True );
}

void EditCharAttribLineBreak::SetFont( SvxFont&, OutputDevice* )
{
}


// class EditCharAttribField

EditCharAttribField::EditCharAttribField( const SvxFieldItem& rAttr, sal_uInt16 nPos )
    : EditCharAttrib( rAttr, nPos, nPos+1 )
{
    SetFeature( sal_True ); // !!!
    pTxtColor = 0;
    pFldColor = 0;
}

void EditCharAttribField::SetFont( SvxFont& rFont, OutputDevice* )
{
    if ( pFldColor )
    {
        rFont.SetFillColor( *pFldColor );
        rFont.SetTransparent( false );
    }
    if ( pTxtColor )
        rFont.SetColor( *pTxtColor );
}

const OUString& EditCharAttribField::GetFieldValue() const
{
    return aFieldValue;
}

void EditCharAttribField::SetFieldValue(const OUString& rVal)
{
    aFieldValue = rVal;
}

void EditCharAttribField::Reset()
{
    aFieldValue = OUString();
    delete pTxtColor; pTxtColor = NULL;
    delete pFldColor; pFldColor = NULL;
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

bool EditCharAttribField::operator == ( const EditCharAttribField& rAttr ) const
{
    if ( aFieldValue != rAttr.aFieldValue )
        return false;

    if ( ( pTxtColor && !rAttr.pTxtColor ) || ( !pTxtColor && rAttr.pTxtColor ) )
        return false;
    if ( ( pTxtColor && rAttr.pTxtColor ) && ( *pTxtColor != *rAttr.pTxtColor ) )
        return false;

    if ( ( pFldColor && !rAttr.pFldColor ) || ( !pFldColor && rAttr.pFldColor ) )
        return false;
    if ( ( pFldColor && rAttr.pFldColor ) && ( *pFldColor != *rAttr.pFldColor ) )
        return false;

    return true;
}


// class EditCharAttribPairKerning

EditCharAttribPairKerning::EditCharAttribPairKerning( const SvxAutoKernItem& rAttr, sal_uInt16 _nStart, sal_uInt16 _nEnd )
: EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_PAIRKERNING, "Not a Pair Kerning!" );
}

void EditCharAttribPairKerning::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetKerning( ((const SvxAutoKernItem*)GetItem())->GetValue() );
}


// class EditCharAttribKerning

EditCharAttribKerning::EditCharAttribKerning( const SvxKerningItem& rAttr, sal_uInt16 _nStart, sal_uInt16 _nEnd )
: EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_KERNING, "Not a Kerning!" );
}

void EditCharAttribKerning::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetFixKerning( ((const SvxKerningItem*)GetItem())->GetValue() );
}


// class EditCharAttribWordLineMode

EditCharAttribWordLineMode::EditCharAttribWordLineMode( const SvxWordLineModeItem& rAttr, sal_uInt16 _nStart, sal_uInt16 _nEnd )
: EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_WLM, "Not a Kerning!" );
}

void EditCharAttribWordLineMode::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetWordLineMode( ((const SvxWordLineModeItem*)GetItem())->GetValue() );
}


// class EditCharAttribEmphasisMark

EditCharAttribEmphasisMark::EditCharAttribEmphasisMark( const SvxEmphasisMarkItem& rAttr, sal_uInt16 _nStart, sal_uInt16 _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_EMPHASISMARK, "Not a Emphasis attribute!" );
}

void EditCharAttribEmphasisMark::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetEmphasisMark( ((const SvxEmphasisMarkItem*)GetItem())->GetEmphasisMark() );
}


// class EditCharAttribRelief

EditCharAttribRelief::EditCharAttribRelief( const SvxCharReliefItem& rAttr, sal_uInt16 _nStart, sal_uInt16 _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_RELIEF, "Not a relief attribute!" );
}

void EditCharAttribRelief::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetRelief( (FontRelief)((const SvxCharReliefItem*)GetItem())->GetValue() );
}

// class EditCharAttribGrabBag

EditCharAttribGrabBag::EditCharAttribGrabBag( const SfxGrabBagItem& rAttr, sal_uInt16 _nStart, sal_uInt16 _nEnd )
    : EditCharAttrib( rAttr, _nStart, _nEnd )
{
    DBG_ASSERT( rAttr.Which() == EE_CHAR_GRABBAG, "Not a grab bage attribute!" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
