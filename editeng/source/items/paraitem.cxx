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

#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <libxml/xmlwriter.h>
#include <comphelper/extract.hxx>
#include <osl/diagnose.h>
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
#include <tools/mapunit.hxx>
#include <tools/UnitConversion.hxx>
#include <svl/itempool.hxx>
#include <svl/memberid.h>
#include <editeng/editrids.hrc>
#include <editeng/lspcitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/pmdlitem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <editeng/hngpnctitem.hxx>
#include <editeng/forbiddenruleitem.hxx>
#include <editeng/paravertalignitem.hxx>
#include <editeng/pgrditem.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <editeng/memberids.h>
#include <editeng/itemtype.hxx>
#include <editeng/eerdll.hxx>

using namespace ::com::sun::star;


SfxPoolItem* SvxLineSpacingItem::CreateDefault() { return new  SvxLineSpacingItem(LINE_SPACE_DEFAULT_HEIGHT, 0);}
SfxPoolItem* SvxAdjustItem::CreateDefault() { return new  SvxAdjustItem(SvxAdjust::Left, 0);}
SfxPoolItem* SvxWidowsItem::CreateDefault() { return new  SvxWidowsItem(0, 0);}
SfxPoolItem* SvxOrphansItem::CreateDefault() { return new  SvxOrphansItem(0, 0);}
SfxPoolItem* SvxHyphenZoneItem::CreateDefault() { return new  SvxHyphenZoneItem(false, 0);}
SfxPoolItem* SvxTabStopItem::CreateDefault() { return new  SvxTabStopItem(0);}
SfxPoolItem* SvxFormatSplitItem::CreateDefault() { return new  SvxFormatSplitItem(false, 0);}
SfxPoolItem* SvxPageModelItem::CreateDefault() { return new  SvxPageModelItem(TypedWhichId<SvxPageModelItem>(0));}
SfxPoolItem* SvxParaVertAlignItem::CreateDefault() { return new  SvxParaVertAlignItem(Align::Automatic, TypedWhichId<SvxParaVertAlignItem>(0));}

namespace {

enum class SvxSpecialLineSpace
{
    User,
    OneLine,
    OnePointFiveLines,
    TwoLines,
    End
};

}

SvxLineSpacingItem::SvxLineSpacingItem( sal_uInt16 nHeight, const sal_uInt16 nId )
    : SfxEnumItemInterface( nId )
{
    nPropLineSpace = 100;
    nInterLineSpace = 0;
    nLineHeight = nHeight;
    eLineSpaceRule = SvxLineSpaceRule::Auto;
    eInterLineSpaceRule = SvxInterLineSpaceRule::Off;
}


bool SvxLineSpacingItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    const SvxLineSpacingItem& rLineSpace = static_cast<const SvxLineSpacingItem&>(rAttr);
    return
        // Same Linespacing Rule?
        (eLineSpaceRule == rLineSpace.eLineSpaceRule)
        // For maximum and minimum Linespacing be the size must coincide.
        && (eLineSpaceRule == SvxLineSpaceRule::Auto ||
            nLineHeight == rLineSpace.nLineHeight)
        // Same Linespacing Rule?
        && ( eInterLineSpaceRule == rLineSpace.eInterLineSpaceRule )
        // Either set proportional or additive.
        && (( eInterLineSpaceRule == SvxInterLineSpaceRule::Off)
            || (eInterLineSpaceRule == SvxInterLineSpaceRule::Prop
                && nPropLineSpace == rLineSpace.nPropLineSpace)
            || (eInterLineSpaceRule == SvxInterLineSpaceRule::Fix
                && (nInterLineSpace == rLineSpace.nInterLineSpace)));
}

/* Who does still know why the LineSpacingItem is so complicated?
   We can not use it for UNO since there are only two values:
      - a sal_uInt16 for the mode
      - a sal_uInt32 for all values (distance, height, rel. detail)
*/
bool SvxLineSpacingItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    style::LineSpacing aLSp;
    switch( eLineSpaceRule )
    {
        case SvxLineSpaceRule::Auto:
            if(eInterLineSpaceRule == SvxInterLineSpaceRule::Fix)
            {
                aLSp.Mode = style::LineSpacingMode::LEADING;
                aLSp.Height = ( bConvert ? static_cast<short>(convertTwipToMm100(nInterLineSpace)) : nInterLineSpace);
            }
            else if(eInterLineSpaceRule == SvxInterLineSpaceRule::Off)
            {
                aLSp.Mode = style::LineSpacingMode::PROP;
                aLSp.Height = 100;
            }
            else
            {
                aLSp.Mode = style::LineSpacingMode::PROP;
                aLSp.Height = nPropLineSpace;
            }
        break;
        case SvxLineSpaceRule::Fix :
        case SvxLineSpaceRule::Min :
            aLSp.Mode = eLineSpaceRule == SvxLineSpaceRule::Fix ? style::LineSpacingMode::FIX : style::LineSpacingMode::MINIMUM;
            aLSp.Height = ( bConvert ? static_cast<short>(convertTwipToMm100(nLineHeight)) : nLineHeight );
        break;
        default:
            ;//prevent warning about SvxLineSpaceRule::End
    }

    switch ( nMemberId )
    {
        case 0 :                rVal <<= aLSp; break;
        case MID_LINESPACE :    rVal <<= aLSp.Mode; break;
        case MID_HEIGHT :       rVal <<= aLSp.Height; break;
        default: OSL_FAIL("Wrong MemberId!"); break;
    }

    return true;
}

bool SvxLineSpacingItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    // fill with current data
    style::LineSpacing aLSp;
    uno::Any aAny;
    bool bRet = QueryValue( aAny, bConvert ? CONVERT_TWIPS : 0 ) && ( aAny >>= aLSp );

    // get new data
    switch ( nMemberId )
    {
        case 0 :                bRet = (rVal >>= aLSp); break;
        case MID_LINESPACE :    bRet = (rVal >>= aLSp.Mode); break;
        case MID_HEIGHT :       bRet = (rVal >>= aLSp.Height); break;
        default: OSL_FAIL("Wrong MemberId!"); break;
    }

    if( bRet )
    {
        nLineHeight = aLSp.Height;
        switch( aLSp.Mode )
        {
            case style::LineSpacingMode::LEADING:
            {
                eInterLineSpaceRule = SvxInterLineSpaceRule::Fix;
                eLineSpaceRule = SvxLineSpaceRule::Auto;
                nInterLineSpace = aLSp.Height;
                if(bConvert)
                    nInterLineSpace = o3tl::toTwips(nInterLineSpace, o3tl::Length::mm100);

            }
            break;
            case style::LineSpacingMode::PROP:
            {
                eLineSpaceRule = SvxLineSpaceRule::Auto;
                nPropLineSpace = aLSp.Height;
                if(100 == aLSp.Height)
                    eInterLineSpaceRule = SvxInterLineSpaceRule::Off;
                else
                    eInterLineSpaceRule = SvxInterLineSpaceRule::Prop;
            }
            break;
            case style::LineSpacingMode::FIX:
            case style::LineSpacingMode::MINIMUM:
            {
                eInterLineSpaceRule =  SvxInterLineSpaceRule::Off;
                eLineSpaceRule = aLSp.Mode == style::LineSpacingMode::FIX ? SvxLineSpaceRule::Fix : SvxLineSpaceRule::Min;
                nLineHeight = aLSp.Height;
                if(bConvert)
                    nLineHeight = o3tl::toTwips(nLineHeight, o3tl::Length::mm100);
            }
            break;
        }
    }

    return bRet;
}

SvxLineSpacingItem* SvxLineSpacingItem::Clone( SfxItemPool * ) const
{
    return new SvxLineSpacingItem( *this );
}

bool SvxLineSpacingItem::GetPresentation
(
    SfxItemPresentation ePres,
    MapUnit             eCoreUnit,
    MapUnit             ePresUnit,
    OUString&           rText, const IntlWrapper& rIntl
)   const
{
    switch ( ePres )
    {
        case SfxItemPresentation::Nameless:
        case SfxItemPresentation::Complete:
        {
            switch( GetLineSpaceRule() )
            {
                case SvxLineSpaceRule::Auto:
                {
                    SvxInterLineSpaceRule eInter = GetInterLineSpaceRule();

                    switch( eInter )
                    {
                        // Default single line spacing
                        case SvxInterLineSpaceRule::Off:
                            rText = EditResId(RID_SVXITEMS_LINESPACING_SINGLE);
                            break;

                        // Default single line spacing
                        case SvxInterLineSpaceRule::Prop:
                            if ( 100 == GetPropLineSpace() )
                            {
                                rText = EditResId(RID_SVXITEMS_LINESPACING_SINGLE);
                                break;
                            }
                            // 1.15 line spacing
                            if ( 115 == GetPropLineSpace() )
                            {
                                rText = EditResId(RID_SVXITEMS_LINESPACING_115);
                                break;
                            }
                            // 1.5 line spacing
                            if ( 150 == GetPropLineSpace() )
                            {
                                rText = EditResId(RID_SVXITEMS_LINESPACING_15);
                                break;
                            }
                            // double line spacing
                            if ( 200 == GetPropLineSpace() )
                            {
                                rText = EditResId(RID_SVXITEMS_LINESPACING_DOUBLE);
                                break;
                            }
                            // the set per cent value
                            rText = EditResId(RID_SVXITEMS_LINESPACING_PROPORTIONAL) + " " + OUString::number(GetPropLineSpace()) + "%";
                            break;

                        case SvxInterLineSpaceRule::Fix:
                            rText = EditResId(RID_SVXITEMS_LINESPACING_LEADING)  +
                                    " " + GetMetricText(GetInterLineSpace(), eCoreUnit, ePresUnit, &rIntl) +
                                    " " + EditResId(GetMetricId(ePresUnit));
                            break;
                        default: ;//prevent warning
                    }
                }
                break;
                case SvxLineSpaceRule::Fix:
                    rText = EditResId(RID_SVXITEMS_LINESPACING_FIXED)  +
                            " " + GetMetricText(GetLineHeight(), eCoreUnit, ePresUnit, &rIntl) +
                            " " + EditResId(GetMetricId(ePresUnit));
                    break;

                case SvxLineSpaceRule::Min:
                    rText = EditResId(RID_SVXITEMS_LINESPACING_MIN) +
                            " " + GetMetricText(GetLineHeight(), eCoreUnit, ePresUnit, &rIntl) +
                            " " + EditResId(GetMetricId(ePresUnit));
                    break;
                default: ;//prevent warning
            }
        }
    }
    return true;
}

sal_uInt16 SvxLineSpacingItem::GetValueCount() const
{
    return sal_uInt16(SvxSpecialLineSpace::End);   // SvxSpecialLineSpace::TwoLines + 1
}


sal_uInt16 SvxLineSpacingItem::GetEnumValue() const
{
    SvxSpecialLineSpace nVal;
    switch ( nPropLineSpace )
    {
        case 100:   nVal = SvxSpecialLineSpace::OneLine;            break;
        case 150:   nVal = SvxSpecialLineSpace::OnePointFiveLines;  break;
        case 200:   nVal = SvxSpecialLineSpace::TwoLines;           break;
        default:    nVal = SvxSpecialLineSpace::User;               break;
    }
    return static_cast<sal_uInt16>(nVal);
}


void SvxLineSpacingItem::SetEnumValue( sal_uInt16 nVal )
{
    switch ( static_cast<SvxSpecialLineSpace>(nVal) )
    {
        case SvxSpecialLineSpace::OneLine:           nPropLineSpace = 100; break;
        case SvxSpecialLineSpace::OnePointFiveLines: nPropLineSpace = 150; break;
        case SvxSpecialLineSpace::TwoLines:          nPropLineSpace = 200; break;
        default: break;
    }
}

// class SvxAdjustItem ---------------------------------------------------

ItemInstanceManager* SvxAdjustItem::getItemInstanceManager() const
{
    static DefaultItemInstanceManager aInstanceManager(typeid(SvxAdjustItem).hash_code());
    return &aInstanceManager;
}

SvxAdjustItem::SvxAdjustItem(const SvxAdjust eAdjst, const sal_uInt16 nId )
    : SfxEnumItemInterface( nId ),
    bOneBlock( false ), bLastCenter( false ), bLastBlock( false )
{
    SetAdjust( eAdjst );
}


bool SvxAdjustItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    const SvxAdjustItem& rItem = static_cast<const SvxAdjustItem&>(rAttr);
    return GetAdjust() == rItem.GetAdjust() &&
           bOneBlock == rItem.bOneBlock &&
           bLastCenter == rItem.bLastCenter &&
           bLastBlock == rItem.bLastBlock;
}

bool SvxAdjustItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_PARA_ADJUST      : rVal <<= static_cast<sal_Int16>(GetAdjust()); break;
        case MID_LAST_LINE_ADJUST : rVal <<= static_cast<sal_Int16>(GetLastBlock()); break;
        case MID_EXPAND_SINGLE    :
        {
            rVal <<= bOneBlock;
            break;
        }
        default: ;//prevent warning
    }
    return true;
}

bool SvxAdjustItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_PARA_ADJUST              :
        case MID_LAST_LINE_ADJUST :
        {
            sal_Int32 eVal = - 1;
            ::cppu::enum2int(eVal,rVal);
            if(eVal >= 0 && eVal <= 4)
            {
                SvxAdjust eAdjust = static_cast<SvxAdjust>(eVal);
                if(MID_LAST_LINE_ADJUST == nMemberId &&
                    eAdjust != SvxAdjust::Left &&
                    eAdjust != SvxAdjust::Block &&
                    eAdjust != SvxAdjust::Center)
                        return false;
                nMemberId == MID_PARA_ADJUST ? SetAdjust(eAdjust) : SetLastBlock(eAdjust);
            }
        }
        break;
        case MID_EXPAND_SINGLE :
            ASSERT_CHANGE_REFCOUNTED_ITEM;
            bOneBlock = Any2Bool(rVal);
            break;
    }
    return true;
}

SvxAdjustItem* SvxAdjustItem::Clone( SfxItemPool * ) const
{
    return new SvxAdjustItem( *this );
}

bool SvxAdjustItem::GetPresentation
(
    SfxItemPresentation ePres,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    switch ( ePres )
    {
        case SfxItemPresentation::Nameless:
        case SfxItemPresentation::Complete:
            rText = GetValueTextByPos( static_cast<sal_uInt16>(GetAdjust()) );
            return true;
        default: ;//prevent warning
    }
    return false;
}


sal_uInt16 SvxAdjustItem::GetValueCount() const
{
    return sal_uInt16(SvxAdjust::End);  // SvxAdjust::BlockLine + 1
}

OUString SvxAdjustItem::GetValueTextByPos( sal_uInt16 nPos )
{
    static TranslateId RID_SVXITEMS_ADJUST[] =
    {
        RID_SVXITEMS_ADJUST_LEFT,
        RID_SVXITEMS_ADJUST_RIGHT,
        RID_SVXITEMS_ADJUST_BLOCK,
        RID_SVXITEMS_ADJUST_CENTER,
        RID_SVXITEMS_ADJUST_BLOCKLINE
    };
    static_assert(SAL_N_ELEMENTS(RID_SVXITEMS_ADJUST) - 1 == size_t(SvxAdjust::BlockLine), "unexpected size");
    assert(nPos <= sal_uInt16(SvxAdjust::BlockLine) && "enum overflow!");
    return EditResId(RID_SVXITEMS_ADJUST[nPos]);
}

sal_uInt16 SvxAdjustItem::GetEnumValue() const
{
    return static_cast<sal_uInt16>(GetAdjust());
}


void SvxAdjustItem::SetEnumValue( sal_uInt16 nVal )
{
    SetAdjust( static_cast<SvxAdjust>(nVal) );
}


// class SvxWidowsItem ---------------------------------------------------

SvxWidowsItem::SvxWidowsItem(const sal_uInt8 nL, const sal_uInt16 nId ) :
    SfxByteItem( nId, nL )
{
}

SvxWidowsItem* SvxWidowsItem::Clone( SfxItemPool * ) const
{
    return new SvxWidowsItem( *this );
}

bool SvxWidowsItem::GetPresentation
(
    SfxItemPresentation ePres,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    switch ( ePres )
    {
        case SfxItemPresentation::Nameless:
        {
            rText = EditResId(RID_SVXITEMS_LINES);
            break;
        }

        case SfxItemPresentation::Complete:
        {
            rText = EditResId(RID_SVXITEMS_WIDOWS_COMPLETE) + " " + EditResId(RID_SVXITEMS_LINES);
            break;
        }

        default:
        {
            SAL_WARN( "editeng.items", "SvxWidowsItem::GetPresentation(): unknown SfxItemPresentation" );
        }
    }

    rText = rText.replaceFirst( "%1", OUString::number( GetValue() ) );
    return true;
}

// class SvxOrphansItem --------------------------------------------------

SvxOrphansItem::SvxOrphansItem(const sal_uInt8 nL, const sal_uInt16 nId ) :
    SfxByteItem( nId, nL )
{
}

SvxOrphansItem* SvxOrphansItem::Clone( SfxItemPool * ) const
{
    return new SvxOrphansItem( *this );
}

bool SvxOrphansItem::GetPresentation
(
    SfxItemPresentation ePres,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    switch ( ePres )
    {
        case SfxItemPresentation::Nameless:
        {
            rText = EditResId(RID_SVXITEMS_LINES);
            break;
        }

        case SfxItemPresentation::Complete:
        {
            rText = EditResId(RID_SVXITEMS_ORPHANS_COMPLETE) + " " + EditResId(RID_SVXITEMS_LINES);
            break;
        }

        default:
        {
            SAL_WARN( "editeng.items", "SvxOrphansItem::GetPresentation(): unknown SfxItemPresentation" );
        }
    }

    rText = rText.replaceFirst( "%1", OUString::number( GetValue() ) );
    return true;
}

// class SvxHyphenZoneItem -----------------------------------------------

SvxHyphenZoneItem::SvxHyphenZoneItem( const bool bHyph, const sal_uInt16 nId ) :
    SfxPoolItem( nId ),
    bHyphen(bHyph),
    bPageEnd(true),
    bNoCapsHyphenation(false),
    bNoLastWordHyphenation(false),
    nMinLead(0),
    nMinTrail(0),
    nMaxHyphens(255),
    nMinWordLength(0),
    nTextHyphenZone(0),
    nKeep(0),           // TODO change default value to COLUMN
    nCompoundMinLead(0)
{
}


bool    SvxHyphenZoneItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case  MID_IS_HYPHEN:
            rVal <<= bHyphen;
        break;
        case MID_HYPHEN_MIN_LEAD:
            rVal <<= static_cast<sal_Int16>(nMinLead);
        break;
        case MID_HYPHEN_MIN_TRAIL:
            rVal <<= static_cast<sal_Int16>(nMinTrail);
        break;
        case MID_HYPHEN_MAX_HYPHENS:
            rVal <<= static_cast<sal_Int16>(nMaxHyphens);
        break;
        case MID_HYPHEN_NO_CAPS:
            rVal <<= bNoCapsHyphenation;
        break;
        case MID_HYPHEN_NO_LAST_WORD:
            rVal <<= bNoLastWordHyphenation;
        break;
        case MID_HYPHEN_MIN_WORD_LENGTH:
            rVal <<= static_cast<sal_Int16>(nMinWordLength);
        break;
        case MID_HYPHEN_ZONE:
            rVal <<= static_cast<sal_Int16>(nTextHyphenZone);
        break;
        case MID_HYPHEN_KEEP:
            rVal <<= static_cast<sal_Int16>(nKeep);
        break;
        case MID_HYPHEN_COMPOUND_MIN_LEAD:
            rVal <<= static_cast<sal_Int16>(nCompoundMinLead);
        break;
    }
    return true;
}

bool SvxHyphenZoneItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    sal_Int32 nNewVal = 0; // sal_Int32 needs for MID_HYPHEN_KEEP

    if( nMemberId != MID_IS_HYPHEN && nMemberId != MID_HYPHEN_NO_CAPS &&
                nMemberId != MID_HYPHEN_NO_LAST_WORD )
    {
        if(!(rVal >>= nNewVal))
            return false;
    }

    switch(nMemberId)
    {
        case  MID_IS_HYPHEN:
            bHyphen = Any2Bool(rVal);
        break;
        case MID_HYPHEN_MIN_LEAD:
            nMinLead = static_cast<sal_uInt8>(nNewVal);
        break;
        case MID_HYPHEN_MIN_TRAIL:
            nMinTrail = static_cast<sal_uInt8>(nNewVal);
        break;
        case MID_HYPHEN_MAX_HYPHENS:
            nMaxHyphens = static_cast<sal_uInt8>(nNewVal);
        break;
        case MID_HYPHEN_NO_CAPS:
            bNoCapsHyphenation = Any2Bool(rVal);
        break;
        case MID_HYPHEN_NO_LAST_WORD:
            bNoLastWordHyphenation = Any2Bool(rVal);
        break;
        case MID_HYPHEN_MIN_WORD_LENGTH:
            nMinWordLength = static_cast<sal_uInt8>(nNewVal);
        break;
        case MID_HYPHEN_ZONE:
            nTextHyphenZone = nNewVal;
        break;
        case MID_HYPHEN_KEEP:
            nKeep = static_cast<sal_uInt8>(nNewVal);
        break;
        case MID_HYPHEN_COMPOUND_MIN_LEAD:
            nCompoundMinLead = static_cast<sal_uInt8>(nNewVal);
        break;
    }
    return true;
}


bool SvxHyphenZoneItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    const SvxHyphenZoneItem& rItem = static_cast<const SvxHyphenZoneItem&>(rAttr);
    return ( rItem.bHyphen == bHyphen
            && rItem.bNoCapsHyphenation == bNoCapsHyphenation
            && rItem.bNoLastWordHyphenation == bNoLastWordHyphenation
            && rItem.bPageEnd == bPageEnd
            && rItem.nMinLead == nMinLead
            && rItem.nMinTrail == nMinTrail
            && rItem.nCompoundMinLead == nCompoundMinLead
            && rItem.nMaxHyphens == nMaxHyphens
            && rItem.nMinWordLength == nMinWordLength
            && rItem.nTextHyphenZone == nTextHyphenZone
            && rItem.nKeep == nKeep );
}

SvxHyphenZoneItem* SvxHyphenZoneItem::Clone( SfxItemPool * ) const
{
    return new SvxHyphenZoneItem( *this );
}

bool SvxHyphenZoneItem::GetPresentation
(
    SfxItemPresentation ePres,
    MapUnit             eCoreUnit,
    MapUnit             ePresUnit,
    OUString&           rText, const IntlWrapper& rIntl
)   const
{
    OUString cpDelimTmp(cpDelim);
    switch ( ePres )
    {
        case SfxItemPresentation::Nameless:
        {
            TranslateId pId = RID_SVXITEMS_HYPHEN_FALSE;

            if ( bHyphen )
                pId = RID_SVXITEMS_HYPHEN_TRUE;
            rText = EditResId(pId) + cpDelimTmp;
            pId = RID_SVXITEMS_PAGE_END_FALSE;

            if ( bPageEnd )
                pId = RID_SVXITEMS_PAGE_END_TRUE;
            rText += EditResId(pId) + cpDelimTmp +
                    OUString::number( nMinLead ) + cpDelimTmp +
                    OUString::number( nMinTrail ) + cpDelimTmp +
                    OUString::number( nCompoundMinLead ) + cpDelimTmp +
                    OUString::number( nMaxHyphens ) + cpDelimTmp +
                    OUString::number( nMinWordLength ) + cpDelimTmp +
                    GetMetricText( nTextHyphenZone, eCoreUnit, ePresUnit, &rIntl ) +
                        " " + EditResId(GetMetricId(ePresUnit));

            if ( bNoCapsHyphenation )
                rText += cpDelimTmp + EditResId(RID_SVXITEMS_HYPHEN_NO_CAPS_TRUE);

            if ( bNoLastWordHyphenation )
                rText += cpDelimTmp + EditResId(RID_SVXITEMS_HYPHEN_LAST_WORD_TRUE);

            rText += OUString::number( nKeep );
            return true;
        }
        case SfxItemPresentation::Complete:
        {
            TranslateId pId = RID_SVXITEMS_HYPHEN_FALSE;

            if ( bHyphen )
                pId = RID_SVXITEMS_HYPHEN_TRUE;
            rText = EditResId(pId) + cpDelimTmp;
            pId = RID_SVXITEMS_PAGE_END_FALSE;

            if ( bPageEnd )
                pId = RID_SVXITEMS_PAGE_END_TRUE;
            rText += EditResId(pId) +
                    cpDelimTmp +
                    EditResId(RID_SVXITEMS_HYPHEN_MINLEAD).replaceAll("%1", OUString::number(nMinLead)) +
                    cpDelimTmp +
                    EditResId(RID_SVXITEMS_HYPHEN_MINTRAIL).replaceAll("%1", OUString::number(nMinTrail)) +
                    cpDelimTmp +
                    EditResId(RID_SVXITEMS_HYPHEN_COMPOUND_MINLEAD).replaceAll("%1", OUString::number(nCompoundMinLead)) +
                    cpDelimTmp +
                    EditResId(RID_SVXITEMS_HYPHEN_MAX).replaceAll("%1", OUString::number(nMaxHyphens)) +
                    cpDelimTmp +
                    EditResId(RID_SVXITEMS_HYPHEN_MINWORDLEN).replaceAll("%1", OUString::number(nMinWordLength));

            if ( nTextHyphenZone > 0 )
            {
                rText += cpDelimTmp + EditResId(RID_SVXITEMS_HYPHEN_ZONE) +
                        GetMetricText( nTextHyphenZone, eCoreUnit, ePresUnit, &rIntl ) +
                        " " + EditResId(GetMetricId(ePresUnit));
            }

            if ( bNoCapsHyphenation )
                rText += cpDelimTmp + EditResId(RID_SVXITEMS_HYPHEN_NO_CAPS_TRUE);

            if ( bNoLastWordHyphenation )
                rText += cpDelimTmp + EditResId(RID_SVXITEMS_HYPHEN_LAST_WORD_TRUE);

            switch ( nKeep )
            {
                case 0:
                    rText += cpDelimTmp + EditResId(RID_SVXITEMS_HYPHEN_KEEP_AUTO);
                    break;
                case 1:
                    rText += cpDelimTmp + EditResId(RID_SVXITEMS_HYPHEN_KEEP_SPREAD);
                    break;
                case 2:
                    rText += cpDelimTmp + EditResId(RID_SVXITEMS_HYPHEN_KEEP_PAGE);
                    break;
                case 3:
                    rText += cpDelimTmp + EditResId(RID_SVXITEMS_HYPHEN_KEEP_COLUMN);
                    break;
                case 4:
                    rText += cpDelimTmp + EditResId(RID_SVXITEMS_HYPHEN_KEEP_ALWAYS);
                    break;
            }

            return true;
        }
        default: ;//prevent warning
    }
    return false;
}


// class SvxTabStop ------------------------------------------------------

SvxTabStop::SvxTabStop()
{
    nTabPos = 0;
    eAdjustment = SvxTabAdjust::Left;
    m_cDecimal = cDfltDecimalChar;
    cFill = cDfltFillChar;
}


SvxTabStop::SvxTabStop( const sal_Int32 nPos, const SvxTabAdjust eAdjst,
                        const sal_Unicode cDec, const sal_Unicode cFil )
{
    nTabPos = nPos;
    eAdjustment = eAdjst;
    m_cDecimal = cDec;
    cFill = cFil;
}

void SvxTabStop::fillDecimal() const
{
    if ( cDfltDecimalChar == m_cDecimal )
        m_cDecimal = SvtSysLocale().GetLocaleData().getNumDecimalSep()[0];
}

void SvxTabStop::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SvxTabStop"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("nTabPos"),
                                BAD_CAST(OString::number(nTabPos).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("eAdjustment"),
                                BAD_CAST(OString::number(static_cast<int>(eAdjustment)).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

// class SvxTabStopItem --------------------------------------------------

SvxTabStopItem::SvxTabStopItem( sal_uInt16 _nWhich ) :
    SfxPoolItem( _nWhich )
{
    const sal_uInt16 nTabs = SVX_TAB_DEFCOUNT, nDist = SVX_TAB_DEFDIST;
    const SvxTabAdjust eAdjst= SvxTabAdjust::Default;

    for (sal_uInt16 i = 0; i < nTabs; ++i)
    {
        SvxTabStop aTab( (i + 1) * nDist, eAdjst );
        maTabStops.insert( aTab );
    }
}


SvxTabStopItem::SvxTabStopItem( const sal_uInt16 nTabs,
                                const sal_uInt16 nDist,
                                const SvxTabAdjust eAdjst,
                                sal_uInt16 _nWhich ) :
    SfxPoolItem( _nWhich )
{
    for ( sal_uInt16 i = 0; i < nTabs; ++i )
    {
        SvxTabStop aTab( (i + 1) * nDist, eAdjst );
        maTabStops.insert( aTab );
    }
}


sal_uInt16 SvxTabStopItem::GetPos( const SvxTabStop& rTab ) const
{
    SvxTabStopArr::const_iterator it = maTabStops.find( rTab );
    return it != maTabStops.end() ? it - maTabStops.begin() : SVX_TAB_NOTFOUND;
}


sal_uInt16 SvxTabStopItem::GetPos( const sal_Int32 nPos ) const
{
    SvxTabStopArr::const_iterator it = maTabStops.find( SvxTabStop( nPos ) );
    return it != maTabStops.end() ? it - maTabStops.begin() : SVX_TAB_NOTFOUND;
}

void SvxTabStopItem::SetDefaultDistance(sal_Int32 nDefaultDistance)
{
    mnDefaultDistance = nDefaultDistance;
}

sal_Int32 SvxTabStopItem::GetDefaultDistance() const
{
    return mnDefaultDistance;
}

bool SvxTabStopItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_TABSTOPS:
        {
            sal_uInt16 nCount = Count();
            uno::Sequence< style::TabStop> aSeq(nCount);
            style::TabStop* pArr = aSeq.getArray();
            for(sal_uInt16 i = 0; i < nCount; i++)
            {
                const SvxTabStop& rTab = (*this)[i];
                pArr[i].Position        = bConvert ? convertTwipToMm100(rTab.GetTabPos()) : rTab.GetTabPos();
                switch(rTab.GetAdjustment())
                {
                case  SvxTabAdjust::Left   : pArr[i].Alignment = style::TabAlign_LEFT; break;
                case  SvxTabAdjust::Right  : pArr[i].Alignment = style::TabAlign_RIGHT; break;
                case  SvxTabAdjust::Decimal: pArr[i].Alignment = style::TabAlign_DECIMAL; break;
                case  SvxTabAdjust::Center : pArr[i].Alignment = style::TabAlign_CENTER; break;
                    default: //SvxTabAdjust::Default
                        pArr[i].Alignment = style::TabAlign_DEFAULT;

                }
                pArr[i].DecimalChar     = rTab.GetDecimal();
                pArr[i].FillChar        = rTab.GetFill();
            }
            rVal <<= aSeq;
            break;
        }
        case MID_STD_TAB:
        {
            const SvxTabStop &rTab = maTabStops.front();
            rVal <<= static_cast<sal_Int32>(bConvert ? convertTwipToMm100(rTab.GetTabPos()) : rTab.GetTabPos());
            break;
        }
        case MID_TABSTOP_DEFAULT_DISTANCE:
        {
            rVal <<= static_cast<sal_Int32>(bConvert ? convertTwipToMm100(mnDefaultDistance) : mnDefaultDistance);
            break;
        }
    }
    return true;
}

bool SvxTabStopItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_TABSTOPS:
        {
            uno::Sequence< style::TabStop> aSeq;
            if(!(rVal >>= aSeq))
            {
                uno::Sequence < uno::Sequence < uno::Any >  > aAnySeq;
                if (!(rVal >>= aAnySeq))
                    return false;
                auto aAnySeqRange = asNonConstRange(aAnySeq);
                sal_Int32 nLength = aAnySeq.getLength();
                aSeq.realloc( nLength );
                auto pSeq = aSeq.getArray();
                for ( sal_Int32 n=0; n<nLength; n++ )
                {
                    uno::Sequence < uno::Any >& rAnySeq = aAnySeqRange[n];
                    if ( rAnySeq.getLength() == 4 )
                    {
                        if (!(rAnySeq[0] >>= pSeq[n].Position)) return false;
                        if (!(rAnySeq[1] >>= pSeq[n].Alignment))
                        {
                            sal_Int32 nVal = 0;
                            if (rAnySeq[1] >>= nVal)
                                pSeq[n].Alignment = static_cast<css::style::TabAlign>(nVal);
                            else
                                return false;
                        }
                        if (!(rAnySeq[2] >>= pSeq[n].DecimalChar))
                        {
                            OUString aVal;
                            if ( (rAnySeq[2] >>= aVal) && aVal.getLength() == 1 )
                                pSeq[n].DecimalChar = aVal.toChar();
                            else
                                return false;
                        }
                        if (!(rAnySeq[3] >>= pSeq[n].FillChar))
                        {
                            OUString aVal;
                            if ( (rAnySeq[3] >>= aVal) && aVal.getLength() == 1 )
                                pSeq[n].FillChar = aVal.toChar();
                            else
                                return false;
                        }
                    }
                    else
                        return false;
                }
            }

            maTabStops.clear();
            const style::TabStop* pArr = aSeq.getConstArray();
            const sal_uInt16 nCount = static_cast<sal_uInt16>(aSeq.getLength());
            for(sal_uInt16 i = 0; i < nCount ; i++)
            {
                SvxTabAdjust eAdjust = SvxTabAdjust::Default;
                switch(pArr[i].Alignment)
                {
                case style::TabAlign_LEFT   : eAdjust = SvxTabAdjust::Left; break;
                case style::TabAlign_CENTER : eAdjust = SvxTabAdjust::Center; break;
                case style::TabAlign_RIGHT  : eAdjust = SvxTabAdjust::Right; break;
                case style::TabAlign_DECIMAL: eAdjust = SvxTabAdjust::Decimal; break;
                default: ;//prevent warning
                }
                sal_Unicode cFill = pArr[i].FillChar;
                sal_Unicode cDecimal = pArr[i].DecimalChar;
                SvxTabStop aTab( bConvert ? o3tl::toTwips(pArr[i].Position, o3tl::Length::mm100) : pArr[i].Position,
                                    eAdjust,
                                    cDecimal,
                                    cFill );
                Insert(aTab);
            }
            break;
        }
        case MID_STD_TAB:
        {
            sal_Int32 nNewPos = 0;
            if (!(rVal >>= nNewPos) )
                return false;
            if (bConvert)
                nNewPos = o3tl::toTwips(nNewPos, o3tl::Length::mm100);
            if (nNewPos <= 0)
                return false;
            const SvxTabStop& rTab = maTabStops.front();
            SvxTabStop aNewTab ( nNewPos, rTab.GetAdjustment(), rTab.GetDecimal(), rTab.GetFill() );
            Remove( 0 );
            Insert( aNewTab );
            break;
        }
        case MID_TABSTOP_DEFAULT_DISTANCE:
        {
            sal_Int32 nNewDefaultDistance = 0;
            if (!(rVal >>= nNewDefaultDistance))
                return false;
            if (bConvert)
                nNewDefaultDistance = o3tl::toTwips(nNewDefaultDistance, o3tl::Length::mm100);
            if (nNewDefaultDistance < 0)
                return false;
            mnDefaultDistance = nNewDefaultDistance;
            break;
        }
    }
    return true;
}


bool SvxTabStopItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    const SvxTabStopItem& rTSI = static_cast<const SvxTabStopItem&>(rAttr);

    if ( mnDefaultDistance != rTSI.GetDefaultDistance() )
        return false;

    if ( Count() != rTSI.Count() )
        return false;

    for ( sal_uInt16 i = 0; i < Count(); ++i )
        if( (*this)[i] != rTSI[i] )
            return false;
    return true;
}

SvxTabStopItem* SvxTabStopItem::Clone( SfxItemPool * ) const
{
    return new SvxTabStopItem( *this );
}

bool SvxTabStopItem::GetPresentation
(
    SfxItemPresentation ePres,
    MapUnit             eCoreUnit,
    MapUnit             ePresUnit,
    OUString&           rText, const IntlWrapper& rIntl
)   const
{
    rText.clear();
    // TODO also consider mnDefaultTabDistance here

    bool bComma = false;

    for ( sal_uInt16 i = 0; i < Count(); ++i )
    {
        if ( SvxTabAdjust::Default != ((*this)[i]).GetAdjustment() )
        {
            if ( bComma )
                rText += ",";
            rText += GetMetricText(
                ((*this)[i]).GetTabPos(), eCoreUnit, ePresUnit, &rIntl );
            if ( SfxItemPresentation::Complete == ePres )
            {
                rText += " " + EditResId(GetMetricId(ePresUnit));
            }
            bComma = true;
        }
    }
    return true;
}


bool SvxTabStopItem::Insert( const SvxTabStop& rTab )
{
    sal_uInt16 nTabPos = GetPos(rTab);
    if(SVX_TAB_NOTFOUND != nTabPos )
        Remove(nTabPos);
    return maTabStops.insert( rTab ).second;
}

void SvxTabStopItem::Insert( const SvxTabStopItem* pTabs )
{
    for( sal_uInt16 i = 0; i < pTabs->Count(); i++ )
    {
        const SvxTabStop& rTab = (*pTabs)[i];
        sal_uInt16 nTabPos = GetPos(rTab);
        if(SVX_TAB_NOTFOUND != nTabPos)
            Remove(nTabPos);
    }
    for( sal_uInt16 i = 0; i < pTabs->Count(); i++ )
    {
        maTabStops.insert( (*pTabs)[i] );
    }
}

void SvxTabStopItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SvxTabStopItem"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("mnDefaultDistance"),
                                      BAD_CAST(OString::number(mnDefaultDistance).getStr()));
    for (const auto& rTabStop : maTabStops)
        rTabStop.dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);
}

// class SvxFormatSplitItem -------------------------------------------------
SvxFormatSplitItem::~SvxFormatSplitItem()
{
}

SvxFormatSplitItem* SvxFormatSplitItem::Clone( SfxItemPool * ) const
{
    return new SvxFormatSplitItem( *this );
}

bool SvxFormatSplitItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    TranslateId pId = RID_SVXITEMS_FMTSPLIT_FALSE;

    if ( GetValue() )
        pId = RID_SVXITEMS_FMTSPLIT_TRUE;
    rText = EditResId(pId);
    return true;
}

SvxPageModelItem* SvxPageModelItem::Clone( SfxItemPool* ) const
{
    return new SvxPageModelItem( *this );
}

bool SvxPageModelItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;

    switch ( nMemberId )
    {
        case MID_AUTO: rVal <<= bAuto; break;
        case MID_NAME: rVal <<= GetValue(); break;
        default: OSL_FAIL("Wrong MemberId!"); return false;
    }

    return true;
}

bool SvxPageModelItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet;
    OUString aStr;
    switch ( nMemberId )
    {
        case MID_AUTO: bRet = ( rVal >>= bAuto ); break;
        case MID_NAME: bRet = ( rVal >>= aStr ); if ( bRet ) SetValue(aStr); break;
        default: OSL_FAIL("Wrong MemberId!"); return false;
    }

    return bRet;
}

bool SvxPageModelItem::operator==( const SfxPoolItem& rAttr ) const
{
    return SfxStringItem::operator==(rAttr) &&
           bAuto == static_cast<const SvxPageModelItem&>( rAttr ).bAuto;
}

bool SvxPageModelItem::GetPresentation
(
    SfxItemPresentation ePres,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText.clear();
    bool bSet = !GetValue().isEmpty();

    switch ( ePres )
    {
        case SfxItemPresentation::Nameless:
            if ( bSet )
                rText = GetValue();
            return true;

        case SfxItemPresentation::Complete:
            if ( bSet )
            {
                rText = EditResId(RID_SVXITEMS_PAGEMODEL_COMPLETE) + GetValue();
            }
            return true;
        default: ;//prevent warning
    }
    return false;
}


SvxScriptSpaceItem::SvxScriptSpaceItem( bool bOn, const sal_uInt16 nId )
    : SfxBoolItem( nId, bOn )
{
}

SvxScriptSpaceItem* SvxScriptSpaceItem::Clone( SfxItemPool * ) const
{
    return new SvxScriptSpaceItem( *this );
}

bool SvxScriptSpaceItem::GetPresentation(
        SfxItemPresentation /*ePres*/,
        MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
        OUString &rText, const IntlWrapper& /*rIntl*/ ) const
{
    rText = EditResId( !GetValue()
                            ? RID_SVXITEMS_SCRPTSPC_OFF
                            : RID_SVXITEMS_SCRPTSPC_ON );
    return true;
}


SvxHangingPunctuationItem::SvxHangingPunctuationItem(
                                    bool bOn, const sal_uInt16 nId )
    : SfxBoolItem( nId, bOn )
{
}

SvxHangingPunctuationItem* SvxHangingPunctuationItem::Clone( SfxItemPool * ) const
{
    return new SvxHangingPunctuationItem( *this );
}

bool SvxHangingPunctuationItem::GetPresentation(
        SfxItemPresentation /*ePres*/,
        MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
        OUString &rText, const IntlWrapper& /*rIntl*/ ) const
{
    rText = EditResId( !GetValue()
                            ? RID_SVXITEMS_HNGPNCT_OFF
                            : RID_SVXITEMS_HNGPNCT_ON );
    return true;
}


SvxForbiddenRuleItem::SvxForbiddenRuleItem(
                                    bool bOn, const sal_uInt16 nId )
    : SfxBoolItem( nId, bOn )
{
}

SvxForbiddenRuleItem* SvxForbiddenRuleItem::Clone( SfxItemPool * ) const
{
    return new SvxForbiddenRuleItem( *this );
}

bool SvxForbiddenRuleItem::GetPresentation(
        SfxItemPresentation /*ePres*/,
        MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
        OUString &rText, const IntlWrapper& /*rIntl*/ ) const
{
    rText = EditResId( !GetValue()
                            ? RID_SVXITEMS_FORBIDDEN_RULE_OFF
                            : RID_SVXITEMS_FORBIDDEN_RULE_ON );
    return true;
}

/*************************************************************************
|*    class SvxParaVertAlignItem
*************************************************************************/

SvxParaVertAlignItem::SvxParaVertAlignItem( Align nValue,
    TypedWhichId<SvxParaVertAlignItem> nW )
    : SfxUInt16Item( nW, static_cast<sal_uInt16>(nValue) )
{
}

SvxParaVertAlignItem* SvxParaVertAlignItem::Clone( SfxItemPool* ) const
{
    return new SvxParaVertAlignItem( *this );
}

bool SvxParaVertAlignItem::GetPresentation(
        SfxItemPresentation /*ePres*/,
        MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
        OUString &rText, const IntlWrapper& ) const
{
    TranslateId pTmp;
    switch( GetValue() )
    {
        case Align::Automatic: pTmp = RID_SVXITEMS_PARAVERTALIGN_AUTO; break;
        case Align::Top:       pTmp = RID_SVXITEMS_PARAVERTALIGN_TOP; break;
        case Align::Center:    pTmp = RID_SVXITEMS_PARAVERTALIGN_CENTER; break;
        case Align::Bottom:    pTmp = RID_SVXITEMS_PARAVERTALIGN_BOTTOM; break;
        default:    pTmp = RID_SVXITEMS_PARAVERTALIGN_BASELINE; break;
    }
    rText = EditResId(pTmp);
    return true;
}

bool SvxParaVertAlignItem::QueryValue( css::uno::Any& rVal,
                                           sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= static_cast<sal_Int16>(GetValue());
    return true;
}

bool SvxParaVertAlignItem::PutValue( const css::uno::Any& rVal,
                                         sal_uInt8 /*nMemberId*/ )
{
    sal_Int16 nVal = sal_Int16();
    if((rVal >>= nVal) && nVal >=0 && nVal <= sal_uInt16(Align::Bottom) )
    {
        SetValue( static_cast<Align>(nVal) );
        return true;
    }
    else
        return false;
}

SvxParaGridItem::SvxParaGridItem( bool bOn, const sal_uInt16 nId )
    : SfxBoolItem( nId, bOn )
{
}

SvxParaGridItem* SvxParaGridItem::Clone( SfxItemPool * ) const
{
    return new SvxParaGridItem( *this );
}

bool SvxParaGridItem::GetPresentation(
        SfxItemPresentation /*ePres*/,
        MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
        OUString &rText, const IntlWrapper& /*rIntl*/ ) const
{
    rText = GetValue() ?
            EditResId( RID_SVXITEMS_PARASNAPTOGRID_ON ) :
            EditResId( RID_SVXITEMS_PARASNAPTOGRID_OFF );

    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
