/*************************************************************************
 *
 *  $RCSfile: hints.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2001-02-21 12:46:39 $
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

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _SWERROR_H
#include <error.h>
#endif



SwFmtChg::SwFmtChg( SwFmt *pFmt )
    : SwMsgPoolItem( RES_FMT_CHG ),
    pChangedFmt( pFmt )
{}


SwInsChr::SwInsChr( xub_StrLen nP )
    : SwMsgPoolItem( RES_INS_CHR ),
    nPos( nP )
{}



SwInsTxt::SwInsTxt( xub_StrLen nP, xub_StrLen nL )
    : SwMsgPoolItem( RES_INS_TXT ),
    nPos( nP ),
    nLen( nL )
{}



SwDelChr::SwDelChr( xub_StrLen nP )
    : SwMsgPoolItem( RES_DEL_CHR ),
    nPos( nP )
{}



SwDelTxt::SwDelTxt( xub_StrLen nS, xub_StrLen nL )
    : SwMsgPoolItem( RES_DEL_TXT ),
    nStart( nS ),
    nLen( nL )
{}



SwUpdateAttr::SwUpdateAttr( xub_StrLen nS, xub_StrLen nE, USHORT nW )
    : SwMsgPoolItem( RES_UPDATE_ATTR ),
    nStart( nS ),
    nEnd( nE ),
    nWhichAttr( nW )
{}


// SwRefMarkFldUpdate wird verschickt, wenn sich die ReferenzMarkierungen
// Updaten sollen. Um Seiten-/KapitelNummer feststellen zu koennen, muss
// der akt. Frame befragt werden. Dafuer wird das akt. OutputDevice benoetigt.


SwRefMarkFldUpdate::SwRefMarkFldUpdate( const OutputDevice* pOutput )
    : SwMsgPoolItem( RES_REFMARKFLD_UPDATE ),
    pOut( pOutput )
{
    ASSERT( pOut, "es muss ein OutputDevice-Pointer gesetzt werden!" );
}


SwDocPosUpdate::SwDocPosUpdate( const SwTwips nDocPos )
    : SwMsgPoolItem( RES_DOCPOS_UPDATE ),
    nDocPos(nDocPos)
{}



// SwTableFmlUpdate wird verschickt, wenn sich die Tabelle neu berechnen soll
SwTableFmlUpdate::SwTableFmlUpdate( const SwTable* pNewTbl )
    : SwMsgPoolItem( RES_TABLEFML_UPDATE ),
    pTbl( pNewTbl ), pHistory( 0 ), eFlags( TBL_CALC ),
    nSplitLine( USHRT_MAX )
{
    DATA.pDelTbl = 0;
    bModified = bBehindSplitLine = FALSE;
    ASSERT( pTbl, "es muss ein Table-Pointer gesetzt werden!" );
}


SwAutoFmtGetDocNode::SwAutoFmtGetDocNode( const SwNodes* pNds )
    : SwMsgPoolItem( RES_AUTOFMT_DOCNODE ),
    pCntntNode( 0 ), pNodes( pNds )
{}


SwAttrSetChg::SwAttrSetChg( const SwAttrSet& rTheSet, SwAttrSet& rSet )
    : SwMsgPoolItem( RES_ATTRSET_CHG ),
    pTheChgdSet( &rTheSet ),
    pChgSet( &rSet ),
    bDelSet( FALSE )
{}


SwAttrSetChg::SwAttrSetChg( const SwAttrSetChg& rChgSet )
    : SwMsgPoolItem( RES_ATTRSET_CHG ),
    pTheChgdSet( rChgSet.pTheChgdSet ),
    bDelSet( TRUE )
{
    pChgSet = new SwAttrSet( *rChgSet.pChgSet );
}


SwAttrSetChg::~SwAttrSetChg()
{
    if( bDelSet )
        delete pChgSet;
}


#ifndef PRODUCT

void SwAttrSetChg::ClearItem( USHORT nWhich )
{
    ASSERT( bDelSet, "der Set darf nicht veraendert werden!" );
    pChgSet->ClearItem( nWhich );
}

#endif


SwMsgPoolItem::SwMsgPoolItem( USHORT nWhich )
    : SfxPoolItem( nWhich )
{}


// "Overhead" vom SfxPoolItem
int SwMsgPoolItem::operator==( const SfxPoolItem& ) const
{
    ASSERT( FALSE, "SwMsgPoolItem kennt kein ==" );
    return 0;
}


SfxPoolItem* SwMsgPoolItem::Clone( SfxItemPool* ) const
{
    ASSERT( FALSE, "SwMsgPoolItem kennt kein Clone" );
    return 0;
}

/******************************************************************************
 * hole aus der Default-Attribut Tabelle ueber den Which-Wert
 * das entsprechende default Attribut.
 * Ist keines vorhanden, returnt ein 0-Pointer !!!
 * inline (hintids.hxx) im PRODUCT.
 ******************************************************************************/
#ifndef PRODUCT


const SfxPoolItem* GetDfltAttr( USHORT nWhich )
{
    ASSERT_ID( nWhich < POOLATTR_END && nWhich >= POOLATTR_BEGIN,
               ERR_OUTOFSCOPE );

    SfxPoolItem *pHt = aAttrTab[ nWhich - POOLATTR_BEGIN ];
    ASSERT( pHt, "GetDfltFmtAttr(): Dflt == 0" );
    return pHt;
}

#endif



SwCondCollCondChg::SwCondCollCondChg( SwFmt *pFmt )
    : SwMsgPoolItem( RES_CONDCOLL_CONDCHG ), pChangedFmt( pFmt )
{
}


SwVirtPageNumInfo::SwVirtPageNumInfo( const SwPageFrm *pPg ) :
    SwMsgPoolItem( RES_VIRTPAGENUM_INFO ),
    pPage( 0 ),
    pOrigPage( pPg ),
    pFrm( 0 )
{
}

SwNumRuleInfo::SwNumRuleInfo( const String& rRuleName )
    : SwMsgPoolItem( RES_GETNUMNODES ), rName( rRuleName )
{
}

void SwNumRuleInfo::AddNode( SwTxtNode& rNd )
{
    aList.Insert( rNd.GetIndex(), &rNd );
}

SwNRuleLowerLevel::SwNRuleLowerLevel( const String& rRuleName, BYTE nSrchLvl )
    : SwMsgPoolItem( RES_GETLOWERNUMLEVEL ), rName( rRuleName ),
    nLvl(nSrchLvl)
{
}


SwFindNearestNode::SwFindNearestNode( const SwNode& rNd )
    : SwMsgPoolItem( RES_FINDNEARESTNODE ), pNd( &rNd ), pFnd( 0 )
{
}

void SwFindNearestNode::CheckNode( const SwNode& rNd )
{
    if( &pNd->GetNodes() == &rNd.GetNodes() )
    {
        ULONG nIdx = rNd.GetIndex();
        if( nIdx < pNd->GetIndex() &&
            ( !pFnd || nIdx > pFnd->GetIndex() ) &&
            nIdx > rNd.GetNodes().GetEndOfExtras().GetIndex() )
            pFnd = &rNd;
    }
}



USHORT GetWhichOfScript( USHORT nWhich, USHORT nScript )
{
    static const USHORT aLangMap[3] =
        { RES_CHRATR_LANGUAGE, RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CTL_LANGUAGE };
    static const USHORT aFontMap[3] =
        { RES_CHRATR_FONT, RES_CHRATR_CJK_FONT,  RES_CHRATR_CTL_FONT};
    static const USHORT aFontSizeMap[3] =
        { RES_CHRATR_FONTSIZE, RES_CHRATR_CJK_FONTSIZE,  RES_CHRATR_CTL_FONTSIZE };
    static const USHORT aWeightMap[3] =
        { RES_CHRATR_WEIGHT, RES_CHRATR_CJK_WEIGHT,  RES_CHRATR_CTL_WEIGHT};
    static const USHORT aPostureMap[3] =
        { RES_CHRATR_POSTURE, RES_CHRATR_CJK_POSTURE,  RES_CHRATR_CTL_POSTURE};

    const USHORT* pM;
    switch( nWhich )
    {
    case RES_CHRATR_LANGUAGE:
    case RES_CHRATR_CJK_LANGUAGE:
    case RES_CHRATR_CTL_LANGUAGE:
        pM = aLangMap;
        break;

    case RES_CHRATR_FONT:
    case RES_CHRATR_CJK_FONT:
    case RES_CHRATR_CTL_FONT:
        pM = aFontMap;
        break;

    case RES_CHRATR_FONTSIZE:
    case RES_CHRATR_CJK_FONTSIZE:
    case RES_CHRATR_CTL_FONTSIZE:
        pM = aFontSizeMap;
        break;

    case  RES_CHRATR_WEIGHT:
    case  RES_CHRATR_CJK_WEIGHT:
    case  RES_CHRATR_CTL_WEIGHT:
        pM = aWeightMap;
        break;
    case RES_CHRATR_POSTURE:
    case RES_CHRATR_CJK_POSTURE:
    case RES_CHRATR_CTL_POSTURE:
        pM = aPostureMap;
        break;

    default:
        pM = 0;
    }

    USHORT nRet;
    if( pM )
    {
        using namespace ::com::sun::star::i18n;
        {
            switch( nScript)
            {
            case ScriptType::COMPLEX:   ++pM;  // no break;
            case ScriptType::ASIAN:     ++pM;  // no break;
            default:                    nRet = *pM;
            }
        }
    }
    else
        nRet = nWhich;
    return nRet;
}
