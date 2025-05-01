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

#include <vcl/outdev.hxx>

#include <svl/grabbagitem.hxx>
#include <svl/voiditem.hxx>
#include <libxml/xmlwriter.h>
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
#include <editeng/kernitem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/cmapitem.hxx>

#include <editattr.hxx>


EditCharAttrib::EditCharAttrib(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 nS, sal_Int32 nE )
: maItemHolder(rPool, &rItem)
, nStart(nS)
, nEnd(nE)
, bFeature(false)
, bEdge(false)
, bExpandable(true)
{
    assert((rItem.Which() >= EE_ITEMS_START) && (rItem.Which() <= EE_ITEMS_END));
    assert((rItem.Which() < EE_FEATURE_START) || (rItem.Which() > EE_FEATURE_END) || (nE == (nS+1)));
}

EditCharAttrib::~EditCharAttrib()
{
}

void EditCharAttrib::SetFont( SvxFont&, OutputDevice* )
{
}

void EditCharAttrib::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("EditCharAttrib"));
    (void)xmlTextWriterWriteFormatAttribute(
    pWriter, BAD_CAST("nStart"), "%" SAL_PRIdINT32, nStart);
    (void)xmlTextWriterWriteFormatAttribute(
    pWriter, BAD_CAST("nEnd"), "%" SAL_PRIdINT32, nEnd);
    GetItem()->dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);
}



EditCharAttribFont::EditCharAttribFont(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert(rItem.Which() == EE_CHAR_FONTINFO || rItem.Which() == EE_CHAR_FONTINFO_CJK || rItem.Which() == EE_CHAR_FONTINFO_CTL);
}

void EditCharAttribFont::SetFont( SvxFont& rFont, OutputDevice* )
{
    const SvxFontItem& rAttr = static_cast<const SvxFontItem&>(*GetItem());

    rFont.SetFamilyName( rAttr.GetFamilyName() );
    rFont.SetFamily( rAttr.GetFamily() );
    rFont.SetPitch( rAttr.GetPitch() );
    rFont.SetCharSet( rAttr.GetCharSet() );
}



EditCharAttribItalic::EditCharAttribItalic(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert(rItem.Which() == EE_CHAR_ITALIC || rItem.Which() == EE_CHAR_ITALIC_CJK || rItem.Which() == EE_CHAR_ITALIC_CTL);
}

void EditCharAttribItalic::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetItalic( static_cast<const SvxPostureItem*>(GetItem())->GetPosture() );
}



EditCharAttribWeight::EditCharAttribWeight(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert(rItem.Which() == EE_CHAR_WEIGHT || rItem.Which() == EE_CHAR_WEIGHT_CJK || rItem.Which() == EE_CHAR_WEIGHT_CTL);
}

void EditCharAttribWeight::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetWeight( static_cast<const SvxWeightItem*>(GetItem())->GetValue() );
}



EditCharAttribUnderline::EditCharAttribUnderline(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert(rItem.Which() == EE_CHAR_UNDERLINE);
}

void EditCharAttribUnderline::SetFont( SvxFont& rFont, OutputDevice* pOutDev )
{
    rFont.SetUnderline( static_cast<const SvxUnderlineItem*>(GetItem())->GetValue() );

    if ( pOutDev )
        pOutDev->SetTextLineColor( static_cast<const SvxUnderlineItem*>(GetItem())->GetColor() );

}



EditCharAttribOverline::EditCharAttribOverline(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert(rItem.Which() == EE_CHAR_OVERLINE);
}

void EditCharAttribOverline::SetFont( SvxFont& rFont, OutputDevice* pOutDev )
{
    rFont.SetOverline( static_cast<const SvxOverlineItem*>(GetItem())->GetValue() );
    if ( pOutDev )
        pOutDev->SetOverlineColor( static_cast<const SvxOverlineItem*>(GetItem())->GetColor() );
}



EditCharAttribFontHeight::EditCharAttribFontHeight(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert(rItem.Which() == EE_CHAR_FONTHEIGHT || rItem.Which() == EE_CHAR_FONTHEIGHT_CJK || rItem.Which() == EE_CHAR_FONTHEIGHT_CTL);
}

void EditCharAttribFontHeight::SetFont( SvxFont& rFont, OutputDevice* )
{
    // Property is ignored
    rFont.SetFontSize( Size( rFont.GetFontSize().Width(), static_cast<const SvxFontHeightItem*>(GetItem())->GetHeight() ) );
}



EditCharAttribFontWidth::EditCharAttribFontWidth(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert(rItem.Which() == EE_CHAR_FONTWIDTH);
}

void EditCharAttribFontWidth::SetFont( SvxFont& /*rFont*/, OutputDevice* )
{
    // must be calculated outside, because f(device)...
}



EditCharAttribStrikeout::EditCharAttribStrikeout(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert(rItem.Which() == EE_CHAR_STRIKEOUT);
}

void EditCharAttribStrikeout::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetStrikeout( static_cast<const SvxCrossedOutItem*>(GetItem())->GetValue() );
}



EditCharAttribCaseMap::EditCharAttribCaseMap(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert(rItem.Which() == EE_CHAR_CASEMAP);
}

void EditCharAttribCaseMap::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetCaseMap( static_cast<const SvxCaseMapItem*>(GetItem())->GetCaseMap() );
}



EditCharAttribColor::EditCharAttribColor(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert(rItem.Which() == EE_CHAR_COLOR);
}

void EditCharAttribColor::SetFont( SvxFont& rFont, OutputDevice* )
{
    Color aColor = static_cast<const SvxColorItem*>(GetItem())->GetValue();
    rFont.SetColor( aColor);
}


EditCharAttribBackgroundColor::EditCharAttribBackgroundColor(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert(rItem.Which() == EE_CHAR_BKGCOLOR);
}

void EditCharAttribBackgroundColor::SetFont( SvxFont& rFont, OutputDevice* )
{
    Color aColor = static_cast<const SvxColorItem*>(GetItem())->GetValue();
    rFont.SetTransparent(aColor.IsTransparent());
    rFont.SetFillColor(aColor);
}

EditCharAttribLanguage::EditCharAttribLanguage(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert((rItem.Which() == EE_CHAR_LANGUAGE) || (rItem.Which() == EE_CHAR_LANGUAGE_CJK) || (rItem.Which() == EE_CHAR_LANGUAGE_CTL));
}

void EditCharAttribLanguage::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetLanguage( static_cast<const SvxLanguageItem*>(GetItem())->GetLanguage() );
}

EditCharAttribRuby::EditCharAttribRuby(SfxItemPool& rPool, const SfxPoolItem& rItem,
                                       sal_Int32 nStartIn, sal_Int32 nEndIn)
    : EditCharAttrib(rPool, rItem, nStartIn, nEndIn)
{
    assert(rItem.Which() == EE_CHAR_RUBY);
    SetExpandable(false);
}

void EditCharAttribRuby::SetFont(SvxFont&, OutputDevice*) {}

EditCharAttribShadow::EditCharAttribShadow(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert(rItem.Which() == EE_CHAR_SHADOW);
}

void EditCharAttribShadow::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetShadow( static_cast<const SvxShadowedItem*>(GetItem())->GetValue() );
}



EditCharAttribEscapement::EditCharAttribEscapement(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert(rItem.Which() == EE_CHAR_ESCAPEMENT);
}

void EditCharAttribEscapement::SetFont( SvxFont& rFont, OutputDevice* pOutDev )
{
    sal_uInt16 const nProp = static_cast<const SvxEscapementItem*>(GetItem())->GetProportionalHeight();
    rFont.SetPropr( static_cast<sal_uInt8>(nProp) );

    short nEsc = static_cast<const SvxEscapementItem*>(GetItem())->GetEsc();
    rFont.SetNonAutoEscapement( nEsc, pOutDev );
}



EditCharAttribOutline::EditCharAttribOutline(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert(rItem.Which() == EE_CHAR_OUTLINE);
}

void EditCharAttribOutline::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetOutline( static_cast<const SvxContourItem*>(GetItem())->GetValue() );
}



EditCharAttribTab::EditCharAttribTab(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 nPos)
: EditCharAttrib(rPool, rItem, nPos, nPos+1)
{
    SetFeature( true );
}

void EditCharAttribTab::SetFont( SvxFont&, OutputDevice* )
{
}



EditCharAttribLineBreak::EditCharAttribLineBreak(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 nPos)
: EditCharAttrib(rPool, rItem, nPos, nPos+1)
{
    SetFeature( true );
}

void EditCharAttribLineBreak::SetFont( SvxFont&, OutputDevice* )
{
}



EditCharAttribField::EditCharAttribField(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 nPos)
: EditCharAttrib(rPool, rItem, nPos, nPos+1)
{
    SetFeature( true ); // !!!
}

void EditCharAttribField::SetFont( SvxFont& rFont, OutputDevice* )
{
    if ( mxFldColor )
    {
        rFont.SetFillColor( *mxFldColor );
        rFont.SetTransparent( false );
    }
    if ( mxTxtColor )
        rFont.SetColor( *mxTxtColor );
    if ( mxFldLineStyle )
        rFont.SetUnderline( *mxFldLineStyle );
}


void EditCharAttribField::SetFieldValue(const OUString& rVal)
{
    aFieldValue = rVal;
}

void EditCharAttribField::Reset()
{
    aFieldValue.clear();
    mxTxtColor.reset();
    mxFldColor.reset();
    mxFldLineStyle.reset();
}

EditCharAttribField::EditCharAttribField(const EditCharAttribField& rAttr)
: EditCharAttrib(rAttr.GetHolder().getPool(), *rAttr.GetHolder().getItem(), rAttr.GetStart(), rAttr.GetEnd())
, aFieldValue( rAttr.aFieldValue )
{
    // Use this constructor only for temporary Objects, Item is not pooled.
    mxTxtColor = rAttr.mxTxtColor;
    mxFldColor = rAttr.mxFldColor;
    mxFldLineStyle = rAttr.mxFldLineStyle;
}

EditCharAttribField::~EditCharAttribField()
{
    Reset();
}

bool EditCharAttribField::operator == ( const EditCharAttribField& rAttr ) const
{
    if ( aFieldValue != rAttr.aFieldValue )
        return false;

    if ( ( mxTxtColor && !rAttr.mxTxtColor ) || ( !mxTxtColor && rAttr.mxTxtColor ) )
        return false;
    if ( ( mxTxtColor && rAttr.mxTxtColor ) && ( *mxTxtColor != *rAttr.mxTxtColor ) )
        return false;

    if ( ( mxFldColor && !rAttr.mxFldColor ) || ( !mxFldColor && rAttr.mxFldColor ) )
        return false;
    if ( ( mxFldColor && rAttr.mxFldColor ) && ( *mxFldColor != *rAttr.mxFldColor ) )
        return false;

    if ( ( mxFldLineStyle && !rAttr.mxFldLineStyle ) || ( !mxFldLineStyle && rAttr.mxFldLineStyle ) )
        return false;
    if ( ( mxFldLineStyle && rAttr.mxFldLineStyle ) && ( *mxFldLineStyle != *rAttr.mxFldLineStyle ) )
        return false;

    return true;
}



EditCharAttribPairKerning::EditCharAttribPairKerning(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert(rItem.Which() == EE_CHAR_PAIRKERNING);
}

void EditCharAttribPairKerning::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetKerning( static_cast<const SvxAutoKernItem*>(GetItem())->GetValue() ? FontKerning::FontSpecific : FontKerning::NONE );
}



EditCharAttribKerning::EditCharAttribKerning(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert(rItem.Which() == EE_CHAR_KERNING);
}

void EditCharAttribKerning::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetFixKerning( static_cast<const SvxKerningItem*>(GetItem())->GetValue() );
}



EditCharAttribWordLineMode::EditCharAttribWordLineMode(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert(rItem.Which() == EE_CHAR_WLM);
}

void EditCharAttribWordLineMode::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetWordLineMode( static_cast<const SvxWordLineModeItem*>(GetItem())->GetValue() );
}



EditCharAttribEmphasisMark::EditCharAttribEmphasisMark(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert(rItem.Which() == EE_CHAR_EMPHASISMARK);
}

void EditCharAttribEmphasisMark::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetEmphasisMark( static_cast<const SvxEmphasisMarkItem*>(GetItem())->GetEmphasisMark() );
}



EditCharAttribRelief::EditCharAttribRelief(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert(rItem.Which() == EE_CHAR_RELIEF);
}

void EditCharAttribRelief::SetFont( SvxFont& rFont, OutputDevice* )
{
    rFont.SetRelief( static_cast<const SvxCharReliefItem*>(GetItem())->GetValue() );
}


EditCharAttribGrabBag::EditCharAttribGrabBag(SfxItemPool& rPool, const SfxPoolItem& rItem, sal_Int32 _nStart, sal_Int32 _nEnd)
: EditCharAttrib(rPool, rItem, _nStart, _nEnd)
{
    assert(rItem.Which() == EE_CHAR_GRABBAG);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
