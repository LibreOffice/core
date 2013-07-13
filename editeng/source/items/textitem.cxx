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
#include <vcl/bitmapex.hxx>
#include <tools/stream.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <math.h>
#include <rtl/math.hxx>
#include <unotools/fontdefs.hxx>
#include <vcl/outdev.hxx>
#include <editeng/eeitem.hxx>
#include <svtools/unitconv.hxx>

#include <svl/memberid.hrc>
#include <editeng/editids.hrc>
#include <editeng/editrids.hrc>
#include <vcl/vclenum.hxx>
#include <tools/tenccvt.hxx>

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
#include <editeng/lcolitem.hxx>
#include <editeng/blinkitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/itemtype.hxx>
#include <editeng/eerdll.hxx>

#define STORE_UNICODE_MAGIC_MARKER  0xFE331188

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::text;

// Conversion for UNO
#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)    ((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))
#define TWIP_TO_MM100_UNSIGNED(TWIP)     ((((TWIP)*127L+36L)/72L))
#define MM100_TO_TWIP_UNSIGNED(MM100)    ((((MM100)*72L+63L)/127L))

sal_Bool SvxFontItem::bEnableStoreUnicodeNames = sal_False;

// STATIC DATA -----------------------------------------------------------

// -----------------------------------------------------------------------

TYPEINIT1(SvxFontListItem, SfxPoolItem);
TYPEINIT1_FACTORY(SvxFontItem, SfxPoolItem, new SvxFontItem(0));
TYPEINIT1_FACTORY(SvxPostureItem, SfxEnumItem, new SvxPostureItem(ITALIC_NONE, 0));
TYPEINIT1_FACTORY(SvxWeightItem, SfxEnumItem, new SvxWeightItem(WEIGHT_NORMAL, 0));
TYPEINIT1_FACTORY(SvxFontHeightItem, SfxPoolItem, new SvxFontHeightItem(240, 100, 0));
TYPEINIT1_FACTORY(SvxFontWidthItem, SfxPoolItem, new SvxFontWidthItem(0, 100, 0));
TYPEINIT1_FACTORY(SvxTextLineItem, SfxEnumItem, new SvxTextLineItem(UNDERLINE_NONE, 0));
TYPEINIT1_FACTORY(SvxUnderlineItem, SfxEnumItem, new SvxUnderlineItem(UNDERLINE_NONE, 0));
TYPEINIT1_FACTORY(SvxOverlineItem, SfxEnumItem, new SvxOverlineItem(UNDERLINE_NONE, 0));
TYPEINIT1_FACTORY(SvxCrossedOutItem, SfxEnumItem, new SvxCrossedOutItem(STRIKEOUT_NONE, 0));
TYPEINIT1_FACTORY(SvxShadowedItem, SfxBoolItem, new SvxShadowedItem(sal_False, 0));
TYPEINIT1_FACTORY(SvxAutoKernItem, SfxBoolItem, new SvxAutoKernItem(sal_False, 0));
TYPEINIT1_FACTORY(SvxWordLineModeItem, SfxBoolItem, new SvxWordLineModeItem(sal_False, 0));
TYPEINIT1_FACTORY(SvxContourItem, SfxBoolItem, new SvxContourItem(sal_False, 0));
TYPEINIT1_FACTORY(SvxPropSizeItem, SfxUInt16Item, new SvxPropSizeItem(100, 0));
TYPEINIT1_FACTORY(SvxColorItem, SfxPoolItem, new SvxColorItem(0));
TYPEINIT1_FACTORY(SvxCharSetColorItem, SvxColorItem, new SvxCharSetColorItem(0));
TYPEINIT1_FACTORY(SvxKerningItem, SfxInt16Item, new SvxKerningItem(0, 0));
TYPEINIT1_FACTORY(SvxCaseMapItem, SfxEnumItem, new SvxCaseMapItem(SVX_CASEMAP_NOT_MAPPED, 0));
TYPEINIT1_FACTORY(SvxEscapementItem, SfxPoolItem, new SvxEscapementItem(0));
TYPEINIT1_FACTORY(SvxLanguageItem, SfxEnumItem, new SvxLanguageItem(LANGUAGE_GERMAN, 0));
TYPEINIT1_FACTORY(SvxNoLinebreakItem, SfxBoolItem, new SvxNoLinebreakItem(sal_True, 0));
TYPEINIT1_FACTORY(SvxNoHyphenItem, SfxBoolItem, new SvxNoHyphenItem(sal_True, 0));
TYPEINIT1_FACTORY(SvxLineColorItem, SvxColorItem, new SvxLineColorItem(0));
TYPEINIT1_FACTORY(SvxBlinkItem, SfxBoolItem, new SvxBlinkItem(sal_False, 0));
TYPEINIT1_FACTORY(SvxEmphasisMarkItem, SfxUInt16Item, new SvxEmphasisMarkItem(EMPHASISMARK_NONE, 0));
TYPEINIT1_FACTORY(SvxTwoLinesItem, SfxPoolItem, new SvxTwoLinesItem(sal_True, 0, 0, 0));
TYPEINIT1_FACTORY(SvxScriptTypeItem, SfxUInt16Item, new SvxScriptTypeItem);
TYPEINIT1_FACTORY(SvxCharRotateItem, SfxUInt16Item, new SvxCharRotateItem(0, sal_False, 0));
TYPEINIT1_FACTORY(SvxCharScaleWidthItem, SfxUInt16Item, new SvxCharScaleWidthItem(100, 0));
TYPEINIT1_FACTORY(SvxCharReliefItem, SfxEnumItem, new SvxCharReliefItem(RELIEF_NONE, 0));
TYPEINIT1_FACTORY(SvxRsidItem, SfxUInt32Item, new SvxRsidItem(0, 0));

TYPEINIT1(SvxScriptSetItem, SfxSetItem );


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

        for ( sal_uInt16 i = 0; i < nCount; i++ )
            aFontNameSeq[i] = pFontList->GetFontName(i).GetName();
    }
}

// -----------------------------------------------------------------------

SvxFontListItem::SvxFontListItem( const SvxFontListItem& rItem ) :

    SfxPoolItem( rItem ),
    pFontList( rItem.GetFontList() ),
    aFontNameSeq( rItem.aFontNameSeq )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFontListItem::Clone( SfxItemPool* ) const
{
    return new SvxFontListItem( *this );
}

// -----------------------------------------------------------------------

int SvxFontListItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return( pFontList == ((SvxFontListItem&)rAttr).pFontList );
}

bool SvxFontListItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= aFontNameSeq;
    return true;
}

//------------------------------------------------------------------------

SfxItemPresentation SvxFontListItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    rText = OUString();
    return SFX_ITEM_PRESENTATION_NONE;
}

// class SvxFontItem -----------------------------------------------------

SvxFontItem::SvxFontItem( const sal_uInt16 nId ) :
    SfxPoolItem( nId )
{
    eFamily = FAMILY_SWISS;
    ePitch = PITCH_VARIABLE;
    eTextEncoding = RTL_TEXTENCODING_DONTKNOW;
}

// -----------------------------------------------------------------------

SvxFontItem::SvxFontItem( const FontFamily eFam, const XubString& aName,
                  const XubString& aStName, const FontPitch eFontPitch,
                  const rtl_TextEncoding eFontTextEncoding, const sal_uInt16 nId ) :

    SfxPoolItem( nId ),

    aFamilyName(aName),
    aStyleName(aStName)
{
    eFamily = eFam;
    ePitch = eFontPitch;
    eTextEncoding = eFontTextEncoding;
}

// -----------------------------------------------------------------------
SvxFontItem& SvxFontItem::operator=(const SvxFontItem& rFont)
{
    aFamilyName =  rFont.GetFamilyName();
    aStyleName =   rFont.GetStyleName();
    eFamily =      rFont.GetFamily();
    ePitch =   rFont.GetPitch();
    eTextEncoding = rFont.GetCharSet();
    return *this;
}
// -----------------------------------------------------------------------

bool SvxFontItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case 0:
        {
            com::sun::star::awt::FontDescriptor aFontDescriptor;
            aFontDescriptor.Name = aFamilyName.GetBuffer();
            aFontDescriptor.StyleName = aStyleName.GetBuffer();
            aFontDescriptor.Family = (sal_Int16)(eFamily);
            aFontDescriptor.CharSet = (sal_Int16)(eTextEncoding);
            aFontDescriptor.Pitch = (sal_Int16)(ePitch);
            rVal <<= aFontDescriptor;
        }
        break;
        case MID_FONT_FAMILY_NAME   :
            rVal <<= OUString(aFamilyName.GetBuffer());
        break;
        case MID_FONT_STYLE_NAME:
            rVal <<= OUString(aStyleName.GetBuffer());
        break;
        case MID_FONT_FAMILY    : rVal <<= (sal_Int16)(eFamily);    break;
        case MID_FONT_CHAR_SET  : rVal <<= (sal_Int16)(eTextEncoding);  break;
        case MID_FONT_PITCH     : rVal <<= (sal_Int16)(ePitch); break;
    }
    return true;
}
// -----------------------------------------------------------------------
bool SvxFontItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId)
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case 0:
        {
            com::sun::star::awt::FontDescriptor aFontDescriptor;
            if ( !( rVal >>= aFontDescriptor ))
                return sal_False;

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
                return sal_False;
            aFamilyName = aStr.getStr();
        }
        break;
        case MID_FONT_STYLE_NAME:
        {
            OUString aStr;
            if(!(rVal >>= aStr))
                return sal_False;
            aStyleName = aStr.getStr();
        }
        break;
        case MID_FONT_FAMILY :
        {
            sal_Int16 nFamily = sal_Int16();
            if(!(rVal >>= nFamily))
                return sal_False;
            eFamily = (FontFamily)nFamily;
        }
        break;
        case MID_FONT_CHAR_SET  :
        {
            sal_Int16 nSet = sal_Int16();
            if(!(rVal >>= nSet))
                return sal_False;
            eTextEncoding = (rtl_TextEncoding)nSet;
        }
        break;
        case MID_FONT_PITCH     :
        {
            sal_Int16 nPitch = sal_Int16();
            if(!(rVal >>= nPitch))
                return sal_False;
            ePitch =  (FontPitch)nPitch;
        }
        break;
    }
    return true;
}

// -----------------------------------------------------------------------

int SvxFontItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxFontItem& rItem = (const SvxFontItem&)rAttr;

    int bRet = ( eFamily == rItem.eFamily &&
                 aFamilyName == rItem.aFamilyName &&
                 aStyleName == rItem.aStyleName );

    if ( bRet )
    {
        if ( ePitch != rItem.ePitch || eTextEncoding != rItem.eTextEncoding )
        {
            bRet = sal_False;
            DBG_WARNING( "FontItem::operator==(): only pitch or rtl_TextEncoding different ");
        }
    }
    return bRet;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFontItem::Clone( SfxItemPool * ) const
{
    return new SvxFontItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxFontItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    sal_Bool bToBats = IsStarSymbol( GetFamilyName() );

    rStrm << (sal_uInt8) GetFamily() << (sal_uInt8) GetPitch()
          << (sal_uInt8)(bToBats ? RTL_TEXTENCODING_SYMBOL : GetSOStoreTextEncoding(GetCharSet()));

    String aStoreFamilyName( GetFamilyName() );
    if( bToBats )
        aStoreFamilyName = String( "StarBats", sizeof("StarBats")-1, RTL_TEXTENCODING_ASCII_US );
    rStrm.WriteUniOrByteString(aStoreFamilyName, rStrm.GetStreamCharSet());
    rStrm.WriteUniOrByteString(GetStyleName(), rStrm.GetStreamCharSet());

    // cach for EditEngine, only set while creating clipboard stream.
    if ( bEnableStoreUnicodeNames )
    {
        sal_uInt32 nMagic = STORE_UNICODE_MAGIC_MARKER;
        rStrm << nMagic;
        rStrm.WriteUniOrByteString( aStoreFamilyName, RTL_TEXTENCODING_UNICODE );
        rStrm.WriteUniOrByteString( GetStyleName(), RTL_TEXTENCODING_UNICODE );
    }

    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFontItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 _eFamily, eFontPitch, eFontTextEncoding;
    String aName, aStyle;
    rStrm >> _eFamily;
    rStrm >> eFontPitch;
    rStrm >> eFontTextEncoding;

    // UNICODE: rStrm >> aName;
    aName = rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet());

    // UNICODE: rStrm >> aStyle;
    aStyle = rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet());

    // Set the "correct" textencoding
    eFontTextEncoding = (sal_uInt8)GetSOLoadTextEncoding( eFontTextEncoding );

    // at some point, the StarBats changes from  ANSI font to SYMBOL font
    if ( RTL_TEXTENCODING_SYMBOL != eFontTextEncoding && aName.EqualsAscii("StarBats") )
        eFontTextEncoding = RTL_TEXTENCODING_SYMBOL;

    // Check if we have stored unicode
    sal_Size nStreamPos = rStrm.Tell();
    sal_uInt32 nMagic = STORE_UNICODE_MAGIC_MARKER;
    rStrm >> nMagic;
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

//------------------------------------------------------------------------

SfxItemPresentation SvxFontItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = aFamilyName;
            return ePres;
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

void SvxFontItem::EnableStoreUnicodeNames( sal_Bool bEnable )
{
    bEnableStoreUnicodeNames = bEnable;
}

// class SvxPostureItem --------------------------------------------------

SvxPostureItem::SvxPostureItem( const FontItalic ePosture, const sal_uInt16 nId ) :
    SfxEnumItem( nId, (sal_uInt16)ePosture )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxPostureItem::Clone( SfxItemPool * ) const
{
    return new SvxPostureItem( *this );
}

// -----------------------------------------------------------------------

sal_uInt16 SvxPostureItem::GetValueCount() const
{
    return ITALIC_NORMAL + 1;   // ITALIC_NONE also belongs here
}

// -----------------------------------------------------------------------

SvStream& SvxPostureItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm << (sal_uInt8)GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxPostureItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 nPosture;
    rStrm >> nPosture;
    return new SvxPostureItem( (const FontItalic)nPosture, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxPostureItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetValueTextByPos( GetValue() );
            return ePres;
        default: ;//prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

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

    return nId ? EE_RESSTR(nId) : OUString();
}

bool SvxPostureItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_ITALIC:
            rVal = Bool2Any(GetBoolValue());
            break;
        case MID_POSTURE:
            rVal <<= (awt::FontSlant)GetValue();    // values from awt::FontSlant and FontItalic are equal
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
                    return sal_False;

                eSlant = (awt::FontSlant)nValue;
            }
            SetValue((sal_uInt16)eSlant);
        }
    }
    return true;
}
// -----------------------------------------------------------------------

int SvxPostureItem::HasBoolValue() const
{
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxPostureItem::GetBoolValue() const
{
    return ( (FontItalic)GetValue() >= ITALIC_OBLIQUE );
}

// -----------------------------------------------------------------------

void SvxPostureItem::SetBoolValue( sal_Bool bVal )
{
    SetValue( (sal_uInt16)(bVal ? ITALIC_NORMAL : ITALIC_NONE) );
}

// class SvxWeightItem ---------------------------------------------------

SvxWeightItem::SvxWeightItem( const FontWeight eWght, const sal_uInt16 nId ) :
    SfxEnumItem( nId, (sal_uInt16)eWght )
{
}



// -----------------------------------------------------------------------

int SvxWeightItem::HasBoolValue() const
{
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxWeightItem::GetBoolValue() const
{
    return  (FontWeight)GetValue() >= WEIGHT_BOLD;
}

// -----------------------------------------------------------------------

void SvxWeightItem::SetBoolValue( sal_Bool bVal )
{
    SetValue( (sal_uInt16)(bVal ? WEIGHT_BOLD : WEIGHT_NORMAL) );
}

// -----------------------------------------------------------------------

sal_uInt16 SvxWeightItem::GetValueCount() const
{
    return WEIGHT_BLACK;    // WEIGHT_DONTKNOW does not belong
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxWeightItem::Clone( SfxItemPool * ) const
{
    return new SvxWeightItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxWeightItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm << (sal_uInt8)GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxWeightItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 nWeight;
    rStrm >> nWeight;
    return new SvxWeightItem( (FontWeight)nWeight, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxWeightItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetValueTextByPos( GetValue() );
            return ePres;
        default: ;//prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

OUString SvxWeightItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos <= (sal_uInt16)WEIGHT_BLACK, "enum overflow!" );
    return EE_RESSTR(RID_SVXITEMS_WEIGHT_BEGIN + nPos);
}

bool SvxWeightItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_BOLD   :
            rVal = Bool2Any(GetBoolValue());
        break;
        case MID_WEIGHT:
        {
            rVal <<= (float)( VCLUnoHelper::ConvertFontWeight( (FontWeight)GetValue() ) );
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
                    return sal_False;
                fValue = (float)nValue;
            }
            SetValue( (sal_uInt16)VCLUnoHelper::ConvertFontWeight((float)fValue) );
        }
        break;
    }
    return true;
}

// class SvxFontHeightItem -----------------------------------------------

SvxFontHeightItem::SvxFontHeightItem( const sal_uLong nSz,
                                      const sal_uInt16 nPrp,
                                      const sal_uInt16 nId ) :
    SfxPoolItem( nId )
{
    SetHeight( nSz,nPrp );  // calculate in percentage
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFontHeightItem::Clone( SfxItemPool * ) const
{
    return new SvxFontHeightItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxFontHeightItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    rStrm << (sal_uInt16)GetHeight();

    if( FONTHEIGHT_UNIT_VERSION <= nItemVersion )
        rStrm << GetProp() << (sal_uInt16)GetPropUnit();
    else
    {
        // When exporting to the old versions the relative information is lost
        // when there is no percentage
        sal_uInt16 _nProp = GetProp();
        if( SFX_MAPUNIT_RELATIVE != GetPropUnit() )
            _nProp = 100;
        rStrm << _nProp;
    }
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFontHeightItem::Create( SvStream& rStrm,
                                                 sal_uInt16 nVersion ) const
{
    sal_uInt16 nsize, nprop = 0, nPropUnit = SFX_MAPUNIT_RELATIVE;

    rStrm >> nsize;

    if( FONTHEIGHT_16_VERSION <= nVersion )
        rStrm >> nprop;
    else
    {
        sal_uInt8 nP;
        rStrm  >> nP;
        nprop = (sal_uInt16)nP;
    }

    if( FONTHEIGHT_UNIT_VERSION <= nVersion )
        rStrm >> nPropUnit;

    SvxFontHeightItem* pItem = new SvxFontHeightItem( nsize, 100, Which() );
    pItem->SetProp( nprop, (SfxMapUnit)nPropUnit );
    return pItem;
}

// -----------------------------------------------------------------------

int SvxFontHeightItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return GetHeight() == ((SvxFontHeightItem&)rItem).GetHeight() &&
            GetProp() == ((SvxFontHeightItem&)rItem).GetProp() &&
            GetPropUnit() == ((SvxFontHeightItem&)rItem).GetPropUnit();
}

bool SvxFontHeightItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    //  In StarOne is the uno::Any always 1/100mm. Through the MemberId it is
    //  controlled if the value in the Item should be 1/100mm or Twips.

    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case 0:
        {
            ::com::sun::star::frame::status::FontHeight aFontHeight;

            // Point (i.e. Twips) is asked for, thus re-calculate if
            // CONVERT_TWIPS is not set.
            if( bConvert )
            {
                long nTwips = bConvert ? nHeight : MM100_TO_TWIP_UNSIGNED(nHeight);
                aFontHeight.Height = (float)( nTwips / 20.0 );
            }
            else
            {
                double fPoints = MM100_TO_TWIP_UNSIGNED(nHeight) / 20.0;
                float fRoundPoints =
                    static_cast<float>(::rtl::math::round(fPoints, 1));
                aFontHeight.Height = fRoundPoints;
            }

            aFontHeight.Prop = (sal_Int16)(SFX_MAPUNIT_RELATIVE == ePropUnit ? nProp : 100);

            float fRet = (float)(short)nProp;
            switch( ePropUnit )
            {
                case SFX_MAPUNIT_RELATIVE:
                    fRet = 0.;
                break;
                case SFX_MAPUNIT_100TH_MM:
                    fRet = MM100_TO_TWIP(fRet);
                    fRet /= 20.;
                break;
                case SFX_MAPUNIT_POINT:

                break;
                case SFX_MAPUNIT_TWIP:
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
                long nTwips = bConvert ? nHeight : MM100_TO_TWIP_UNSIGNED(nHeight);
                rVal <<= (float)( nTwips / 20.0 );
            }
            else
            {
                double fPoints = MM100_TO_TWIP_UNSIGNED(nHeight) / 20.0;
                float fRoundPoints =
                    static_cast<float>(::rtl::math::round(fPoints, 1));
                rVal <<= fRoundPoints;
            }
        }
        break;
        case MID_FONTHEIGHT_PROP:
            rVal <<= (sal_Int16)(SFX_MAPUNIT_RELATIVE == ePropUnit ? nProp : 100);
        break;
        case MID_FONTHEIGHT_DIFF:
        {
            float fRet = (float)(short)nProp;
            switch( ePropUnit )
            {
                case SFX_MAPUNIT_RELATIVE:
                    fRet = 0.;
                break;
                case SFX_MAPUNIT_100TH_MM:
                    fRet = MM100_TO_TWIP(fRet);
                    fRet /= 20.;
                break;
                case SFX_MAPUNIT_POINT:

                break;
                case SFX_MAPUNIT_TWIP:
                    fRet /= 20.;
                break;
                default: ;//prevent warning
            }
            rVal <<= fRet;
        }
        break;
    }
    return sal_True;
}

// Calculate the relative deviation from the expected height.
static sal_uInt32 lcl_GetRealHeight_Impl(sal_uInt32 nHeight, sal_uInt16 nProp, SfxMapUnit eProp, sal_Bool bCoreInTwip)
{
    sal_uInt32 nRet = nHeight;
    short nDiff = 0;
    switch( eProp )
    {
        case SFX_MAPUNIT_RELATIVE:
            nRet *= 100;
            nRet /= nProp;
        break;
        case SFX_MAPUNIT_POINT:
        {
            short nTemp = (short)nProp;
            nDiff = nTemp * 20;
            if(!bCoreInTwip)
                nDiff = (short)TWIP_TO_MM100((long)(nDiff));
        }
        break;
        case SFX_MAPUNIT_100TH_MM:
            //then the core is surely also in 1/100 mm
            nDiff = (short)nProp;
        break;
        case SFX_MAPUNIT_TWIP:
            // Here surely TWIP
            nDiff = ((short)nProp);
        break;
        default: ;//prevent warning
    }
    nRet -= nDiff;

    return nRet;
}

bool SvxFontHeightItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case 0:
        {
            ::com::sun::star::frame::status::FontHeight aFontHeight;
            if ( rVal >>= aFontHeight )
            {
                // Height
                ePropUnit = SFX_MAPUNIT_RELATIVE;
                nProp = 100;
                double fPoint = aFontHeight.Height;
                if( fPoint < 0. || fPoint > 10000. )
                    return sal_False;

                nHeight = (long)( fPoint * 20.0 + 0.5 );        // Twips
                if (!bConvert)
                    nHeight = TWIP_TO_MM100_UNSIGNED(nHeight);  // Convert, if the item contains 1/100mm

                nProp = aFontHeight.Prop;
            }
            else
                return sal_False;
        }
        break;
        case MID_FONTHEIGHT:
        {
            ePropUnit = SFX_MAPUNIT_RELATIVE;
            nProp = 100;
            double fPoint = 0;
            if(!(rVal >>= fPoint))
            {
                sal_Int32 nValue = 0;
                if(!(rVal >>= nValue))
                    return sal_False;
                fPoint = (float)nValue;
            }
            if(fPoint < 0. || fPoint > 10000.)
                    return sal_False;

            nHeight = (long)( fPoint * 20.0 + 0.5 );        // Twips
            if (!bConvert)
                nHeight = TWIP_TO_MM100_UNSIGNED(nHeight);  // Convert, if the item contains 1/100mm
        }
        break;
        case MID_FONTHEIGHT_PROP:
        {
            sal_Int16 nNew = sal_Int16();
            if(!(rVal >>= nNew))
                return sal_True;

            nHeight = lcl_GetRealHeight_Impl(nHeight, nProp, ePropUnit, bConvert);

            nHeight *= nNew;
            nHeight /= 100;
            nProp = nNew;
            ePropUnit = SFX_MAPUNIT_RELATIVE;
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
                    return sal_False;
                fValue = (float)nValue;
            }
            sal_Int16 nCoreDiffValue = (sal_Int16)(fValue * 20.);
            nHeight += bConvert ? nCoreDiffValue : TWIP_TO_MM100(nCoreDiffValue);
            nProp = (sal_uInt16)((sal_Int16)fValue);
            ePropUnit = SFX_MAPUNIT_POINT;
        }
        break;
    }
    return sal_True;
}

//------------------------------------------------------------------------

SfxItemPresentation SvxFontHeightItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if( SFX_MAPUNIT_RELATIVE != ePropUnit )
            {
                rText = OUString::number( (short)nProp ) +
                        EE_RESSTR( GetMetricId( ePropUnit ) );
                if( 0 <= (short)nProp )
                    rText = "+" + rText;
            }
            else if( 100 == nProp )
            {
                rText = GetMetricText( (long)nHeight,
                                        eCoreUnit, SFX_MAPUNIT_POINT, pIntl ) +
                        EE_RESSTR(GetMetricId(SFX_MAPUNIT_POINT));
            }
            else
                rText = OUString::number( nProp ) + "%";
            return ePres;
        }
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

sal_uInt16 SvxFontHeightItem::GetVersion(sal_uInt16 nFileVersion) const
{
    return (nFileVersion <= SOFFICE_FILEFORMAT_40)
               ? FONTHEIGHT_16_VERSION
               : FONTHEIGHT_UNIT_VERSION;
}

// -----------------------------------------------------------------------

bool SvxFontHeightItem::ScaleMetrics( long nMult, long nDiv )
{
    nHeight = (sal_uInt32)Scale( nHeight, nMult, nDiv );
    return true;
}

// -----------------------------------------------------------------------

bool SvxFontHeightItem::HasMetrics() const
{
    return true;
}

void SvxFontHeightItem::SetHeight( sal_uInt32 nNewHeight, const sal_uInt16 nNewProp,
                                    SfxMapUnit eUnit )
{
    DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );

    if( SFX_MAPUNIT_RELATIVE != eUnit )
        nHeight = nNewHeight + ::ItemToControl( (short)nNewProp, eUnit,
                                                SFX_FUNIT_TWIP );
    else if( 100 != nNewProp )
        nHeight = sal_uInt32(( nNewHeight * nNewProp ) / 100 );
    else
        nHeight = nNewHeight;

    nProp = nNewProp;
    ePropUnit = eUnit;
}

void SvxFontHeightItem::SetHeight( sal_uInt32 nNewHeight, sal_uInt16 nNewProp,
                                 SfxMapUnit eMetric, SfxMapUnit eCoreMetric )
{
    DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );

    if( SFX_MAPUNIT_RELATIVE != eMetric )
        nHeight = nNewHeight +
                ::ControlToItem( ::ItemToControl((short)nNewProp, eMetric,
                                        SFX_FUNIT_TWIP ), SFX_FUNIT_TWIP,
                                        eCoreMetric );
    else if( 100 != nNewProp )
        nHeight = sal_uInt32(( nNewHeight * nNewProp ) / 100 );
    else
        nHeight = nNewHeight;

    nProp = nNewProp;
    ePropUnit = eMetric;
}

// class SvxFontWidthItem -----------------------------------------------

SvxFontWidthItem::SvxFontWidthItem( const sal_uInt16 nSz, const sal_uInt16 nPrp, const sal_uInt16 nId ) :
    SfxPoolItem( nId )
{
    nWidth = nSz;
    nProp = nPrp;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFontWidthItem::Clone( SfxItemPool * ) const
{
    return new SvxFontWidthItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxFontWidthItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm << GetWidth() << GetProp();
    return rStrm;
}

// -----------------------------------------------------------------------

bool SvxFontWidthItem::ScaleMetrics( long nMult, long nDiv )
{
    nWidth = (sal_uInt16)Scale( nWidth, nMult, nDiv );
    return true;
}

// -----------------------------------------------------------------------

bool SvxFontWidthItem::HasMetrics() const
{
    return true;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFontWidthItem::Create( SvStream& rStrm,
                                                 sal_uInt16 /*nVersion*/ ) const
{
    sal_uInt16 nS;
    sal_uInt16 nP;

    rStrm >> nS;
    rStrm >> nP;
    SvxFontWidthItem* pItem = new SvxFontWidthItem( 0, nP, Which() );
    pItem->SetWidthValue( nS );
    return pItem;
}

// -----------------------------------------------------------------------

int SvxFontWidthItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return GetWidth() == ((SvxFontWidthItem&)rItem).GetWidth() &&
            GetProp() == ((SvxFontWidthItem&)rItem).GetProp();
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
        return sal_False;

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

//------------------------------------------------------------------------

SfxItemPresentation SvxFontWidthItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if ( 100 == nProp )
            {
                rText = GetMetricText( (long)nWidth,
                                        eCoreUnit, SFX_MAPUNIT_POINT, pIntl ) +
                        EE_RESSTR(GetMetricId(SFX_MAPUNIT_POINT));
            }
            else
                rText = OUString::number( nProp ) + "%";
            return ePres;
        }
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// class SvxTextLineItem ------------------------------------------------

SvxTextLineItem::SvxTextLineItem( const FontUnderline eSt, const sal_uInt16 nId )
    : SfxEnumItem( nId, (sal_uInt16)eSt ), mColor( COL_TRANSPARENT )
{
}

// -----------------------------------------------------------------------

int SvxTextLineItem::HasBoolValue() const
{
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxTextLineItem::GetBoolValue() const
{
    return  (FontUnderline)GetValue() != UNDERLINE_NONE;
}

// -----------------------------------------------------------------------

void SvxTextLineItem::SetBoolValue( sal_Bool bVal )
{
    SetValue( (sal_uInt16)(bVal ? UNDERLINE_SINGLE : UNDERLINE_NONE) );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxTextLineItem::Clone( SfxItemPool * ) const
{
    SvxTextLineItem* pNew = new SvxTextLineItem( *this );
    pNew->SetColor( GetColor() );
    return pNew;
}

// -----------------------------------------------------------------------

sal_uInt16 SvxTextLineItem::GetValueCount() const
{
    return UNDERLINE_DOTTED + 1;    // UNDERLINE_NONE also belongs here
}

// -----------------------------------------------------------------------

SvStream& SvxTextLineItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm << (sal_uInt8)GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxTextLineItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 nState;
    rStrm >> nState;
    return new SvxTextLineItem(  (FontUnderline)nState, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxTextLineItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetValueTextByPos( GetValue() );
            if( !mColor.GetTransparency() )
                rText = rText + OUString(cpDelim) + ::GetColorString( mColor );
            return ePres;
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

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
        rVal = Bool2Any(GetBoolValue());
        break;
    case MID_TL_STYLE:
        rVal <<= (sal_Int16)(GetValue());
        break;
    case MID_TL_COLOR:
        rVal <<= (sal_Int32)( mColor.GetColor() );
        break;
    case MID_TL_HASCOLOR:
        rVal = Bool2Any( !mColor.GetTransparency() );
        break;
    }
    return true;

}

bool SvxTextLineItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_True;
    switch(nMemberId)
    {
    case MID_TEXTLINED:
        SetBoolValue(Any2Bool(rVal));
    break;
    case MID_TL_STYLE:
    {
        sal_Int32 nValue = 0;
        if(!(rVal >>= nValue))
            bRet = sal_False;
        else
            SetValue((sal_Int16)nValue);
    }
    break;
    case MID_TL_COLOR:
    {
        sal_Int32 nCol = 0;
        if( !( rVal >>= nCol ) )
            bRet = sal_False;
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

int SvxTextLineItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return SfxEnumItem::operator==( rItem ) &&
           GetColor() == ((SvxTextLineItem&)rItem).GetColor();
}

// class SvxUnderlineItem ------------------------------------------------

SvxUnderlineItem::SvxUnderlineItem( const FontUnderline eSt, const sal_uInt16 nId )
    : SvxTextLineItem( eSt, nId )
{
}

//------------------------------------------------------------------------

SfxPoolItem* SvxUnderlineItem::Clone( SfxItemPool * ) const
{
    SvxUnderlineItem* pNew = new SvxUnderlineItem( *this );
    pNew->SetColor( GetColor() );
    return pNew;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxUnderlineItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 nState;
    rStrm >> nState;
    return new SvxUnderlineItem(  (FontUnderline)nState, Which() );
}

// -----------------------------------------------------------------------

OUString SvxUnderlineItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos <= (sal_uInt16)UNDERLINE_BOLDWAVE, "enum overflow!" );
    return EE_RESSTR(RID_SVXITEMS_UL_BEGIN + nPos);
}

// class SvxOverlineItem ------------------------------------------------

SvxOverlineItem::SvxOverlineItem( const FontUnderline eSt, const sal_uInt16 nId )
    : SvxTextLineItem( eSt, nId )
{
}

//------------------------------------------------------------------------

SfxPoolItem* SvxOverlineItem::Clone( SfxItemPool * ) const
{
    SvxOverlineItem* pNew = new SvxOverlineItem( *this );
    pNew->SetColor( GetColor() );
    return pNew;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxOverlineItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 nState;
    rStrm >> nState;
    return new SvxOverlineItem(  (FontUnderline)nState, Which() );
}

// -----------------------------------------------------------------------

OUString SvxOverlineItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos <= (sal_uInt16)UNDERLINE_BOLDWAVE, "enum overflow!" );
    return EE_RESSTR(RID_SVXITEMS_OL_BEGIN + nPos);
}

// class SvxCrossedOutItem -----------------------------------------------

SvxCrossedOutItem::SvxCrossedOutItem( const FontStrikeout eSt, const sal_uInt16 nId )
    : SfxEnumItem( nId, (sal_uInt16)eSt )
{
}

// -----------------------------------------------------------------------

int SvxCrossedOutItem::HasBoolValue() const
{
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxCrossedOutItem::GetBoolValue() const
{
    return (FontStrikeout)GetValue() != STRIKEOUT_NONE;
}

// -----------------------------------------------------------------------

void SvxCrossedOutItem::SetBoolValue( sal_Bool bVal )
{
    SetValue( (sal_uInt16)(bVal ? STRIKEOUT_SINGLE : STRIKEOUT_NONE) );
}

// -----------------------------------------------------------------------

sal_uInt16 SvxCrossedOutItem::GetValueCount() const
{
    return STRIKEOUT_DOUBLE + 1;    // STRIKEOUT_NONE belongs also here
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxCrossedOutItem::Clone( SfxItemPool * ) const
{
    return new SvxCrossedOutItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxCrossedOutItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm << (sal_uInt8)GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxCrossedOutItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 eCross;
    rStrm >> eCross;
    return new SvxCrossedOutItem(  (FontStrikeout)eCross, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxCrossedOutItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetValueTextByPos( GetValue() );
            return ePres;
        default: ;//prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

OUString SvxCrossedOutItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos <= (sal_uInt16)STRIKEOUT_X, "enum overflow!" );
    return EE_RESSTR(RID_SVXITEMS_STRIKEOUT_BEGIN + nPos);
}

bool SvxCrossedOutItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_CROSSED_OUT:
            rVal = Bool2Any(GetBoolValue());
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
                return sal_False;
            SetValue((sal_Int16)nValue);
        }
        break;
    }
    return sal_True;
}
// class SvxShadowedItem -------------------------------------------------

SvxShadowedItem::SvxShadowedItem( const sal_Bool bShadowed, const sal_uInt16 nId ) :
    SfxBoolItem( nId, bShadowed )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxShadowedItem::Clone( SfxItemPool * ) const
{
    return new SvxShadowedItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxShadowedItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm << (sal_uInt8) GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxShadowedItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 nState;
    rStrm >> nState;
    return new SvxShadowedItem( nState, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxShadowedItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            sal_uInt16 nId = RID_SVXITEMS_SHADOWED_FALSE;

            if ( GetValue() )
                nId = RID_SVXITEMS_SHADOWED_TRUE;
            rText = EE_RESSTR(nId);
            return ePres;
        }
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// class SvxAutoKernItem -------------------------------------------------

SvxAutoKernItem::SvxAutoKernItem( const sal_Bool bAutoKern, const sal_uInt16 nId ) :
    SfxBoolItem( nId, bAutoKern )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxAutoKernItem::Clone( SfxItemPool * ) const
{
    return new SvxAutoKernItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxAutoKernItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm << (sal_uInt8) GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxAutoKernItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 nState;
    rStrm >> nState;
    return new SvxAutoKernItem( nState, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxAutoKernItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            sal_uInt16 nId = RID_SVXITEMS_AUTOKERN_FALSE;

            if ( GetValue() )
                nId = RID_SVXITEMS_AUTOKERN_TRUE;
            rText = EE_RESSTR(nId);
            return ePres;
        }
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// class SvxWordLineModeItem ---------------------------------------------

SvxWordLineModeItem::SvxWordLineModeItem( const sal_Bool bWordLineMode,
                                          const sal_uInt16 nId ) :
    SfxBoolItem( nId, bWordLineMode )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxWordLineModeItem::Clone( SfxItemPool * ) const
{
    return new SvxWordLineModeItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxWordLineModeItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm << (sal_Bool) GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxWordLineModeItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_Bool bValue;
    rStrm >> bValue;
    return new SvxWordLineModeItem( bValue, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxWordLineModeItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            sal_uInt16 nId = RID_SVXITEMS_WORDLINE_FALSE;

            if ( GetValue() )
                nId = RID_SVXITEMS_WORDLINE_TRUE;
            rText = EE_RESSTR(nId);
            return ePres;
        }
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// class SvxContourItem --------------------------------------------------

SvxContourItem::SvxContourItem( const sal_Bool bContoured, const sal_uInt16 nId ) :
    SfxBoolItem( nId, bContoured )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxContourItem::Clone( SfxItemPool * ) const
{
    return new SvxContourItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxContourItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm << (sal_Bool) GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxContourItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_Bool bValue;
    rStrm >> bValue;
    return new SvxContourItem( bValue, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxContourItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            sal_uInt16 nId = RID_SVXITEMS_CONTOUR_FALSE;

            if ( GetValue() )
                nId = RID_SVXITEMS_CONTOUR_TRUE;
            rText = EE_RESSTR(nId);
            return ePres;
        }
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// class SvxPropSizeItem -------------------------------------------------

SvxPropSizeItem::SvxPropSizeItem( const sal_uInt16 nPercent, const sal_uInt16 nId ) :
    SfxUInt16Item( nId, nPercent )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxPropSizeItem::Clone( SfxItemPool * ) const
{
    return new SvxPropSizeItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxPropSizeItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm << (sal_uInt16) GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxPropSizeItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt16 nSize;
    rStrm >> nSize;
    return new SvxPropSizeItem( nSize, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxPropSizeItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    rText = OUString();
    return SFX_ITEM_PRESENTATION_NONE;
}

// class SvxColorItem ----------------------------------------------------

SvxColorItem::SvxColorItem( const sal_uInt16 nId ) :
    SfxPoolItem( nId ),
    mColor( COL_BLACK )
{
}

// -----------------------------------------------------------------------

SvxColorItem::SvxColorItem( const Color& rCol, const sal_uInt16 nId ) :
    SfxPoolItem( nId ),
    mColor( rCol )
{
}

// -----------------------------------------------------------------------

SvxColorItem::SvxColorItem( SvStream &rStrm, const sal_uInt16 nId ) :
    SfxPoolItem( nId )
{
    Color aColor;
    rStrm >> aColor;
    mColor = aColor;
}

// -----------------------------------------------------------------------

SvxColorItem::SvxColorItem( const SvxColorItem &rCopy ) :
    SfxPoolItem( rCopy ),
    mColor( rCopy.mColor )
{
}

// -----------------------------------------------------------------------

SvxColorItem::~SvxColorItem()
{
}

// -----------------------------------------------------------------------
sal_uInt16 SvxColorItem::GetVersion( sal_uInt16 nFFVer ) const
{
    DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
            SOFFICE_FILEFORMAT_40==nFFVer ||
            SOFFICE_FILEFORMAT_50==nFFVer,
            "SvxColorItem: Is there a new file format? ");
    return  SOFFICE_FILEFORMAT_50 >= nFFVer ? VERSION_USEAUTOCOLOR : 0;
}

// -----------------------------------------------------------------------

int SvxColorItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return  mColor == ( (const SvxColorItem&)rAttr ).mColor;
}

// -----------------------------------------------------------------------

bool SvxColorItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= (sal_Int32)(mColor.GetColor());
    return true;
}

// -----------------------------------------------------------------------

bool SvxColorItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    sal_Int32 nColor = 0;
    if(!(rVal >>= nColor))
        return sal_False;

    mColor.SetColor( nColor );
    return true;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxColorItem::Clone( SfxItemPool * ) const
{
    return new SvxColorItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxColorItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    if( VERSION_USEAUTOCOLOR == nItemVersion &&
        COL_AUTO == mColor.GetColor() )
        rStrm << Color( COL_BLACK );
    else
        rStrm << mColor;
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxColorItem::Create(SvStream& rStrm, sal_uInt16 /*nVer*/ ) const
{
    return new SvxColorItem( rStrm, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxColorItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = ::GetColorString( mColor );
            return ePres;
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

SvxCharSetColorItem::SvxCharSetColorItem( const Color& rCol,
                                          const rtl_TextEncoding _eFrom,
                                          const sal_uInt16 nId ) :
    SvxColorItem( rCol, nId ),

    eFrom( _eFrom )
{
}


// -----------------------------------------------------------------------

SfxPoolItem* SvxCharSetColorItem::Clone( SfxItemPool * ) const
{
    return new SvxCharSetColorItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxCharSetColorItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm << (sal_uInt8)GetSOStoreTextEncoding(GetCharSet()) << GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxCharSetColorItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 cSet;
    Color aColor;
    rStrm >> cSet >> aColor;
    return new SvxCharSetColorItem( aColor,  (rtl_TextEncoding)cSet, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxCharSetColorItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    rText = OUString();
    return SFX_ITEM_PRESENTATION_NONE;
}

// class SvxKerningItem --------------------------------------------------

SvxKerningItem::SvxKerningItem( const short nKern, const sal_uInt16 nId ) :
    SfxInt16Item( nId, nKern )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxKerningItem::Clone( SfxItemPool * ) const
{
    return new SvxKerningItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxKerningItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm << (short) GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

bool SvxKerningItem::ScaleMetrics( long nMult, long nDiv )
{
    SetValue( (sal_Int16)Scale( GetValue(), nMult, nDiv ) );
    return true;
}

// -----------------------------------------------------------------------

bool SvxKerningItem::HasMetrics() const
{
    return true;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxKerningItem::Create(SvStream& rStrm, sal_uInt16) const
{
    short nValue;
    rStrm >> nValue;
    return new SvxKerningItem( nValue, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxKerningItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = GetMetricText( (long)GetValue(), eCoreUnit, SFX_MAPUNIT_POINT, pIntl ) +
                    EE_RESSTR(GetMetricId(SFX_MAPUNIT_POINT));
            return ePres;
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = EE_RESSTR(RID_SVXITEMS_KERNING_COMPLETE);
            sal_uInt16 nId = 0;

            if ( GetValue() > 0 )
                nId = RID_SVXITEMS_KERNING_EXPANDED;
            else if ( GetValue() < 0 )
                nId = RID_SVXITEMS_KERNING_CONDENSED;

            if ( nId )
                rText += EE_RESSTR(nId);
            rText = rText +
                    GetMetricText( (long)GetValue(), eCoreUnit, SFX_MAPUNIT_POINT, pIntl ) +
                    EE_RESSTR(GetMetricId(SFX_MAPUNIT_POINT));
            return ePres;
        }
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

bool SvxKerningItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    sal_Int16 nVal = GetValue();
    if(nMemberId & CONVERT_TWIPS)
        nVal = (sal_Int16)TWIP_TO_MM100(nVal);
    rVal <<= nVal;
    return true;
}
// -----------------------------------------------------------------------
bool SvxKerningItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId)
{
    sal_Int16 nVal = sal_Int16();
    if(!(rVal >>= nVal))
        return false;
    if(nMemberId & CONVERT_TWIPS)
        nVal = (sal_Int16)MM100_TO_TWIP(nVal);
    SetValue(nVal);
    return true;
}

// class SvxCaseMapItem --------------------------------------------------

SvxCaseMapItem::SvxCaseMapItem( const SvxCaseMap eMap, const sal_uInt16 nId ) :
    SfxEnumItem( nId, (sal_uInt16)eMap )
{
}

// -----------------------------------------------------------------------

sal_uInt16 SvxCaseMapItem::GetValueCount() const
{
    return SVX_CASEMAP_END; // SVX_CASEMAP_KAPITAELCHEN + 1
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxCaseMapItem::Clone( SfxItemPool * ) const
{
    return new SvxCaseMapItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxCaseMapItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm << (sal_uInt8) GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxCaseMapItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 cMap;
    rStrm >> cMap;
    return new SvxCaseMapItem( (const SvxCaseMap)cMap, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxCaseMapItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetValueTextByPos( GetValue() );
            return ePres;
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

OUString SvxCaseMapItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos < (sal_uInt16)SVX_CASEMAP_END, "enum overflow!" );
    return EE_RESSTR(RID_SVXITEMS_CASEMAP_BEGIN + nPos);
}

bool SvxCaseMapItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    sal_Int16 nRet = style::CaseMap::NONE;
    switch( GetValue() )
    {
        case SVX_CASEMAP_VERSALIEN   :      nRet = style::CaseMap::UPPERCASE; break;
        case SVX_CASEMAP_GEMEINE     :      nRet = style::CaseMap::LOWERCASE; break;
        case SVX_CASEMAP_TITEL       :      nRet = style::CaseMap::TITLE    ; break;
        case SVX_CASEMAP_KAPITAELCHEN:      nRet = style::CaseMap::SMALLCAPS; break;
    }
    rVal <<= (sal_Int16)(nRet);
    return true;
}

bool SvxCaseMapItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    sal_uInt16 nVal = sal_uInt16();
    if(!(rVal >>= nVal))
        return sal_False;

    switch( nVal )
    {
    case style::CaseMap::NONE    :  nVal = SVX_CASEMAP_NOT_MAPPED  ; break;
    case style::CaseMap::UPPERCASE:  nVal = SVX_CASEMAP_VERSALIEN   ; break;
    case style::CaseMap::LOWERCASE:  nVal = SVX_CASEMAP_GEMEINE     ; break;
    case style::CaseMap::TITLE    :  nVal = SVX_CASEMAP_TITEL       ; break;
    case style::CaseMap::SMALLCAPS:  nVal = SVX_CASEMAP_KAPITAELCHEN; break;
    }
    SetValue(nVal);
    return true;
}

// class SvxEscapementItem -----------------------------------------------

SvxEscapementItem::SvxEscapementItem( const sal_uInt16 nId ) :
    SfxEnumItemInterface( nId ),

    nEsc    ( 0 ),
    nProp   ( 100 )
{
}

// -----------------------------------------------------------------------

SvxEscapementItem::SvxEscapementItem( const SvxEscapement eEscape,
                                      const sal_uInt16 nId ) :
    SfxEnumItemInterface( nId ),
    nProp( 100 )
{
    SetEscapement( eEscape );
    if( nEsc )
        nProp = 58;
}

// -----------------------------------------------------------------------

SvxEscapementItem::SvxEscapementItem( const short _nEsc,
                                      const sal_uInt8 _nProp,
                                      const sal_uInt16 nId ) :
    SfxEnumItemInterface( nId ),
    nEsc    ( _nEsc ),
    nProp   ( _nProp )
{
}

// -----------------------------------------------------------------------

int SvxEscapementItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return( nEsc  == ((SvxEscapementItem&)rAttr).nEsc &&
            nProp == ((SvxEscapementItem&)rAttr).nProp );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxEscapementItem::Clone( SfxItemPool * ) const
{
    return new SvxEscapementItem( *this );
}

// -----------------------------------------------------------------------

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
    rStrm << (sal_uInt8) GetProp()
          << (short) _nEsc;
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxEscapementItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 _nProp;
    short _nEsc;
    rStrm >> _nProp >> _nEsc;
    return new SvxEscapementItem( _nEsc, _nProp, Which() );
}

// -----------------------------------------------------------------------

sal_uInt16 SvxEscapementItem::GetValueCount() const
{
    return SVX_ESCAPEMENT_END;  // SVX_ESCAPEMENT_SUBSCRIPT + 1
}

//------------------------------------------------------------------------

SfxItemPresentation SvxEscapementItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = GetValueTextByPos( GetEnumValue() );

            if ( nEsc != 0 )
            {
                if( DFLT_ESC_AUTO_SUPER == nEsc || DFLT_ESC_AUTO_SUB == nEsc )
                    rText += EE_RESSTR(RID_SVXITEMS_ESCAPEMENT_AUTO);
                else
                    rText = rText + OUString::number( nEsc ) + "%";
            }
            return ePres;
        }
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

OUString SvxEscapementItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos < (sal_uInt16)SVX_ESCAPEMENT_END, "enum overflow!" );
    return EE_RESSTR(RID_SVXITEMS_ESCAPEMENT_BEGIN + nPos);
}

// -----------------------------------------------------------------------

sal_uInt16 SvxEscapementItem::GetEnumValue() const
{
    if ( nEsc < 0 )
        return SVX_ESCAPEMENT_SUBSCRIPT;
    else if ( nEsc > 0 )
        return SVX_ESCAPEMENT_SUPERSCRIPT;
    return SVX_ESCAPEMENT_OFF;
}

// -----------------------------------------------------------------------

void SvxEscapementItem::SetEnumValue( sal_uInt16 nVal )
{
    SetEscapement( (const SvxEscapement)nVal );
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
            rVal = Bool2Any(DFLT_ESC_AUTO_SUB == nEsc || DFLT_ESC_AUTO_SUPER == nEsc);
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
                return sal_False;
        }
        break;
        case MID_ESC_HEIGHT:
        {
            sal_Int8 nVal = sal_Int8();
            if( (rVal >>= nVal) && (nVal <= 100))
                nProp = nVal;
            else
                return sal_False;
        }
        break;
        case MID_AUTO_ESC:
        {
            sal_Bool bVal = Any2Bool(rVal);
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
    : SfxEnumItem( nId , eLang )
{
}

// -----------------------------------------------------------------------

sal_uInt16 SvxLanguageItem::GetValueCount() const
{
    // #i50205# got rid of class International
    SAL_WARN( "editeng.items", "SvxLanguageItem::GetValueCount: supposed to return a count of what?");
    // FIXME: previously returned LANGUAGE_COUNT from tools/intn.hxx which was wrong anyway.
    // Could be SvtLanguageTable::GetEntryCount() (all locales with resource string)?
    // Could be LocaleDataWrapper::getInstalledLanguageTypes() (all locales with locale data)?
    return 0;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxLanguageItem::Clone( SfxItemPool * ) const
{
    return new SvxLanguageItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxLanguageItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm << (sal_uInt16) GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxLanguageItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt16 nValue;
    rStrm >> nValue;
    return new SvxLanguageItem( (LanguageType)nValue, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxLanguageItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            SvtLanguageTable aLangTable;
            rText = aLangTable.GetString( (LanguageType)GetValue() );
            return ePres;
        }
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

bool SvxLanguageItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_LANG_INT:  // for basic conversions!
            rVal <<= (sal_Int16)(GetValue());
        break;
        case MID_LANG_LOCALE:
            lang::Locale aRet( LanguageTag( GetValue()).getLocale( false));
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

            SetValue((sal_Int16)nValue);
        }
        break;
        case MID_LANG_LOCALE:
        {
            lang::Locale aLocale;
            if(!(rVal >>= aLocale))
                return sal_False;

            SetValue( LanguageTag::convertToLanguageType( aLocale, false));
        }
        break;
    }
    return true;
}

// class SvxNoLinebreakItem ----------------------------------------------
SvxNoLinebreakItem::SvxNoLinebreakItem( const sal_Bool bBreak, const sal_uInt16 nId ) :
      SfxBoolItem( nId, bBreak )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxNoLinebreakItem::Clone( SfxItemPool* ) const
{
    return new SvxNoLinebreakItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxNoLinebreakItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm << (sal_Bool)GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxNoLinebreakItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_Bool bValue;
    rStrm >> bValue;
    return new SvxNoLinebreakItem( bValue, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxNoLinebreakItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    rText = OUString();
    return SFX_ITEM_PRESENTATION_NONE;
}

// class SvxNoHyphenItem -------------------------------------------------

SvxNoHyphenItem::SvxNoHyphenItem( const sal_Bool bHyphen, const sal_uInt16 nId ) :
    SfxBoolItem( nId , bHyphen )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxNoHyphenItem::Clone( SfxItemPool* ) const
{
    return new SvxNoHyphenItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxNoHyphenItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm << (sal_Bool) GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxNoHyphenItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Bool bValue;
    rStrm >> bValue;
    return new SvxNoHyphenItem( bValue, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxNoHyphenItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    rText = OUString();
    return SFX_ITEM_PRESENTATION_NONE;
}

/*
 * Dummy item for ToolBox controls:
 *
 */

// -----------------------------------------------------------------------
// class SvxLineColorItem (== SvxColorItem)
// -----------------------------------------------------------------------

SvxLineColorItem::SvxLineColorItem( const sal_uInt16 nId ) :
    SvxColorItem( nId )
{
}

// -----------------------------------------------------------------------

SvxLineColorItem::SvxLineColorItem( const SvxLineColorItem &rCopy ) :
    SvxColorItem( rCopy )
{
}

// -----------------------------------------------------------------------

SvxLineColorItem::~SvxLineColorItem()
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxLineColorItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    OUString&           rText,
    const IntlWrapper * pIntlWrapper
)   const
{
    return SvxColorItem::GetPresentation( ePres, eCoreUnit, ePresUnit,
                                          rText, pIntlWrapper );
}

// class SvxBlinkItem -------------------------------------------------


SvxBlinkItem::SvxBlinkItem( const sal_Bool bBlink, const sal_uInt16 nId ) :
    SfxBoolItem( nId, bBlink )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxBlinkItem::Clone( SfxItemPool * ) const
{
    return new SvxBlinkItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxBlinkItem::Store( SvStream& rStrm , sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm << (sal_uInt8) GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxBlinkItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_uInt8 nState;
    rStrm >> nState;
    return new SvxBlinkItem( nState, Which() );
}

// -----------------------------------------------------------------------

SfxItemPresentation SvxBlinkItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            sal_uInt16 nId = RID_SVXITEMS_BLINK_FALSE;

            if ( GetValue() )
                nId = RID_SVXITEMS_BLINK_TRUE;
            rText = EE_RESSTR(nId);
            return ePres;
        }
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// class SvxEmphaisMarkItem ---------------------------------------------------

SvxEmphasisMarkItem::SvxEmphasisMarkItem( const FontEmphasisMark nValue,
                                        const sal_uInt16 nId )
    : SfxUInt16Item( nId, nValue )
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxEmphasisMarkItem::Clone( SfxItemPool * ) const
{
    return new SvxEmphasisMarkItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxEmphasisMarkItem::Store( SvStream& rStrm,
                                     sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm << (sal_uInt16)GetValue();
    return rStrm;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxEmphasisMarkItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_uInt16 nValue;
    rStrm >> nValue;
    return new SvxEmphasisMarkItem( (FontEmphasisMark)nValue, Which() );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxEmphasisMarkItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper * /*pIntl*/
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return ePres;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            {
                sal_uInt16 nVal = GetValue();
                rText = EE_RESSTR( RID_SVXITEMS_EMPHASIS_BEGIN_STYLE +
                                        ( EMPHASISMARK_STYLE & nVal ));
                sal_uInt16 nId = ( EMPHASISMARK_POS_ABOVE & nVal )
                                ? RID_SVXITEMS_EMPHASIS_ABOVE_POS
                                : ( EMPHASISMARK_POS_BELOW & nVal )
                                    ? RID_SVXITEMS_EMPHASIS_BELOW_POS
                                    : 0;
                if( nId )
                    rText += EE_RESSTR( nId );
                return ePres;
            }
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

bool SvxEmphasisMarkItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
    case MID_EMPHASIS:
    {
        sal_Int16 nValue = GetValue();
        sal_Int16 nRet = 0;
        switch(nValue & EMPHASISMARK_STYLE)
        {
            case EMPHASISMARK_NONE   : nRet = FontEmphasis::NONE;           break;
            case EMPHASISMARK_DOT    : nRet = FontEmphasis::DOT_ABOVE;      break;
            case EMPHASISMARK_CIRCLE : nRet = FontEmphasis::CIRCLE_ABOVE;   break;
            case EMPHASISMARK_DISC   : nRet = FontEmphasis::DISK_ABOVE;     break;
            case EMPHASISMARK_ACCENT : nRet = FontEmphasis::ACCENT_ABOVE;   break;
        }
        if(nRet && nValue & EMPHASISMARK_POS_BELOW)
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
    sal_Bool bRet = true;
    switch( nMemberId )
    {
    case MID_EMPHASIS:
    {
        sal_Int32 nValue = -1;
        rVal >>= nValue;
        switch(nValue)
        {
            case FontEmphasis::NONE        : nValue = EMPHASISMARK_NONE;   break;
            case FontEmphasis::DOT_ABOVE   : nValue = EMPHASISMARK_DOT|EMPHASISMARK_POS_ABOVE;    break;
            case FontEmphasis::CIRCLE_ABOVE: nValue = EMPHASISMARK_CIRCLE|EMPHASISMARK_POS_ABOVE; break;
            case FontEmphasis::DISK_ABOVE  : nValue = EMPHASISMARK_DISC|EMPHASISMARK_POS_ABOVE;   break;
            case FontEmphasis::ACCENT_ABOVE: nValue = EMPHASISMARK_ACCENT|EMPHASISMARK_POS_ABOVE; break;
            case FontEmphasis::DOT_BELOW   : nValue = EMPHASISMARK_DOT|EMPHASISMARK_POS_BELOW;    break;
            case FontEmphasis::CIRCLE_BELOW: nValue = EMPHASISMARK_CIRCLE|EMPHASISMARK_POS_BELOW; break;
            case FontEmphasis::DISK_BELOW  : nValue = EMPHASISMARK_DISC|EMPHASISMARK_POS_BELOW;   break;
            case FontEmphasis::ACCENT_BELOW: nValue = EMPHASISMARK_ACCENT|EMPHASISMARK_POS_BELOW; break;
            default: return false;
        }
        SetValue( (sal_Int16)nValue );
    }
    break;
    }
    return bRet;
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

SvxTwoLinesItem::SvxTwoLinesItem( sal_Bool bFlag, sal_Unicode nStartBracket,
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

int SvxTwoLinesItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rAttr ), "not equal attribute types" );
    return bOn == ((SvxTwoLinesItem&)rAttr).bOn &&
           cStartBracket == ((SvxTwoLinesItem&)rAttr).cStartBracket &&
           cEndBracket == ((SvxTwoLinesItem&)rAttr).cEndBracket;
}

SfxPoolItem* SvxTwoLinesItem::Clone( SfxItemPool* ) const
{
    return new SvxTwoLinesItem( *this );
}

bool SvxTwoLinesItem::QueryValue( com::sun::star::uno::Any& rVal,
                                sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = true;
    switch( nMemberId )
    {
    case MID_TWOLINES:
        rVal = Bool2Any( bOn );
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

bool SvxTwoLinesItem::PutValue( const com::sun::star::uno::Any& rVal,
                                    sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_False;
    OUString s;
    switch( nMemberId )
    {
    case MID_TWOLINES:
        bOn = Any2Bool( rVal );
        bRet = sal_True;
        break;
    case MID_START_BRACKET:
        if( rVal >>= s )
        {
            cStartBracket = s.isEmpty() ? 0 : s[ 0 ];
            bRet = sal_True;
        }
        break;
    case MID_END_BRACKET:
        if( rVal >>= s )
        {
            cEndBracket = s.isEmpty() ? 0 : s[ 0 ];
            bRet = sal_True;
        }
        break;
    }
    return bRet;
}

SfxItemPresentation SvxTwoLinesItem::GetPresentation( SfxItemPresentation ePres,
                            SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/,
                            OUString &rText, const IntlWrapper* /*pIntl*/ ) const
{
    switch( ePres )
    {
    case SFX_ITEM_PRESENTATION_NONE:
        rText = OUString();
        break;
    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if( !GetValue() )
                rText = EE_RESSTR( RID_SVXITEMS_TWOLINES_OFF );
            else
            {
                rText = EE_RESSTR( RID_SVXITEMS_TWOLINES );
                if( GetStartBracket() )
                    rText = OUString(GetStartBracket()) + rText;
                if( GetEndBracket() )
                    rText += OUString(GetEndBracket());
            }
            return ePres;
        }
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}


SfxPoolItem* SvxTwoLinesItem::Create( SvStream & rStrm, sal_uInt16 /*nVer*/) const
{
    sal_Bool _bOn;
    sal_Unicode cStart, cEnd;
    rStrm >> _bOn >> cStart >> cEnd;
    return new SvxTwoLinesItem( _bOn, cStart, cEnd, Which() );
}

SvStream& SvxTwoLinesItem::Store(SvStream & rStrm, sal_uInt16 /*nIVer*/) const
{
    rStrm << GetValue() << GetStartBracket() << GetEndBracket();
    return rStrm;
}

sal_uInt16 SvxTwoLinesItem::GetVersion( sal_uInt16 nFFVer ) const
{
    DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
            SOFFICE_FILEFORMAT_40==nFFVer ||
            SOFFICE_FILEFORMAT_50==nFFVer,
            "SvxTwoLinesItem: Gibt es ein neues Fileformat?" );

    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}


/*************************************************************************
|*    class SvxCharRotateItem
*************************************************************************/

SvxCharRotateItem::SvxCharRotateItem( sal_uInt16 nValue,
                                       sal_Bool bFitIntoLine,
                                       const sal_uInt16 nW )
    : SfxUInt16Item( nW, nValue ), bFitToLine( bFitIntoLine )
{
}

SfxPoolItem* SvxCharRotateItem::Clone( SfxItemPool* ) const
{
    return new SvxCharRotateItem( GetValue(), IsFitToLine(), Which() );
}

SfxPoolItem* SvxCharRotateItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_uInt16 nVal;
    sal_Bool b;
    rStrm >> nVal >> b;
    return new SvxCharRotateItem( nVal, b, Which() );
}

SvStream& SvxCharRotateItem::Store( SvStream & rStrm, sal_uInt16 ) const
{
    sal_Bool bFlag = IsFitToLine();
    rStrm << GetValue() << bFlag;
    return rStrm;
}

sal_uInt16 SvxCharRotateItem::GetVersion( sal_uInt16 nFFVer ) const
{
    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}

SfxItemPresentation SvxCharRotateItem::GetPresentation(
        SfxItemPresentation ePres,
        SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/,
        OUString &rText, const IntlWrapper*  ) const
{
    switch( ePres )
    {
    case SFX_ITEM_PRESENTATION_NONE:
        rText = OUString();
        break;
    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if( !GetValue() )
                rText = EE_RESSTR( RID_SVXITEMS_CHARROTATE_OFF );
            else
            {
                rText = EE_RESSTR( RID_SVXITEMS_CHARROTATE );
                rText = rText.replaceFirst( "$(ARG1)",
                            OUString::number( GetValue() / 10 ));
                if( IsFitToLine() )
                    rText += EE_RESSTR( RID_SVXITEMS_CHARROTATE_FITLINE );
            }
            return ePres;
        }
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

bool SvxCharRotateItem::QueryValue( com::sun::star::uno::Any& rVal,
                                sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch( nMemberId )
    {
    case MID_ROTATE:
        rVal <<= (sal_Int16)GetValue();
        break;
    case MID_FITTOLINE:
        rVal = Bool2Any( IsFitToLine() );
        break;
    default:
        bRet = false;
        break;
    }
    return bRet;
}

bool SvxCharRotateItem::PutValue( const com::sun::star::uno::Any& rVal,
                                    sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = true;
    switch( nMemberId )
    {
    case MID_ROTATE:
        {
            sal_Int16 nVal = 0;
            if((rVal >>= nVal) && (0 == nVal || 900 == nVal || 2700 == nVal))
                SetValue( (sal_uInt16)nVal );
            else
                bRet = sal_False;
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

int SvxCharRotateItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return SfxUInt16Item::operator==( rItem ) &&
           IsFitToLine() == ((const SvxCharRotateItem&)rItem).IsFitToLine();
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
    rStrm >> nVal;
    SvxCharScaleWidthItem* pItem = new SvxCharScaleWidthItem( nVal, Which() );

    if ( Which() == EE_CHAR_FONTWIDTH )
    {
        // Was a SvxFontWidthItem in 5.2
        // sal_uInt16 nFixWidth, sal_uInt16 nPropWidth.
        // nFixWidth has never been used...
        rStrm >> nVal;
        sal_uInt16 nTest;
        rStrm >> nTest;
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
        rRet << (sal_uInt16)0;
        rRet << GetValue();
        // Really ugly, but not a problem for reading the doc in 5.2
        rRet << (sal_uInt16)0x1234;
    }
    return rRet;
}


sal_uInt16 SvxCharScaleWidthItem::GetVersion( sal_uInt16 nFFVer ) const
{
    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}

SfxItemPresentation SvxCharScaleWidthItem::GetPresentation(
        SfxItemPresentation ePres,
        SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/,
        OUString &rText, const IntlWrapper*  ) const
{
    switch( ePres )
    {
    case SFX_ITEM_PRESENTATION_NONE:
        rText = OUString();
        break;
    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            if( !GetValue() )
                rText = EE_RESSTR( RID_SVXITEMS_CHARSCALE_OFF );
            else
            {
                rText = EE_RESSTR( RID_SVXITEMS_CHARSCALE );
                rText = rText.replaceFirst( "$(ARG1)",
                            OUString::number( GetValue() ));
            }
            return ePres;
        }
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
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

    OSL_TRACE( "SvxCharScaleWidthItem::PutValue - Wrong type!" );
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
    : SfxEnumItem( nId, (sal_uInt16)eValue )
{
}

SfxPoolItem* SvxCharReliefItem::Clone( SfxItemPool * ) const
{
    return new SvxCharReliefItem( *this );
}

SfxPoolItem* SvxCharReliefItem::Create(SvStream & rStrm, sal_uInt16) const
{
    sal_uInt16 nVal;
    rStrm >> nVal;
    return new SvxCharReliefItem( (FontRelief)nVal, Which() );
}

SvStream& SvxCharReliefItem::Store(SvStream & rStrm, sal_uInt16 /*nIVer*/) const
{
    sal_uInt16 nVal = GetValue();
    rStrm << nVal;
    return rStrm;
}

sal_uInt16 SvxCharReliefItem::GetVersion( sal_uInt16 nFFVer ) const
{
    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}

OUString SvxCharReliefItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( RID_SVXITEMS_RELIEF_ENGRAVED - RID_SVXITEMS_RELIEF_NONE,
                    "enum overflow" );
    return EE_RESSTR(RID_SVXITEMS_RELIEF_BEGIN + nPos);
}

sal_uInt16 SvxCharReliefItem::GetValueCount() const
{
    return RID_SVXITEMS_RELIEF_ENGRAVED - RID_SVXITEMS_RELIEF_NONE;
}

SfxItemPresentation SvxCharReliefItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * /*pIntl*/
)   const
{
    SfxItemPresentation eRet = ePres;
    switch( ePres )
    {
    case SFX_ITEM_PRESENTATION_NONE:
        rText = OUString();
        break;

    case SFX_ITEM_PRESENTATION_NAMELESS:
    case SFX_ITEM_PRESENTATION_COMPLETE:
        rText = GetValueTextByPos( GetValue() );
        break;

    default:
        eRet = SFX_ITEM_PRESENTATION_NONE;
    }
    return eRet;
}

bool SvxCharReliefItem::PutValue( const com::sun::star::uno::Any& rVal,
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
            if(nVal >= 0 && nVal <= RELIEF_ENGRAVED)
                SetValue( (sal_uInt16)nVal );
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

bool SvxCharReliefItem::QueryValue( com::sun::star::uno::Any& rVal,
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

SvxScriptTypeItem::SvxScriptTypeItem( sal_uInt16 nType )
    : SfxUInt16Item( SID_ATTR_CHAR_SCRIPTTYPE, nType )
{
}
SfxPoolItem* SvxScriptTypeItem::Clone( SfxItemPool * ) const
{
    return new SvxScriptTypeItem( GetValue() );
}

/*************************************************************************
|*    class SvxScriptSetItem
*************************************************************************/

SvxScriptSetItem::SvxScriptSetItem( sal_uInt16 nSlotId, SfxItemPool& rPool )
    : SfxSetItem( nSlotId, new SfxItemSet( rPool,
                        SID_ATTR_CHAR_FONT, SID_ATTR_CHAR_FONT ))
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
    p->GetItemSet().Put( GetItemSet(), sal_False );
    return p;
}

SfxPoolItem* SvxScriptSetItem::Create( SvStream &, sal_uInt16 ) const
{
    return 0;
}

const SfxPoolItem* SvxScriptSetItem::GetItemOfScriptSet(
                            const SfxItemSet& rSet, sal_uInt16 nId )
{
    const SfxPoolItem* pI;
    SfxItemState eSt = rSet.GetItemState( nId, sal_False, &pI );
    if( SFX_ITEM_SET != eSt )
        pI = SFX_ITEM_DEFAULT == eSt ? &rSet.Get( nId ) : 0;
    return pI;
}

const SfxPoolItem* SvxScriptSetItem::GetItemOfScript( sal_uInt16 nSlotId, const SfxItemSet& rSet, sal_uInt16 nScript )
{
    sal_uInt16 nLatin, nAsian, nComplex;
    GetWhichIds( nSlotId, rSet, nLatin, nAsian, nComplex );

    const SfxPoolItem *pRet, *pAsn, *pCmplx;
    switch( nScript )
    {
    default:                //no one valid -> match to latin
    //  case SCRIPTTYPE_LATIN:
        pRet = GetItemOfScriptSet( rSet, nLatin );
        break;
    case SCRIPTTYPE_ASIAN:
        pRet = GetItemOfScriptSet( rSet, nAsian );
        break;
    case SCRIPTTYPE_COMPLEX:
        pRet = GetItemOfScriptSet( rSet, nComplex );
        break;

    case SCRIPTTYPE_LATIN|SCRIPTTYPE_ASIAN:
        if( 0 == (pRet = GetItemOfScriptSet( rSet, nLatin )) ||
            0 == (pAsn = GetItemOfScriptSet( rSet, nAsian )) ||
            *pRet != *pAsn )
            pRet = 0;
        break;

    case SCRIPTTYPE_LATIN|SCRIPTTYPE_COMPLEX:
        if( 0 == (pRet = GetItemOfScriptSet( rSet, nLatin )) ||
            0 == (pCmplx = GetItemOfScriptSet( rSet, nComplex )) ||
            *pRet != *pCmplx )
            pRet = 0;
        break;

    case SCRIPTTYPE_ASIAN|SCRIPTTYPE_COMPLEX:
        if( 0 == (pRet = GetItemOfScriptSet( rSet, nAsian )) ||
            0 == (pCmplx = GetItemOfScriptSet( rSet, nComplex )) ||
            *pRet != *pCmplx )
            pRet = 0;
        break;

    case SCRIPTTYPE_LATIN|SCRIPTTYPE_ASIAN|SCRIPTTYPE_COMPLEX:
        if( 0 == (pRet = GetItemOfScriptSet( rSet, nLatin )) ||
            0 == (pAsn = GetItemOfScriptSet( rSet, nAsian )) ||
            0 == (pCmplx = GetItemOfScriptSet( rSet, nComplex )) ||
            *pRet != *pAsn || *pRet != *pCmplx )
            pRet = 0;
        break;
    }
    return pRet;
}

const SfxPoolItem* SvxScriptSetItem::GetItemOfScript( sal_uInt16 nScript ) const
{
    return GetItemOfScript( Which(), GetItemSet(), nScript );
}

void SvxScriptSetItem::PutItemForScriptType( sal_uInt16 nScriptType,
                                             const SfxPoolItem& rItem )
{
    sal_uInt16 nLatin, nAsian, nComplex;
    GetWhichIds( nLatin, nAsian, nComplex );

    SfxPoolItem* pCpy = rItem.Clone();
    if( SCRIPTTYPE_LATIN & nScriptType )
    {
        pCpy->SetWhich( nLatin );
        GetItemSet().Put( *pCpy );
    }
    if( SCRIPTTYPE_ASIAN & nScriptType )
    {
        pCpy->SetWhich( nAsian );
        GetItemSet().Put( *pCpy );
    }
    if( SCRIPTTYPE_COMPLEX & nScriptType )
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
        DBG_ASSERT( sal_False, "wrong SlotId for class SvxScriptSetItem" );
        // no break - default to font - Id Range !!

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
        sal_uInt16 nFontType;
        sal_uInt16 nLanguage;
    }
    aOutTypeArr[ nItemCnt ] =
    {
        {  DEFAULTFONT_LATIN_TEXT, LANGUAGE_ENGLISH_US },
        {  DEFAULTFONT_CJK_TEXT, LANGUAGE_ENGLISH_US },
        {  DEFAULTFONT_CTL_TEXT, LANGUAGE_ARABIC_SAUDI_ARABIA }
    };

    SvxFontItem* aItemArr[ nItemCnt ] = { &rLatin, &rAsian, &rComplex };

    for ( sal_uInt16 n = 0; n < nItemCnt; ++n )
    {
        Font aFont( OutputDevice::GetDefaultFont( aOutTypeArr[ n ].nFontType,
                                                  aOutTypeArr[ n ].nLanguage,
                                                  DEFAULTFONT_FLAGS_ONLYONE, 0 ) );
        SvxFontItem* pItem = aItemArr[ n ];
        pItem->SetFamily( aFont.GetFamily() );
        pItem->SetFamilyName( aFont.GetName() );
        pItem->SetStyleName( String() );
        pItem->SetPitch( aFont.GetPitch());
        pItem->SetCharSet(aFont.GetCharSet());
    }
}


sal_uInt16 GetI18NScriptTypeOfLanguage( sal_uInt16 nLang )
{
    return GetI18NScriptType( SvtLanguageOptions::GetScriptTypeOfLanguage( nLang ) );
}

sal_uInt16 GetItemScriptType( short nI18NType )
{
    switch ( nI18NType )
    {
        case i18n::ScriptType::LATIN:   return SCRIPTTYPE_LATIN;
        case i18n::ScriptType::ASIAN:   return SCRIPTTYPE_ASIAN;
        case i18n::ScriptType::COMPLEX: return SCRIPTTYPE_COMPLEX;
    }
    return 0;
}

short GetI18NScriptType( sal_uInt16 nItemType )
{
    switch ( nItemType )
    {
        case SCRIPTTYPE_LATIN:      return i18n::ScriptType::LATIN;
        case SCRIPTTYPE_ASIAN:      return i18n::ScriptType::ASIAN;
        case SCRIPTTYPE_COMPLEX:    return i18n::ScriptType::COMPLEX;
    }
    return 0;
}

bool SvxRsidItem::QueryValue( uno::Any& rVal, sal_uInt8 ) const
{
    rVal <<= ( (sal_uInt32)GetValue() );
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
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
