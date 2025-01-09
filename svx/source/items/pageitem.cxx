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

#include <osl/diagnose.h>
#include <comphelper/processfactory.hxx>
#include <tools/resary.hxx>
#include <svx/pageitem.hxx>
#include <svx/strarray.hxx>
#include <editeng/itemtype.hxx>
#include <svx/unomid.hxx>
#include <com/sun/star/text/DefaultNumberingProvider.hpp>
#include <com/sun/star/text/XNumberingTypeInfo.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <svl/itemset.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>

using namespace ::com::sun::star;

SfxPoolItem* SvxPageItem::CreateDefault() { return new SvxPageItem(TypedWhichId<SvxPageItem>(0));}

SvxPageItem::SvxPageItem( const TypedWhichId<SvxPageItem> nId )
    : SfxPoolItem( nId ),

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
SvxPageItem* SvxPageItem::Clone( SfxItemPool * ) const
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

static OUString GetUsageText( const SvxPageUsage eU )
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

const TranslateId RID_SVXITEMS_PAGE_NUMS[] =
{
    RID_SVXITEMS_PAGE_NUM_CHR_UPPER,
    RID_SVXITEMS_PAGE_NUM_CHR_LOWER,
    RID_SVXITEMS_PAGE_NUM_ROM_UPPER,
    RID_SVXITEMS_PAGE_NUM_ROM_LOWER,
    RID_SVXITEMS_PAGE_NUM_ARABIC,
    RID_SVXITEMS_PAGE_NUM_NONE
};

namespace
{
    OUString GetNumberingDescription(SvxNumType eNumType)
    {
        // classic ones, keep displaying the old name
        if (eNumType <= css::style::NumberingType::NUMBER_NONE)
            return SvxResId(RID_SVXITEMS_PAGE_NUMS[eNumType]);
        // new ones, reuse the text used in the numbering dropdown list
        sal_uInt32 n = SvxNumberingTypeTable::FindIndex(eNumType);
        if (n != RESARRAY_INDEX_NOTFOUND)
            return SvxNumberingTypeTable::GetString(n);
        const css::uno::Reference<css::uno::XComponentContext>& xContext = comphelper::getProcessComponentContext();
        css::uno::Reference<css::text::XDefaultNumberingProvider> xDefNum = css::text::DefaultNumberingProvider::create(xContext);
        css::uno::Reference<css::text::XNumberingTypeInfo> xInfo(xDefNum, css::uno::UNO_QUERY);
        if (!xInfo.is())
            return OUString();
        return xInfo->getNumberingIdentifier(eNumType);
    }
}

bool SvxPageItem::GetPresentation
(
    SfxItemPresentation ePres,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText.clear();
    OUString cpDelimTmp(cpDelim);

    switch ( ePres )
    {
        case SfxItemPresentation::Nameless:
        {
            if ( !aDescName.isEmpty() )
            {
                rText = aDescName + cpDelimTmp;
            }
            rText += GetNumberingDescription(eNumType) + cpDelimTmp;
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
            rText += GetNumberingDescription(eNumType) + cpDelimTmp;
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
            rVal <<= static_cast<sal_Int16>( eNumType );
        }
        break;
        case MID_PAGE_ORIENTATION:
            rVal <<= bLandscape;
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

            eNumType = static_cast<SvxNumType>(nValue);
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
                eLayout = static_cast<style::PageStyleLayout>(nValue);
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

// HeaderFooterSet
SvxSetItem::SvxSetItem( const TypedWhichId<SvxSetItem> nId, const SfxItemSet& rSet ) :

    SfxSetItem( nId, rSet )
{
}

SvxSetItem::SvxSetItem( const SvxSetItem& rItem, SfxItemPool* pPool ) :

    SfxSetItem( rItem, pPool )
{
}

SvxSetItem::SvxSetItem( const TypedWhichId<SvxSetItem> nId, SfxItemSet&& _pSet ) :

    SfxSetItem( nId, std::move(_pSet) )
{
}

SvxSetItem* SvxSetItem::Clone( SfxItemPool * pPool ) const
{
    return new SvxSetItem(*this, pPool);
}

bool SvxSetItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText.clear();
    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
