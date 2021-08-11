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



EditCharAttrib::EditCharAttrib( const SfxPoolItem& rAttr, sal_Int32 nS, sal_Int32 nE, EditCharAttribType nType, bool bFeature_ ) :
    nStart(nS), nEnd(nE), mnType(nType), bFeature(bFeature_), bEdge(false)
{
    pItem = &rAttr;

    assert((rAttr.Which() >= EE_ITEMS_START) && (rAttr.Which() <= EE_ITEMS_END));
    assert((rAttr.Which() < EE_FEATURE_START) || (rAttr.Which() > EE_FEATURE_END) || (nE == (nS+1)));
}

void EditCharAttrib::SetFont( SvxFont& rFont, OutputDevice* pOutDev )
{
    switch (mnType)
    {
        case EditCharAttribType::Font:
        {
            const SvxFontItem& rAttr = static_cast<const SvxFontItem&>(*GetItem());
            rFont.SetFamilyName( rAttr.GetFamilyName() );
            rFont.SetFamily( rAttr.GetFamily() );
            rFont.SetPitch( rAttr.GetPitch() );
            rFont.SetCharSet( rAttr.GetCharSet() );
            break;
        }
        case EditCharAttribType::Italic:
        {
            rFont.SetItalic( static_cast<const SvxPostureItem*>(GetItem())->GetPosture() );
            break;
        }
        case EditCharAttribType::Weight:
        {
            rFont.SetWeight( static_cast<const SvxWeightItem*>(GetItem())->GetValue() );
            break;
        }
        case EditCharAttribType::Underline:
        {
            rFont.SetUnderline( static_cast<const SvxUnderlineItem*>(GetItem())->GetValue() );
            if ( pOutDev )
                pOutDev->SetTextLineColor( static_cast<const SvxUnderlineItem*>(GetItem())->GetColor() );
            break;
        }
        case EditCharAttribType::Overline:
        {
            rFont.SetOverline( static_cast<const SvxOverlineItem*>(GetItem())->GetValue() );
            if ( pOutDev )
                pOutDev->SetOverlineColor( static_cast<const SvxOverlineItem*>(GetItem())->GetColor() );
            break;
        }
        case EditCharAttribType::FontHeight:
        {
            // Property is ignored
            rFont.SetFontSize( Size( rFont.GetFontSize().Width(), static_cast<const SvxFontHeightItem*>(GetItem())->GetHeight() ) );
            break;
        }
        case EditCharAttribType::FontWidth:
        {
            // must be calculated outside, because f(device)...
            break;
        }
        case EditCharAttribType::Strikeout:
        {
            rFont.SetStrikeout( static_cast<const SvxCrossedOutItem*>(GetItem())->GetValue() );
            break;
        }
        case EditCharAttribType::CaseMap:
        {
            rFont.SetCaseMap( static_cast<const SvxCaseMapItem*>(GetItem())->GetCaseMap() );
            break;
        }
        case EditCharAttribType::Color:
        {
            Color aColor = static_cast<const SvxColorItem*>(GetItem())->GetValue();
            rFont.SetColor( aColor);
            break;
        }
        case EditCharAttribType::BackgroundColor:
        {
            Color aColor = static_cast<const SvxColorItem*>(GetItem())->GetValue();
            rFont.SetTransparent(false);
            rFont.SetFillColor(aColor);
            break;
        }
        case EditCharAttribType::Language:
        {
            rFont.SetLanguage( static_cast<const SvxLanguageItem*>(GetItem())->GetLanguage() );
            break;
        }
        case EditCharAttribType::Shadow:
        {
            rFont.SetShadow( static_cast<const SvxShadowedItem*>(GetItem())->GetValue() );
            break;
        }
        case EditCharAttribType::Escapement:
        {
            sal_uInt16 const nProp = static_cast<const SvxEscapementItem*>(GetItem())->GetProportionalHeight();
            rFont.SetPropr( static_cast<sal_uInt8>(nProp) );

            short nEsc = static_cast<const SvxEscapementItem*>(GetItem())->GetEsc();
            rFont.SetNonAutoEscapement( nEsc, pOutDev );
            break;
        }
        case EditCharAttribType::Outline:
        {
            rFont.SetOutline( static_cast<const SvxContourItem*>(GetItem())->GetValue() );
            break;
        }
        case EditCharAttribType::Tab:
            break;
        case EditCharAttribType::LineBreak:
            break;
        case EditCharAttribType::Field:
        {
            if ( mxFldColor )
            {
                rFont.SetFillColor( *mxFldColor );
                rFont.SetTransparent( false );
            }
            if ( mxTxtColor )
                rFont.SetColor( *mxTxtColor );
            break;
        }
        case EditCharAttribType::PairKerning:
        {
            rFont.SetKerning( static_cast<const SvxAutoKernItem*>(GetItem())->GetValue() ? FontKerning::FontSpecific : FontKerning::NONE );
            break;
        }
        case EditCharAttribType::Kerning:
        {
            rFont.SetFixKerning( static_cast<const SvxKerningItem*>(GetItem())->GetValue() );
            break;
        }
        case EditCharAttribType::WordLineMode:
        {
            rFont.SetWordLineMode( static_cast<const SvxWordLineModeItem*>(GetItem())->GetValue() );
            break;
        }
        case EditCharAttribType::EmphasisMark:
        {
            rFont.SetEmphasisMark( static_cast<const SvxEmphasisMarkItem*>(GetItem())->GetEmphasisMark() );
            break;
        }
        case EditCharAttribType::Relief:
        {
            rFont.SetRelief( static_cast<const SvxCharReliefItem*>(GetItem())->GetValue() );
            break;
        }
        case EditCharAttribType::GrabBag:
            break;
        case EditCharAttribType::Xml:
            break;

        default: assert(false); break;
    };
}

void EditCharAttrib::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("EditCharAttrib"));
    (void)xmlTextWriterWriteFormatAttribute(
    pWriter, BAD_CAST("nStart"), "%" SAL_PRIdINT32, nStart);
    (void)xmlTextWriterWriteFormatAttribute(
    pWriter, BAD_CAST("nEnd"), "%" SAL_PRIdINT32, nEnd);
    pItem->dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);
}



EditCharAttrib EditCharAttrib::MakeFont( const SvxFontItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    assert(rAttr.Which() == EE_CHAR_FONTINFO || rAttr.Which() == EE_CHAR_FONTINFO_CJK || rAttr.Which() == EE_CHAR_FONTINFO_CTL);
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::Font );
}

EditCharAttrib EditCharAttrib::MakeItalic( const SvxPostureItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    assert(rAttr.Which() == EE_CHAR_ITALIC || rAttr.Which() == EE_CHAR_ITALIC_CJK || rAttr.Which() == EE_CHAR_ITALIC_CTL);
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::Italic );
}

EditCharAttrib EditCharAttrib::MakeWeight( const SvxWeightItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    assert(rAttr.Which() == EE_CHAR_WEIGHT || rAttr.Which() == EE_CHAR_WEIGHT_CJK || rAttr.Which() == EE_CHAR_WEIGHT_CTL);
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::Weight );
}

EditCharAttrib EditCharAttrib::MakeUnderline( const SvxUnderlineItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    assert(rAttr.Which() == EE_CHAR_UNDERLINE);
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::Underline );
}


EditCharAttrib EditCharAttrib::MakeOverline( const SvxOverlineItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::Overline );
}

EditCharAttrib EditCharAttrib::MakeFontHeight( const SvxFontHeightItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    assert(rAttr.Which() == EE_CHAR_FONTHEIGHT || rAttr.Which() == EE_CHAR_FONTHEIGHT_CJK || rAttr.Which() == EE_CHAR_FONTHEIGHT_CTL);
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::FontHeight );
}

EditCharAttrib EditCharAttrib::MakeFontWidth( const SvxCharScaleWidthItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    assert(rAttr.Which() == EE_CHAR_FONTWIDTH);
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::FontWidth );
}


EditCharAttrib EditCharAttrib::MakeStrikeout( const SvxCrossedOutItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    assert(rAttr.Which() == EE_CHAR_STRIKEOUT);
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::Strikeout);
}

EditCharAttrib EditCharAttrib::MakeCaseMap( const SvxCaseMapItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    assert(rAttr.Which() == EE_CHAR_CASEMAP);
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::CaseMap );
}

EditCharAttrib EditCharAttrib::MakeColor( const SvxColorItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    assert(rAttr.Which() == EE_CHAR_COLOR);
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::Color);
}

EditCharAttrib EditCharAttrib::MakeBackgroundColor(
                                const SvxColorItem& rAttr,
                                  sal_Int32 _nStart,
                                  sal_Int32 _nEnd )
{
    assert(rAttr.Which() == EE_CHAR_BKGCOLOR);
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::BackgroundColor );
}

EditCharAttrib EditCharAttrib::MakeLanguage( const SvxLanguageItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    assert((rAttr.Which() == EE_CHAR_LANGUAGE) || (rAttr.Which() == EE_CHAR_LANGUAGE_CJK) || (rAttr.Which() == EE_CHAR_LANGUAGE_CTL));
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::Language );
}

EditCharAttrib EditCharAttrib::MakeShadow( const SvxShadowedItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    assert(rAttr.Which() == EE_CHAR_SHADOW);
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::Shadow );
}

EditCharAttrib EditCharAttrib::MakeEscapement( const SvxEscapementItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    assert(rAttr.Which() == EE_CHAR_ESCAPEMENT);
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::Escapement );
}

EditCharAttrib EditCharAttrib::MakeOutline( const SvxContourItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    assert(rAttr.Which() == EE_CHAR_OUTLINE);
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::Outline );
}

EditCharAttrib EditCharAttrib::MakeTab( const SfxVoidItem& rAttr, sal_Int32 nPos )
{
    return EditCharAttrib( rAttr, nPos, nPos+1, EditCharAttribType::Tab, /*bFeature*/true );
}

EditCharAttrib EditCharAttrib::MakeLineBreak( const SfxVoidItem& rAttr, sal_Int32 nPos )
{
    return EditCharAttrib( rAttr, nPos, nPos+1, EditCharAttribType::LineBreak, /*bFeature*/true );
}

EditCharAttrib EditCharAttrib::MakeField( const SvxFieldItem& rAttr, sal_Int32 nPos )
{
    return EditCharAttrib( rAttr, nPos, nPos+1, EditCharAttribType::Field, /*bFeature*/true );
}

void EditCharAttrib::SetFieldValue(const OUString& rVal)
{
    assert(mnType == EditCharAttribType::Field);
    aFieldValue = rVal;
}

void EditCharAttrib::ResetField()
{
    assert(mnType == EditCharAttribType::Field);
    aFieldValue.clear();
    mxTxtColor.reset();
    mxFldColor.reset();
}

EditCharAttrib EditCharAttrib::MakeField( const EditCharAttrib& rAttr )
{
    // Use this constructor only for temporary Objects, Item is not pooled.
    assert(rAttr.mnType == EditCharAttribType::Field);
    EditCharAttrib aNewAttr( *rAttr.GetItem(), rAttr.GetStart(), rAttr.GetEnd(), EditCharAttribType::Field );
    aNewAttr.aFieldValue = rAttr.aFieldValue;
    aNewAttr.mxTxtColor = rAttr.mxTxtColor;
    aNewAttr.mxFldColor = rAttr.mxFldColor;
    return aNewAttr;
}

bool EditCharAttrib::operator == ( const EditCharAttrib& rAttr ) const
{
    if ( mnType != rAttr.mnType )
        return false;

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

    return true;
}

EditCharAttrib EditCharAttrib::MakePairKerning( const SvxAutoKernItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    assert(rAttr.Which() == EE_CHAR_PAIRKERNING);
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::PairKerning );
}

EditCharAttrib EditCharAttrib::MakeKerning( const SvxKerningItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    assert(rAttr.Which() == EE_CHAR_KERNING);
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::Kerning );
}

EditCharAttrib EditCharAttrib::MakeWordLineMode( const SvxWordLineModeItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    assert(rAttr.Which() == EE_CHAR_WLM);
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::WordLineMode );
}

EditCharAttrib EditCharAttrib::MakeEmphasisMark( const SvxEmphasisMarkItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    assert(rAttr.Which() == EE_CHAR_EMPHASISMARK);
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::EmphasisMark );
}

EditCharAttrib EditCharAttrib::MakeRelief( const SvxCharReliefItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    assert(rAttr.Which() == EE_CHAR_RELIEF);
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::Relief );
}

EditCharAttrib EditCharAttrib::MakeGrabBag( const SfxGrabBagItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    assert(rAttr.Which() == EE_CHAR_GRABBAG);
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::GrabBag );
}

EditCharAttrib EditCharAttrib::MakeXml( const SfxPoolItem& rAttr, sal_Int32 _nStart, sal_Int32 _nEnd )
{
    return EditCharAttrib( rAttr, _nStart, _nEnd, EditCharAttribType::Xml );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
