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
#include <comphelper/processfactory.hxx>
#include <unotools/syslocale.hxx>
#include <comphelper/types.hxx>
#include <tools/rtti.hxx>
#include <tools/mapunit.hxx>
#include <svl/itempool.hxx>
#include <svl/memberid.hrc>
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
#include <editeng/memberids.hrc>
#include <editeng/editids.hrc>
#include <editeng/itemtype.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/paperinf.hxx>
#include <vcl/svapp.hxx>
#include <algorithm>

using namespace ::com::sun::star;

TYPEINIT1_FACTORY(SvxLineSpacingItem, SfxPoolItem , new SvxLineSpacingItem(LINE_SPACE_DEFAULT_HEIGHT, 0));
TYPEINIT1_FACTORY(SvxAdjustItem, SfxPoolItem, new SvxAdjustItem(SVX_ADJUST_LEFT, 0));
TYPEINIT1_FACTORY(SvxWidowsItem, SfxByteItem, new SvxWidowsItem(0, 0));
TYPEINIT1_FACTORY(SvxOrphansItem, SfxByteItem, new SvxOrphansItem(0, 0));
TYPEINIT1_FACTORY(SvxHyphenZoneItem, SfxPoolItem, new SvxHyphenZoneItem(false, 0));
TYPEINIT1_FACTORY(SvxTabStopItem, SfxPoolItem, new SvxTabStopItem(0));
TYPEINIT1_FACTORY(SvxFormatSplitItem, SfxBoolItem, new SvxFormatSplitItem(false, 0));
TYPEINIT1_FACTORY(SvxPageModelItem, SfxStringItem, new SvxPageModelItem(0));
TYPEINIT1_FACTORY(SvxScriptSpaceItem, SfxBoolItem, new SvxScriptSpaceItem(false, 0));
TYPEINIT1_FACTORY(SvxHangingPunctuationItem, SfxBoolItem, new SvxHangingPunctuationItem(false, 0));
TYPEINIT1_FACTORY(SvxForbiddenRuleItem, SfxBoolItem, new SvxForbiddenRuleItem(false, 0));
TYPEINIT1_FACTORY(SvxParaVertAlignItem, SfxUInt16Item, new SvxParaVertAlignItem(0, 0));
TYPEINIT1_FACTORY(SvxParaGridItem, SfxBoolItem, new SvxParaGridItem(true, 0));

SfxPoolItem* SvxLineSpacingItem::CreateDefault() { return new  SvxLineSpacingItem(LINE_SPACE_DEFAULT_HEIGHT, 0);}
SfxPoolItem* SvxAdjustItem::CreateDefault() { return new  SvxAdjustItem(SVX_ADJUST_LEFT, 0);}
SfxPoolItem* SvxWidowsItem::CreateDefault() { return new  SvxWidowsItem(0, 0);}
SfxPoolItem* SvxOrphansItem::CreateDefault() { return new  SvxOrphansItem(0, 0);}
SfxPoolItem* SvxHyphenZoneItem::CreateDefault() { return new  SvxHyphenZoneItem(false, 0);}
SfxPoolItem* SvxTabStopItem::CreateDefault() { return new  SvxTabStopItem(0);}
SfxPoolItem* SvxFormatSplitItem::CreateDefault() { return new  SvxFormatSplitItem(false, 0);}
SfxPoolItem* SvxPageModelItem::CreateDefault() { return new  SvxPageModelItem(0);}
SfxPoolItem* SvxScriptSpaceItem::CreateDefault() { return new  SvxScriptSpaceItem(false, 0);}
SfxPoolItem* SvxHangingPunctuationItem::CreateDefault() { return new  SvxHangingPunctuationItem(false, 0);}
SfxPoolItem* SvxForbiddenRuleItem::CreateDefault() { return new  SvxForbiddenRuleItem(false, 0);}
SfxPoolItem* SvxParaVertAlignItem::CreateDefault() { return new  SvxParaVertAlignItem(0, 0);}
SfxPoolItem* SvxParaGridItem::CreateDefault() { return new  SvxParaGridItem(true, 0);}


SvxLineSpacingItem::SvxLineSpacingItem( sal_uInt16 nHeight, const sal_uInt16 nId )
    : SfxEnumItemInterface( nId )
{
    nPropLineSpace = 100;
    nInterLineSpace = 0;
    nLineHeight = nHeight;
    eLineSpace = SVX_LINE_SPACE_AUTO;
    eInterLineSpace = SVX_INTER_LINE_SPACE_OFF;
}



bool SvxLineSpacingItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxLineSpacingItem& rLineSpace = static_cast<const SvxLineSpacingItem&>(rAttr);
    return
        // Same Linespacing Rule?
        (eLineSpace == rLineSpace.eLineSpace)
        // For maximum and minimum Linespacing be the size must coincide.
        && (eLineSpace == SVX_LINE_SPACE_AUTO ||
            nLineHeight == rLineSpace.nLineHeight)
        // Same Linespacing Rule?
        && ( eInterLineSpace == rLineSpace.eInterLineSpace )
        // Either set proportional or additive.
        && (( eInterLineSpace == SVX_INTER_LINE_SPACE_OFF)
            || (eInterLineSpace == SVX_INTER_LINE_SPACE_PROP
                && nPropLineSpace == rLineSpace.nPropLineSpace)
            || (eInterLineSpace == SVX_INTER_LINE_SPACE_FIX
                && (nInterLineSpace == rLineSpace.nInterLineSpace)));
}

/* Who does still know why the LineSpacingItem is so complicated?
   We can not use it for UNO since there are only two values:
      - ein sal_uInt16 for the mode
      - ein sal_uInt32 for all values (distance, height, rel. detail)
*/
bool SvxLineSpacingItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    style::LineSpacing aLSp;
    switch( eLineSpace )
    {
        case SVX_LINE_SPACE_AUTO:
            if(eInterLineSpace == SVX_INTER_LINE_SPACE_FIX)
            {
                aLSp.Mode = style::LineSpacingMode::LEADING;
                aLSp.Height = ( bConvert ? (short)convertTwipToMm100(nInterLineSpace) : nInterLineSpace);
            }
            else if(eInterLineSpace == SVX_INTER_LINE_SPACE_OFF)
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
        case SVX_LINE_SPACE_FIX :
        case SVX_LINE_SPACE_MIN :
            aLSp.Mode = eLineSpace == SVX_LINE_SPACE_FIX ? style::LineSpacingMode::FIX : style::LineSpacingMode::MINIMUM;
            aLSp.Height = ( bConvert ? (short)convertTwipToMm100(nLineHeight) : nLineHeight );
        break;
        default:
            ;//prevent warning about SVX_LINE_SPACE_END
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
                eInterLineSpace = SVX_INTER_LINE_SPACE_FIX;
                eLineSpace = SVX_LINE_SPACE_AUTO;
                nInterLineSpace = aLSp.Height;
                if(bConvert)
                    nInterLineSpace = (short)convertMm100ToTwip(nInterLineSpace);

            }
            break;
            case style::LineSpacingMode::PROP:
            {
                eLineSpace = SVX_LINE_SPACE_AUTO;
                nPropLineSpace = (sal_Int16)aLSp.Height;
                if(100 == aLSp.Height)
                    eInterLineSpace = SVX_INTER_LINE_SPACE_OFF;
                else
                    eInterLineSpace = SVX_INTER_LINE_SPACE_PROP;
            }
            break;
            case style::LineSpacingMode::FIX:
            case style::LineSpacingMode::MINIMUM:
            {
                eInterLineSpace =  SVX_INTER_LINE_SPACE_OFF;
                eLineSpace = aLSp.Mode == style::LineSpacingMode::FIX ? SVX_LINE_SPACE_FIX : SVX_LINE_SPACE_MIN;
                nLineHeight = aLSp.Height;
                if(bConvert)
                    nLineHeight = (sal_uInt16)convertMm100ToTwip(nLineHeight);
            }
            break;
        }
    }

    return bRet;
}



SfxPoolItem* SvxLineSpacingItem::Clone( SfxItemPool * ) const
{
    return new SvxLineSpacingItem( *this );
}



bool SvxLineSpacingItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
#ifdef DBG_UTIL
    rText = "SvxLineSpacingItem";
#else
    rText.clear();
#endif
    return false;
}



SfxPoolItem* SvxLineSpacingItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_Int8    nPropSpace;
    short   nInterSpace;
    sal_uInt16  nHeight;
    sal_Int8    nRule, nInterRule;

    rStrm.ReadSChar( nPropSpace )
         .ReadInt16( nInterSpace )
         .ReadUInt16( nHeight )
         .ReadSChar( nRule )
         .ReadSChar( nInterRule );

    SvxLineSpacingItem* pAttr = new SvxLineSpacingItem( nHeight, Which() );
    pAttr->SetInterLineSpace( nInterSpace );
    pAttr->SetPropLineSpace( nPropSpace );
    pAttr->GetLineSpaceRule() = (SvxLineSpace)nRule;
    pAttr->GetInterLineSpaceRule() = (SvxInterLineSpace)nInterRule;
    return pAttr;
}



SvStream& SvxLineSpacingItem::Store( SvStream& rStrm, sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteSChar( GetPropLineSpace() )
         .WriteInt16( GetInterLineSpace() )
         .WriteUInt16( GetLineHeight() )
         .WriteSChar( GetLineSpaceRule() )
         .WriteSChar( GetInterLineSpaceRule() );
    return rStrm;
}



sal_uInt16 SvxLineSpacingItem::GetValueCount() const
{
    return SVX_LINESPACE_END;   // SVX_LINESPACE_TWO_LINES + 1
}



OUString SvxLineSpacingItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    //! load strings from resource
    OUString aText;
    switch ( nPos )
    {
        case SVX_LINESPACE_USER:
            aText = "User";
            break;
        case SVX_LINESPACE_ONE_LINE:
            aText = "One line";
            break;
        case SVX_LINESPACE_ONE_POINT_FIVE_LINES:
            aText = "1.5 line";
            break;
        case SVX_LINESPACE_TWO_LINES:
            aText = "Two lines";
            break;
    }
    return aText;
}



sal_uInt16 SvxLineSpacingItem::GetEnumValue() const
{
    sal_uInt16 nVal;
    switch ( nPropLineSpace )
    {
        case 100:   nVal = SVX_LINESPACE_ONE_LINE;              break;
        case 150:   nVal = SVX_LINESPACE_ONE_POINT_FIVE_LINES;  break;
        case 200:   nVal = SVX_LINESPACE_TWO_LINES;             break;
        default:    nVal = SVX_LINESPACE_USER;                  break;
    }
    return nVal;
}



void SvxLineSpacingItem::SetEnumValue( sal_uInt16 nVal )
{
    switch ( nVal )
    {
        case SVX_LINESPACE_ONE_LINE:             nPropLineSpace = 100; break;
        case SVX_LINESPACE_ONE_POINT_FIVE_LINES: nPropLineSpace = 150; break;
        case SVX_LINESPACE_TWO_LINES:            nPropLineSpace = 200; break;
    }
}

// class SvxAdjustItem ---------------------------------------------------

SvxAdjustItem::SvxAdjustItem(const SvxAdjust eAdjst, const sal_uInt16 nId )
    : SfxEnumItemInterface( nId ),
    bOneBlock( false ), bLastCenter( false ), bLastBlock( false )
{
    SetAdjust( eAdjst );
}



bool SvxAdjustItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

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
        case MID_PARA_ADJUST      : rVal <<= (sal_Int16)GetAdjust(); break;
        case MID_LAST_LINE_ADJUST : rVal <<= (sal_Int16)GetLastBlock(); break;
        case MID_EXPAND_SINGLE    :
        {
            sal_Bool bValue = bOneBlock;
            rVal.setValue( &bValue, cppu::UnoType<bool>::get() );
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
            try
            {
                eVal = ::comphelper::getEnumAsINT32(rVal);
            }
            catch(...) {}
            if(eVal >= 0 && eVal <= 4)
            {
                if(MID_LAST_LINE_ADJUST == nMemberId &&
                    eVal != SVX_ADJUST_LEFT &&
                    eVal != SVX_ADJUST_BLOCK &&
                    eVal != SVX_ADJUST_CENTER)
                        return false;
                if(eVal < (sal_uInt16)SVX_ADJUST_END)
                    nMemberId == MID_PARA_ADJUST ?
                        SetAdjust((SvxAdjust)eVal) :
                            SetLastBlock((SvxAdjust)eVal);
            }
        }
        break;
        case MID_EXPAND_SINGLE :
            bOneBlock = Any2Bool(rVal);
            break;
    }
    return true;
}



SfxPoolItem* SvxAdjustItem::Clone( SfxItemPool * ) const
{
    return new SvxAdjustItem( *this );
}



bool SvxAdjustItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetValueTextByPos( (sal_uInt16)GetAdjust() );
            return true;
        default: ;//prevent warning
    }
    return false;
}



sal_uInt16 SvxAdjustItem::GetValueCount() const
{
    return SVX_ADJUST_END;  // SVX_ADJUST_BLOCKLINE + 1
}



OUString SvxAdjustItem::GetValueTextByPos( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos <= (sal_uInt16)SVX_ADJUST_BLOCKLINE, "enum overflow!" );
    return EE_RESSTR(RID_SVXITEMS_ADJUST_BEGIN + nPos);
}



sal_uInt16 SvxAdjustItem::GetEnumValue() const
{
    return (sal_uInt16)GetAdjust();
}



void SvxAdjustItem::SetEnumValue( sal_uInt16 nVal )
{
    SetAdjust( (const SvxAdjust)nVal );
}



sal_uInt16 SvxAdjustItem::GetVersion( sal_uInt16 nFileVersion ) const
{
    return (nFileVersion == SOFFICE_FILEFORMAT_31)
               ? 0 : ADJUST_LASTBLOCK_VERSION;
}



SfxPoolItem* SvxAdjustItem::Create(SvStream& rStrm, sal_uInt16 nVersion) const
{
    char eAdjustment;
    rStrm.ReadChar( eAdjustment );
    SvxAdjustItem *pRet = new SvxAdjustItem( (SvxAdjust)eAdjustment, Which() );
    if( nVersion >= ADJUST_LASTBLOCK_VERSION )
    {
        sal_Int8 nFlags;
        rStrm.ReadSChar( nFlags );
        pRet->bOneBlock = 0 != (nFlags & 0x0001);
        pRet->bLastCenter = 0 != (nFlags & 0x0002);
        pRet->bLastBlock = 0 != (nFlags & 0x0004);
    }
    return pRet;
}



SvStream& SvxAdjustItem::Store( SvStream& rStrm, sal_uInt16 nItemVersion ) const
{
    rStrm.WriteChar( (char)GetAdjust() );
    if ( nItemVersion >= ADJUST_LASTBLOCK_VERSION )
    {
        sal_Int8 nFlags = 0;
        if ( bOneBlock )
            nFlags |= 0x0001;
        if ( bLastCenter )
            nFlags |= 0x0002;
        if ( bLastBlock )
            nFlags |= 0x0004;
        rStrm.WriteSChar( nFlags );
    }
    return rStrm;
}

// class SvxWidowsItem ---------------------------------------------------

SvxWidowsItem::SvxWidowsItem(const sal_uInt8 nL, const sal_uInt16 nId ) :
    SfxByteItem( nId, nL )
{
}



SfxPoolItem* SvxWidowsItem::Clone( SfxItemPool * ) const
{
    return new SvxWidowsItem( *this );
}



SfxPoolItem* SvxWidowsItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_Int8 nLines;
    rStrm.ReadSChar( nLines );
    return new SvxWidowsItem( nLines, Which() );
}



SvStream& SvxWidowsItem::Store( SvStream& rStrm, sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteSChar( GetValue() );
    return rStrm;
}



bool SvxWidowsItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NAMELESS:
        {
            rText = EE_RESSTR(RID_SVXITEMS_LINES);
            break;
        }

        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = EE_RESSTR(RID_SVXITEMS_WIDOWS_COMPLETE) + " " + EE_RESSTR(RID_SVXITEMS_LINES);
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



SfxPoolItem* SvxOrphansItem::Clone( SfxItemPool * ) const
{
    return new SvxOrphansItem( *this );
}



SfxPoolItem* SvxOrphansItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_Int8 nLines;
    rStrm.ReadSChar( nLines );
    return new SvxOrphansItem( nLines, Which() );
}



SvStream& SvxOrphansItem::Store( SvStream& rStrm, sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteSChar( GetValue() );
    return rStrm;
}



bool SvxOrphansItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NAMELESS:
        {
            rText = EE_RESSTR(RID_SVXITEMS_LINES);
            break;
        }

        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = EE_RESSTR(RID_SVXITEMS_ORPHANS_COMPLETE) + " " + EE_RESSTR(RID_SVXITEMS_LINES);
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
    SfxPoolItem( nId )
{
    bHyphen = bHyph;
    bPageEnd = true;
    nMinLead = nMinTrail = 0;
    nMaxHyphens = 255;
}


bool    SvxHyphenZoneItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case  MID_IS_HYPHEN:
            rVal = css::uno::makeAny<bool>(bHyphen);
        break;
        case MID_HYPHEN_MIN_LEAD:
            rVal <<= (sal_Int16)nMinLead;
        break;
        case MID_HYPHEN_MIN_TRAIL:
            rVal <<= (sal_Int16)nMinTrail;
        break;
        case MID_HYPHEN_MAX_HYPHENS:
            rVal <<= (sal_Int16)nMaxHyphens;
        break;
    }
    return true;
}

bool SvxHyphenZoneItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    sal_Int16 nNewVal = 0;

    if( nMemberId != MID_IS_HYPHEN )
        if(!(rVal >>= nNewVal))
            return false;

    switch(nMemberId)
    {
        case  MID_IS_HYPHEN:
            bHyphen = Any2Bool(rVal);
        break;
        case MID_HYPHEN_MIN_LEAD:
            nMinLead = (sal_uInt8)nNewVal;
        break;
        case MID_HYPHEN_MIN_TRAIL:
            nMinTrail = (sal_uInt8)nNewVal;
        break;
        case MID_HYPHEN_MAX_HYPHENS:
            nMaxHyphens = (sal_uInt8)nNewVal;
        break;
    }
    return true;
}



bool SvxHyphenZoneItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxHyphenZoneItem& rItem = static_cast<const SvxHyphenZoneItem&>(rAttr);
    return ( rItem.bHyphen == bHyphen
            && rItem.bPageEnd == bPageEnd
            && rItem.nMinLead == nMinLead
            && rItem.nMinTrail == nMinTrail
            && rItem.nMaxHyphens == nMaxHyphens );
}



SfxPoolItem* SvxHyphenZoneItem::Clone( SfxItemPool * ) const
{
    return new SvxHyphenZoneItem( *this );
}



bool SvxHyphenZoneItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    OUString cpDelimTmp(cpDelim);
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NAMELESS:
        {
            sal_uInt16 nId = RID_SVXITEMS_HYPHEN_FALSE;

            if ( bHyphen )
                nId = RID_SVXITEMS_HYPHEN_TRUE;
            rText = EE_RESSTR(nId) + cpDelimTmp;
            nId = RID_SVXITEMS_PAGE_END_FALSE;

            if ( bPageEnd )
                nId = RID_SVXITEMS_PAGE_END_TRUE;
            rText = rText + EE_RESSTR(nId) + cpDelimTmp +
                    OUString::number( nMinLead ) + cpDelimTmp +
                    OUString::number( nMinTrail ) + cpDelimTmp +
                    OUString::number( nMaxHyphens );
            return true;
        }
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            sal_uInt16 nId = RID_SVXITEMS_HYPHEN_FALSE;

            if ( bHyphen )
                nId = RID_SVXITEMS_HYPHEN_TRUE;
            rText = EE_RESSTR(nId) + cpDelimTmp;
            nId = RID_SVXITEMS_PAGE_END_FALSE;

            if ( bPageEnd )
                nId = RID_SVXITEMS_PAGE_END_TRUE;
            rText = rText +
                    EE_RESSTR(nId) +
                    cpDelimTmp +
                    EE_RESSTR(RID_SVXITEMS_HYPHEN_MINLEAD).replaceAll("%1", OUString::number(nMinLead)) +
                    cpDelimTmp +
                    EE_RESSTR(RID_SVXITEMS_HYPHEN_MINTRAIL).replaceAll("%1", OUString::number(nMinTrail)) +
                    cpDelimTmp +
                    EE_RESSTR(RID_SVXITEMS_HYPHEN_MAX).replaceAll("%1", OUString::number(nMaxHyphens));
            return true;
        }
        default: ;//prevent warning
    }
    return false;
}



SfxPoolItem* SvxHyphenZoneItem::Create(SvStream& rStrm, sal_uInt16) const
{
    sal_Int8 _bHyphen, _bHyphenPageEnd;
    sal_Int8 _nMinLead, _nMinTrail, _nMaxHyphens;
    rStrm.ReadSChar( _bHyphen ).ReadSChar( _bHyphenPageEnd ).ReadSChar( _nMinLead ).ReadSChar( _nMinTrail ).ReadSChar( _nMaxHyphens );
    SvxHyphenZoneItem* pAttr = new SvxHyphenZoneItem( false, Which() );
    pAttr->SetHyphen( _bHyphen != 0 );
    pAttr->SetPageEnd( _bHyphenPageEnd != 0 );
    pAttr->GetMinLead() = _nMinLead;
    pAttr->GetMinTrail() = _nMinTrail;
    pAttr->GetMaxHyphens() = _nMaxHyphens;
    return pAttr;
}



SvStream& SvxHyphenZoneItem::Store( SvStream& rStrm, sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteSChar( (sal_Int8) IsHyphen() )
         .WriteSChar( (sal_Int8) IsPageEnd() )
         .WriteSChar( GetMinLead() )
         .WriteSChar( GetMinTrail() )
         .WriteSChar( GetMaxHyphens() );
    return rStrm;
}

// class SvxTabStop ------------------------------------------------------

SvxTabStop::SvxTabStop()
{
    nTabPos = 0;
    eAdjustment = SVX_TAB_ADJUST_LEFT;
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


// class SvxTabStopItem --------------------------------------------------

SvxTabStopItem::SvxTabStopItem( sal_uInt16 _nWhich ) :
    SfxPoolItem( _nWhich ),
    maTabStops()
{
    const sal_uInt16 nTabs = SVX_TAB_DEFCOUNT, nDist = SVX_TAB_DEFDIST;
    const SvxTabAdjust eAdjst= SVX_TAB_ADJUST_DEFAULT;

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
    SfxPoolItem( _nWhich ),
    maTabStops()
{
    for ( sal_uInt16 i = 0; i < nTabs; ++i )
    {
        SvxTabStop aTab( (i + 1) * nDist, eAdjst );
        maTabStops.insert( aTab );
    }
}



SvxTabStopItem::SvxTabStopItem( const SvxTabStopItem& rTSI ) :
    SfxPoolItem( rTSI.Which() ),
    maTabStops( rTSI.maTabStops )
{
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



SvxTabStopItem& SvxTabStopItem::operator=( const SvxTabStopItem& rTSI )
{
    maTabStops = rTSI.maTabStops;
    return *this;
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
                case  SVX_TAB_ADJUST_LEFT   : pArr[i].Alignment = style::TabAlign_LEFT; break;
                case  SVX_TAB_ADJUST_RIGHT  : pArr[i].Alignment = style::TabAlign_RIGHT; break;
                case  SVX_TAB_ADJUST_DECIMAL: pArr[i].Alignment = style::TabAlign_DECIMAL; break;
                case  SVX_TAB_ADJUST_CENTER : pArr[i].Alignment = style::TabAlign_CENTER; break;
                    default: //SVX_TAB_ADJUST_DEFAULT
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
            rVal <<= (static_cast<sal_Int32>(bConvert ? convertTwipToMm100(rTab.GetTabPos()) : rTab.GetTabPos()));
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
                sal_Int32 nLength = aAnySeq.getLength();
                aSeq.realloc( nLength );
                for ( sal_Int32 n=0; n<nLength; n++ )
                {
                    uno::Sequence < uno::Any >& rAnySeq = aAnySeq[n];
                    if ( rAnySeq.getLength() == 4 )
                    {
                        if (!(rAnySeq[0] >>= aSeq[n].Position)) return false;
                        if (!(rAnySeq[1] >>= aSeq[n].Alignment))
                        {
                            sal_Int32 nVal = 0;
                            if (rAnySeq[1] >>= nVal)
                                aSeq[n].Alignment = (css::style::TabAlign) nVal;
                            else
                                return false;
                        }
                        if (!(rAnySeq[2] >>= aSeq[n].DecimalChar))
                        {
                            OUString aVal;
                            if ( (rAnySeq[2] >>= aVal) && aVal.getLength() == 1 )
                                aSeq[n].DecimalChar = aVal.toChar();
                            else
                                return false;
                        }
                        if (!(rAnySeq[3] >>= aSeq[n].FillChar))
                        {
                            OUString aVal;
                            if ( (rAnySeq[3] >>= aVal) && aVal.getLength() == 1 )
                                aSeq[n].FillChar = aVal.toChar();
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
            const sal_uInt16 nCount = (sal_uInt16)aSeq.getLength();
            for(sal_uInt16 i = 0; i < nCount ; i++)
            {
                SvxTabAdjust eAdjust = SVX_TAB_ADJUST_DEFAULT;
                switch(pArr[i].Alignment)
                {
                case style::TabAlign_LEFT   : eAdjust = SVX_TAB_ADJUST_LEFT; break;
                case style::TabAlign_CENTER : eAdjust = SVX_TAB_ADJUST_CENTER; break;
                case style::TabAlign_RIGHT  : eAdjust = SVX_TAB_ADJUST_RIGHT; break;
                case style::TabAlign_DECIMAL: eAdjust = SVX_TAB_ADJUST_DECIMAL; break;
                default: ;//prevent warning
                }
                sal_Unicode cFill = pArr[i].FillChar;
                sal_Unicode cDecimal = pArr[i].DecimalChar;
                SvxTabStop aTab( bConvert ? convertMm100ToTwip(pArr[i].Position) : pArr[i].Position,
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
                nNewPos = convertMm100ToTwip ( nNewPos );
            if (nNewPos <= 0)
                return false;
            const SvxTabStop& rTab = maTabStops.front();
            SvxTabStop aNewTab ( nNewPos, rTab.GetAdjustment(), rTab.GetDecimal(), rTab.GetFill() );
            Remove( 0 );
            Insert( aNewTab );
            break;
        }
    }
    return true;
}


bool SvxTabStopItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxTabStopItem& rTSI = static_cast<const SvxTabStopItem&>(rAttr);

    if ( Count() != rTSI.Count() )
        return false;

    for ( sal_uInt16 i = 0; i < Count(); ++i )
        if( !(*this)[i].IsEqual( rTSI[i] ) )
            return false;
    return true;
}



SfxPoolItem* SvxTabStopItem::Clone( SfxItemPool * ) const
{
    return new SvxTabStopItem( *this );
}



bool SvxTabStopItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    OUString&           rText, const IntlWrapper *pIntl
)   const
{
    rText.clear();

    bool bComma = false;

    for ( sal_uInt16 i = 0; i < Count(); ++i )
    {
        if ( SVX_TAB_ADJUST_DEFAULT != ((*this)[i]).GetAdjustment() )
        {
            if ( bComma )
                rText += ",";
            rText += GetMetricText(
                ((*this)[i]).GetTabPos(), eCoreUnit, ePresUnit, pIntl );
            if ( SFX_ITEM_PRESENTATION_COMPLETE == ePres )
            {
                rText += " " + EE_RESSTR(GetMetricId(ePresUnit));
            }
            bComma = true;
        }
    }
    return true;
}



SfxPoolItem* SvxTabStopItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int8 nTabs;
    rStrm.ReadSChar( nTabs );
    SvxTabStopItem* pAttr =
        new SvxTabStopItem( 0, 0, SVX_TAB_ADJUST_DEFAULT, Which() );

    for ( sal_Int8 i = 0; i < nTabs; i++ )
    {
        sal_Int32 nPos(0);
        sal_Int8 eAdjust;
        unsigned char cDecimal, cFill;
        rStrm.ReadInt32( nPos ).ReadSChar( eAdjust ).ReadUChar( cDecimal ).ReadUChar( cFill );
        if( !i || SVX_TAB_ADJUST_DEFAULT != eAdjust )
            pAttr->Insert( SvxTabStop
                ( nPos, (SvxTabAdjust)eAdjust, sal_Unicode(cDecimal), sal_Unicode(cFill) ) );
    }
    return pAttr;
}



SvStream& SvxTabStopItem::Store( SvStream& rStrm, sal_uInt16 /*nItemVersion*/ ) const
{
    // Default-Tabs are only expanded for the default Attribute. For complete
    // backward compabillity (<=304) all tabs have to be expanded, this makes
    // the files grow large in size. All only SWG!

    const SfxItemPool *pPool = SfxItemPool::GetStoringPool();
    const bool bStoreDefTabs = pPool
        && pPool->GetName() == "SWG"
        && ::IsDefaultItem( this );

    const short nTabs = Count();
    sal_uInt16  nCount = 0, nDefDist = 0;
    sal_Int32 nNew = 0;

    if( bStoreDefTabs )
    {
        const SvxTabStopItem& rDefTab = static_cast<const SvxTabStopItem &>(
            pPool->GetDefaultItem( pPool->GetWhich( SID_ATTR_TABSTOP, false ) ) );
        nDefDist = sal_uInt16( rDefTab.maTabStops.front().GetTabPos() );
        const sal_Int32 nPos = nTabs > 0 ? (*this)[nTabs-1].GetTabPos() : 0;
        nCount  = (sal_uInt16)(nPos / nDefDist);
        nNew    = (nCount + 1) * nDefDist;

        if( nNew <= nPos + 50 )
            nNew += nDefDist;

        sal_Int32 lA3Width = SvxPaperInfo::GetPaperSize(PAPER_A3).Width();
        nCount = (sal_uInt16)(nNew < lA3Width ? ( lA3Width - nNew ) / nDefDist + 1 : 0);
    }

    rStrm.WriteSChar( nTabs + nCount );
    for ( short i = 0; i < nTabs; i++ )
    {
        const SvxTabStop& rTab = (*this)[ i ];
        rStrm.WriteInt32( rTab.GetTabPos() )
             .WriteSChar( rTab.GetAdjustment() )
             .WriteUChar( rTab.GetDecimal() )
             .WriteUChar( rTab.GetFill() );
    }

    if ( bStoreDefTabs )
        for( ; nCount; --nCount )
        {
            SvxTabStop aSwTabStop(nNew, SVX_TAB_ADJUST_DEFAULT);
            rStrm.WriteInt32( aSwTabStop.GetTabPos() )
                 .WriteSChar( aSwTabStop.GetAdjustment() )
                 .WriteUChar( aSwTabStop.GetDecimal() )
                 .WriteUChar( aSwTabStop.GetFill() );
            nNew += nDefDist;
        }

    return rStrm;
}


bool SvxTabStopItem::Insert( const SvxTabStop& rTab )
{
    sal_uInt16 nTabPos = GetPos(rTab);
    if(SVX_TAB_NOTFOUND != nTabPos )
        Remove(nTabPos);
    return maTabStops.insert( rTab ).second;
}

void SvxTabStopItem::Insert( const SvxTabStopItem* pTabs, sal_uInt16 nStart,
                            sal_uInt16 nEnd )
{
    for( sal_uInt16 i = nStart; i < nEnd && i < pTabs->Count(); i++ )
    {
        const SvxTabStop& rTab = (*pTabs)[i];
        sal_uInt16 nTabPos = GetPos(rTab);
        if(SVX_TAB_NOTFOUND != nTabPos)
            Remove(nTabPos);
    }
    for( sal_uInt16 i = nStart; i < nEnd && i < pTabs->Count(); i++ )
    {
        maTabStops.insert( (*pTabs)[i] );
    }
}



// class SvxFormatSplitItem -------------------------------------------------
SvxFormatSplitItem::~SvxFormatSplitItem()
{
}

SfxPoolItem* SvxFormatSplitItem::Clone( SfxItemPool * ) const
{
    return new SvxFormatSplitItem( *this );
}



SvStream& SvxFormatSplitItem::Store( SvStream& rStrm, sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm.WriteSChar( (sal_Int8)GetValue() );
    return rStrm;
}



SfxPoolItem* SvxFormatSplitItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_Int8 bIsSplit;
    rStrm.ReadSChar( bIsSplit );
    return new SvxFormatSplitItem( bIsSplit != 0, Which() );
}



bool SvxFormatSplitItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    sal_uInt16 nId = RID_SVXITEMS_FMTSPLIT_FALSE;

    if ( GetValue() )
        nId = RID_SVXITEMS_FMTSPLIT_TRUE;
    rText = EE_RESSTR(nId);
    return true;
}



SfxPoolItem* SvxPageModelItem::Clone( SfxItemPool* ) const
{
    return new SvxPageModelItem( *this );
}



bool SvxPageModelItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;

    switch ( nMemberId )
    {
        case MID_AUTO: rVal <<= bAuto; break;
        case MID_NAME: rVal <<= OUString( GetValue() ); break;
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

bool SvxPageModelItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText.clear();
    bool bSet = !GetValue().isEmpty();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NAMELESS:
            if ( bSet )
                rText = GetValue();
            return true;

        case SFX_ITEM_PRESENTATION_COMPLETE:
            if ( bSet )
            {
                rText = EE_RESSTR(RID_SVXITEMS_PAGEMODEL_COMPLETE) + GetValue();
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

SfxPoolItem* SvxScriptSpaceItem::Clone( SfxItemPool * ) const
{
    return new SvxScriptSpaceItem( GetValue(), Which() );
}

SfxPoolItem* SvxScriptSpaceItem::Create(SvStream & rStrm, sal_uInt16) const
{
    bool bFlag;
    rStrm.ReadCharAsBool( bFlag );
    return new SvxScriptSpaceItem( bFlag, Which() );
}

sal_uInt16  SvxScriptSpaceItem::GetVersion( sal_uInt16 nFFVer ) const
{
    DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
            SOFFICE_FILEFORMAT_40==nFFVer ||
            SOFFICE_FILEFORMAT_50==nFFVer,
            "SvxTwoLinesItem: Is there a new file format? ");

    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}

bool SvxScriptSpaceItem::GetPresentation(
        SfxItemPresentation /*ePres*/,
        SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/,
        OUString &rText, const IntlWrapper* /*pIntl*/ ) const
{
    rText = EE_RESSTR( !GetValue()
                            ? RID_SVXITEMS_SCRPTSPC_OFF
                            : RID_SVXITEMS_SCRPTSPC_ON );
    return true;
}



SvxHangingPunctuationItem::SvxHangingPunctuationItem(
                                    bool bOn, const sal_uInt16 nId )
    : SfxBoolItem( nId, bOn )
{
}

SfxPoolItem* SvxHangingPunctuationItem::Clone( SfxItemPool * ) const
{
    return new SvxHangingPunctuationItem( GetValue(), Which() );
}

SfxPoolItem* SvxHangingPunctuationItem::Create(SvStream & rStrm, sal_uInt16) const
{
    bool bValue;
    rStrm.ReadCharAsBool( bValue );
    return new SvxHangingPunctuationItem( bValue, Which() );
}

sal_uInt16 SvxHangingPunctuationItem::GetVersion( sal_uInt16 nFFVer ) const
{
    DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
            SOFFICE_FILEFORMAT_40==nFFVer ||
            SOFFICE_FILEFORMAT_50==nFFVer,
            "SvxHangingPunctuationItem: Is there a new file format? ");

    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}

bool SvxHangingPunctuationItem::GetPresentation(
        SfxItemPresentation /*ePres*/,
        SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/,
        OUString &rText, const IntlWrapper* /*pIntl*/ ) const
{
    rText = EE_RESSTR( !GetValue()
                            ? RID_SVXITEMS_HNGPNCT_OFF
                            : RID_SVXITEMS_HNGPNCT_ON );
    return true;
}


SvxForbiddenRuleItem::SvxForbiddenRuleItem(
                                    bool bOn, const sal_uInt16 nId )
    : SfxBoolItem( nId, bOn )
{
}

SfxPoolItem* SvxForbiddenRuleItem::Clone( SfxItemPool * ) const
{
    return new SvxForbiddenRuleItem( GetValue(), Which() );
}

SfxPoolItem* SvxForbiddenRuleItem::Create(SvStream & rStrm, sal_uInt16) const
{
    bool bValue;
    rStrm.ReadCharAsBool( bValue );
    return new SvxForbiddenRuleItem( bValue, Which() );
}

sal_uInt16 SvxForbiddenRuleItem::GetVersion( sal_uInt16 nFFVer ) const
{
    DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
            SOFFICE_FILEFORMAT_40==nFFVer ||
            SOFFICE_FILEFORMAT_50==nFFVer,
            "SvxForbiddenRuleItem: Is there a new file format? ");

    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}

bool SvxForbiddenRuleItem::GetPresentation(
        SfxItemPresentation /*ePres*/,
        SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/,
        OUString &rText, const IntlWrapper* /*pIntl*/ ) const
{
    rText = EE_RESSTR( !GetValue()
                            ? RID_SVXITEMS_FORBIDDEN_RULE_OFF
                            : RID_SVXITEMS_FORBIDDEN_RULE_ON );
    return true;
}

/*************************************************************************
|*    class SvxParaVertAlignItem
*************************************************************************/

SvxParaVertAlignItem::SvxParaVertAlignItem( sal_uInt16 nValue,
    const sal_uInt16 nW )
    : SfxUInt16Item( nW, nValue )
{
}

SfxPoolItem* SvxParaVertAlignItem::Clone( SfxItemPool* ) const
{
    return new SvxParaVertAlignItem( GetValue(), Which() );
}

SfxPoolItem* SvxParaVertAlignItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    sal_uInt16 nVal;
    rStrm.ReadUInt16( nVal );
    return new SvxParaVertAlignItem( nVal, Which() );
}

SvStream& SvxParaVertAlignItem::Store( SvStream & rStrm, sal_uInt16 ) const
{
    rStrm.WriteUInt16( GetValue() );
    return rStrm;
}

sal_uInt16 SvxParaVertAlignItem::GetVersion( sal_uInt16 nFFVer ) const
{
    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}

bool SvxParaVertAlignItem::GetPresentation(
        SfxItemPresentation /*ePres*/,
        SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/,
        OUString &rText, const IntlWrapper*  ) const
{
    sal_uInt16 nTmp;
    switch( GetValue() )
    {
        case AUTOMATIC: nTmp = RID_SVXITEMS_PARAVERTALIGN_AUTO; break;
        case TOP:       nTmp = RID_SVXITEMS_PARAVERTALIGN_TOP; break;
        case CENTER:    nTmp = RID_SVXITEMS_PARAVERTALIGN_CENTER; break;
        case BOTTOM:    nTmp = RID_SVXITEMS_PARAVERTALIGN_BOTTOM; break;
        default:    nTmp = RID_SVXITEMS_PARAVERTALIGN_BASELINE; break;
    }
    rText = EE_RESSTR( nTmp );
    return true;
}

bool SvxParaVertAlignItem::QueryValue( css::uno::Any& rVal,
                                           sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= (sal_Int16)GetValue();
    return true;
}

bool SvxParaVertAlignItem::PutValue( const css::uno::Any& rVal,
                                         sal_uInt8 /*nMemberId*/ )
{
    sal_Int16 nVal = sal_Int16();
    if((rVal >>= nVal) && nVal >=0 && nVal <= BOTTOM )
    {
        SetValue( (sal_uInt16)nVal );
        return true;
    }
    else
        return false;
}

bool SvxParaVertAlignItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return SfxUInt16Item::operator==( rItem );
}


SvxParaGridItem::SvxParaGridItem( bool bOn, const sal_uInt16 nId )
    : SfxBoolItem( nId, bOn )
{
}

SfxPoolItem* SvxParaGridItem::Clone( SfxItemPool * ) const
{
    return new SvxParaGridItem( GetValue(), Which() );
}

SfxPoolItem* SvxParaGridItem::Create(SvStream & rStrm, sal_uInt16) const
{
    bool bFlag;
    rStrm.ReadCharAsBool( bFlag );
    return new SvxParaGridItem( bFlag, Which() );
}

sal_uInt16  SvxParaGridItem::GetVersion( sal_uInt16 nFFVer ) const
{
    DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFFVer ||
            SOFFICE_FILEFORMAT_40==nFFVer ||
            SOFFICE_FILEFORMAT_50==nFFVer,
            "SvxParaGridItem: Is there a new file format? ");

    return SOFFICE_FILEFORMAT_50 > nFFVer ? USHRT_MAX : 0;
}

bool SvxParaGridItem::GetPresentation(
        SfxItemPresentation /*ePres*/,
        SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/,
        OUString &rText, const IntlWrapper* /*pIntl*/ ) const
{
    rText = GetValue() ?
            EE_RESSTR( RID_SVXITEMS_PARASNAPTOGRID_ON ) :
            EE_RESSTR( RID_SVXITEMS_PARASNAPTOGRID_OFF );

    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
