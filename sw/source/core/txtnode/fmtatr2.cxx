/*************************************************************************
 *
 *  $RCSfile: fmtatr2.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-02 17:28:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include "hintids.hxx"
#include "unomid.h"

#ifndef __SBX_SBXVARIABLE_HXX //autogen
#include <svtools/sbxvar.hxx>
#endif
#ifndef _SFXMACITEM_HXX //autogen
#include <svtools/macitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif

#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _TXTATR_HXX //autogen
#include <txtatr.hxx>
#endif
#ifndef _FMT2LINES_HXX
#include <fmt2lines.hxx>
#endif
#ifndef _FMTRUBY_HXX
#include <fmtruby.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>        // SwUpdateAttr
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef _UNOSTYLE_HXX
#include <unostyle.hxx>
#endif


using namespace ::com::sun::star;
using namespace ::rtl;

/*************************************************************************
|*
|*    class SwFmtCharFmt
|*    Beschreibung
|*    Ersterstellung    JP 23.11.90
|*    Letzte Aenderung  JP 09.08.94
|*
*************************************************************************/



SwFmtCharFmt::SwFmtCharFmt( SwCharFmt *pFmt )
    : SfxPoolItem( RES_TXTATR_CHARFMT ),
    SwClient(pFmt),
    pTxtAttr( 0 )
{
}



SwFmtCharFmt::SwFmtCharFmt( const SwFmtCharFmt& rAttr )
    : SfxPoolItem( RES_TXTATR_CHARFMT ),
    SwClient( rAttr.GetCharFmt() ),
    pTxtAttr( 0 )
{
}



SwFmtCharFmt::~SwFmtCharFmt() {}



int SwFmtCharFmt::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return GetCharFmt() == ((SwFmtCharFmt&)rAttr).GetCharFmt();
}



SfxPoolItem* SwFmtCharFmt::Clone( SfxItemPool* ) const
{
    return new SwFmtCharFmt( *this );
}



// weiterleiten an das TextAttribut
void SwFmtCharFmt::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    if( pTxtAttr )
        pTxtAttr->Modify( pOld, pNew );
}



// weiterleiten an das TextAttribut
BOOL SwFmtCharFmt::GetInfo( SfxPoolItem& rInfo ) const
{
    return pTxtAttr ? pTxtAttr->GetInfo( rInfo ) : FALSE;
}
BOOL SwFmtCharFmt::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    XubString sCharFmtName;
    if(GetCharFmt())
        sCharFmtName = SwXStyleFamilies::GetProgrammaticName(GetCharFmt()->GetName(), SFX_STYLE_FAMILY_CHAR );
    rVal <<= OUString( sCharFmtName );
    return TRUE;
}
BOOL SwFmtCharFmt::PutValue( const uno::Any& rVal, BYTE nMemberId  )
{
    DBG_ERROR("Zeichenvorlage kann mit PutValue nicht gesetzt werden!")
    return FALSE;
}

/*************************************************************************
|*
|*    class SwFmtINetFmt
|*    Beschreibung
|*    Ersterstellung    AMA 02.08.96
|*    Letzte Aenderung  AMA 02.08.96
|*
*************************************************************************/



SwFmtINetFmt::SwFmtINetFmt( const XubString& rURL, const XubString& rTarget )
    : SfxPoolItem( RES_TXTATR_INETFMT ),
    aURL( rURL ),
    aTargetFrame( rTarget ),
    pTxtAttr( 0 ),
    pMacroTbl( 0 ),
    nINetId( 0 ),
    nVisitedId( 0 )
{
}



SwFmtINetFmt::SwFmtINetFmt( const SwFmtINetFmt& rAttr )
    : SfxPoolItem( RES_TXTATR_INETFMT ),
    aURL( rAttr.GetValue() ),
    aName( rAttr.aName ),
    aTargetFrame( rAttr.aTargetFrame ),
    aINetFmt( rAttr.aINetFmt ),
    aVisitedFmt( rAttr.aVisitedFmt ),
    pTxtAttr( 0 ),
    pMacroTbl( 0 ),
    nINetId( rAttr.nINetId ),
    nVisitedId( rAttr.nVisitedId )
{
    if( rAttr.GetMacroTbl() )
        pMacroTbl = new SvxMacroTableDtor( *rAttr.GetMacroTbl() );
}



SwFmtINetFmt::~SwFmtINetFmt()
{
    delete pMacroTbl;
}



int SwFmtINetFmt::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    BOOL bRet = SfxPoolItem::operator==( (SfxPoolItem&) rAttr )
                && aURL == ((SwFmtINetFmt&)rAttr).aURL
                && aName == ((SwFmtINetFmt&)rAttr).aName
                && aTargetFrame == ((SwFmtINetFmt&)rAttr).aTargetFrame
                && aINetFmt == ((SwFmtINetFmt&)rAttr).aINetFmt
                && aVisitedFmt == ((SwFmtINetFmt&)rAttr).aVisitedFmt
                && nINetId == ((SwFmtINetFmt&)rAttr).nINetId
                && nVisitedId == ((SwFmtINetFmt&)rAttr).nVisitedId;

    if( !bRet )
        return FALSE;

    const SvxMacroTableDtor* pOther = ((SwFmtINetFmt&)rAttr).pMacroTbl;
    if( !pMacroTbl )
        return ( !pOther || !pOther->Count() );
    if( !pOther )
        return 0 == pMacroTbl->Count();

    const SvxMacroTableDtor& rOwn = *pMacroTbl;
    const SvxMacroTableDtor& rOther = *pOther;

    // Anzahl unterschiedlich => auf jeden Fall ungleich
    if( rOwn.Count() != rOther.Count() )
        return FALSE;

    // einzeln vergleichen; wegen Performance ist die Reihenfolge wichtig
    for( USHORT nNo = 0; nNo < rOwn.Count(); ++nNo )
    {
        const SvxMacro *pOwnMac = rOwn.GetObject(nNo);
        const SvxMacro *pOtherMac = rOther.GetObject(nNo);
        if (    rOwn.GetKey(pOwnMac) != rOther.GetKey(pOtherMac)  ||
                pOwnMac->GetLibName() != pOtherMac->GetLibName() ||
                pOwnMac->GetMacName() != pOtherMac->GetMacName() )
            return FALSE;
    }
    return TRUE;
}



SfxPoolItem* SwFmtINetFmt::Clone( SfxItemPool* ) const
{
    return new SwFmtINetFmt( *this );
}



void SwFmtINetFmt::SetMacroTbl( const SvxMacroTableDtor* pNewTbl )
{
    if( pNewTbl )
    {
        if( pMacroTbl )
            *pMacroTbl = *pNewTbl;
        else
            pMacroTbl = new SvxMacroTableDtor( *pNewTbl );
    }
    else if( pMacroTbl )
        delete pMacroTbl, pMacroTbl = 0;
}



void SwFmtINetFmt::SetMacro( USHORT nEvent, const SvxMacro& rMacro )
{
    if( !pMacroTbl )
        pMacroTbl = new SvxMacroTableDtor;

    SvxMacro *pOldMacro;
    if( 0 != ( pOldMacro = pMacroTbl->Get( nEvent )) )
    {
        delete pOldMacro;
        pMacroTbl->Replace( nEvent, new SvxMacro( rMacro ) );
    }
    else
        pMacroTbl->Insert( nEvent, new SvxMacro( rMacro ) );
}



const SvxMacro* SwFmtINetFmt::GetMacro( USHORT nEvent ) const
{
    const SvxMacro* pRet = 0;
    if( pMacroTbl && pMacroTbl->IsKeyValid( nEvent ) )
        pRet = pMacroTbl->Get( nEvent );
    return pRet;
}



BOOL SwFmtINetFmt::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    BOOL bRet = TRUE;
    XubString sVal;
    switch(nMemberId)
    {
        case MID_URL_URL:
            sVal = aURL;
        break;
        case MID_URL_TARGET:
            sVal = aTargetFrame;
        break;
        case MID_URL_HYPERLINKNAME:
            sVal = aName;
        break;
        case MID_URL_VISITED_FMT:
            sVal = SwXStyleFamilies::GetProgrammaticName( aVisitedFmt,
                                                    SFX_STYLE_FAMILY_CHAR );
        break;
        case MID_URL_UNVISITED_FMT:
            sVal = SwXStyleFamilies::GetProgrammaticName( aINetFmt,
                                                    SFX_STYLE_FAMILY_CHAR );
        break;
        default:
            bRet = FALSE;
    }
    rVal <<= OUString(sVal);
    return bRet;
}
BOOL SwFmtINetFmt::PutValue( const uno::Any& rVal, BYTE nMemberId  )
{
    BOOL bRet = TRUE;
    if(rVal.getValueType() != ::getCppuType((rtl::OUString*)0))
        return FALSE;
    XubString sVal = *(rtl::OUString*)rVal.getValue();
    switch(nMemberId)
    {
        case MID_URL_URL:
            aURL = sVal;
        break;
        case MID_URL_TARGET:
            aTargetFrame = sVal;
        break;
        case MID_URL_HYPERLINKNAME:
             aName = sVal;
        break;
        case MID_URL_VISITED_FMT:
            aVisitedFmt = SwXStyleFamilies::GetUIName( sVal,
                                                       SFX_STYLE_FAMILY_CHAR );
            nVisitedId = USHRT_MAX;
        break;
        case MID_URL_UNVISITED_FMT:
            aINetFmt = SwXStyleFamilies::GetUIName( sVal,
                                                    SFX_STYLE_FAMILY_CHAR );
            nINetId = USHRT_MAX;
        break;
        default:
            bRet = FALSE;
    }
    return bRet;
}




/*************************************************************************
|*    class SwFmt2Lines
*************************************************************************/

SwFmt2Lines::SwFmt2Lines( sal_Bool bFlag, sal_Unicode nStartBracket,
                                          sal_Unicode nEndBracket )
    : SfxPoolItem( RES_CHRATR_TWO_LINES ),
    bOn( bFlag ), cStartBracket( nStartBracket ), cEndBracket( nEndBracket )
{
}

SwFmt2Lines::SwFmt2Lines( const SwFmt2Lines& rAttr )
    : SfxPoolItem( RES_CHRATR_TWO_LINES ),
    bOn( rAttr.bOn ), cStartBracket( rAttr.cStartBracket ),
    cEndBracket( rAttr.cEndBracket )
{
}

SwFmt2Lines::~SwFmt2Lines()
{
}

int SwFmt2Lines::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return bOn == ((SwFmt2Lines&)rAttr).bOn &&
           cStartBracket == ((SwFmt2Lines&)rAttr).cStartBracket &&
           cEndBracket == ((SwFmt2Lines&)rAttr).cEndBracket;
}

SfxPoolItem* SwFmt2Lines::Clone( SfxItemPool* ) const
{
    return new SwFmt2Lines( *this );
}

BOOL SwFmt2Lines::QueryValue( com::sun::star::uno::Any& rVal,
                                BYTE nMemberId ) const
{
    BOOL bRet = TRUE;
    XubString sVal;
    switch( nMemberId )
    {
/*  case MID_URL_URL:
        sVal = aURL;
        break;
    case MID_URL_TARGET:
        sVal = aTargetFrame;
        break;
    case MID_URL_HYPERLINKNAME:
        sVal = aName;
        break;
    case MID_URL_VISITED_FMT:
        sVal = SwXStyleFamilies::GetProgrammaticName( aVisitedFmt,
                                                    SFX_STYLE_FAMILY_CHAR );
        break;
    case MID_URL_UNVISITED_FMT:
        sVal = SwXStyleFamilies::GetProgrammaticName( aINetFmt,
                                                    SFX_STYLE_FAMILY_CHAR );
        break;
*/
    default:
        bRet = FALSE;
        break;
    }
    rVal <<= OUString(sVal);
    return bRet;
}

BOOL SwFmt2Lines::PutValue( const com::sun::star::uno::Any& rVal,
                            BYTE nMemberId )
{
    BOOL bRet;
    if( rVal.getValueType() == ::getCppuType((rtl::OUString*)0) )
    {
        bRet = TRUE;

        XubString sVal = *(rtl::OUString*)rVal.getValue();
        switch( nMemberId )
        {
/*      case MID_URL_URL:
            aURL = sVal;
            break;
        case MID_URL_TARGET:
            aTargetFrame = sVal;
            break;
        case MID_URL_HYPERLINKNAME:
            aName = sVal;
            break;
        case MID_URL_VISITED_FMT:
            aVisitedFmt = SwXStyleFamilies::GetUIName( sVal,
                                                    SFX_STYLE_FAMILY_CHAR );
            nVisitedId = USHRT_MAX;
            break;
        case MID_URL_UNVISITED_FMT:
            aINetFmt = SwXStyleFamilies::GetUIName( sVal,
                                                    SFX_STYLE_FAMILY_CHAR );
            nINetId = USHRT_MAX;
            break;
*/      default:
            bRet = FALSE;
            break;
        }
    }
    else
        bRet = FALSE;
    return bRet;
}





/*************************************************************************
|*    class SwFmtRuby
*************************************************************************/

SwFmtRuby::SwFmtRuby( const String& rRubyTxt )
    : SfxPoolItem( RES_TXTATR_CJK_RUBY ),
    sRubyTxt( rRubyTxt ),
    nCharFmtId( 0 ),
    nPosition( 0 ), nAdjustment( 0 ),
    pTxtAttr( 0 )
{
}

SwFmtRuby::SwFmtRuby( const SwFmtRuby& rAttr )
    : SfxPoolItem( RES_TXTATR_CJK_RUBY ),
    sRubyTxt( rAttr.sRubyTxt ),
    sCharFmtName( rAttr.sCharFmtName ),
    nCharFmtId( rAttr.nCharFmtId),
    nPosition( rAttr.nPosition ), nAdjustment( rAttr.nAdjustment ),
    pTxtAttr( 0 )
{
}

SwFmtRuby::~SwFmtRuby()
{
}

int SwFmtRuby::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return sRubyTxt == ((SwFmtRuby&)rAttr).sRubyTxt &&
           sCharFmtName == ((SwFmtRuby&)rAttr).sCharFmtName &&
           nCharFmtId == ((SwFmtRuby&)rAttr).nCharFmtId &&
           nPosition == ((SwFmtRuby&)rAttr).nPosition &&
           nAdjustment == ((SwFmtRuby&)rAttr).nAdjustment;
}

SfxPoolItem* SwFmtRuby::Clone( SfxItemPool* ) const
{
    return new SwFmtRuby( *this );
}

BOOL SwFmtRuby::QueryValue( com::sun::star::uno::Any& rVal,
                            BYTE nMemberId ) const
{
    BOOL bRet = TRUE;
    XubString sVal;
    switch( nMemberId )
    {
/*  case MID_URL_URL:
        sVal = aURL;
        break;
    case MID_URL_TARGET:
        sVal = aTargetFrame;
        break;
    case MID_URL_HYPERLINKNAME:
        sVal = aName;
        break;
    case MID_URL_VISITED_FMT:
        sVal = SwXStyleFamilies::GetProgrammaticName( aVisitedFmt,
                                                    SFX_STYLE_FAMILY_CHAR );
        break;
    case MID_URL_UNVISITED_FMT:
        sVal = SwXStyleFamilies::GetProgrammaticName( aINetFmt,
                                                    SFX_STYLE_FAMILY_CHAR );
        break;
*/
    default:
        bRet = FALSE;
        break;
    }
    rVal <<= OUString(sVal);
    return bRet;
}
BOOL SwFmtRuby::PutValue( const com::sun::star::uno::Any& rVal,
                            BYTE nMemberId  )
{
    BOOL bRet;
    if( rVal.getValueType() == ::getCppuType((rtl::OUString*)0) )
    {
        bRet = TRUE;

        XubString sVal = *(rtl::OUString*)rVal.getValue();
        switch( nMemberId )
        {
/*      case MID_URL_URL:
            aURL = sVal;
            break;
        case MID_URL_TARGET:
            aTargetFrame = sVal;
            break;
        case MID_URL_HYPERLINKNAME:
            aName = sVal;
            break;
        case MID_URL_VISITED_FMT:
            aVisitedFmt = SwXStyleFamilies::GetUIName( sVal,
                                                    SFX_STYLE_FAMILY_CHAR );
            nVisitedId = USHRT_MAX;
            break;
        case MID_URL_UNVISITED_FMT:
            aINetFmt = SwXStyleFamilies::GetUIName( sVal,
                                                    SFX_STYLE_FAMILY_CHAR );
            nINetId = USHRT_MAX;
            break;
*/      default:
            bRet = FALSE;
            break;
        }
    }
    else
        bRet = FALSE;
    return bRet;
}

