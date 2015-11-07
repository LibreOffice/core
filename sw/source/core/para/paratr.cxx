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

#include "hintids.hxx"
#include <swtypes.hxx>
#include "unomid.h"
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/DropCapFormat.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/HorizontalAdjust.hpp>
#include <com/sun/star/text/DocumentStatistic.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/HoriOrientationFormat.hpp>
#include <com/sun/star/text/NotePrintMode.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/VertOrientationFormat.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <unostyle.hxx>
#include <SwStyleNameMapper.hxx>
#include "paratr.hxx"
#include "charfmt.hxx"
#include "cmdid.h"
#include <libxml/xmlwriter.h>

using namespace ::com::sun::star;


SfxPoolItem* SwFormatDrop::CreateDefault() { return new SwFormatDrop; }
SfxPoolItem* SwRegisterItem::CreateDefault() { return new SwRegisterItem; }
SfxPoolItem* SwNumRuleItem::CreateDefault() { return new SwNumRuleItem; }

SwFormatDrop::SwFormatDrop()
    : SfxPoolItem( RES_PARATR_DROP ),
    SwClient( 0 ),
    pDefinedIn( 0 ),
    nDistance( 0 ),
    nReadFormat( USHRT_MAX ),
    nLines( 0 ),
    nChars( 0 ),
    bWholeWord( false )
{
}

SwFormatDrop::SwFormatDrop( const SwFormatDrop &rCpy )
    : SfxPoolItem( RES_PARATR_DROP ),
    SwClient( rCpy.GetRegisteredInNonConst() ),
    pDefinedIn( 0 ),
    nDistance( rCpy.GetDistance() ),
    nReadFormat( rCpy.nReadFormat ),
    nLines( rCpy.GetLines() ),
    nChars( rCpy.GetChars() ),
    bWholeWord( rCpy.GetWholeWord() )
{
}

SwFormatDrop::~SwFormatDrop()
{
}

void SwFormatDrop::SetCharFormat( SwCharFormat *pNew )
{
    // Rewire
    if ( GetRegisteredIn() )
        GetRegisteredInNonConst()->Remove( this );
    if(pNew)
        pNew->Add( this );
    nReadFormat = USHRT_MAX;
}

void SwFormatDrop::Modify( const SfxPoolItem*, const SfxPoolItem * )
{
    if( pDefinedIn )
    {
        if( dynamic_cast< const SwFormat *>( pDefinedIn ) ==  nullptr)
            pDefinedIn->ModifyNotification( this, this );
        else if( pDefinedIn->HasWriterListeners() &&
                !pDefinedIn->IsModifyLocked() )
        {
            // Notify those who are dependent on the format on our own.
            // The format itself wouldn't pass on the notify as it does not get past the check.
            pDefinedIn->ModifyBroadcast( this, this );
        }
    }
}

bool SwFormatDrop::GetInfo( SfxPoolItem& ) const
{
    return true; // Continue
}

bool SwFormatDrop::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return ( nLines == static_cast<const SwFormatDrop&>(rAttr).GetLines() &&
             nChars == static_cast<const SwFormatDrop&>(rAttr).GetChars() &&
             nDistance ==  static_cast<const SwFormatDrop&>(rAttr).GetDistance() &&
             bWholeWord == static_cast<const SwFormatDrop&>(rAttr).GetWholeWord() &&
             GetCharFormat() == static_cast<const SwFormatDrop&>(rAttr).GetCharFormat() &&
             pDefinedIn == static_cast<const SwFormatDrop&>(rAttr).pDefinedIn );
}

SfxPoolItem* SwFormatDrop::Clone( SfxItemPool* ) const
{
    return new SwFormatDrop( *this );
}

bool SwFormatDrop::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    switch(nMemberId&~CONVERT_TWIPS)
    {
        case MID_DROPCAP_LINES : rVal <<= (sal_Int16)nLines; break;
        case MID_DROPCAP_COUNT : rVal <<= (sal_Int16)nChars; break;
        case MID_DROPCAP_DISTANCE : rVal <<= (sal_Int16) convertTwipToMm100(nDistance); break;
        case MID_DROPCAP_FORMAT:
        {
             style::DropCapFormat aDrop;
            aDrop.Lines = nLines   ;
            aDrop.Count = nChars   ;
            aDrop.Distance  = convertTwipToMm100(nDistance);
            rVal.setValue(&aDrop, ::cppu::UnoType<style::DropCapFormat>::get());
        }
        break;
        case MID_DROPCAP_WHOLE_WORD:
            rVal <<= bWholeWord;
        break;
        case MID_DROPCAP_CHAR_STYLE_NAME :
        {
            OUString sName;
            if(GetCharFormat())
                sName = SwStyleNameMapper::GetProgName(
                        GetCharFormat()->GetName(), nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
            rVal <<= sName;
        }
        break;
    }
    return true;
}

bool SwFormatDrop::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    switch(nMemberId&~CONVERT_TWIPS)
    {
        case MID_DROPCAP_LINES :
        {
            sal_Int8 nTemp = 0;
            rVal >>= nTemp;
            if(nTemp >=1 && nTemp < 0x7f)
                nLines = (sal_uInt8)nTemp;
        }
        break;
        case MID_DROPCAP_COUNT :
        {
            sal_Int16 nTemp = 0;
            rVal >>= nTemp;
            if(nTemp >=1 && nTemp < 0x7f)
                nChars = (sal_uInt8)nTemp;
        }
        break;
        case MID_DROPCAP_DISTANCE :
        {
            sal_Int16 nVal = 0;
            if ( rVal >>= nVal )
                nDistance = (sal_Int16) convertMm100ToTwip((sal_Int32)nVal);
            else
                return false;
            break;
        }
        case MID_DROPCAP_FORMAT:
        {
            if(rVal.getValueType()  == ::cppu::UnoType<style::DropCapFormat>::get())
            {
                const style::DropCapFormat* pDrop = static_cast<const style::DropCapFormat*>(rVal.getValue());
                nLines      = pDrop->Lines;
                nChars      = pDrop->Count;
                nDistance   = convertMm100ToTwip(pDrop->Distance);
            }
            else {
            }
        }
        break;
        case MID_DROPCAP_WHOLE_WORD:
            bWholeWord = *static_cast<sal_Bool const *>(rVal.getValue());
        break;
        case MID_DROPCAP_CHAR_STYLE_NAME :
            OSL_FAIL("char format cannot be set in PutValue()!");
        break;
    }
    return true;
}

SfxPoolItem* SwRegisterItem::Clone( SfxItemPool * ) const
{
    return new SwRegisterItem( *this );
}

SfxPoolItem* SwNumRuleItem::Clone( SfxItemPool * ) const
{
    return new SwNumRuleItem( *this );
}
bool SwNumRuleItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    return GetValue() == static_cast<const SwNumRuleItem&>(rAttr).GetValue();
}

bool    SwNumRuleItem::QueryValue( uno::Any& rVal, sal_uInt8 ) const
{
    OUString sRet = SwStyleNameMapper::GetProgName(GetValue(), nsSwGetPoolIdFromName::GET_POOLID_NUMRULE );
    rVal <<= sRet;
    return true;
}

bool    SwNumRuleItem::PutValue( const uno::Any& rVal, sal_uInt8 )
{
    OUString uName;
    rVal >>= uName;
    SetValue(SwStyleNameMapper::GetUIName(uName, nsSwGetPoolIdFromName::GET_POOLID_NUMRULE));
    return true;
}

void SwNumRuleItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swNumRuleItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(GetValue().toUtf8().getStr()));
    xmlTextWriterEndElement(pWriter);
}

SfxPoolItem* SwParaConnectBorderItem::Clone( SfxItemPool * ) const
{
    return new SwParaConnectBorderItem( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
