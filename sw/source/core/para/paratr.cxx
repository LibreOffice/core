/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#include "errhdl.hxx"
#include "paratr.hxx"
#include "charfmt.hxx"
#include "cmdid.h"

using namespace ::com::sun::star;

/*************************************************************************
|*    Beschreibung      Methoden von SwFmtDrop
|*    Ersterstellung    MS  19.02.91
|*    Letzte Aenderung  JP 08.08.94
*************************************************************************/

IMPL_POOLITEM_FACTORY(SwFmtDrop)

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
    SwClient( rCpy.GetRegisteredInNonConst() ),
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
    if ( GetRegisteredIn() )
        GetRegisteredInNonConst()->Remove( this );
    if(pNew)
        pNew->Add( this );
    nReadFmt = USHRT_MAX;
}



void SwFmtDrop::Modify( const SfxPoolItem*, const SfxPoolItem * )
{
    if( pDefinedIn )
    {
        if( !dynamic_cast< SwFmt* >(pDefinedIn))
            pDefinedIn->ModifyNotification( this, this );
        else if( pDefinedIn->GetDepends() &&
                !pDefinedIn->IsModifyLocked() )
        {
            // selbst den Abhaengigen vom Format bescheid sagen. Das
            // Format selbst wuerde es nicht weitergeben, weil es ueber
            // die Abpruefung nicht hinauskommt.
            pDefinedIn->ModifyBroadcast( this, this );
        }
    }
}

sal_Bool SwFmtDrop::GetInfo( SfxPoolItem& ) const
{
    return sal_True;    // weiter
}

int SwFmtDrop::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
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

sal_Bool SwFmtDrop::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
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
    return sal_True;
}

sal_Bool SwFmtDrop::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
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
                nDistance = (sal_Int16) MM100_TO_TWIP((sal_Int32)nVal);
            else
                return sal_False;
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
                //exception( wrong_type)
                ;
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
    return sal_True;
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
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    // --> OD 2008-03-04 #refactorlists# - removed <pDefinedIn>
    return GetValue() == ((SwNumRuleItem&)rAttr).GetValue();
    // <--
}
/* -----------------------------27.06.00 11:05--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool    SwNumRuleItem::QueryValue( uno::Any& rVal, sal_uInt8 ) const
{
    rtl::OUString sRet = SwStyleNameMapper::GetProgName(GetValue(), nsSwGetPoolIdFromName::GET_POOLID_NUMRULE );
    rVal <<= sRet;
    return sal_True;
}
/* -----------------------------27.06.00 11:05--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool    SwNumRuleItem::PutValue( const uno::Any& rVal, sal_uInt8 )
{
    rtl::OUString uName;
    rVal >>= uName;
    SetValue(SwStyleNameMapper::GetUIName(uName, nsSwGetPoolIdFromName::GET_POOLID_NUMRULE));
    return sal_True;
}
/* -----------------19.05.2003 10:44-----------------

 --------------------------------------------------*/
SfxPoolItem* SwParaConnectBorderItem::Clone( SfxItemPool * ) const
{
    return new SwParaConnectBorderItem( *this );
}



