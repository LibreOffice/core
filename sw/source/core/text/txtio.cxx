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

#ifdef DBG_UTIL

#include "viewsh.hxx"       // IsDbg()
#include "viewopt.hxx"      // IsDbg()
#include "txtatr.hxx"
#include "errhdl.hxx"
#include "txtcfg.hxx"
#include "txtfrm.hxx"       // IsDbg()
#include "rootfrm.hxx"
#include "flyfrms.hxx"
#include "inftxt.hxx"
#include "porexp.hxx"
#include "porfld.hxx"
#include "porfly.hxx"
#include "porftn.hxx"
#include "porglue.hxx"
#include "porhyph.hxx"
#include "porlay.hxx"
#include "porlin.hxx"
#include "porref.hxx"
#include "porrst.hxx"
#include "portab.hxx"
#include "portox.hxx"
#include "portxt.hxx"
#include "pordrop.hxx"
#include "pormulti.hxx"
#include "ndhints.hxx"

// So kann man die Layoutstruktur ausgeben lassen
// #define AMA_LAYOUT
#ifdef AMA_LAYOUT
#include <stdio.h>
#include <stdlib.h>         // getenv()
#include <flowfrm.hxx>
#include <pagefrm.hxx>
#include <svx/svdobj.hxx>
#include <dflyobj.hxx>


void lcl_OutFollow( XubString &rTmp, const SwFrm* pFrm )
{
    if( pFrm->IsFlowFrm() )
    {
        const SwFlowFrm *pFlow = SwFlowFrm::CastFlowFrm( pFrm );
        if( pFlow->IsFollow() || pFlow->GetFollow() )
        {
            rTmp += "(";
            if( pFlow->IsFollow() )
                rTmp += ".";
            if( pFlow->GetFollow() )
            {
                MSHORT nFrmId = pFlow->GetFollow()->GetFrm()->GetFrmId();
                rTmp += nFrmId;
            }
            rTmp += ")";
        }
    }
}

void lcl_OutFrame( SvFileStream& rStr, const SwFrm* pFrm, ByteString& rSp, sal_Bool bNxt )
{
    if( !pFrm )
        return;
    KSHORT nSpc = 0;
    MSHORT nFrmId = pFrm->GetFrmId();
    ByteString aTmp;
    if( pFrm->IsLayoutFrm() )
    {
        if( pFrm->IsRootFrm() )
            aTmp = "R";
        else if( pFrm->IsPageFrm() )
            aTmp = "P";
        else if( pFrm->IsBodyFrm() )
            aTmp = "B";
        else if( pFrm->IsColumnFrm() )
            aTmp = "C";
        else if( pFrm->IsTabFrm() )
            aTmp = "Tb";
        else if( pFrm->IsRowFrm() )
            aTmp = "Rw";
        else if( pFrm->IsCellFrm() )
            aTmp = "Ce";
        else if( pFrm->IsSctFrm() )
            aTmp = "S";
        else if( pFrm->IsFlyFrm() )
        {
            aTmp = "F";
            const SwFlyFrm *pFly = (SwFlyFrm*)pFrm;
            if( pFly->IsFlyInCntFrm() )
                aTmp += "in";
            else if( pFly->IsFlyAtCntFrm() )
            {
                aTmp += "a";
                if( pFly->IsAutoPos() )
                    aTmp += "u";
                else
                    aTmp += "t";
            }
            else
                aTmp += "l";
        }
        else if( pFrm->IsHeaderFrm() )
            aTmp = "H";
        else if( pFrm->IsFooterFrm() )
            aTmp = "Fz";
        else if( pFrm->IsFtnContFrm() )
            aTmp = "Fc";
        else if( pFrm->IsFtnFrm() )
            aTmp = "Fn";
        else
            aTmp = "?L?";
        aTmp += nFrmId;
        lcl_OutFollow( aTmp, pFrm );
        aTmp += " ";
        rStr << aTmp;
        nSpc = aTmp.Len();
        rSp.Expand( nSpc + rSp.Len() );
        lcl_OutFrame( rStr, ((SwLayoutFrm*)pFrm)->Lower(), rSp, sal_True );
    }
    else
    {
        if( pFrm->IsTxtFrm() )
            aTmp = "T";
        else if( pFrm->IsNoTxtFrm() )
            aTmp = "N";
        else
            aTmp = "?C?";
        aTmp += nFrmId;
        lcl_OutFollow( aTmp, pFrm );
        aTmp += " ";
        rStr << aTmp;
        nSpc = aTmp.Len();
        rSp.Expand( nSpc + rSp.Len() );
    }
    if( pFrm->IsPageFrm() )
    {
        const SwPageFrm* pPg = (SwPageFrm*)pFrm;
        const SwSortedObjs *pSorted = pPg->GetSortedObjs();
        const MSHORT nCnt = pSorted ? pSorted->Count() : 0;
        if( nCnt )
        {
            for( MSHORT i=0; i < nCnt; ++i )
            {
                // --> OD 2004-07-07 #i28701# - consider changed type of
                // <SwSortedObjs> entries
                SwAnchoredObject* pAnchoredObj = (*pSorted)[ i ];
                if( pAnchoredObj->ISA(SwFlyFrm) )
                {
                    SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                    lcl_OutFrame( rStr, pFly, rSp, sal_False );
                }
                else
                {
                    aTmp = pAnchoredObj->GetDrawObj()->IsUnoObj() ? "UNO" : "Drw";
                    rStr << aTmp;
                }
                // <--
                if( i < nCnt - 1 )
                    rStr << endl << rSp;
            }
        }
    }
    else if( pFrm->GetDrawObjs() )
    {
        MSHORT nCnt = pFrm->GetDrawObjs()->Count();
        if( nCnt )
        {
            for( MSHORT i=0; i < nCnt; ++i )
            {
                // --> OD 2004-07-07 #i28701# - consider changed type of
                // <SwSortedObjs> entries
                SwAnchoredObject* pAnchoredObj = (*pFrm->GetDrawObjs())[ i ];
                if( pAnchoredObj->ISA(SwFlyFrm) )
                {
                    SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                    lcl_OutFrame( rStr, pFly, rSp, sal_False );
                }
                else
                {
                    aTmp = pAnchoredObj->GetDrawObj()->IsUnoObj() ? "UNO" : "Drw";
                    rStr << aTmp;
                }
                if( i < nCnt - 1 )
                    rStr << endl << rSp;
            }
        }
    }
    if( nSpc )
        rSp.Erase( rSp.Len() - nSpc );
    if( bNxt && pFrm->GetNext() )
    {
        do
        {
            pFrm = pFrm->GetNext();
            rStr << endl << rSp;
            lcl_OutFrame( rStr, pFrm, rSp, sal_False );
        } while ( pFrm->GetNext() );
    }
}

void LayOutPut( const SwFrm* pFrm )
{
    static char* pOutName = 0;
    const sal_Bool bFirstOpen = pOutName ? sal_False : sal_True;
    if( bFirstOpen )
    {
        char *pPath = getenv( "TEMP" );
        char *pName = "layout.txt";
        if( !pPath )
            pOutName = pName;
        else
        {
            const int nLen = strlen(pPath);
            // fuer dieses new wird es kein delete geben.
            pOutName = new char[nLen + strlen(pName) + 3];
            if(nLen && (pPath[nLen-1] == '\\') || (pPath[nLen-1] == '/'))
                snprintf( pOutName, sizeof(pOutName), "%s%s", pPath, pName );
            else
                snprintf( pOutName, sizeof(pOutName), "%s/%s", pPath, pName );
        }
    }
    SvFileStream aStream( pOutName, (bFirstOpen
                                        ? STREAM_WRITE | STREAM_TRUNC
                                        : STREAM_WRITE ));

    if( !aStream.GetError() )
    {
        if ( bFirstOpen )
            aStream << "Layout-Struktur";
        else
            aStream.Seek( STREAM_SEEK_TO_END );
        aStream << endl;
        aStream << "---------------------------------------------" << endl;
        XubString aSpace;
        lcl_OutFrame( aStream, pFrm, aSpace, sal_False );
    }
}

#endif

SvStream &operator<<( SvStream &rOs, const SwpHints & ) //$ ostream
{
    rOs << " {HINTS:";

// REMOVED

    rOs << '}';
    return rOs;
}

/*************************************************************************
 *                          IsDbg()
 *************************************************************************/

sal_Bool IsDbg( const SwTxtFrm *pFrm )
{
    if( pFrm && pFrm->getRootFrm()->GetCurrShell() )
        return pFrm->getRootFrm()->GetCurrShell()->GetViewOptions()->IsTest4();
    else
        return sal_False;
}

#if OSL_DEBUG_LEVEL < 2

static void Error()
{
    // wegen PM und BCC
    sal_Bool bFalse = sal_False;
    ASSERT( bFalse, "txtio: No debug version" );
}

#define IMPL_OUTOP(class) \
        SvStream &class::operator<<( SvStream &rOs ) const /*$ostream*/\
        { \
            Error(); \
            return rOs; \
        }

IMPL_OUTOP( SwTxtPortion )
IMPL_OUTOP( SwLinePortion )
IMPL_OUTOP( SwBreakPortion )
IMPL_OUTOP( SwGluePortion )
IMPL_OUTOP( SwFldPortion )
IMPL_OUTOP( SwHiddenPortion )
IMPL_OUTOP( SwHyphPortion )
IMPL_OUTOP( SwFixPortion )
IMPL_OUTOP( SwFlyPortion )
IMPL_OUTOP( SwFlyCntPortion )
IMPL_OUTOP( SwMarginPortion )
IMPL_OUTOP( SwNumberPortion )
IMPL_OUTOP( SwBulletPortion )
IMPL_OUTOP( SwGrfNumPortion )
IMPL_OUTOP( SwLineLayout )
IMPL_OUTOP( SwParaPortion )
IMPL_OUTOP( SwFtnPortion )
IMPL_OUTOP( SwFtnNumPortion )
IMPL_OUTOP( SwTmpEndPortion )
IMPL_OUTOP( SwHyphStrPortion )
IMPL_OUTOP( SwExpandPortion )
IMPL_OUTOP( SwBlankPortion )
IMPL_OUTOP( SwToxPortion )
IMPL_OUTOP( SwRefPortion )
IMPL_OUTOP( SwIsoToxPortion )
IMPL_OUTOP( SwIsoRefPortion )
IMPL_OUTOP( SwSoftHyphPortion )
IMPL_OUTOP( SwSoftHyphStrPortion )
IMPL_OUTOP( SwTabPortion )
IMPL_OUTOP( SwTabLeftPortion )
IMPL_OUTOP( SwTabRightPortion )
IMPL_OUTOP( SwTabCenterPortion )
IMPL_OUTOP( SwTabDecimalPortion )
IMPL_OUTOP( SwPostItsPortion )
IMPL_OUTOP( SwQuoVadisPortion )
IMPL_OUTOP( SwErgoSumPortion )
IMPL_OUTOP( SwHolePortion )
IMPL_OUTOP( SwDropPortion )
IMPL_OUTOP( SwKernPortion )
IMPL_OUTOP( SwArrowPortion )
IMPL_OUTOP( SwMultiPortion )
IMPL_OUTOP( SwCombinedPortion )

const char *GetPortionName( const MSHORT )
{
    return 0;
}

const char *GetPrepName( const PrepareHint )
{
    return 0;
}

void SwLineLayout::DebugPortions( SvStream &, const XubString &, //$ ostream
                                                const xub_StrLen  )
{
}

const char *GetLangName( const MSHORT  )
{
    return 0;
}

#else
# include <limits.h>
# include <stdlib.h>
# include "swtypes.hxx"      // ZTCCONST
# include "swfont.hxx"     // SwDropPortion

CONSTCHAR( pClose, "} " );

/*************************************************************************
 *                    GetPortionName()
 *************************************************************************/

CONSTCHAR( pPOR_LIN, "LIN" );
CONSTCHAR( pPOR_TXT, "TXT" );
CONSTCHAR( pPOR_SHADOW, "SHADOW" );
CONSTCHAR( pPOR_TAB, "TAB" );
CONSTCHAR( pPOR_TABLEFT, "TABLEFT" );
CONSTCHAR( pPOR_TABRIGHT, "TABRIGHT" );
CONSTCHAR( pPOR_TABCENTER, "TABCENTER" );
CONSTCHAR( pPOR_TABDECIMAL, "TABDECIMAL" );
CONSTCHAR( pPOR_EXP, "EXP" );
CONSTCHAR( pPOR_HYPH, "HYPH" );
CONSTCHAR( pPOR_HYPHSTR, "HYPHSTR" );
CONSTCHAR( pPOR_FLD, "FLD" );
CONSTCHAR( pPOR_FIX, "FIX" );
CONSTCHAR( pPOR_FLY, "FLY" );
CONSTCHAR( pPOR_FLYCNT, "FLYCNT" );
CONSTCHAR( pPOR_MARGIN, "MARGIN" );
CONSTCHAR( pPOR_GLUE, "GLUE" );
CONSTCHAR( pPOR_HOLE, "HOLE" );
CONSTCHAR( pPOR_END, "END" );
CONSTCHAR( pPOR_BRK, "BRK" );
CONSTCHAR( pPOR_LAY, "LAY" );
CONSTCHAR( pPOR_BLANK, "BLANK" );
CONSTCHAR( pPOR_FTN, "FTN" );
CONSTCHAR( pPOR_FTNNUM, "FTNNUM" );
CONSTCHAR( pPOR_POSTITS, "POSTITS" );
CONSTCHAR( pPOR_SOFTHYPH, "SOFTHYPH" );
CONSTCHAR( pPOR_SOFTHYPHSTR, "SOFTHYPHSTR" );
CONSTCHAR( pPOR_TOX, "TOX" );
CONSTCHAR( pPOR_REF, "REF" );

CONSTCHAR( pPOR_ISOTOX, "ISOTOX" );
CONSTCHAR( pPOR_ISOREF, "ISOREF" );
CONSTCHAR( pPOR_HIDDEN, "Hidden" );
CONSTCHAR( pPOR_QUOVADIS, "QuoVadis" );
CONSTCHAR( pPOR_ERGOSUM, "ErgoSum" );
CONSTCHAR( pPOR_NUMBER, "NUMBER" );
CONSTCHAR( pPOR_BULLET, "BULLET" );
CONSTCHAR( pPOR_UNKW, "UNKW" );
CONSTCHAR( pPOR_PAR, "PAR" );

const char *GetPortionName( const MSHORT /*nType*/ )
{
    return 0;
}

CONSTCHAR( pPREP_CLEAR, "CLEAR" );
CONSTCHAR( pPREP_WIDOWS_ORPHANS, "WIDOWS_ORPHANS" );
CONSTCHAR( pPREP_FIXSIZE_CHG, "FIXSIZE_CHG" );
CONSTCHAR( pPREP_FOLLOW_FOLLOWS, "FOLLOW_FOLLOWS" );
CONSTCHAR( pPREP_ADJUST_FRM, "ADJUST_FRM" );
CONSTCHAR( pPREP_FREE_SPACE, "FREE_SPACE" );
CONSTCHAR( pPREP_FLY_CHGD, "FLY_CHGD" );
CONSTCHAR( pPREP_FLY_ATTR_CHG, "FLY_ATTR_CHG" );
CONSTCHAR( pPREP_FLY_ARRIVE, "FLY_ARRIVE" );
CONSTCHAR( pPREP_FLY_LEAVE, "FLY_LEAVE" );
CONSTCHAR( pPREP_VIEWOPT, "VIEWOPT" );
CONSTCHAR( pPREP_FTN, "FTN" );
CONSTCHAR( pPREP_POS_CHGD, "POS" );
CONSTCHAR( pPREP_UL_SPACE, "UL_SPACE" );
CONSTCHAR( pPREP_MUST_FIT, "MUST_FIT" );
CONSTCHAR( pPREP_WIDOWS, "ORPHANS" );
CONSTCHAR( pPREP_QUOVADIS, "QUOVADIS" );
CONSTCHAR( pPREP_PAGE, "PAGE" );

const char *GetPrepName( const PrepareHint ePrep )
{
    // Kurz und schmerzlos:
    const char *ppNameArr[PREP_END] =
    {
        pPREP_CLEAR, pPREP_WIDOWS_ORPHANS, pPREP_FIXSIZE_CHG,
        pPREP_FOLLOW_FOLLOWS, pPREP_ADJUST_FRM, pPREP_FREE_SPACE,
        pPREP_FLY_CHGD, pPREP_FLY_ATTR_CHG, pPREP_FLY_ARRIVE,
        pPREP_FLY_LEAVE, pPREP_VIEWOPT, pPREP_FTN, pPREP_POS_CHGD,
        pPREP_UL_SPACE, pPREP_MUST_FIT, pPREP_WIDOWS, pPREP_QUOVADIS,
        pPREP_PAGE
    };
    ASSERT( ePrep < PREP_END, "GetPrepName: unknown PrepareHint" );
    return( ppNameArr[ePrep] );
}

/*************************************************************************
 *                    SwLineLayout::DebugPortions()
 *
 * DebugPortion() iteriert ueber alle Portions einer Zeile und deckt die
 * internen Strukturen auf.
 * Im Gegensatz zum Ausgabe-Operator werden auch die Textteile ausgegeben.
 *************************************************************************/

void SwLineLayout::DebugPortions( SvStream &rOs, const XubString &/*rTxt*/, //$ ostream
                                                const xub_StrLen /*nStart*/ )
{
    SwLinePortion *pPortion2 = GetPortion();

    xub_StrLen nPos = 0;
    MSHORT nNr = 0;
    KSHORT nPrtWidth, nLastPrt;
    nPrtWidth = nLastPrt = 0;

    SwLinePortion::operator<<( rOs );
    rOs << '\"' << endl;

    while( pPortion2 )
    {
        DBG_LOOP;
        SwTxtPortion *pTxtPor = pPortion2->InTxtGrp() ?
                                (SwTxtPortion *)pPortion2 : NULL ;
        (void)pTxtPor;
        ++nNr;
        nLastPrt = nPrtWidth;
        nPrtWidth = nPrtWidth + pPortion2->PrtWidth();
        rOs << "\tNr:"  << nNr
            << " Pos:" << nPos
            << " Org:" << nLastPrt
            << endl;

        rOs << "\t";
        pPortion2->operator<<( rOs );
        rOs << endl;
        nPos = nPos + pPortion2->GetLen();
        pPortion2 = pPortion2->GetPortion();
    }
}

const char *GetLangName( const MSHORT /*nLang*/ )
{
    return "???";
}

SvStream &SwLinePortion::operator<<( SvStream &rOs ) const //$ ostream
{
    rOs << " {";
    rOs <<  "L:" << nLineLength;
    rOs << " H:" << Height();
    rOs << " W:" << PrtWidth();
    rOs << " A:" << nAscent;
    rOs << pClose;
    return rOs;
}

SvStream &SwTxtPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {TXT:" );
    rOs << pTxt;
    SwLinePortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwTmpEndPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {END:" );
    rOs << pTxt;
    SwLinePortion::operator<<( rOs );
    if( PrtWidth() )
        rOs << "(view)";
    rOs << pClose;
    return rOs;
}

SvStream &SwBreakPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {BREAK:" );
    rOs << pTxt;
    SwLinePortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwKernPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {KERN:" );
    rOs << pTxt;
    SwLinePortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwArrowPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {ARROW:" );
    rOs << pTxt;
    SwLinePortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwMultiPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {MULTI:" );
    rOs << pTxt;
    SwLinePortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwCombinedPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {COMBINED:" );
    rOs << pTxt;
    SwLinePortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwLineLayout::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {LINE:" );
    rOs << pTxt;
    SwLinePortion::operator<<( rOs );
    SwLinePortion *pPos = GetPortion();
    while( pPos )
    {
        DBG_LOOP;
        rOs << "\t";
        pPos->operator<<( rOs );
        pPos = pPos->GetPortion();
    }
    rOs << pClose;
    return rOs;
}

SvStream &SwGluePortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {GLUE:" );
    rOs << pTxt;
    SwLinePortion::operator<<( rOs );
    rOs << " F:" << GetFixWidth();
    rOs << " G:" << GetPrtGlue();
    rOs << pClose;
    return rOs;
}

SvStream &SwFixPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {FIX:" );
    rOs << pTxt;
    SwGluePortion::operator<<( rOs );
    rOs << " Fix:" << nFix;
    rOs << pClose;
    return rOs;
}

SvStream &SwFlyPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {FLY:" );
    rOs << pTxt;
    SwFixPortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwMarginPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {MAR:" );
    rOs << pTxt;
    SwGluePortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwFlyCntPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {FLYCNT:" );
    rOs << pTxt;
    SwLinePortion::operator<<( rOs );
    if( bDraw )
    {
        CONSTCHAR( pTxt2, " {DRAWINCNT" );
        rOs << pTxt2;
        rOs << pClose;
    }
    else
    {
        CONSTCHAR( pTxt2, " {FRM:" );
        rOs << pTxt2;
        rOs << " {FRM:" << GetFlyFrm()->Frm() << pClose;
        rOs << " {PRT:" << GetFlyFrm()->Prt() << pClose;
        rOs << pClose;
    }
    rOs << pClose;
    return rOs;
}

SvStream &SwExpandPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {EXP:" );
    rOs << pTxt;
    SwLinePortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwFtnPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {FTN:" );
    rOs << pTxt;
    SwExpandPortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwFtnNumPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {FTNNUM:" );
    rOs << pTxt;
    SwNumberPortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwNumberPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {NUMBER:" );
    rOs << pTxt;
    SwExpandPortion::operator<<( rOs );
    rOs << " Exp:\"" << '\"';
    rOs << pClose;
    return rOs;
}

SvStream &SwBulletPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {BULLET:" );
    rOs << pTxt;
    SwNumberPortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwGrfNumPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {GRFNUM:" );
    rOs << pTxt;
    SwNumberPortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwHiddenPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {Hidden:" );
    rOs << pTxt;
    SwFldPortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwToxPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {TOX:" );
    rOs << pTxt;
    SwTxtPortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwRefPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {Ref:" );
    rOs << pTxt;
    SwTxtPortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwIsoToxPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {ISOTOX:" );
    rOs << pTxt;
    SwToxPortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwIsoRefPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {ISOREF:" );
    rOs << pTxt;
    SwRefPortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwHyphPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {HYPH:" );
    rOs << pTxt;
    SwExpandPortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwHyphStrPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {HYPHSTR:" );
    rOs << pTxt;
    SwExpandPortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwSoftHyphPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {SOFTHYPH:" );
    rOs << pTxt;
    SwHyphPortion::operator<<( rOs );
    rOs << (IsExpand() ? " on" : " off");
    rOs << pClose;
    return rOs;
}

SvStream &SwSoftHyphStrPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {SOFTHYPHSTR:" );
    rOs << pTxt;
    SwHyphStrPortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwBlankPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {BLANK:" );
    rOs << pTxt;
    SwExpandPortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwFldPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {FLD:" );
    rOs << pTxt;
    SwLinePortion::operator<<( rOs );
    if( IsFollow() )
        rOs << " F!";
    rOs << pClose;
    return rOs;
}

SvStream &SwPostItsPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {POSTITS" );
    rOs << pTxt;
    SwLinePortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwTabPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {TAB" );
    rOs << pTxt;
    SwFixPortion::operator<<( rOs );
    rOs << " T:" << nTabPos;
    if( IsFilled() )
        rOs << " \"" << cFill << '\"';
    rOs << pClose;
    return rOs;
}

SvStream &SwTabLeftPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {TABLEFT" );
    rOs << pTxt;
    SwTabPortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwTabRightPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {TABRIGHT" );
    rOs << pTxt;
    SwTabPortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwTabCenterPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {TABCENTER" );
    rOs << pTxt;
    SwTabPortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwTabDecimalPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {TABDECIMAL" );
    rOs << pTxt;
    SwTabPortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwParaPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {PAR" );
    rOs << pTxt;
    SwLineLayout::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwHolePortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {HOLE" );
    rOs << pTxt;
    SwLinePortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwQuoVadisPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {QUOVADIS" );
    rOs << pTxt;
    SwFldPortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &SwErgoSumPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {ERGOSUM" );
    rOs << pTxt;
    SwFldPortion::operator<<( rOs );
    rOs << pClose;
    return rOs;
}

SvStream &operator<<( SvStream &rOs, const SwTxtSizeInfo &rInf ) //$ ostream
{
    CONSTCHAR( pTxt, " {SIZEINFO:" );
    rOs << pTxt;
    rOs << ' ' << (rInf.OnWin() ? "WIN:" : "PRT:" );
    rOs << " Idx:" << rInf.GetIdx();
    rOs << " Len:" << rInf.GetLen();
    rOs << pClose;
    return rOs;
}

SvStream &SwDropPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pTxt, " {DROP:" );
    rOs << pTxt;
    SwTxtPortion::operator<<( rOs );
    if( pPart && nDropHeight )
    {
        rOs << " H:" << nDropHeight;
        rOs << " L:" << nLines;
        rOs <<" Fnt:" << pPart->GetFont().GetHeight();
        if( nX || nY )
            rOs << " [" << nX << '/' << nY << ']';
    }
    rOs << pClose;
    return rOs;
}

#endif /* OSL_DEBUG_LEVEL */

#endif // DBG_UTIL

