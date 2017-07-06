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

#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/frame/status/FontHeight.hpp>
#include <o3tl/make_unique.hxx>
#include <vcl/bitmapex.hxx>
#include <tools/stream.hxx>
#include <math.h>
#include <rtl/math.hxx>
#include <unotools/fontdefs.hxx>
#include <vcl/outdev.hxx>
#include <vcl/unohelp.hxx>
#include <editeng/eeitem.hxx>
#include <svtools/unitconv.hxx>

#include <svl/memberid.hrc>
#include <editeng/editids.hrc>
#include <editeng/editrids.hrc>
#include <vcl/vclenum.hxx>
#include <tools/tenccvt.hxx>
#include <tools/mapunit.hxx>

#include <rtl/ustring.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <svl/itemset.hxx>

#include <svtools/langtab.hxx>
#include <svl/itempool.hxx>
#include <svtools/ctrltool.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/awt/SimpleFontMetric.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/awt/FontWidth.hpp>
#include <com/sun/star/awt/XFont.hpp>
#include <com/sun/star/awt/FontType.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontPitch.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <editeng/rsiditem.hxx>
#include <editeng/memberids.hrc>
#include <editeng/flstitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fwdtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/prszitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/charsetcoloritem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/nlbkitem.hxx>
#include <editeng/nhypitem.hxx>
#include <editeng/blinkitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/itemtype.hxx>
#include <editeng/eerdll.hxx>
#include <libxml/xmlwriter.h>

#define STORE_UNICODE_MAGIC_MARKER  0xFE331188

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;

bool SvxFontItem::bEnableStoreUnicodeNames = false;

SfxPoolItem* SvxFontItem::CreateDefault() {return new SvxFontItem(0);}
SfxPoolItem* SvxPostureItem::CreateDefault() { return new SvxPostureItem(ITALIC_NONE, 0);}
SfxPoolItem* SvxWeightItem::CreateDefault() {return new SvxWeightItem(WEIGHT_NORMAL, 0);}
SfxPoolItem* SvxFontHeightItem::CreateDefault() {return new SvxFontHeightItem(240, 100, 0);}
SfxPoolItem* SvxTextLineItem::CreateDefault() {return new SvxTextLineItem(LINESTYLE_NONE, 0);}
SfxPoolItem* SvxCrossedOutItem::CreateDefault() {return new SvxCrossedOutItem(STRIKEOUT_NONE, 0);}
SfxPoolItem* SvxShadowedItem::CreateDefault() {return new SvxShadowedItem(false, 0);}
SfxPoolItem* SvxAutoKernItem::CreateDefault() {return new SvxAutoKernItem(false, 0);}
SfxPoolItem* SvxWordLineModeItem::CreateDefault() {return new SvxWordLineModeItem(false, 0);}
SfxPoolItem* SvxContourItem::CreateDefault() {return new SvxContourItem(false, 0);}
SfxPoolItem* SvxColorItem::CreateDefault() {return new SvxColorItem(0);}
SfxPoolItem* SvxBackgroundColorItem::CreateDefault() {return new SvxBackgroundColorItem(0);}
SfxPoolItem* SvxKerningItem::CreateDefault() {return new SvxKerningItem(0, 0);}
SfxPoolItem* SvxCaseMapItem::CreateDefault() {return new SvxCaseMapItem(SvxCaseMap::NotMapped, 0);}
SfxPoolItem* SvxEscapementItem::CreateDefault() {return new SvxEscapementItem(0);}
SfxPoolItem* SvxLanguageItem::CreateDefault() {return new SvxLanguageItem(LANGUAGE_GERMAN, 0);}
SfxPoolItem* SvxBlinkItem::CreateDefault() {return new SvxBlinkItem(false, 0);}
SfxPoolItem* SvxEmphasisMarkItem::CreateDefault() {return new SvxEmphasisMarkItem(FontEmphasisMark::NONE, 0);}
SfxPoolItem* SvxCharRotateItem::CreateDefault() {return new SvxCharRotateItem(0, false, 0);}
SfxPoolItem* SvxCharScaleWidthItem::CreateDefault() {return new SvxCharScaleWidthItem(100, 0);}
SfxPoolItem* SvxCharReliefItem::CreateDefault() {return new SvxCharReliefItem(FontRelief::NONE, 0);}


// class SvxFontListItem -------------------------------------------------

SvxFontListItem::SvxFontListItem( const FontList* pFontLst,
                                  const sal_uInt16 nId ) :
    SfxPoolItem( nId ),
    pFontList( pFontLst )
{
    if ( pFontList )
    {
        sal_Int32 nCount = pFontList->GetFontNameCount();
        aFontNameSeq.realloc( nCount );

        for ( sal_Int32 i = 0; i < nCount; i++ )
            aFontNameSeq[i] = pFontList->GetFontName(i).GetFamilyName();
    }
}


SvxFontListItem::SvxFontListItem( const SvxFontListItem& rItem ) :

    SfxPoolItem( rItem ),
    pFontList( rItem.GetFontList() ),
    aFontNameSeq( rItem.aFontNameSeq )
{
}


SfxPoolItem* SvxFontListItem::Clone( SfxItemPool* ) const
{
    return new SvxFontListItem( *this );
}


bool SvxFontListItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    return( pFontList == static_cast<const SvxFontListItem&>(rAttr).pFontList );
}

bool SvxFontListItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= aFontNameSeq;
    return true;
}


bool SvxFontListItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    rText.clear();
    return false;
}

// class SvxFontItem -----------------------------------------------------

SvxFontItem::SvxFontItem( const sal_uInt16 nId ) :
    SfxPoolItem( nId )
{
    eFamily = FAMILY_SWISS;
    ePitch = PITCH_VARIABLE;
    eTextEncoding = RTL_TEXTENCODING_DONTKNOW;
}


SvxFontItem::SvxFontItem( const FontFamily eFam, const OUString& aName,
                  const OUString& aStName, const FontPitch eFontPitch,
                  const rtl_TextEncoding eFontTextEncoding, const sal_uInt16 nId ) :

    SfxPoolItem( nId ),

    aFamilyName(aName),
    aStyleName(aStName)
{
    eFamily = eFam;
    ePitch = eFontPitch;
    eTextEncoding = eFontTextEncoding;
}


SvxFontItem& SvxFontItem::operator=(const SvxFontItem& rFont)
{
    aFamilyName =  rFont.GetFamilyName();
    aStyleName =   rFont.GetStyleName();
    eFamily =      rFont.GetFamily();
    ePitch =       rFont.GetPitch();
    eTextEncoding = rFont.GetCharSet();
    return *this;
}


bool SvxFontItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case 0:
        {
            css::awt::FontDescriptor aFontDescriptor;
            aFontDescriptor.Name = aFamilyName;
            aFontDescriptor.StyleName = aStyleName;
            aFontDescriptor.Family = (sal_Int16)(eFamily);
            aFontDescriptor.CharSet = (sal_Int16)(eTextEncoding);
            aFontDescriptor.Pitch = (sal_Int16)(ePitch);
            rVal <<= aFontDescriptor;
        }
        break;
        case MID_FONT_FAMILY_NAME:
            rVal <<= aFamilyName;
        break;
        case MID_FONT_STYLE_NAME:
            rVal <<= aStyleName;
        break;
        case MID_FONT_FAMILY    : rVal <<= (sal_Int16)(eFamily);    break;
        case MID_FONT_CHAR_SET  : rVal <<= (sal_Int16)(eTextEncoding);  break;
        case MID_FONT_PITCH     : rVal <<= (sal_Int16)(ePitch); break;
    }
    return true;
}

bool SvxFontItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId)
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case 0:
        {
            css::awt::FontDescriptor aFontDescriptor;
            if ( !( rVal >>= aFontDescriptor ))
                return false;

            aFamilyName = aFontDescriptor.Name;
            aStyleName = aFontDescriptor.StyleName;
            eFamily = (FontFamily)aFontDescriptor.Family;
            eTextEncoding = (rtl_TextEncoding)aFontDescriptor.CharSet;
            ePitch = (FontPitch)aFontDescriptor.Pitch;
        }
        break;
        case MID_FONT_FAMILY_NAME   :
        {
            OUString aStr;
            if(!(rVal >>= aStr))
                return false;
            aFamilyName = aStr;
        }
        break;
        case MID_FONT_STYLE_NAME:
        {
            OUString aStr;
            if(!(rVal >>= aStr))
                return false;
            aStyleName = aStr;
        }
        break;
        case MID_FONT_FAMILY :
        {
            sal_Int16 nFamily = sal_Int16();
            if(!(rVal >>= nFamily))
                return false;
            eFamily = (FontFamily)nFamily;
        }
        break;
        case MID_FONT_CHAR_SET  :
        {
            sal_Int16 nSet = sal_Int16();
            if(!(rVal >>= nSet))
                return false;
            eTextEncoding = (rtl_TextEncoding)nSet;
        }
        break;
        case MID_FONT_PITCH     :
        {
            sal_Int16 nPitch = sal_Int16();
            if(!(rVal >>= nPitch))
                return false;
            ePitch =  (FontPitch)nPitch;
        }
        break;
    }
    return true;
}


bool SvxFontItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    const SvxFontItem& rItem = static_cast<const SvxFontItem&>(rAttr);

    bool bRet = ( eFamily == rItem.eFamily &&
                 aFamilyName == rItem.aFamilyName &&
                 aStyleName == rItem.aStyleName );

    if ( bRet )
    {
        if ( ePitch != rItem.ePitch || eTextEncoding != rItem.eTextEncoding )
        {
            bRet = false;
            SAL_INFO( "editeng.items", "FontItem::operator==(): only pitch or rtl_TextEncoding different ");
        }
    }
    return bRet;
}


SfxPoolItem* SvxFontItem::Clone( SfxItemPool * ) const
{
    return new SvxFontItem( *this );
}


SvStream& SvxFontItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    bool bToBats = IsStarSymbol( GetFamilyName() );

    rStrm.WriteUChar( GetFamily() ).WriteUChar( GetPitch() )
         .WriteUChar( bToBats ? RTL_TEXTENCODING_SYMBOL : GetSOStoreTextEncoding(GetCharSet()) );

    OUString aStoreFamilyName( GetFamilyName() );
    if( bToBats )
        aStoreFamilyName = "StarBats";
    rStrm.WriteUniOrByteString(aStoreFamilyName, rStrm.GetStreamCharSet());
    rStrm.WriteUniOrByteString(GetStyleName(), rStrm.GetStreamCharSet());

    // catch for EditEngine, only set while creating clipboard stream.
    if ( bEnableStoreUnicodeNames )
    {
        rStrm.WriteUInt32( STORE_UNICODE_MAGIC_MARKER );
        rStrm.WriteUniOrByteString( aStoreFamilyName, RTL_TEXTENCODING_UNICODE );
        rStrm.WriteUniOrByteString( GetStyleName(), RTL_TEXTENCODING_UNICODE );
    }

    return rStrm;
}


SfxPoolItem* SvxFontItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 _eFamily, eFontPitch, eFontTextEncoding;
    OUString aName, aStyle;
    rStrm.ReadUChar( _eFamily );
    rStrm.ReadUChar( eFontPitch );
    rStrm.ReadUChar( eFontTextEncoding );

    // UNICODE: rStrm >> aName;
    aName = rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet());

    // UNICODE: rStrm >> aStyle;
    aStyle = rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet());

    // Set the "correct" textencoding
    eFontTextEncoding = (sal_uInt8)GetSOLoadTextEncoding( eFontTextEncoding );

    // at some point, the StarBats changes from  ANSI font to SYMBOL font
    if ( RTL_TEXTENCODING_SYMBOL != eFontTextEncoding && aName == "StarBats" )
        eFontTextEncoding = RTL_TEXTENCODING_SYMBOL;

    // Check if we have stored unicode
    sal_uInt64 const nStreamPos = rStrm.Tell();
    sal_uInt32 nMagic = STORE_UNICODE_MAGIC_MARKER;
    rStrm.ReadUInt32( nMagic );
    if ( nMagic == STORE_UNICODE_MAGIC_MARKER )
    {
        aName = rStrm.ReadUniOrByteString( RTL_TEXTENCODING_UNICODE );
        aStyle = rStrm.ReadUniOrByteString( RTL_TEXTENCODING_UNICODE );
    }
    else
    {
        rStrm.Seek( nStreamPos );
    }


    return new SvxFontItem( (FontFamily)_eFamily, aName, aStyle,
                            (FontPitch)eFontPitch, (rtl_TextEncoding)eFontTextEncoding, Which() );
}


bool SvxFontItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    rText = aFamilyName;
    return true;
}


void SvxFontItem::EnableStoreUnicodeNames( bool bEnable )
{
    bEnableStoreUnicodeNames = bEnable;
}

void SvxFontItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SvxFontItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("familyName"), BAD_CAST(aFamilyName.toUtf8().getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("styleName"), BAD_CAST(aStyleName.toUtf8().getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("family"), BAD_CAST(OString::number(eFamily).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("pitch"), BAD_CAST(OString::number(ePitch).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("textEncoding"), BAD_CAST(OString::number(eTextEncoding).getStr()));
    xmlTextWriterEndElement(pWriter);
}

// class SvxPostureItem --------------------------------------------------

SvxPostureItem::SvxPostureItem( const FontItalic ePosture, const sal_uInt16 nId ) :
    SfxEnumItem( nId, ePosture )
{
}


SfxPoolItem* SvxPostureItem::Clone( SfxItemPool * ) const
{
    return new SvxPostureItem( *this );
}


sal_uInt16 SvxPostureItem::GetValueCount() const
{
    return ITALIC_NORMAL + 1;   // ITALIC_NONE also belongs here
}


SvStream& SvxPostureItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteUChar( GetValue() );
    return rStrm;
}


SfxPoolItem* SvxPostureItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 nPosture;
    rStrm.ReadUChar( nPosture );
    return new SvxPostureItem( (FontItalic)nPosture, Which() );
}


bool SvxPostureItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    rText = GetValueTextByPos( GetValue() );
    return true;
}


OUString SvxPostureItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos <= (sal_uInt16)ITALIC_NORMAL, "enum overflow!" );

    FontItalic eItalic = (FontItalic)nPos;
    sal_uInt16 nId = 0;

    switch ( eItalic )
    {
        case ITALIC_NONE:       nId = RID_SVXITEMS_ITALIC_NONE;     break;
        case ITALIC_OBLIQUE:    nId = RID_SVXITEMS_ITALIC_OBLIQUE;  break;
        case ITALIC_NORMAL:     nId = RID_SVXITEMS_ITALIC_NORMAL;   break;
        default: ;//prevent warning
    }

    return nId ? EditResId::GetString(nId) : OUString();
}

bool SvxPostureItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_ITALIC:
            rVal <<= GetBoolValue();
            break;
        case MID_POSTURE:
            rVal <<= vcl::unohelper::ConvertFontSlant(GetValue());
            break;
    }
    return true;
}

bool SvxPostureItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_ITALIC:
            SetBoolValue(Any2Bool(rVal));
        break;
        case MID_POSTURE:
        {
            awt::FontSlant eSlant;
            if(!(rVal >>= eSlant))
            {
                sal_Int32 nValue = 0;
                if(!(rVal >>= nValue))
                    return false;

                eSlant = (awt::FontSlant)nValue;
            }
            SetValue(vcl::unohelper::ConvertFontSlant(eSlant));
        }
    }
    return true;
}

bool SvxPostureItem::HasBoolValue() const
{
    return true;
}

bool SvxPostureItem::GetBoolValue() const
{
    return ( GetValue() >= ITALIC_OBLIQUE );
}

void SvxPostureItem::SetBoolValue( bool bVal )
{
    SetValue( bVal ? ITALIC_NORMAL : ITALIC_NONE );
}

void SvxPostureItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SvxPostureItem"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("whichId"), "%d", Which());
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("value"), "%d", GetValue());
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("presentation"), BAD_CAST(GetValueTextByPos(GetValue()).toUtf8().getStr()));
    xmlTextWriterEndElement(pWriter);
}

// class SvxWeightItem ---------------------------------------------------

SvxWeightItem::SvxWeightItem( const FontWeight eWght, const sal_uInt16 nId ) :
    SfxEnumItem( nId, eWght )
{
}


bool SvxWeightItem::HasBoolValue() const
{
    return true;
}


bool SvxWeightItem::GetBoolValue() const
{
    return GetValue() >= WEIGHT_BOLD;
}


void SvxWeightItem::SetBoolValue( bool bVal )
{
    SetValue( bVal ? WEIGHT_BOLD : WEIGHT_NORMAL );
}


sal_uInt16 SvxWeightItem::GetValueCount() const
{
    return WEIGHT_BLACK;    // WEIGHT_DONTKNOW does not belong
}


SfxPoolItem* SvxWeightItem::Clone( SfxItemPool * ) const
{
    return new SvxWeightItem( *this );
}


SvStream& SvxWeightItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteUChar( GetValue() );
    return rStrm;
}


SfxPoolItem* SvxWeightItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 nWeight;
    rStrm.ReadUChar( nWeight );
    return new SvxWeightItem( (FontWeight)nWeight, Which() );
}


bool SvxWeightItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    rText = GetValueTextByPos( GetValue() );
    return true;
}


OUString SvxWeightItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos <= (sal_uInt16)WEIGHT_BLACK, "enum overflow!" );
    return EditResId::GetString(RID_SVXITEMS_WEIGHT_BEGIN + nPos);
}

bool SvxWeightItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_BOLD   :
            rVal <<= GetBoolValue();
        break;
        case MID_WEIGHT:
        {
            rVal <<= vcl::unohelper::ConvertFontWeight( GetValue() );
        }
        break;
    }
    return true;
}

bool SvxWeightItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_BOLD   :
            SetBoolValue(Any2Bool(rVal));
        break;
        case MID_WEIGHT:
        {
            double fValue = 0;
            if(!(rVal >>= fValue))
            {
                sal_Int32 nValue = 0;
                if(!(rVal >>= nValue))
                    return false;
                fValue = (float)nValue;
            }
            SetValue( vcl::unohelper::ConvertFontWeight((float)fValue) );
        }
        break;
    }
    return true;
}

void SvxWeightItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SvxWeightItem"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("whichId"), "%d", Which());
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("value"), "%d", GetValue());
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("presentation"), BAD_CAST(GetValueTextByPos(GetValue()).toUtf8().getStr()));
    xmlTextWriterEndElement(pWriter);
}

// class SvxFontHeightItem -----------------------------------------------

SvxFontHeightItem::SvxFontHeightItem( const sal_uLong nSz,
                                      const sal_uInt16 nPrp,
                                      const sal_uInt16 nId ) :
    SfxPoolItem( nId )
{
    SetHeight( nSz,nPrp );  // calculate in percentage
}


SfxPoolItem* SvxFontHeightItem::Clone( SfxItemPool * ) const
{
    return new SvxFontHeightItem( *this );
}


SvStream& SvxFontHeightItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    rStrm.WriteUInt16( GetHeight() );

    if( FONTHEIGHT_UNIT_VERSION <= nItemVersion )
        rStrm.WriteUInt16( GetProp() ).WriteUInt16( (sal_uInt16)GetPropUnit() );
    else
    {
        // When exporting to the old versions the relative information is lost
        // when there is no percentage
        sal_uInt16 _nProp = GetProp();
        if( MapUnit::MapRelative != GetPropUnit() )
            _nProp = 100;
        rStrm.WriteUInt16( _nProp );
    }
    return rStrm;
}


SfxPoolItem* SvxFontHeightItem::Create( SvStream& rStrm,
                                                 sal_uInt16 nVersion ) const
{
    sal_uInt16 nsize, nprop = 0;
    MapUnit nPropUnit = MapUnit::MapRelative;

    rStrm.ReadUInt16( nsize );

    if( FONTHEIGHT_16_VERSION <= nVersion )
        rStrm.ReadUInt16( nprop );
    else
    {
        sal_uInt8 nP;
        rStrm .ReadUChar( nP );
        nprop = (sal_uInt16)nP;
    }

    if( FONTHEIGHT_UNIT_VERSION <= nVersion )
    {
        sal_uInt16 nTmp;
        rStrm.ReadUInt16( nTmp );
        nPropUnit = (MapUnit)nTmp;
    }

    SvxFontHeightItem* pItem = new SvxFontHeightItem( nsize, 100, Which() );
    pItem->SetProp( nprop, nPropUnit );
    return pItem;
}


bool SvxFontHeightItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));
    return GetHeight() == static_cast<const SvxFontHeightItem&>(rItem).GetHeight() &&
            GetProp() == static_cast<const SvxFontHeightItem&>(rItem).GetProp() &&
            GetPropUnit() == static_cast<const SvxFontHeightItem&>(rItem).GetPropUnit();
}

bool SvxFontHeightItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    //  In StarOne is the uno::Any always 1/100mm. Through the MemberId it is
    //  controlled if the value in the Item should be 1/100mm or Twips.

    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case 0:
        {
            css::frame::status::FontHeight aFontHeight;

            // Point (i.e. Twips) is asked for, thus re-calculate if
            // CONVERT_TWIPS is not set.
            if( bConvert )
            {
                aFontHeight.Height = (float)( nHeight / 20.0 );
            }
            else
            {
                double fPoints = convertMm100ToTwip(nHeight) / 20.0;
                float fRoundPoints =
                    static_cast<float>(::rtl::math::round(fPoints, 1));
                aFontHeight.Height = fRoundPoints;
            }

            aFontHeight.Prop = (sal_Int16)(MapUnit::MapRelative == ePropUnit ? nProp : 100);

            float fRet = (float)(short)nProp;
            switch( ePropUnit )
            {
                case MapUnit::MapRelative:
                    fRet = 0.;
                break;
                case MapUnit::Map100thMM:
                    fRet = convertMm100ToTwip(fRet);
                    fRet /= 20.;
                break;
                case MapUnit::MapPoint:

                break;
                case MapUnit::MapTwip:
                    fRet /= 20.;
                break;
                default: ;//prevent warning
            }
            aFontHeight.Diff = fRet;
            rVal <<= aFontHeight;
        }
        break;
        case MID_FONTHEIGHT:
        {
            // Point (i.e. Twips) is asked for, thus re-calculate if
            // CONVERT_TWIPS is not set.
            if( bConvert )
            {
                rVal <<= (float)( nHeight / 20.0 );
            }
            else
            {
                double fPoints = convertMm100ToTwip(nHeight) / 20.0;
                float fRoundPoints =
                    static_cast<float>(::rtl::math::round(fPoints, 1));
                rVal <<= fRoundPoints;
            }
        }
        break;
        case MID_FONTHEIGHT_PROP:
            rVal <<= (sal_Int16)(MapUnit::MapRelative == ePropUnit ? nProp : 100);
        break;
        case MID_FONTHEIGHT_DIFF:
        {
            float fRet = (float)(short)nProp;
            switch( ePropUnit )
            {
                case MapUnit::MapRelative:
                    fRet = 0.;
                break;
                case MapUnit::Map100thMM:
                    fRet = convertMm100ToTwip(fRet);
                    fRet /= 20.;
                break;
                case MapUnit::MapPoint:

                break;
                case MapUnit::MapTwip:
                    fRet /= 20.;
                break;
                default: ;//prevent warning
            }
            rVal <<= fRet;
        }
        break;
    }
    return true;
}

// Try to reconstruct the original height input value from the modified height
// and the prop data; this seems somewhat futile given the various ways how the
// modified height is calculated (with and without conversion between twips and
// 100th mm; with an additional eCoreMetric input in one of the SetHeight
// overloads), and indeed known to occasionally produce nRet values that would
// be negative, so just guard against negative results here and throw the hands
// up in despair:
static sal_uInt32 lcl_GetRealHeight_Impl(sal_uInt32 nHeight, sal_uInt16 nProp, MapUnit eProp, bool bCoreInTwip)
{
    sal_uInt32 nRet = nHeight;
    short nDiff = 0;
    switch( eProp )
    {
        case MapUnit::MapRelative:
            nRet *= 100;
            nRet /= nProp;
        break;
        case MapUnit::MapPoint:
        {
            short nTemp = (short)nProp;
            nDiff = nTemp * 20;
            if(!bCoreInTwip)
                nDiff = (short)convertTwipToMm100((long)nDiff);
        }
        break;
        case MapUnit::Map100thMM:
            //then the core is surely also in 1/100 mm
            nDiff = (short)nProp;
        break;
        case MapUnit::MapTwip:
            // Here surely TWIP
            nDiff = ((short)nProp);
        break;
        default: ;//prevent warning
    }
    nRet = (nDiff < 0 || nRet >= static_cast<unsigned short>(nDiff))
        ? nRet - nDiff : 0;
        //TODO: overflow in case nDiff < 0 and nRet - nDiff > SAL_MAX_UINT32

    return nRet;
}

bool SvxFontHeightItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case 0:
        {
            css::frame::status::FontHeight aFontHeight;
            if ( rVal >>= aFontHeight )
            {
                // Height
                ePropUnit = MapUnit::MapRelative;
                nProp = 100;
                double fPoint = aFontHeight.Height;
                if( fPoint < 0. || fPoint > 10000. )
                    return false;

                nHeight = (long)( fPoint * 20.0 + 0.5 );        // Twips
                if (!bConvert)
                    nHeight = convertTwipToMm100(nHeight);  // Convert, if the item contains 1/100mm

                nProp = aFontHeight.Prop;
            }
            else
                return false;
        }
        break;
        case MID_FONTHEIGHT:
        {
            ePropUnit = MapUnit::MapRelative;
            nProp = 100;
            double fPoint = 0;
            if(!(rVal >>= fPoint))
            {
                sal_Int32 nValue = 0;
                if(!(rVal >>= nValue))
                    return false;
                fPoint = (float)nValue;
            }
            if(fPoint < 0. || fPoint > 10000.)
                    return false;

            nHeight = (long)( fPoint * 20.0 + 0.5 );        // Twips
            if (!bConvert)
                nHeight = convertTwipToMm100(nHeight);  // Convert, if the item contains 1/100mm
        }
        break;
        case MID_FONTHEIGHT_PROP:
        {
            sal_Int16 nNew = sal_Int16();
            if(!(rVal >>= nNew))
                return true;

            nHeight = lcl_GetRealHeight_Impl(nHeight, nProp, ePropUnit, bConvert);

            nHeight *= nNew;
            nHeight /= 100;
            nProp = nNew;
            ePropUnit = MapUnit::MapRelative;
        }
        break;
        case MID_FONTHEIGHT_DIFF:
        {
            nHeight = lcl_GetRealHeight_Impl(nHeight, nProp, ePropUnit, bConvert);
            float fValue = 0;
            if(!(rVal >>= fValue))
            {
                sal_Int32 nValue = 0;
                if(!(rVal >>= nValue))
                    return false;
                fValue = (float)nValue;
            }
            sal_Int16 nCoreDiffValue = (sal_Int16)(fValue * 20.);
            nHeight += bConvert ? nCoreDiffValue : convertTwipToMm100(nCoreDiffValue);
            nProp = (sal_uInt16)((sal_Int16)fValue);
            ePropUnit = MapUnit::MapPoint;
        }
        break;
    }
    return true;
}


bool SvxFontHeightItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             eCoreUnit,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *pIntl
)   const
{
    if( MapUnit::MapRelative != ePropUnit )
    {
        rText = OUString::number( (short)nProp ) +
                " " + EditResId::GetString( GetMetricId( ePropUnit ) );
        if( 0 <= (short)nProp )
            rText = "+" + rText;
    }
    else if( 100 == nProp )
    {
        rText = GetMetricText( (long)nHeight,
                                eCoreUnit, MapUnit::MapPoint, pIntl ) +
                " " + EditResId::GetString(GetMetricId(MapUnit::MapPoint));
    }
    else
        rText = OUString::number( nProp ) + "%";
    return true;
}


sal_uInt16 SvxFontHeightItem::GetVersion(sal_uInt16 nFileVersion) const
{
    return (nFileVersion <= SOFFICE_FILEFORMAT_40)
               ? FONTHEIGHT_16_VERSION
               : FONTHEIGHT_UNIT_VERSION;
}


void SvxFontHeightItem::ScaleMetrics( long nMult, long nDiv )
{
    nHeight = (sal_uInt32)Scale( nHeight, nMult, nDiv );
}


bool SvxFontHeightItem::HasMetrics() const
{
    return true;
}

void SvxFontHeightItem::SetHeight( sal_uInt32 nNewHeight, const sal_uInt16 nNewProp,
                                   MapUnit eUnit )
{
    DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );

    if( MapUnit::MapRelative != eUnit )
        nHeight = nNewHeight + ::ItemToControl( (short)nNewProp, eUnit,
                                                FUNIT_TWIP );
    else if( 100 != nNewProp )
        nHeight = sal_uInt32(( nNewHeight * nNewProp ) / 100 );
    else
        nHeight = nNewHeight;

    nProp = nNewProp;
    ePropUnit = eUnit;
}

void SvxFontHeightItem::SetHeight( sal_uInt32 nNewHeight, sal_uInt16 nNewProp,
                                   MapUnit eMetric, MapUnit eCoreMetric )
{
    DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );

    if( MapUnit::MapRelative != eMetric )
        nHeight = nNewHeight +
                ::ControlToItem( ::ItemToControl((short)nNewProp, eMetric,
                                        FUNIT_TWIP ), FUNIT_TWIP,
                                        eCoreMetric );
    else if( 100 != nNewProp )
        nHeight = sal_uInt32(( nNewHeight * nNewProp ) / 100 );
    else
        nHeight = nNewHeight;

    nProp = nNewProp;
    ePropUnit = eMetric;
}

void SvxFontHeightItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SvxFontHeightItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("height"), BAD_CAST(OString::number(nHeight).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("prop"), BAD_CAST(OString::number(nProp).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("propUnit"), BAD_CAST(OString::number((int)ePropUnit).getStr()));
    xmlTextWriterEndElement(pWriter);
}

// class SvxFontWidthItem -----------------------------------------------

SvxFontWidthItem::SvxFontWidthItem( const sal_uInt16 nSz, const sal_uInt16 nPrp, const sal_uInt16 nId ) :
    SfxPoolItem( nId )
{
    nWidth = nSz;
    nProp = nPrp;
}


SfxPoolItem* SvxFontWidthItem::Clone( SfxItemPool * ) const
{
    return new SvxFontWidthItem( *this );
}


SvStream& SvxFontWidthItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteUInt16( GetWidth() ).WriteUInt16( GetProp() );
    return rStrm;
}


void SvxFontWidthItem::ScaleMetrics( long nMult, long nDiv )
{
    nWidth = (sal_uInt16)Scale( nWidth, nMult, nDiv );
}


bool SvxFontWidthItem::HasMetrics() const
{
    return true;
}


SfxPoolItem* SvxFontWidthItem::Create( SvStream& rStrm,
                                                 sal_uInt16 /*nVersion*/ ) const
{
    sal_uInt16 nS;
    sal_uInt16 nP;

    rStrm.ReadUInt16( nS );
    rStrm.ReadUInt16( nP );
    SvxFontWidthItem* pItem = new SvxFontWidthItem( 0, nP, Which() );
    pItem->SetWidthValue( nS );
    return pItem;
}


bool SvxFontWidthItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));
    return GetWidth() == static_cast<const SvxFontWidthItem&>(rItem).GetWidth() &&
            GetProp() == static_cast<const SvxFontWidthItem&>(rItem).GetProp();
}

bool SvxFontWidthItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
//    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_FONTWIDTH:
            rVal <<= (sal_Int16)(nWidth);
        break;
        case MID_FONTWIDTH_PROP:
            rVal <<= (sal_Int16)(nProp);
        break;
    }
    return true;
}

bool SvxFontWidthItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
//    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    sal_Int16 nVal = sal_Int16();
    if(!(rVal >>= nVal))
        return false;

    switch(nMemberId)
    {
        case MID_FONTWIDTH:
            nProp = nVal;
        break;
        case MID_FONTWIDTH_PROP:
            nWidth = nVal;
        break;
    }
    return true;
}


bool SvxFontWidthItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             eCoreUnit,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *pIntl
)   const
{
    if ( 100 == nProp )
    {
        rText = GetMetricText( (long)nWidth,
                                eCoreUnit, MapUnit::MapPoint, pIntl ) +
                " " + EditResId::GetString(GetMetricId(MapUnit::MapPoint));
    }
    else
        rText = OUString::number( nProp ) + "%";
    return true;
}

// class SvxTextLineItem ------------------------------------------------

SvxTextLineItem::SvxTextLineItem( const FontLineStyle eSt, const sal_uInt16 nId )
    : SfxEnumItem( nId, eSt ), mColor( COL_TRANSPARENT )
{
}


bool SvxTextLineItem::HasBoolValue() const
{
    return true;
}


bool SvxTextLineItem::GetBoolValue() const
{
    return  GetValue() != LINESTYLE_NONE;
}


void SvxTextLineItem::SetBoolValue( bool bVal )
{
    SetValue( bVal ? LINESTYLE_SINGLE : LINESTYLE_NONE );
}


SfxPoolItem* SvxTextLineItem::Clone( SfxItemPool * ) const
{
    SvxTextLineItem* pNew = new SvxTextLineItem( *this );
    pNew->SetColor( GetColor() );
    return pNew;
}


sal_uInt16 SvxTextLineItem::GetValueCount() const
{
    return LINESTYLE_DOTTED + 1;    // LINESTYLE_NONE also belongs here
}


SvStream& SvxTextLineItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteUChar( GetValue() );
    return rStrm;
}


SfxPoolItem* SvxTextLineItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 nState;
    rStrm.ReadUChar( nState );
    return new SvxTextLineItem(  (FontLineStyle)nState, Which() );
}


bool SvxTextLineItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    rText = GetValueTextByPos( GetValue() );
    if( !mColor.GetTransparency() )
        rText = rText + OUString(cpDelim) + ::GetColorString( mColor );
    return true;
}


OUString SvxTextLineItem::GetValueTextByPos( sal_uInt16 /*nPos*/ ) const
{
    OSL_FAIL("SvxTextLineItem::GetValueTextByPos: Pure virtual method");
    return OUString();
}

bool SvxTextLineItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
    case MID_TEXTLINED:
        rVal <<= GetBoolValue();
        break;
    case MID_TL_STYLE:
        rVal <<= (sal_Int16)(GetValue());
        break;
    case MID_TL_COLOR:
        rVal <<= (sal_Int32)( mColor.GetColor() );
        break;
    case MID_TL_HASCOLOR:
        rVal <<= !mColor.GetTransparency();
        break;
    }
    return true;

}

bool SvxTextLineItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch(nMemberId)
    {
    case MID_TEXTLINED:
        SetBoolValue(Any2Bool(rVal));
    break;
    case MID_TL_STYLE:
    {
        sal_Int32 nValue = 0;
        if(!(rVal >>= nValue))
            bRet = false;
        else
            SetValue((FontLineStyle)nValue);
    }
    break;
    case MID_TL_COLOR:
    {
        sal_Int32 nCol = 0;
        if( !( rVal >>= nCol ) )
            bRet = false;
        else
        {
            // Keep transparence, because it contains the information
            // whether the font color or the stored color should be used
            sal_uInt8 nTrans = mColor.GetTransparency();
            mColor = Color( nCol );
            mColor.SetTransparency( nTrans );
        }
    }
    break;
    case MID_TL_HASCOLOR:
        mColor.SetTransparency( Any2Bool( rVal ) ? 0 : 0xff );
    break;
    }
    return bRet;
}

bool SvxTextLineItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));
    return SfxEnumItem::operator==( static_cast<const SfxEnumItem<FontLineStyle>&>(rItem) ) &&
           GetColor() == static_cast<const SvxTextLineItem&>(rItem).GetColor();
}

// class SvxUnderlineItem ------------------------------------------------

SvxUnderlineItem::SvxUnderlineItem( const FontLineStyle eSt, const sal_uInt16 nId )
    : SvxTextLineItem( eSt, nId )
{
}


SfxPoolItem* SvxUnderlineItem::Clone( SfxItemPool * ) const
{
    SvxUnderlineItem* pNew = new SvxUnderlineItem( *this );
    pNew->SetColor( GetColor() );
    return pNew;
}


SfxPoolItem* SvxUnderlineItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 nState;
    rStrm.ReadUChar( nState );
    return new SvxUnderlineItem(  (FontLineStyle)nState, Which() );
}


OUString SvxUnderlineItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos <= (sal_uInt16)LINESTYLE_BOLDWAVE, "enum overflow!" );
    return EditResId::GetString(RID_SVXITEMS_UL_BEGIN + nPos);
}

// class SvxOverlineItem ------------------------------------------------

SvxOverlineItem::SvxOverlineItem( const FontLineStyle eSt, const sal_uInt16 nId )
    : SvxTextLineItem( eSt, nId )
{
}


SfxPoolItem* SvxOverlineItem::Clone( SfxItemPool * ) const
{
    SvxOverlineItem* pNew = new SvxOverlineItem( *this );
    pNew->SetColor( GetColor() );
    return pNew;
}


SfxPoolItem* SvxOverlineItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 nState;
    rStrm.ReadUChar( nState );
    return new SvxOverlineItem(  (FontLineStyle)nState, Which() );
}


OUString SvxOverlineItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos <= (sal_uInt16)LINESTYLE_BOLDWAVE, "enum overflow!" );
    return EditResId::GetString(RID_SVXITEMS_OL_BEGIN + nPos);
}

// class SvxCrossedOutItem -----------------------------------------------

SvxCrossedOutItem::SvxCrossedOutItem( const FontStrikeout eSt, const sal_uInt16 nId )
    : SfxEnumItem( nId, eSt )
{
}


bool SvxCrossedOutItem::HasBoolValue() const
{
    return true;
}


bool SvxCrossedOutItem::GetBoolValue() const
{
    return GetValue() != STRIKEOUT_NONE;
}


void SvxCrossedOutItem::SetBoolValue( bool bVal )
{
    SetValue( bVal ? STRIKEOUT_SINGLE : STRIKEOUT_NONE );
}


sal_uInt16 SvxCrossedOutItem::GetValueCount() const
{
    return STRIKEOUT_DOUBLE + 1;    // STRIKEOUT_NONE belongs also here
}


SfxPoolItem* SvxCrossedOutItem::Clone( SfxItemPool * ) const
{
    return new SvxCrossedOutItem( *this );
}


SvStream& SvxCrossedOutItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteUChar( GetValue() );
    return rStrm;
}


SfxPoolItem* SvxCrossedOutItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 eCross;
    rStrm.ReadUChar( eCross );
    return new SvxCrossedOutItem(  (FontStrikeout)eCross, Which() );
}


bool SvxCrossedOutItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    rText = GetValueTextByPos( GetValue() );
    return true;
}


OUString SvxCrossedOutItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos <= (sal_uInt16)STRIKEOUT_X, "enum overflow!" );
    return EditResId::GetString(RID_SVXITEMS_STRIKEOUT_BEGIN + nPos);
}

bool SvxCrossedOutItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_CROSSED_OUT:
            rVal <<= GetBoolValue();
        break;
        case MID_CROSS_OUT:
            rVal <<= (sal_Int16)(GetValue());
        break;
    }
    return true;
}

bool SvxCrossedOutItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_CROSSED_OUT:
            SetBoolValue(Any2Bool(rVal));
        break;
        case MID_CROSS_OUT:
        {
            sal_Int32 nValue = 0;
            if(!(rVal >>= nValue))
                return false;
            SetValue((FontStrikeout)nValue);
        }
        break;
    }
    return true;
}
// class SvxShadowedItem -------------------------------------------------

SvxShadowedItem::SvxShadowedItem( const bool bShadowed, const sal_uInt16 nId ) :
    SfxBoolItem( nId, bShadowed )
{
}


SfxPoolItem* SvxShadowedItem::Clone( SfxItemPool * ) const
{
    return new SvxShadowedItem( *this );
}


SvStream& SvxShadowedItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteBool( GetValue() );
    return rStrm;
}


SfxPoolItem* SvxShadowedItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 nState;
    rStrm.ReadUChar( nState );
    return new SvxShadowedItem( nState, Which() );
}


bool SvxShadowedItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    sal_uInt16 nId = RID_SVXITEMS_SHADOWED_FALSE;

    if ( GetValue() )
        nId = RID_SVXITEMS_SHADOWED_TRUE;
    rText = EditResId::GetString(nId);
    return true;
}

// class SvxAutoKernItem -------------------------------------------------

SvxAutoKernItem::SvxAutoKernItem( const bool bAutoKern, const sal_uInt16 nId ) :
    SfxBoolItem( nId, bAutoKern )
{
}


SfxPoolItem* SvxAutoKernItem::Clone( SfxItemPool * ) const
{
    return new SvxAutoKernItem( *this );
}


SvStream& SvxAutoKernItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteBool( GetValue() );
    return rStrm;
}


SfxPoolItem* SvxAutoKernItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 nState;
    rStrm.ReadUChar( nState );
    return new SvxAutoKernItem( nState, Which() );
}


bool SvxAutoKernItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    sal_uInt16 nId = RID_SVXITEMS_AUTOKERN_FALSE;

    if ( GetValue() )
        nId = RID_SVXITEMS_AUTOKERN_TRUE;
    rText = EditResId::GetString(nId);
    return true;
}

// class SvxWordLineModeItem ---------------------------------------------

SvxWordLineModeItem::SvxWordLineModeItem( const bool bWordLineMode,
                                          const sal_uInt16 nId ) :
    SfxBoolItem( nId, bWordLineMode )
{
}


SfxPoolItem* SvxWordLineModeItem::Clone( SfxItemPool * ) const
{
    return new SvxWordLineModeItem( *this );
}


SvStream& SvxWordLineModeItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteBool( GetValue() );
    return rStrm;
}


SfxPoolItem* SvxWordLineModeItem::Create(SvStream& rStrm, sal_uInt16) const
{
    bool bValue;
    rStrm.ReadCharAsBool( bValue );
    return new SvxWordLineModeItem( bValue, Which() );
}


bool SvxWordLineModeItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    sal_uInt16 nId = RID_SVXITEMS_WORDLINE_FALSE;

    if ( GetValue() )
        nId = RID_SVXITEMS_WORDLINE_TRUE;
    rText = EditResId::GetString(nId);
    return true;
}

// class SvxContourItem --------------------------------------------------

SvxContourItem::SvxContourItem( const bool bContoured, const sal_uInt16 nId ) :
    SfxBoolItem( nId, bContoured )
{
}


SfxPoolItem* SvxContourItem::Clone( SfxItemPool * ) const
{
    return new SvxContourItem( *this );
}


SvStream& SvxContourItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteBool( GetValue() );
    return rStrm;
}


SfxPoolItem* SvxContourItem::Create(SvStream& rStrm, sal_uInt16) const
{
    bool bValue;
    rStrm.ReadCharAsBool( bValue );
    return new SvxContourItem( bValue, Which() );
}


bool SvxContourItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    sal_uInt16 nId = RID_SVXITEMS_CONTOUR_FALSE;

    if ( GetValue() )
        nId = RID_SVXITEMS_CONTOUR_TRUE;
    rText = EditResId::GetString(nId);
    return true;
}

// class SvxPropSizeItem -------------------------------------------------

SvxPropSizeItem::SvxPropSizeItem( const sal_uInt16 nPercent, const sal_uInt16 nId ) :
    SfxUInt16Item( nId, nPercent )
{
}


SfxPoolItem* SvxPropSizeItem::Clone( SfxItemPool * ) const
{
    return new SvxPropSizeItem( *this );
}


SvStream& SvxPropSizeItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteUInt16( GetValue() );
    return rStrm;
}


SfxPoolItem* SvxPropSizeItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt16 nSize;
    rStrm.ReadUInt16( nSize );
    return new SvxPropSizeItem( nSize, Which() );
}


bool SvxPropSizeItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    rText.clear();
    return false;
}

// class SvxBackgroundColorItem -----------------------------------------

SvxBackgroundColorItem::SvxBackgroundColorItem( const sal_uInt16 nId ) :
    SvxColorItem( nId )
{
}


SvxBackgroundColorItem::SvxBackgroundColorItem( const Color& rCol,
                                                const sal_uInt16 nId ) :
    SvxColorItem( rCol, nId )
{
}

SvxBackgroundColorItem::SvxBackgroundColorItem(SvStream& rStrm, const sal_uInt16 nId)
    : SvxColorItem(nId)
{
    Color aColor;
    aColor.Read(rStrm);
    SetValue(aColor);
}

SvxBackgroundColorItem::SvxBackgroundColorItem( const SvxBackgroundColorItem& rCopy ) :
    SvxColorItem( rCopy )
{
}

SfxPoolItem* SvxBackgroundColorItem::Clone( SfxItemPool * ) const
{
    return new SvxBackgroundColorItem(*this);
}

SvStream& SvxBackgroundColorItem::Store(SvStream& rStrm, sal_uInt16) const
{
    GetValue().Write(rStrm);
    return rStrm;
}

SfxPoolItem* SvxBackgroundColorItem::Create(SvStream& rStrm, sal_uInt16 ) const
{
    return new SvxBackgroundColorItem( rStrm, Which() );
}

bool SvxBackgroundColorItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    Color aColor = SvxColorItem::GetValue();

    switch( nMemberId )
    {
        case MID_GRAPHIC_TRANSPARENT:
        {
            rVal <<= aColor.GetTransparency() == 0xff;
            break;
        }
        default:
        {
            rVal <<= (sal_Int32)(aColor.GetColor());
            break;
        }
    }
    return true;
}

bool SvxBackgroundColorItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    sal_Int32 nColor = 0;
    Color aColor = SvxColorItem::GetValue();

    switch( nMemberId )
    {
        case MID_GRAPHIC_TRANSPARENT:
        {
            aColor.SetTransparency( Any2Bool( rVal ) ? 0xff : 0 );
            SvxColorItem::SetValue( aColor );
            break;
        }
        default:
        {
            if(!(rVal >>= nColor))
                return false;
            SvxColorItem::SetValue( Color(nColor) );
            break;
        }
    }
    return true;
}

// class SvxColorItem ----------------------------------------------------
SvxColorItem::SvxColorItem( const sal_uInt16 nId ) :
    SfxPoolItem( nId ),
    mColor( COL_BLACK )
{
}

SvxColorItem::SvxColorItem( const Color& rCol, const sal_uInt16 nId ) :
    SfxPoolItem( nId ),
    mColor( rCol )
{
}

SvxColorItem::SvxColorItem( SvStream &rStrm, const sal_uInt16 nId ) :
    SfxPoolItem( nId )
{
    Color aColor;
    ReadColor( rStrm, aColor );
    mColor = aColor;
}

SvxColorItem::SvxColorItem( const SvxColorItem &rCopy ) :
    SfxPoolItem( rCopy ),
    mColor( rCopy.mColor )
{
}

SvxColorItem::~SvxColorItem()
{
}

sal_uInt16 SvxColorItem::GetVersion( sal_uInt16 nFFVer ) const
{
    DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
            SOFFICE_FILEFORMAT_40==nFFVer ||
            SOFFICE_FILEFORMAT_50==nFFVer,
            "SvxColorItem: Is there a new file format? ");
    return  SOFFICE_FILEFORMAT_50 >= nFFVer ? VERSION_USEAUTOCOLOR : 0;
}

bool SvxColorItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    return  mColor == static_cast<const SvxColorItem&>( rAttr ).mColor;
}

bool SvxColorItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= (sal_Int32)(mColor.GetColor());
    return true;
}

bool SvxColorItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    sal_Int32 nColor = 0;
    if(!(rVal >>= nColor))
        return false;

    mColor.SetColor( nColor );
    return true;
}

SfxPoolItem* SvxColorItem::Clone( SfxItemPool * ) const
{
    return new SvxColorItem( *this );
}

SvStream& SvxColorItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    if( VERSION_USEAUTOCOLOR == nItemVersion &&
        COL_AUTO == mColor.GetColor() )
        WriteColor( rStrm, Color( COL_BLACK ) );
    else
        WriteColor( rStrm, mColor );
    return rStrm;
}

SfxPoolItem* SvxColorItem::Create(SvStream& rStrm, sal_uInt16 /*nVer*/ ) const
{
    return new SvxColorItem( rStrm, Which() );
}

bool SvxColorItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    rText = ::GetColorString( mColor );
    return true;
}


void SvxColorItem::SetValue( const Color& rNewCol )
{
    mColor = rNewCol;
}

// class SvxCharSetColorItem ---------------------------------------------

SvxCharSetColorItem::SvxCharSetColorItem( const sal_uInt16 nId ) :
    SvxColorItem( nId ),

    eFrom( RTL_TEXTENCODING_DONTKNOW )
{
}


SvxCharSetColorItem::SvxCharSetColorItem( const Color& rCol,
                                          const rtl_TextEncoding _eFrom,
                                          const sal_uInt16 nId ) :
    SvxColorItem( rCol, nId ),

    eFrom( _eFrom )
{
}


SfxPoolItem* SvxCharSetColorItem::Clone( SfxItemPool * ) const
{
    return new SvxCharSetColorItem( *this );
}


SvStream& SvxCharSetColorItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteUChar( GetSOStoreTextEncoding(GetCharSet()) );
    WriteColor( rStrm, GetValue() );
    return rStrm;
}


SfxPoolItem* SvxCharSetColorItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 cSet;
    Color aColor;
    rStrm.ReadUChar( cSet );
    ReadColor( rStrm, aColor );
    return new SvxCharSetColorItem( aColor,  (rtl_TextEncoding)cSet, Which() );
}


bool SvxCharSetColorItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    rText.clear();
    return false;
}

// class SvxKerningItem --------------------------------------------------

SvxKerningItem::SvxKerningItem( const short nKern, const sal_uInt16 nId ) :
    SfxInt16Item( nId, nKern )
{
}


SfxPoolItem* SvxKerningItem::Clone( SfxItemPool * ) const
{
    return new SvxKerningItem( *this );
}


SvStream& SvxKerningItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteInt16( GetValue() );
    return rStrm;
}


void SvxKerningItem::ScaleMetrics( long nMult, long nDiv )
{
    SetValue( (sal_Int16)Scale( GetValue(), nMult, nDiv ) );
}


bool SvxKerningItem::HasMetrics() const
{
    return true;
}


SfxPoolItem* SvxKerningItem::Create(SvStream& rStrm, sal_uInt16) const
{
    short nValue;
    rStrm.ReadInt16( nValue );
    return new SvxKerningItem( nValue, Which() );
}


bool SvxKerningItem::GetPresentation
(
    SfxItemPresentation ePres,
    MapUnit             eCoreUnit,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *pIntl
)   const
{
    switch ( ePres )
    {
        case SfxItemPresentation::Nameless:
            rText = GetMetricText( (long)GetValue(), eCoreUnit, MapUnit::MapPoint, pIntl ) +
                    " " + EditResId::GetString(GetMetricId(MapUnit::MapPoint));
            return true;
        case SfxItemPresentation::Complete:
        {
            rText = EditResId::GetString(RID_SVXITEMS_KERNING_COMPLETE);
            sal_uInt16 nId = 0;

            if ( GetValue() > 0 )
                nId = RID_SVXITEMS_KERNING_EXPANDED;
            else if ( GetValue() < 0 )
                nId = RID_SVXITEMS_KERNING_CONDENSED;

            if ( nId )
                rText += EditResId::GetString(nId);
            rText = rText +
                    GetMetricText( (long)GetValue(), eCoreUnit, MapUnit::MapPoint, pIntl ) +
                    " " + EditResId::GetString(GetMetricId(MapUnit::MapPoint));
            return true;
        }
        default: ; //prevent warning
    }
    return false;
}

bool SvxKerningItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    sal_Int16 nVal = GetValue();
    if(nMemberId & CONVERT_TWIPS)
        nVal = (sal_Int16)convertTwipToMm100(nVal);
    rVal <<= nVal;
    return true;
}

bool SvxKerningItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId)
{
    sal_Int16 nVal = sal_Int16();
    if(!(rVal >>= nVal))
        return false;
    if(nMemberId & CONVERT_TWIPS)
        nVal = (sal_Int16)convertMm100ToTwip(nVal);
    SetValue(nVal);
    return true;
}

// class SvxCaseMapItem --------------------------------------------------

SvxCaseMapItem::SvxCaseMapItem( const SvxCaseMap eMap, const sal_uInt16 nId ) :
    SfxEnumItem( nId, eMap )
{
}


sal_uInt16 SvxCaseMapItem::GetValueCount() const
{
    return (sal_uInt16)SvxCaseMap::End; // SvxCaseMap::SmallCaps + 1
}


SfxPoolItem* SvxCaseMapItem::Clone( SfxItemPool * ) const
{
    return new SvxCaseMapItem( *this );
}


SvStream& SvxCaseMapItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteUChar( (sal_uInt8)GetValue() );
    return rStrm;
}


SfxPoolItem* SvxCaseMapItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 cMap;
    rStrm.ReadUChar( cMap );
    return new SvxCaseMapItem( (SvxCaseMap)cMap, Which() );
}


bool SvxCaseMapItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    rText = GetValueTextByPos( (sal_uInt16)GetValue() );
    return true;
}


OUString SvxCaseMapItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos < (sal_uInt16)SvxCaseMap::End, "enum overflow!" );
    return EditResId::GetString(RID_SVXITEMS_CASEMAP_BEGIN + nPos);
}

bool SvxCaseMapItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    sal_Int16 nRet = style::CaseMap::NONE;
    switch( GetValue() )
    {
        case SvxCaseMap::Uppercase   :      nRet = style::CaseMap::UPPERCASE; break;
        case SvxCaseMap::Lowercase     :      nRet = style::CaseMap::LOWERCASE; break;
        case SvxCaseMap::Capitalize       :      nRet = style::CaseMap::TITLE    ; break;
        case SvxCaseMap::SmallCaps:      nRet = style::CaseMap::SMALLCAPS; break;
        default: break;
    }
    rVal <<= nRet;
    return true;
}

bool SvxCaseMapItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    sal_uInt16 nVal = sal_uInt16();
    if(!(rVal >>= nVal))
        return false;

    SvxCaseMap eVal;
    switch( nVal )
    {
    case style::CaseMap::NONE    :   eVal = SvxCaseMap::NotMapped; break;
    case style::CaseMap::UPPERCASE:  eVal = SvxCaseMap::Uppercase;  break;
    case style::CaseMap::LOWERCASE:  eVal = SvxCaseMap::Lowercase; break;
    case style::CaseMap::TITLE    :  eVal = SvxCaseMap::Capitalize; break;
    case style::CaseMap::SMALLCAPS:  eVal = SvxCaseMap::SmallCaps; break;
    default: return false;
    }
    SetValue(eVal);
    return true;
}

// class SvxEscapementItem -----------------------------------------------

SvxEscapementItem::SvxEscapementItem( const sal_uInt16 nId ) :
    SfxEnumItemInterface( nId ),

    nEsc    ( 0 ),
    nProp   ( 100 )
{
}


SvxEscapementItem::SvxEscapementItem( const SvxEscapement eEscape,
                                      const sal_uInt16 nId ) :
    SfxEnumItemInterface( nId ),
    nProp( 100 )
{
    SetEscapement( eEscape );
    if( nEsc )
        nProp = 58;
}


SvxEscapementItem::SvxEscapementItem( const short _nEsc,
                                      const sal_uInt8 _nProp,
                                      const sal_uInt16 nId ) :
    SfxEnumItemInterface( nId ),
    nEsc    ( _nEsc ),
    nProp   ( _nProp )
{
}


bool SvxEscapementItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    return( nEsc  == static_cast<const SvxEscapementItem&>(rAttr).nEsc &&
            nProp == static_cast<const SvxEscapementItem&>(rAttr).nProp );
}


SfxPoolItem* SvxEscapementItem::Clone( SfxItemPool * ) const
{
    return new SvxEscapementItem( *this );
}


SvStream& SvxEscapementItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    short _nEsc = GetEsc();
    if( SOFFICE_FILEFORMAT_31 == rStrm.GetVersion() )
    {
        if( DFLT_ESC_AUTO_SUPER == _nEsc )
            _nEsc = DFLT_ESC_SUPER;
        else if( DFLT_ESC_AUTO_SUB == _nEsc )
            _nEsc = DFLT_ESC_SUB;
    }
    rStrm.WriteUChar( GetProportionalHeight() )
         .WriteInt16( _nEsc );
    return rStrm;
}


SfxPoolItem* SvxEscapementItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 _nProp;
    short _nEsc;
    rStrm.ReadUChar( _nProp ).ReadInt16( _nEsc );
    return new SvxEscapementItem( _nEsc, _nProp, Which() );
}


sal_uInt16 SvxEscapementItem::GetValueCount() const
{
    return (sal_uInt16)SvxEscapement::End;  // SvxEscapement::Subscript + 1
}


bool SvxEscapementItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    rText = GetValueTextByPos( GetEnumValue() );

    if ( nEsc != 0 )
    {
        if( DFLT_ESC_AUTO_SUPER == nEsc || DFLT_ESC_AUTO_SUB == nEsc )
            rText += EditResId::GetString(RID_SVXITEMS_ESCAPEMENT_AUTO);
        else
            rText = rText + OUString::number( nEsc ) + "%";
    }
    return true;
}


OUString SvxEscapementItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos < (sal_uInt16)SvxEscapement::End, "enum overflow!" );
    return EditResId::GetString(RID_SVXITEMS_ESCAPEMENT_BEGIN + nPos);
}


sal_uInt16 SvxEscapementItem::GetEnumValue() const
{
    if ( nEsc < 0 )
        return (sal_uInt16)SvxEscapement::Subscript;
    else if ( nEsc > 0 )
        return (sal_uInt16)SvxEscapement::Superscript;
    return (sal_uInt16)SvxEscapement::Off;
}


void SvxEscapementItem::SetEnumValue( sal_uInt16 nVal )
{
    SetEscapement( (SvxEscapement)nVal );
}

bool SvxEscapementItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_ESC:
            rVal <<= (sal_Int16)(nEsc);
        break;
        case MID_ESC_HEIGHT:
            rVal <<= (sal_Int8)(nProp);
        break;
        case MID_AUTO_ESC:
            rVal <<= (DFLT_ESC_AUTO_SUB == nEsc || DFLT_ESC_AUTO_SUPER == nEsc);
        break;
    }
    return true;
}

bool SvxEscapementItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_ESC:
        {
            sal_Int16 nVal = sal_Int16();
            if( (rVal >>= nVal) && (std::abs(nVal) <= 101))
                nEsc = nVal;
            else
                return false;
        }
        break;
        case MID_ESC_HEIGHT:
        {
            sal_Int8 nVal = sal_Int8();
            if( (rVal >>= nVal) && (nVal <= 100))
                nProp = nVal;
            else
                return false;
        }
        break;
        case MID_AUTO_ESC:
        {
            bool bVal = Any2Bool(rVal);
            if(bVal)
            {
                if(nEsc < 0)
                    nEsc = DFLT_ESC_AUTO_SUB;
                else
                    nEsc = DFLT_ESC_AUTO_SUPER;
            }
            else
                if(DFLT_ESC_AUTO_SUPER == nEsc )
                    --nEsc;
                else if(DFLT_ESC_AUTO_SUB == nEsc)
                    ++nEsc;
        }
        break;
    }
    return true;
}

// class SvxLanguageItem -------------------------------------------------

SvxLanguageItem::SvxLanguageItem( const LanguageType eLang, const sal_uInt16 nId )
    : SvxLanguageItem_Base( nId , eLang )
{
}


sal_uInt16 SvxLanguageItem::GetValueCount() const
{
    // #i50205# got rid of class International
    SAL_WARN( "editeng.items", "SvxLanguageItem::GetValueCount: supposed to return a count of what?");
    // Could be SvtLanguageTable::GetEntryCount() (all locales with resource string)?
    // Could be LocaleDataWrapper::getInstalledLanguageTypes() (all locales with locale data)?
    return 0;
}


SfxPoolItem* SvxLanguageItem::Clone( SfxItemPool * ) const
{
    return new SvxLanguageItem( *this );
}


SvStream& SvxLanguageItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteUInt16( (sal_uInt16)GetValue() );
    return rStrm;
}


SfxPoolItem* SvxLanguageItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt16 nValue;
    rStrm.ReadUInt16( nValue );
    return new SvxLanguageItem( LanguageType(nValue), Which() );
}


bool SvxLanguageItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    rText = SvtLanguageTable::GetLanguageString( GetValue() );
    return true;
}

bool SvxLanguageItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_LANG_INT:  // for basic conversions!
            rVal <<= (sal_Int16)(sal_uInt16)GetValue();
        break;
        case MID_LANG_LOCALE:
            lang::Locale aRet( LanguageTag::convertToLocale( GetValue(), false));
            rVal <<= aRet;
        break;
    }
    return true;
}

bool SvxLanguageItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_LANG_INT:  // for basic conversions!
        {
            sal_Int32 nValue = 0;
            if(!(rVal >>= nValue))
                return false;

            SetValue(LanguageType(nValue));
        }
        break;
        case MID_LANG_LOCALE:
        {
            lang::Locale aLocale;
            if(!(rVal >>= aLocale))
                return false;

            SetValue( LanguageTag::convertToLanguageType( aLocale, false));
        }
        break;
    }
    return true;
}

// class SvxNoLinebreakItem ----------------------------------------------
SvxNoLinebreakItem::SvxNoLinebreakItem( const bool bBreak, const sal_uInt16 nId ) :
      SfxBoolItem( nId, bBreak )
{
}


SfxPoolItem* SvxNoLinebreakItem::Clone( SfxItemPool* ) const
{
    return new SvxNoLinebreakItem( *this );
}


SvStream& SvxNoLinebreakItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteBool( GetValue() );
    return rStrm;
}


SfxPoolItem* SvxNoLinebreakItem::Create(SvStream& rStrm, sal_uInt16) const
{
    bool bValue;
    rStrm.ReadCharAsBool( bValue );
    return new SvxNoLinebreakItem( bValue, Which() );
}


bool SvxNoLinebreakItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    rText.clear();
    return false;
}

// class SvxNoHyphenItem -------------------------------------------------

SvxNoHyphenItem::SvxNoHyphenItem( const bool bHyphen, const sal_uInt16 nId ) :
    SfxBoolItem( nId , bHyphen )
{
}


SfxPoolItem* SvxNoHyphenItem::Clone( SfxItemPool* ) const
{
    return new SvxNoHyphenItem( *this );
}


SvStream& SvxNoHyphenItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteBool( GetValue() );
    return rStrm;
}


SfxPoolItem* SvxNoHyphenItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    bool bValue;
    rStrm.ReadCharAsBool( bValue );
    return new SvxNoHyphenItem( bValue, Which() );
}


bool SvxNoHyphenItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    rText.clear();
    return false;
}

/*
 * Dummy item for ToolBox controls:
 *
 */


// class SvxBlinkItem -------------------------------------------------


SvxBlinkItem::SvxBlinkItem( const bool bBlink, const sal_uInt16 nId ) :
    SfxBoolItem( nId, bBlink )
{
}


SfxPoolItem* SvxBlinkItem::Clone( SfxItemPool * ) const
{
    return new SvxBlinkItem( *this );
}


SvStream& SvxBlinkItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteBool( GetValue() );
    return rStrm;
}


SfxPoolItem* SvxBlinkItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 nState;
    rStrm.ReadUChar( nState );
    return new SvxBlinkItem( nState, Which() );
}


bool SvxBlinkItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    sal_uInt16 nId = RID_SVXITEMS_BLINK_FALSE;

    if ( GetValue() )
        nId = RID_SVXITEMS_BLINK_TRUE;
    rText = EditResId::GetString(nId);
    return true;
}

// class SvxEmphaisMarkItem ---------------------------------------------------

SvxEmphasisMarkItem::SvxEmphasisMarkItem( const FontEmphasisMark nValue,
                                        const sal_uInt16 nId )
    : SfxUInt16Item( nId, (sal_uInt16)nValue )
{
}


SfxPoolItem* SvxEmphasisMarkItem::Clone( SfxItemPool * ) const
{
    return new SvxEmphasisMarkItem( *this );
}


SvStream& SvxEmphasisMarkItem::Store( SvStream& rStrm,
                                     sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteUInt16( GetValue() );
    return rStrm;
}


SfxPoolItem* SvxEmphasisMarkItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_uInt16 nValue;
    rStrm.ReadUInt16( nValue );
    return new SvxEmphasisMarkItem( (FontEmphasisMark)nValue, Which() );
}


bool SvxEmphasisMarkItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper * /*pIntl*/
)   const
{
    FontEmphasisMark nVal = GetEmphasisMark();
    rText = EditResId::GetString( RID_SVXITEMS_EMPHASIS_BEGIN_STYLE +
                            (sal_uInt16)(FontEmphasisMark)( nVal & FontEmphasisMark::Style ));
    sal_uInt16 nId = ( FontEmphasisMark::PosAbove & nVal )
                    ? RID_SVXITEMS_EMPHASIS_ABOVE_POS
                    : ( FontEmphasisMark::PosBelow & nVal )
                        ? RID_SVXITEMS_EMPHASIS_BELOW_POS
                        : 0;
    if( nId )
        rText += EditResId::GetString( nId );
    return true;
}


bool SvxEmphasisMarkItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
    case MID_EMPHASIS:
    {
        FontEmphasisMark nValue = GetEmphasisMark();
        sal_Int16 nRet = 0;
        switch(nValue & FontEmphasisMark::Style)
        {
            case FontEmphasisMark::NONE   : nRet = FontEmphasis::NONE;           break;
            case FontEmphasisMark::Dot    : nRet = FontEmphasis::DOT_ABOVE;      break;
            case FontEmphasisMark::Circle : nRet = FontEmphasis::CIRCLE_ABOVE;   break;
            case FontEmphasisMark::Disc   : nRet = FontEmphasis::DISK_ABOVE;     break;
            case FontEmphasisMark::Accent : nRet = FontEmphasis::ACCENT_ABOVE;   break;
            default: break;
        }
        if(nRet && nValue & FontEmphasisMark::PosBelow)
            nRet += 10;
        rVal <<= nRet;
    }
    break;
    }
    return true;
}

bool SvxEmphasisMarkItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
    case MID_EMPHASIS:
    {
        sal_Int32 nValue = -1;
        rVal >>= nValue;
        FontEmphasisMark nMark;
        switch(nValue)
        {
            case FontEmphasis::NONE        : nMark = FontEmphasisMark::NONE;   break;
            case FontEmphasis::DOT_ABOVE   : nMark = FontEmphasisMark::Dot|FontEmphasisMark::PosAbove;    break;
            case FontEmphasis::CIRCLE_ABOVE: nMark = FontEmphasisMark::Circle|FontEmphasisMark::PosAbove; break;
            case FontEmphasis::DISK_ABOVE  : nMark = FontEmphasisMark::Disc|FontEmphasisMark::PosAbove;   break;
            case FontEmphasis::ACCENT_ABOVE: nMark = FontEmphasisMark::Accent|FontEmphasisMark::PosAbove; break;
            case FontEmphasis::DOT_BELOW   : nMark = FontEmphasisMark::Dot|FontEmphasisMark::PosBelow;    break;
            case FontEmphasis::CIRCLE_BELOW: nMark = FontEmphasisMark::Circle|FontEmphasisMark::PosBelow; break;
            case FontEmphasis::DISK_BELOW  : nMark = FontEmphasisMark::Disc|FontEmphasisMark::PosBelow;   break;
            case FontEmphasis::ACCENT_BELOW: nMark = FontEmphasisMark::Accent|FontEmphasisMark::PosBelow; break;
            default: return false;
        }
        SetValue( (sal_Int16)nMark );
    }
    break;
    }
    return true;
}

sal_uInt16 SvxEmphasisMarkItem::GetVersion( sal_uInt16 nFFVer ) const
{
    DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
            SOFFICE_FILEFORMAT_40==nFFVer ||
            SOFFICE_FILEFORMAT_50==nFFVer,
            "SvxEmphasisMarkItem: Is there a new file format? ");

    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}


/*************************************************************************
|*    class SvxTwoLinesItem
*************************************************************************/

SvxTwoLinesItem::SvxTwoLinesItem( bool bFlag, sal_Unicode nStartBracket,
                                    sal_Unicode nEndBracket, sal_uInt16 nW )
    : SfxPoolItem( nW ),
    cStartBracket( nStartBracket ), cEndBracket( nEndBracket ), bOn( bFlag )
{
}

SvxTwoLinesItem::SvxTwoLinesItem( const SvxTwoLinesItem& rAttr )
    : SfxPoolItem( rAttr.Which() ),
    cStartBracket( rAttr.cStartBracket ),
    cEndBracket( rAttr.cEndBracket ),
    bOn( rAttr.bOn )
{
}

SvxTwoLinesItem::~SvxTwoLinesItem()
{
}

bool SvxTwoLinesItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return bOn == static_cast<const SvxTwoLinesItem&>(rAttr).bOn &&
           cStartBracket == static_cast<const SvxTwoLinesItem&>(rAttr).cStartBracket &&
           cEndBracket == static_cast<const SvxTwoLinesItem&>(rAttr).cEndBracket;
}

SfxPoolItem* SvxTwoLinesItem::Clone( SfxItemPool* ) const
{
    return new SvxTwoLinesItem( *this );
}

bool SvxTwoLinesItem::QueryValue( css::uno::Any& rVal,
                                sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch( nMemberId )
    {
    case MID_TWOLINES:
        rVal <<= (bool) bOn;
        break;
    case MID_START_BRACKET:
        {
            OUString s;
            if( cStartBracket )
                s = OUString( cStartBracket );
            rVal <<= s;
        }
        break;
    case MID_END_BRACKET:
        {
            OUString s;
            if( cEndBracket )
                s = OUString( cEndBracket );
            rVal <<= s;
        }
        break;
    default:
        bRet = false;
        break;
    }
    return bRet;
}

bool SvxTwoLinesItem::PutValue( const css::uno::Any& rVal,
                                    sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = false;
    OUString s;
    switch( nMemberId )
    {
    case MID_TWOLINES:
        bOn = Any2Bool( rVal );
        bRet = true;
        break;
    case MID_START_BRACKET:
        if( rVal >>= s )
        {
            cStartBracket = s.isEmpty() ? 0 : s[ 0 ];
            bRet = true;
        }
        break;
    case MID_END_BRACKET:
        if( rVal >>= s )
        {
            cEndBracket = s.isEmpty() ? 0 : s[ 0 ];
            bRet = true;
        }
        break;
    }
    return bRet;
}

bool SvxTwoLinesItem::GetPresentation( SfxItemPresentation /*ePres*/,
                            MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
                            OUString &rText, const IntlWrapper* /*pIntl*/ ) const
{
    if( !GetValue() )
        rText = EditResId::GetString( RID_SVXITEMS_TWOLINES_OFF );
    else
    {
        rText = EditResId::GetString( RID_SVXITEMS_TWOLINES );
        if( GetStartBracket() )
            rText = OUStringLiteral1(GetStartBracket()) + rText;
        if( GetEndBracket() )
            rText += OUStringLiteral1(GetEndBracket());
    }
    return true;
}


SfxPoolItem* SvxTwoLinesItem::Create( SvStream & rStrm, sal_uInt16 /*nVer*/) const
{
    bool _bOn;
    sal_Unicode cStart, cEnd;
    rStrm.ReadCharAsBool( _bOn ).ReadUtf16( cStart ).ReadUtf16( cEnd );
    return new SvxTwoLinesItem( _bOn, cStart, cEnd, Which() );
}

SvStream& SvxTwoLinesItem::Store(SvStream & rStrm, sal_uInt16 /*nIVer*/) const
{
    rStrm.WriteBool( GetValue() ).WriteUInt16( GetStartBracket() ).WriteUInt16( GetEndBracket() );
    return rStrm;
}

sal_uInt16 SvxTwoLinesItem::GetVersion( sal_uInt16 nFFVer ) const
{
    DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
            SOFFICE_FILEFORMAT_40==nFFVer ||
            SOFFICE_FILEFORMAT_50==nFFVer,
            "SvxTwoLinesItem: Is there a new file format?" );

    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}


/*************************************************************************
|*    class SvxTextRotateItem
*************************************************************************/

SvxTextRotateItem::SvxTextRotateItem(sal_uInt16 nValue, const sal_uInt16 nW)
    : SfxUInt16Item(nW, nValue)
{
}

SfxPoolItem* SvxTextRotateItem::Clone(SfxItemPool*) const
{
    return new SvxTextRotateItem(GetValue(), Which());
}

SfxPoolItem* SvxTextRotateItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt16 nVal;
    rStrm.ReadUInt16(nVal);
    return new SvxTextRotateItem(nVal, Which());
}

SvStream& SvxTextRotateItem::Store(SvStream & rStrm, sal_uInt16) const
{
    rStrm.WriteUInt16(GetValue());
    return rStrm;
}

sal_uInt16 SvxTextRotateItem::GetVersion(sal_uInt16 nFFVer) const
{
    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}

bool SvxTextRotateItem::GetPresentation(
    SfxItemPresentation /*ePres*/,
    MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
    OUString &rText, const IntlWrapper*) const
{
    if (!GetValue())
        rText = EditResId::GetString(RID_SVXITEMS_TEXTROTATE_OFF);
    else
    {
        rText = EditResId::GetString(RID_SVXITEMS_TEXTROTATE);
        rText = rText.replaceFirst("$(ARG1)",
            OUString::number(GetValue() / 10));
    }
    return true;
}

bool SvxTextRotateItem::QueryValue(css::uno::Any& rVal,
    sal_uInt8 nMemberId) const
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch (nMemberId)
    {
    case MID_ROTATE:
        rVal <<= (sal_Int16)GetValue();
        break;
    default:
        bRet = false;
        break;
    }
    return bRet;
}

bool SvxTextRotateItem::PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId)
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch (nMemberId)
    {
    case MID_ROTATE:
    {
        sal_Int16 nVal = 0;
        if ((rVal >>= nVal) && (0 == nVal || 900 == nVal || 2700 == nVal))
            SetValue((sal_uInt16)nVal);
        else
            bRet = false;
        break;
    }
    default:
        bRet = false;
    }
    return bRet;
}

bool SvxTextRotateItem::operator==(const SfxPoolItem& rItem) const
{
    assert(SfxPoolItem::operator==(rItem));
    return SfxUInt16Item::operator==(rItem);
}

void SvxTextRotateItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SvxTextRotateItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::number(GetValue()).getStr()));
    xmlTextWriterEndElement(pWriter);
}


/*************************************************************************
|*    class SvxCharRotateItem
*************************************************************************/

SvxCharRotateItem::SvxCharRotateItem( sal_uInt16 nValue,
                                       bool bFitIntoLine,
                                       const sal_uInt16 nW )
    : SvxTextRotateItem(nValue, nW), bFitToLine( bFitIntoLine )
{
}

SfxPoolItem* SvxCharRotateItem::Clone( SfxItemPool* ) const
{
    return new SvxCharRotateItem( GetValue(), IsFitToLine(), Which() );
}

SfxPoolItem* SvxCharRotateItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_uInt16 nVal;
    bool b;
    rStrm.ReadUInt16( nVal ).ReadCharAsBool( b );
    return new SvxCharRotateItem( nVal, b, Which() );
}

SvStream& SvxCharRotateItem::Store( SvStream & rStrm, sal_uInt16 ) const
{
    bool bFlag = IsFitToLine();
    rStrm.WriteUInt16( GetValue() ).WriteBool( bFlag );
    return rStrm;
}

sal_uInt16 SvxCharRotateItem::GetVersion( sal_uInt16 nFFVer ) const
{
    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}

bool SvxCharRotateItem::GetPresentation(
        SfxItemPresentation /*ePres*/,
        MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
        OUString &rText, const IntlWrapper*  ) const
{
    if( !GetValue() )
        rText = EditResId::GetString( RID_SVXITEMS_CHARROTATE_OFF );
    else
    {
        rText = EditResId::GetString( RID_SVXITEMS_CHARROTATE );
        rText = rText.replaceFirst( "$(ARG1)",
                    OUString::number( GetValue() / 10 ));
        if( IsFitToLine() )
            rText += EditResId::GetString( RID_SVXITEMS_CHARROTATE_FITLINE );
    }
    return true;
}

bool SvxCharRotateItem::QueryValue( css::uno::Any& rVal,
                                sal_uInt8 nMemberId ) const
{
    bool bRet = true;
    switch(nMemberId & ~CONVERT_TWIPS)
    {
    case MID_ROTATE:
        SvxTextRotateItem::QueryValue(rVal, nMemberId);
        break;
    case MID_FITTOLINE:
        rVal <<= IsFitToLine();
        break;
    default:
        bRet = false;
        break;
    }
    return bRet;
}

bool SvxCharRotateItem::PutValue( const css::uno::Any& rVal,
                                    sal_uInt8 nMemberId )
{
    bool bRet = true;
    switch(nMemberId & ~CONVERT_TWIPS)
    {
    case MID_ROTATE:
        {
            bRet = SvxTextRotateItem::PutValue(rVal, nMemberId);
            break;
        }

    case MID_FITTOLINE:
        SetFitToLine( Any2Bool( rVal ) );
        break;
    default:
        bRet = false;
    }
    return bRet;
}

bool SvxCharRotateItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));
    return SvxTextRotateItem::operator==( rItem ) &&
           IsFitToLine() == static_cast<const SvxCharRotateItem&>(rItem).IsFitToLine();
}

void SvxCharRotateItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SvxCharRotateItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::number(GetValue()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("fitToLine"), BAD_CAST(OString::boolean(IsFitToLine()).getStr()));
    xmlTextWriterEndElement(pWriter);
}

/*************************************************************************
|*    class SvxCharScaleItem
*************************************************************************/

SvxCharScaleWidthItem::SvxCharScaleWidthItem( sal_uInt16 nValue,
                                               const sal_uInt16 nW )
    : SfxUInt16Item( nW, nValue )
{
}

SfxPoolItem* SvxCharScaleWidthItem::Clone( SfxItemPool* ) const
{
    return new SvxCharScaleWidthItem( GetValue(), Which() );
}

SfxPoolItem* SvxCharScaleWidthItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_uInt16 nVal;
    rStrm.ReadUInt16( nVal );
    SvxCharScaleWidthItem* pItem = new SvxCharScaleWidthItem( nVal, Which() );

    if ( Which() == EE_CHAR_FONTWIDTH )
    {
        // Was a SvxFontWidthItem in 5.2
        // sal_uInt16 nFixWidth, sal_uInt16 nPropWidth.
        // nFixWidth has never been used...
        rStrm.ReadUInt16( nVal );
        sal_uInt16 nTest;
        rStrm.ReadUInt16( nTest );
        if ( nTest == 0x1234 )
            pItem->SetValue( nVal );
        else
            rStrm.SeekRel( -2*(long)sizeof(sal_uInt16) );
    }

    return pItem;
}

SvStream& SvxCharScaleWidthItem::Store( SvStream& rStream, sal_uInt16 nVer ) const
{
    SvStream& rRet = SfxUInt16Item::Store( rStream, nVer );
    if ( Which() == EE_CHAR_FONTWIDTH )
    {
        // see comment in Create()....
        rRet.SeekRel( -1*(long)sizeof(sal_uInt16) );
        rRet.WriteUInt16( 0 );
        rRet.WriteUInt16( GetValue() );
        // Really ugly, but not a problem for reading the doc in 5.2
        rRet.WriteUInt16( 0x1234 );
    }
    return rRet;
}


sal_uInt16 SvxCharScaleWidthItem::GetVersion( sal_uInt16 nFFVer ) const
{
    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}

bool SvxCharScaleWidthItem::GetPresentation(
        SfxItemPresentation /*ePres*/,
        MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
        OUString &rText, const IntlWrapper*  ) const
{
    if( !GetValue() )
        rText = EditResId::GetString( RID_SVXITEMS_CHARSCALE_OFF );
    else
    {
        rText = EditResId::GetString( RID_SVXITEMS_CHARSCALE );
        rText = rText.replaceFirst( "$(ARG1)",
                    OUString::number( GetValue() ));
    }
    return true;
}

bool SvxCharScaleWidthItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    // SfxUInt16Item::QueryValue returns sal_Int32 in Any now... (srx642w)
    // where we still want this to be a sal_Int16
    sal_Int16 nValue = sal_Int16();
    if (rVal >>= nValue)
    {
        SetValue( (sal_uInt16) nValue );
        return true;
    }

    SAL_WARN("editeng.items", "SvxCharScaleWidthItem::PutValue - Wrong type!" );
    return false;
}

bool SvxCharScaleWidthItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    // SfxUInt16Item::QueryValue returns sal_Int32 in Any now... (srx642w)
    // where we still want this to be a sal_Int16
    rVal <<= (sal_Int16)GetValue();
    return true;
}

/*************************************************************************
|*    class SvxCharReliefItem
*************************************************************************/

SvxCharReliefItem::SvxCharReliefItem( FontRelief eValue,
                                         const sal_uInt16 nId )
    : SfxEnumItem( nId, eValue )
{
}

SfxPoolItem* SvxCharReliefItem::Clone( SfxItemPool * ) const
{
    return new SvxCharReliefItem( *this );
}

SfxPoolItem* SvxCharReliefItem::Create(SvStream & rStrm, sal_uInt16) const
{
    sal_uInt16 nVal;
    rStrm.ReadUInt16( nVal );
    return new SvxCharReliefItem( (FontRelief)nVal, Which() );
}

SvStream& SvxCharReliefItem::Store(SvStream & rStrm, sal_uInt16 /*nIVer*/) const
{
    FontRelief nVal = GetValue();
    rStrm.WriteUInt16( (sal_uInt16)nVal );
    return rStrm;
}

sal_uInt16 SvxCharReliefItem::GetVersion( sal_uInt16 nFFVer ) const
{
    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}

OUString SvxCharReliefItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos < RID_SVXITEMS_RELIEF_ENGRAVED - RID_SVXITEMS_RELIEF_NONE,
                    "enum overflow" );
    return EditResId::GetString(RID_SVXITEMS_RELIEF_BEGIN + nPos);
}

sal_uInt16 SvxCharReliefItem::GetValueCount() const
{
    return RID_SVXITEMS_RELIEF_ENGRAVED - RID_SVXITEMS_RELIEF_NONE;
}

bool SvxCharReliefItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    rText = GetValueTextByPos( (sal_uInt16)GetValue() );
    return true;
}

bool SvxCharReliefItem::PutValue( const css::uno::Any& rVal,
                                        sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch( nMemberId )
    {
    case MID_RELIEF:
        {
            sal_Int16 nVal = -1;
            rVal >>= nVal;
            if(nVal >= 0 && nVal <= (sal_Int16)FontRelief::Engraved)
                SetValue( (FontRelief)nVal );
            else
                bRet = false;
        }
        break;
    default:
        bRet = false;
        break;
    }
    return bRet;
}

bool SvxCharReliefItem::QueryValue( css::uno::Any& rVal,
                                        sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch( nMemberId )
    {
    case MID_RELIEF:
        rVal <<= (sal_Int16)GetValue();
        break;
    default:
        bRet = false;
        break;
    }
    return bRet;
}

/*************************************************************************
|*    class SvxScriptTypeItemItem
*************************************************************************/

SvxScriptTypeItem::SvxScriptTypeItem( SvtScriptType nType )
    : SfxUInt16Item( SID_ATTR_CHAR_SCRIPTTYPE, static_cast<sal_uInt16>(nType) )
{
}
SfxPoolItem* SvxScriptTypeItem::Clone( SfxItemPool * ) const
{
    return new SvxScriptTypeItem( static_cast<SvtScriptType>(GetValue()) );
}

/*************************************************************************
|*    class SvxScriptSetItem
*************************************************************************/

SvxScriptSetItem::SvxScriptSetItem( sal_uInt16 nSlotId, SfxItemPool& rPool )
    : SfxSetItem( nSlotId, o3tl::make_unique<SfxItemSet>( rPool,
                        svl::Items<SID_ATTR_CHAR_FONT, SID_ATTR_CHAR_FONT>{} ))
{
    sal_uInt16 nLatin, nAsian, nComplex;
    GetWhichIds( nLatin, nAsian, nComplex );
    GetItemSet().MergeRange( nLatin, nLatin );
    GetItemSet().MergeRange( nAsian, nAsian );
    GetItemSet().MergeRange( nComplex, nComplex );
    GetItemSet().MergeRange( SID_ATTR_CHAR_SCRIPTTYPE, SID_ATTR_CHAR_SCRIPTTYPE );
}

SfxPoolItem* SvxScriptSetItem::Clone( SfxItemPool * ) const
{
    SvxScriptSetItem* p = new SvxScriptSetItem( Which(), *GetItemSet().GetPool() );
    p->GetItemSet().Put( GetItemSet(), false );
    return p;
}

SfxPoolItem* SvxScriptSetItem::Create( SvStream &, sal_uInt16 ) const
{
    return nullptr;
}

const SfxPoolItem* SvxScriptSetItem::GetItemOfScriptSet(
                            const SfxItemSet& rSet, sal_uInt16 nId )
{
    const SfxPoolItem* pI;
    SfxItemState eSt = rSet.GetItemState( nId, false, &pI );
    if( SfxItemState::SET != eSt )
        pI = SfxItemState::DEFAULT == eSt ? &rSet.Get( nId ) : nullptr;
    return pI;
}

const SfxPoolItem* SvxScriptSetItem::GetItemOfScript( sal_uInt16 nSlotId, const SfxItemSet& rSet, SvtScriptType nScript )
{
    sal_uInt16 nLatin, nAsian, nComplex;
    GetWhichIds( nSlotId, rSet, nLatin, nAsian, nComplex );

    const SfxPoolItem *pRet, *pAsn, *pCmplx;
    if (nScript == SvtScriptType::ASIAN)
    {
        pRet = GetItemOfScriptSet( rSet, nAsian );
    } else if (nScript == SvtScriptType::COMPLEX)
    {
        pRet = GetItemOfScriptSet( rSet, nComplex );
    } else if (nScript == (SvtScriptType::LATIN|SvtScriptType::ASIAN))
    {
        if( nullptr == (pRet = GetItemOfScriptSet( rSet, nLatin )) ||
            nullptr == (pAsn = GetItemOfScriptSet( rSet, nAsian )) ||
            *pRet != *pAsn )
            pRet = nullptr;
    } else if (nScript == (SvtScriptType::LATIN|SvtScriptType::COMPLEX))
    {
        if( nullptr == (pRet = GetItemOfScriptSet( rSet, nLatin )) ||
            nullptr == (pCmplx = GetItemOfScriptSet( rSet, nComplex )) ||
            *pRet != *pCmplx )
            pRet = nullptr;
    } else if (nScript == (SvtScriptType::ASIAN|SvtScriptType::COMPLEX))
    {
        if( nullptr == (pRet = GetItemOfScriptSet( rSet, nAsian )) ||
            nullptr == (pCmplx = GetItemOfScriptSet( rSet, nComplex )) ||
            *pRet != *pCmplx )
            pRet = nullptr;
    } else if (nScript == (SvtScriptType::LATIN|SvtScriptType::ASIAN|SvtScriptType::COMPLEX))
    {
        if( nullptr == (pRet = GetItemOfScriptSet( rSet, nLatin )) ||
            nullptr == (pAsn = GetItemOfScriptSet( rSet, nAsian )) ||
            nullptr == (pCmplx = GetItemOfScriptSet( rSet, nComplex )) ||
            *pRet != *pAsn || *pRet != *pCmplx )
            pRet = nullptr;
    } else {
        //no one valid -> match to latin
        pRet = GetItemOfScriptSet( rSet, nLatin );
    }
    return pRet;
}

const SfxPoolItem* SvxScriptSetItem::GetItemOfScript( SvtScriptType nScript ) const
{
    return GetItemOfScript( Which(), GetItemSet(), nScript );
}

void SvxScriptSetItem::PutItemForScriptType( SvtScriptType nScriptType,
                                             const SfxPoolItem& rItem )
{
    sal_uInt16 nLatin, nAsian, nComplex;
    GetWhichIds( nLatin, nAsian, nComplex );

    SfxPoolItem* pCpy = rItem.Clone();
    if( SvtScriptType::LATIN & nScriptType )
    {
        pCpy->SetWhich( nLatin );
        GetItemSet().Put( *pCpy );
    }
    if( SvtScriptType::ASIAN & nScriptType )
    {
        pCpy->SetWhich( nAsian );
        GetItemSet().Put( *pCpy );
    }
    if( SvtScriptType::COMPLEX & nScriptType )
    {
        pCpy->SetWhich( nComplex );
        GetItemSet().Put( *pCpy );
    }
    delete pCpy;
}

void SvxScriptSetItem::GetWhichIds( sal_uInt16 nSlotId, const SfxItemSet& rSet, sal_uInt16& rLatin, sal_uInt16& rAsian, sal_uInt16& rComplex )
{
    const SfxItemPool& rPool = *rSet.GetPool();
    GetSlotIds( nSlotId, rLatin, rAsian, rComplex );
    rLatin = rPool.GetWhich( rLatin );
    rAsian = rPool.GetWhich( rAsian );
    rComplex = rPool.GetWhich( rComplex );
}

void SvxScriptSetItem::GetWhichIds( sal_uInt16& rLatin, sal_uInt16& rAsian,
                                    sal_uInt16& rComplex ) const
{
    GetWhichIds( Which(), GetItemSet(), rLatin, rAsian, rComplex );
}

void SvxScriptSetItem::GetSlotIds( sal_uInt16 nSlotId, sal_uInt16& rLatin,
                                    sal_uInt16& rAsian, sal_uInt16& rComplex )
{
    switch( nSlotId )
    {
    default:
        SAL_WARN( "editeng.items", "wrong SlotId for class SvxScriptSetItem" );
        SAL_FALLTHROUGH; // default to font - Id Range !!

    case SID_ATTR_CHAR_FONT:
        rLatin = SID_ATTR_CHAR_FONT;
        rAsian = SID_ATTR_CHAR_CJK_FONT;
        rComplex = SID_ATTR_CHAR_CTL_FONT;
        break;
    case SID_ATTR_CHAR_FONTHEIGHT:
        rLatin = SID_ATTR_CHAR_FONTHEIGHT;
        rAsian = SID_ATTR_CHAR_CJK_FONTHEIGHT;
        rComplex = SID_ATTR_CHAR_CTL_FONTHEIGHT;
        break;
    case SID_ATTR_CHAR_WEIGHT:
        rLatin = SID_ATTR_CHAR_WEIGHT;
        rAsian = SID_ATTR_CHAR_CJK_WEIGHT;
        rComplex = SID_ATTR_CHAR_CTL_WEIGHT;
        break;
    case SID_ATTR_CHAR_POSTURE:
        rLatin = SID_ATTR_CHAR_POSTURE;
        rAsian = SID_ATTR_CHAR_CJK_POSTURE;
        rComplex = SID_ATTR_CHAR_CTL_POSTURE;
        break;
    case SID_ATTR_CHAR_LANGUAGE:
        rLatin = SID_ATTR_CHAR_LANGUAGE;
        rAsian = SID_ATTR_CHAR_CJK_LANGUAGE;
        rComplex = SID_ATTR_CHAR_CTL_LANGUAGE;
        break;
    case SID_ATTR_CHAR_SHADOWED:
        rLatin = SID_ATTR_CHAR_SHADOWED;
        rAsian = SID_ATTR_CHAR_SHADOWED;
        rComplex = SID_ATTR_CHAR_SHADOWED;
        break;
    case SID_ATTR_CHAR_STRIKEOUT:
        rLatin = SID_ATTR_CHAR_STRIKEOUT;
        rAsian = SID_ATTR_CHAR_STRIKEOUT;
        rComplex = SID_ATTR_CHAR_STRIKEOUT;
        break;
    }
}

void GetDefaultFonts( SvxFontItem& rLatin, SvxFontItem& rAsian, SvxFontItem& rComplex )
{
    const sal_uInt16 nItemCnt = 3;

    static struct
    {
        DefaultFontType nFontType;
        LanguageType    nLanguage;
    }
    aOutTypeArr[ nItemCnt ] =
    {
        {  DefaultFontType::LATIN_TEXT, LANGUAGE_ENGLISH_US },
        {  DefaultFontType::CJK_TEXT, LANGUAGE_ENGLISH_US },
        {  DefaultFontType::CTL_TEXT, LANGUAGE_ARABIC_SAUDI_ARABIA }
    };

    SvxFontItem* aItemArr[ nItemCnt ] = { &rLatin, &rAsian, &rComplex };

    for ( sal_uInt16 n = 0; n < nItemCnt; ++n )
    {
        vcl::Font aFont( OutputDevice::GetDefaultFont( aOutTypeArr[ n ].nFontType,
                                                  aOutTypeArr[ n ].nLanguage,
                                                  GetDefaultFontFlags::OnlyOne ) );
        SvxFontItem* pItem = aItemArr[ n ];
        pItem->SetFamily( aFont.GetFamilyType() );
        pItem->SetFamilyName( aFont.GetFamilyName() );
        pItem->SetStyleName( OUString() );
        pItem->SetPitch( aFont.GetPitch());
        pItem->SetCharSet(aFont.GetCharSet());
    }
}


bool SvxRsidItem::QueryValue( uno::Any& rVal, sal_uInt8 ) const
{
    rVal <<= GetValue();
    return true;
}

bool SvxRsidItem::PutValue( const uno::Any& rVal, sal_uInt8 )
{
    sal_uInt32 nRsid = 0;
    if( !( rVal >>= nRsid ) )
        return false;

    SetValue( nRsid );
    return true;
}

SfxPoolItem* SvxRsidItem::Clone( SfxItemPool * ) const
{
    return new SvxRsidItem( *this );
}

SfxPoolItem* SvxRsidItem::Create(SvStream& rIn, sal_uInt16 ) const
{
    return new SvxRsidItem( rIn, Which() );
}

void SvxRsidItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SvxRsidItem"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("whichId"), "%d", Which());
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("value"), "%" SAL_PRIuUINT32, GetValue());
    xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
