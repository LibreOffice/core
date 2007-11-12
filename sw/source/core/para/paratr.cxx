/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: paratr.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-12 16:26:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include "hintids.hxx"
#include <swtypes.hxx>
#include "unomid.h"
#ifndef _COM_SUN_STAR_STYLE_LINESPACINGMODE_HPP_
#include <com/sun/star/style/LineSpacingMode.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_PARAGRAPHADJUST_HPP_
#include <com/sun/star/style/ParagraphAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_DROPCAPFORMAT_HPP_
#include <com/sun/star/style/DropCapFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_LINESPACING_HPP_
#include <com/sun/star/style/LineSpacing.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_RELORIENTATION_HPP_
#include <com/sun/star/text/RelOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATION_HPP_
#include <com/sun/star/text/VertOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIZONTALADJUST_HPP_
#include <com/sun/star/text/HorizontalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_DOCUMENTSTATISTIC_HPP_
#include <com/sun/star/text/DocumentStatistic.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HPP_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATIONFORMAT_HPP_
#include <com/sun/star/text/HoriOrientationFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_NOTEPRINTMODE_HPP_
#include <com/sun/star/text/NotePrintMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_SIZETYPE_HPP_
#include <com/sun/star/text/SizeType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATIONFORMAT_HPP_
#include <com/sun/star/text/VertOrientationFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRAPTEXTMODE_HPP_
#include <com/sun/star/text/WrapTextMode.hpp>
#endif
#ifndef _UNOSTYLE_HXX
#include <unostyle.hxx>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
#include "errhdl.hxx"
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
|*    Ersterstellung    MS  19.02.91
|*    Letzte Aenderung  JP 08.08.94
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
            else
                //exception( wrong_type)
                ;
        }
        break;
        case MID_DROPCAP_WHOLE_WORD:
            bWholeWord = *(sal_Bool*)rVal.getValue();
        break;
        case MID_DROPCAP_CHAR_STYLE_NAME :
            DBG_ERROR("char format cannot be set in PutValue()!")
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
    return GetValue() == ((SwNumRuleItem&)rAttr).GetValue() &&
            GetDefinedIn() == ((SwNumRuleItem&)rAttr).GetDefinedIn();
}
/* -----------------------------27.06.00 11:05--------------------------------

 ---------------------------------------------------------------------------*/
BOOL    SwNumRuleItem::QueryValue( uno::Any& rVal, BYTE ) const
{
    rtl::OUString sRet = SwStyleNameMapper::GetProgName(GetValue(), nsSwGetPoolIdFromName::GET_POOLID_NUMRULE );
    rVal <<= sRet;
    return TRUE;
}
/* -----------------------------27.06.00 11:05--------------------------------

 ---------------------------------------------------------------------------*/
BOOL    SwNumRuleItem::PutValue( const uno::Any& rVal, BYTE )
{
    rtl::OUString uName;
    rVal >>= uName;
    SetValue(SwStyleNameMapper::GetUIName(uName, nsSwGetPoolIdFromName::GET_POOLID_NUMRULE));
    return TRUE;
}
/* -----------------19.05.2003 10:44-----------------

 --------------------------------------------------*/
SfxPoolItem* SwParaConnectBorderItem::Clone( SfxItemPool * ) const
{
    return new SwParaConnectBorderItem( *this );
}



