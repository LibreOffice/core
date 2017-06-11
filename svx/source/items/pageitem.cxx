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

#include <sal/config.h>

#include <utility>

#include <tools/stream.hxx>

#include <svx/pageitem.hxx>
#include <editeng/itemtype.hxx>
#include <svx/unomid.hxx>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <svl/itemset.hxx>
#include <svx/strings.hrc>
#include <svx/svxitems.hrc>
#include <svx/dialmgr.hxx>

using namespace ::com::sun::star;

SfxPoolItem* SvxPageItem::CreateDefault() { return new   SvxPageItem(0);}

SvxPageItem::SvxPageItem( const sal_uInt16 nId ) : SfxPoolItem( nId ),

    eNumType    ( SVX_NUM_ARABIC ),
    bLandscape  ( false ),
    eUse        ( SvxPageUsage::All )
{
}

// Copy-Ctor
SvxPageItem::SvxPageItem( const SvxPageItem& rItem )
    : SfxPoolItem( rItem )
{
    eNumType    = rItem.eNumType;
    bLandscape  = rItem.bLandscape;
    eUse        = rItem.eUse;
}

SvxPageItem::~SvxPageItem() {}

// Clone
SfxPoolItem* SvxPageItem::Clone( SfxItemPool * ) const
{
    return new SvxPageItem( *this );
}

// Test for equality
bool SvxPageItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    const SvxPageItem& rItem = static_cast<const SvxPageItem&>(rAttr);
    return ( eNumType   == rItem.eNumType   &&
             bLandscape == rItem.bLandscape &&
             eUse       == rItem.eUse );
}

inline OUString GetUsageText( const SvxPageUsage eU )
{
    switch( eU )
    {
        case SvxPageUsage::Left  : return SvxResId(RID_SVXITEMS_PAGE_USAGE_LEFT);
        case SvxPageUsage::Right : return SvxResId(RID_SVXITEMS_PAGE_USAGE_RIGHT);
        case SvxPageUsage::All   : return SvxResId(RID_SVXITEMS_PAGE_USAGE_ALL);
        case SvxPageUsage::Mirror: return SvxResId(RID_SVXITEMS_PAGE_USAGE_MIRROR);
        default:              return OUString();
    }
}

static const char* RID_SVXITEMS_PAGE_NUMS[] =
{
    RID_SVXITEMS_PAGE_NUM_CHR_UPPER,
    RID_SVXITEMS_PAGE_NUM_CHR_LOWER,
    RID_SVXITEMS_PAGE_NUM_ROM_UPPER,
    RID_SVXITEMS_PAGE_NUM_ROM_LOWER,
    RID_SVXITEMS_PAGE_NUM_ARABIC,
    RID_SVXITEMS_PAGE_NUM_NONE
};

bool SvxPageItem::GetPresentation
(
    SfxItemPresentation ePres,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText.clear();
    OUString cpDelimTmp = OUString(cpDelim);

    switch ( ePres )
    {
        case SfxItemPresentation::Nameless:
        {
            if ( !aDescName.isEmpty() )
            {
                rText = aDescName + cpDelimTmp;
            }
            assert(eNumType <= css::style::NumberingType::NUMBER_NONE && "enum overflow");
            rText += SvxResId(RID_SVXITEMS_PAGE_NUMS[eNumType]) + cpDelimTmp;
            if ( bLandscape )
                rText += SvxResId(RID_SVXITEMS_PAGE_LAND_TRUE);
            else
                rText += SvxResId(RID_SVXITEMS_PAGE_LAND_FALSE);
            OUString aUsageText = GetUsageText( eUse );
            if (!aUsageText.isEmpty())
            {
                rText += cpDelimTmp + aUsageText;
            }
            return true;
        }
        case SfxItemPresentation::Complete:
        {
            rText += SvxResId(RID_SVXITEMS_PAGE_COMPLETE);
            if ( !aDescName.isEmpty() )
            {
                rText += aDescName + cpDelimTmp;
            }
            assert(eNumType <= css::style::NumberingType::NUMBER_NONE && "enum overflow");
            rText += SvxResId(RID_SVXITEMS_PAGE_NUMS[eNumType]) + cpDelimTmp;
            if ( bLandscape )
                rText += SvxResId(RID_SVXITEMS_PAGE_LAND_TRUE);
            else
                rText += SvxResId(RID_SVXITEMS_PAGE_LAND_FALSE);
            OUString aUsageText = GetUsageText( eUse );
            if (!aUsageText.isEmpty())
            {
                rText += cpDelimTmp + aUsageText;
            }
            return true;
        }
        default: ;//prevent warning
    }
    return false;
}


bool SvxPageItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
//    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_PAGE_NUMTYPE:
        {
            //! constants aren't in IDLs any more ?!?
            rVal <<= (sal_Int16)( eNumType );
        }
        break;
        case MID_PAGE_ORIENTATION:
            rVal <<= (bool) bLandscape;
        break;
        case MID_PAGE_LAYOUT     :
        {
            style::PageStyleLayout eRet;
            switch(eUse)
            {
                case SvxPageUsage::Left  : eRet = style::PageStyleLayout_LEFT;      break;
                case SvxPageUsage::Right : eRet = style::PageStyleLayout_RIGHT;     break;
                case SvxPageUsage::All   : eRet = style::PageStyleLayout_ALL;       break;
                case SvxPageUsage::Mirror: eRet = style::PageStyleLayout_MIRRORED; break;
                default:
                    OSL_FAIL("what layout is this?");
                    return false;
            }
            rVal <<= eRet;
        }
        break;
    }

    return true;
}

bool SvxPageItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    switch( nMemberId & ~CONVERT_TWIPS )
    {
        case MID_PAGE_NUMTYPE:
        {
            sal_Int32 nValue = 0;
            if(!(rVal >>= nValue))
                return false;

            eNumType = (SvxNumType)nValue;
        }
        break;
        case MID_PAGE_ORIENTATION:
            bLandscape = Any2Bool(rVal);
        break;
        case MID_PAGE_LAYOUT     :
        {
            style::PageStyleLayout eLayout;
            if(!(rVal >>= eLayout))
            {
                sal_Int32 nValue = 0;
                if(!(rVal >>= nValue))
                    return false;
                eLayout = (style::PageStyleLayout)nValue;
            }
            switch( eLayout )
            {
                case style::PageStyleLayout_LEFT    : eUse = SvxPageUsage::Left ; break;
                case style::PageStyleLayout_RIGHT   : eUse = SvxPageUsage::Right; break;
                case style::PageStyleLayout_ALL     : eUse = SvxPageUsage::All  ; break;
                case style::PageStyleLayout_MIRRORED: eUse = SvxPageUsage::Mirror;break;
                default: ;//prevent warning
            }
        }
        break;
    }
    return true;
}


SfxPoolItem* SvxPageItem::Create( SvStream& rStream, sal_uInt16 ) const
{
    sal_uInt8 eType;
    bool bLand;
    sal_uInt16 nUse;

    // UNICODE: rStream >> sStr;
    OUString sStr = rStream.ReadUniOrByteString( rStream.GetStreamCharSet() );

    rStream.ReadUChar( eType );
    rStream.ReadCharAsBool( bLand );
    rStream.ReadUInt16( nUse );

    SvxPageItem* pPage = new SvxPageItem( Which() );
    pPage->SetDescName( sStr );
    pPage->SetNumType( (SvxNumType)eType );
    pPage->SetLandscape( bLand );
    pPage->SetPageUsage( (SvxPageUsage)nUse );
    return pPage;
}


SvStream& SvxPageItem::Store( SvStream &rStrm, sal_uInt16 /*nItemVersion*/ ) const
{
    // UNICODE: rStrm << aDescName;
    rStrm.WriteUniOrByteString(aDescName, rStrm.GetStreamCharSet());

    rStrm.WriteUChar( eNumType ).WriteBool( bLandscape ).WriteUInt16( (sal_uInt16)eUse );
    return rStrm;
}

// HeaderFooterSet
SvxSetItem::SvxSetItem( const sal_uInt16 nId, const SfxItemSet& rSet ) :

    SfxSetItem( nId, rSet )
{
}

SvxSetItem::SvxSetItem( const SvxSetItem& rItem ) :

    SfxSetItem( rItem )
{
}

SvxSetItem::SvxSetItem( const sal_uInt16 nId, std::unique_ptr<SfxItemSet>&& _pSet ) :

    SfxSetItem( nId, std::move(_pSet) )
{
}

SfxPoolItem* SvxSetItem::Clone( SfxItemPool * ) const
{
    return new SvxSetItem(*this);
}


bool SvxSetItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText.clear();
    return false;
}

SfxPoolItem* SvxSetItem::Create(SvStream &rStrm, sal_uInt16 /*nVersion*/) const
{
    SfxItemSet* _pSet = new SfxItemSet( *GetItemSet().GetPool(),
                                       GetItemSet().GetRanges() );

    _pSet->Load( rStrm );

    return new SvxSetItem( Which(), *_pSet );
}

SvStream& SvxSetItem::Store(SvStream &rStrm, sal_uInt16 nItemVersion) const
{
    GetItemSet().Store( rStrm, nItemVersion );

    return rStrm;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
