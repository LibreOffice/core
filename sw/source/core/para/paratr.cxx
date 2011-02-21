/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

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

using namespace ::com::sun::star;

TYPEINIT2_AUTOFACTORY( SwFmtDrop, SfxPoolItem, SwClient);
TYPEINIT1_AUTOFACTORY( SwRegisterItem, SfxBoolItem);
TYPEINIT1_AUTOFACTORY( SwNumRuleItem, SfxStringItem);
TYPEINIT1_AUTOFACTORY( SwParaConnectBorderItem, SfxBoolItem);

/*************************************************************************
|*    Beschreibung      Methoden von SwFmtDrop
*************************************************************************/



SwFmtDrop::SwFmtDrop()
    : SfxPoolItem( RES_PARATR_DROP ),
    SwClient( 0 ),
    pDefinedIn( 0 ),
    nDistance( 0 ),
    nReadFmt( USHRT_MAX ),
    nLines( 0 ),
    nChars( 0 ),
    bWholeWord( sal_False )
{
}



SwFmtDrop::SwFmtDrop( const SwFmtDrop &rCpy )
    : SfxPoolItem( RES_PARATR_DROP ),
    SwClient( rCpy.pRegisteredIn ),
    pDefinedIn( 0 ),
    nDistance( rCpy.GetDistance() ),
    nReadFmt( rCpy.nReadFmt ),
    nLines( rCpy.GetLines() ),
    nChars( rCpy.GetChars() ),
    bWholeWord( rCpy.GetWholeWord() )
{
}



SwFmtDrop::~SwFmtDrop()
{
}



void SwFmtDrop::SetCharFmt( SwCharFmt *pNew )
{
    //Ummelden
    if ( pRegisteredIn )
        pRegisteredIn->Remove( this );
    if(pNew)
        pNew->Add( this );
    nReadFmt = USHRT_MAX;
}



void SwFmtDrop::Modify( SfxPoolItem *, SfxPoolItem * )
{
    if( pDefinedIn )
    {
        if( !pDefinedIn->ISA( SwFmt ))
            pDefinedIn->Modify( this, this );
        else if( pDefinedIn->GetDepends() &&
                !pDefinedIn->IsModifyLocked() )
        {
            // selbst den Abhaengigen vom Format bescheid sagen. Das
            // Format selbst wuerde es nicht weitergeben, weil es ueber
            // die Abpruefung nicht hinauskommt.
            SwClientIter aIter( *pDefinedIn );
            SwClient * pLast = aIter.GoStart();
            if( pLast )     // konnte zum Anfang gesprungen werden ??
                do {
                    pLast->Modify( this, this );
                    if( !pDefinedIn->GetDepends() ) // Baum schon Weg ??
                        break;
                } while( 0 != ( pLast = aIter++ ));
        }
    }
}

sal_Bool SwFmtDrop::GetInfo( SfxPoolItem& ) const
{
    return sal_True;    // weiter
}

int SwFmtDrop::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return ( nLines == ((SwFmtDrop&)rAttr).GetLines() &&
             nChars == ((SwFmtDrop&)rAttr).GetChars() &&
             nDistance ==  ((SwFmtDrop&)rAttr).GetDistance() &&
             bWholeWord == ((SwFmtDrop&)rAttr).GetWholeWord() &&
             GetCharFmt() == ((SwFmtDrop&)rAttr).GetCharFmt() &&
             pDefinedIn == ((SwFmtDrop&)rAttr).pDefinedIn );
}

SfxPoolItem* SwFmtDrop::Clone( SfxItemPool* ) const
{
    return new SwFmtDrop( *this );
}

bool SwFmtDrop::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    switch(nMemberId&~CONVERT_TWIPS)
    {
        case MID_DROPCAP_LINES : rVal <<= (sal_Int16)nLines; break;
        case MID_DROPCAP_COUNT : rVal <<= (sal_Int16)nChars; break;
        case MID_DROPCAP_DISTANCE : rVal <<= (sal_Int16) TWIP_TO_MM100_UNSIGNED(nDistance); break;
        case MID_DROPCAP_FORMAT:
        {
             style::DropCapFormat aDrop;
            aDrop.Lines = nLines   ;
            aDrop.Count = nChars   ;
            aDrop.Distance  = TWIP_TO_MM100_UNSIGNED(nDistance);
            rVal.setValue(&aDrop, ::getCppuType((const style::DropCapFormat*)0));
        }
        break;
        case MID_DROPCAP_WHOLE_WORD:
            rVal.setValue(&bWholeWord, ::getBooleanCppuType());
        break;
        case MID_DROPCAP_CHAR_STYLE_NAME :
        {
            rtl::OUString sName;
            if(GetCharFmt())
                sName = SwStyleNameMapper::GetProgName(
                        GetCharFmt()->GetName(), nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
            rVal <<= sName;
        }
        break;
    }
    return true;
}

bool SwFmtDrop::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    switch(nMemberId&~CONVERT_TWIPS)
    {
        case MID_DROPCAP_LINES :
        {
            sal_Int8 nTemp = 0;
            rVal >>= nTemp;
            if(nTemp >=1 && nTemp < 0x7f)
                nLines = (BYTE)nTemp;
        }
        break;
        case MID_DROPCAP_COUNT :
        {
            sal_Int16 nTemp = 0;
            rVal >>= nTemp;
            if(nTemp >=1 && nTemp < 0x7f)
                nChars = (BYTE)nTemp;
        }
        break;
        case MID_DROPCAP_DISTANCE :
        {
            sal_Int16 nVal = 0;
            if ( rVal >>= nVal )
                nDistance = (sal_Int16) MM100_TO_TWIP((sal_Int32)nVal);
            else
                return false;
            break;
        }
        case MID_DROPCAP_FORMAT:
        {
            if(rVal.getValueType()  == ::getCppuType((const style::DropCapFormat*)0))
            {
                const style::DropCapFormat* pDrop = (const style::DropCapFormat*)rVal.getValue();
                nLines      = pDrop->Lines;
                nChars      = pDrop->Count;
                nDistance   = MM100_TO_TWIP(pDrop->Distance);
            }
            else {
            }
        }
        break;
        case MID_DROPCAP_WHOLE_WORD:
            bWholeWord = *(sal_Bool*)rVal.getValue();
        break;
        case MID_DROPCAP_CHAR_STYLE_NAME :
            DBG_ERROR("char format cannot be set in PutValue()!");
        break;
    }
    return true;
}

// class SwRegisterItem -------------------------------------------------


SfxPoolItem* SwRegisterItem::Clone( SfxItemPool * ) const
{
    return new SwRegisterItem( *this );
}

// class SwNumRuleItem -------------------------------------------------
SfxPoolItem* SwNumRuleItem::Clone( SfxItemPool * ) const
{
    return new SwNumRuleItem( *this );
}
int SwNumRuleItem::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );

    return GetValue() == ((SwNumRuleItem&)rAttr).GetValue();
}

bool    SwNumRuleItem::QueryValue( uno::Any& rVal, BYTE ) const
{
    rtl::OUString sRet = SwStyleNameMapper::GetProgName(GetValue(), nsSwGetPoolIdFromName::GET_POOLID_NUMRULE );
    rVal <<= sRet;
    return true;
}

bool    SwNumRuleItem::PutValue( const uno::Any& rVal, BYTE )
{
    rtl::OUString uName;
    rVal >>= uName;
    SetValue(SwStyleNameMapper::GetUIName(uName, nsSwGetPoolIdFromName::GET_POOLID_NUMRULE));
    return true;
}

SfxPoolItem* SwParaConnectBorderItem::Clone( SfxItemPool * ) const
{
    return new SwParaConnectBorderItem( *this );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
