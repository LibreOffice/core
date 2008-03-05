/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hints.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:52:01 $
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

#ifndef _HINTS_HXX
#include <hints.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <svx/scripttypeitem.hxx>
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


SwDocPosUpdate::SwDocPosUpdate( const SwTwips nDcPos )
    : SwMsgPoolItem( RES_DOCPOS_UPDATE ),
    nDocPos(nDcPos)
{}



// SwTableFmlUpdate wird verschickt, wenn sich die Tabelle neu berechnen soll
SwTableFmlUpdate::SwTableFmlUpdate( const SwTable* pNewTbl )
    : SwMsgPoolItem( RES_TABLEFML_UPDATE ),
    pTbl( pNewTbl ), pHistory( 0 ), nSplitLine( USHRT_MAX ),
    eFlags( TBL_CALC )
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
    bDelSet( FALSE ),
    pChgSet( &rSet ),
    pTheChgdSet( &rTheSet )
{}


SwAttrSetChg::SwAttrSetChg( const SwAttrSetChg& rChgSet )
    : SwMsgPoolItem( RES_ATTRSET_CHG ),
    bDelSet( TRUE ),
    pTheChgdSet( rChgSet.pTheChgdSet )
{
    pChgSet = new SwAttrSet( *rChgSet.pChgSet );
}


SwAttrSetChg::~SwAttrSetChg()
{
    if( bDelSet )
        delete pChgSet;
}


#ifndef PRODUCT

void SwAttrSetChg::ClearItem( USHORT nWhch )
{
    ASSERT( bDelSet, "der Set darf nicht veraendert werden!" );
    pChgSet->ClearItem( nWhch );
}

#endif


SwMsgPoolItem::SwMsgPoolItem( USHORT nWhch )
    : SfxPoolItem( nWhch )
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
    aList.Insert(rNd.GetIndex(), &rNd);
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
            if( ScriptType::WEAK == nScript )
                nScript = GetI18NScriptTypeOfLanguage( (USHORT)GetAppLanguage() );
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
