/*************************************************************************
 *
 *  $RCSfile: txtio.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ama $ $Date: 2000-10-16 12:13:55 $
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

#include "viewsh.hxx"       // IsDbg()
#include "viewopt.hxx"      // IsDbg()
#include "txtatr.hxx"
#include "errhdl.hxx"

#ifdef PRODUCT
#error Wer fummelt denn an den makefiles?
#endif

#include "txtcfg.hxx"

#include "txtfrm.hxx"       // IsDbg()
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
#include "frmsh.hxx"

// So kann man die Layoutstruktur ausgeben lassen
// #define AMA_LAYOUT
#ifdef AMA_LAYOUT
#include <stdio.h>          // sprintf
#include <stdlib.h>         // getenv()
#include <flowfrm.hxx>
#include <pagefrm.hxx>
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _DFLYOBJ_HXX //autogen
#include <dflyobj.hxx>
#endif


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
        const SwSortDrawObjs *pSorted = pPg->GetSortedObjs();
        const MSHORT nCnt = pSorted ? pSorted->Count() : 0;
        if( nCnt )
        {
            for( MSHORT i=0; i < nCnt; ++i )
            {
                SdrObject *pObj = (*pSorted)[ i ];
                if( pObj->IsWriterFlyFrame() )
                {
                    SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
                    lcl_OutFrame( rStr, pFly, rSp, sal_False );
                }
                else
                {
                    aTmp = pObj->IsUnoObj() ? "UNO" : "Drw";
                    rStr << aTmp;
                }
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
                SdrObject *pObj = (*pFrm->GetDrawObjs())[ i ];
                if( pObj->IsWriterFlyFrame() )
                {
                    SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
                    lcl_OutFrame( rStr, pFly, rSp, sal_False );
                }
                else
                {
                    aTmp = pObj->IsUnoObj() ? "UNO" : "Drw";
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

#ifdef USED
        IsFtnContFrm()
        IsFtnFrm()
#endif

void LayOutPut( const SwFrm* pFrm )
{
#ifndef MAC
    static char *pOutName = 0;
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
                sprintf( pOutName, "%s%s", pPath, pName );
            else
                sprintf( pOutName, "%s/%s", pPath, pName );
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
#endif
}

#endif

SvStream &operator<<( SvStream &rOs, const SwpHints &rHints ) //$ ostream
{
    rOs << " {HINTS:";
#ifdef JP_NEWCORE
    for( MSHORT i = 0; i < rHints.GetSize(); ++i)
    {
        SwTxtHint *pHint = (SwTxtHint*) rHints[i];

        if(0 != GetCharWidth(pHint))
            rOs << "CHARWIDTH" << ' '; // << GetCharWidth(pHint)->frCPI;
        else if(0 != GetColor(pHint))
            rOs << "COLOR" << ' ' ; // << GetColor(pHint)->aColor;
        else if(0 != GetCrossedOut(pHint))
            rOs << "CROSSEDOUT" << ' ' << (MSHORT)(GetCrossedOut(pHint)->nState);
        else if(0 != GetAttrFont(pHint))
            rOs << "ATTRFONT" << ' ' <<
            (const char *)(GetAttrFont(pHint)->sFamilyName) << ',' <<
            ((MSHORT) GetAttrFont(pHint)->eFamily);
        else if(0 != GetPosture(pHint))
            rOs << "POSTURE" << ' ' << GetPosture(pHint)->nPosture;
        else if(0 != GetFontSize(pHint))
            rOs << "FONTSIZE" << ' ' << GetFontSize(pHint)->nSize;
        else if(0 != GetUnderline(pHint))
            rOs << "UNDERLINE" << ' ' << (MSHORT)(GetUnderline(pHint)->nState);
        else if(0 != GetWeight(pHint))
            rOs << "WEIGHT" << ' ' << GetWeight(pHint)->nWeight;
        else if(0 != GetContour(pHint))
            rOs << "CONTOUR" << ' ' << GetContour(pHint)->nState;
        else if(0 != GetShadowed(pHint))
            rOs << "SHADOWED" << ' ' << GetShadowed(pHint)->nState;
        else if(0 != GetAutoKern(pHint))
            rOs << "AUTOKERN" << ' ' << GetAutoKern(pHint)->nState;
        else if(0 != GetWordLineMode(pHint))
            rOs << "WORDLINEMODE" << ' ' << GetWordLineMode(pHint)->nState;
        else
            rOs << pHint->Which();

        rOs << ',' << pHint->GetStart()->GetIndex()
                << '-'
                << (pHint->GetEnd() ? pHint->GetEnd()->GetIndex() : STRING_LEN)
                << "\n";
    }
#endif
    // JP_NEWCORE

    rOs << '}';
    return rOs;
}

/*************************************************************************
 *                          IsDbg()
 *************************************************************************/

sal_Bool IsDbg( const SwTxtFrm *pFrm )
{
    if( pFrm && pFrm->GetShell() )
        return pFrm->GetShell()->GetViewOptions()->IsTest4();
    else
        return sal_False;
}

#ifndef DEBUG

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

const char *GetPortionName( const MSHORT nType )
{
    return 0;
}

const char *GetPrepName( const PrepareHint ePrep )
{
    return 0;
}

void SwLineLayout::DebugPortions( SvStream &rOs, const XubString &rTxt, //$ ostream
                                                const xub_StrLen nStart )
{
}

const char *GetLangName( const MSHORT nLang )
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

const char *GetPortionName( const MSHORT nType )
{
#ifdef USED
    // Kurz und schmerzlos:
    const char *ppNameArr[PORTYPE_END] = {
      pPOR_LIN, pPOR_TXT, pPOR_HOLE, pPOR_SHADOW,
      pPOR_TAB, pPOR_TABLEFT, pPOR_TABRIGHT, pPOR_TABCENTER, pPOR_TABDECIMAL,
      pPOR_EXP, pPOR_HYPH, pPOR_HYPHSTR, pPOR_FLD,
      pPOR_FIX, pPOR_FLY, pPOR_FLYCNT, pPOR_MARGIN,
      pPOR_GLUE, pPOR_END, pPOR_BRK, pPOR_LAY,
      pPOR_BLANK, pPOR_FTN, pPOR_FTNNUM,
      pPOR_POSTITS, pPOR_SOFTHYPH, pPOR_SOFTHYPHSTR,
      pPOR_TOX, pPOR_REF, pPOR_ISOTOX, pPOR_ISOREF,
      pPOR_HIDDEN, pPOR_QUOVADIS, pPOR_ERGOSUM,
      pPOR_NUMBER, pPOR_BULLET, pPOR_UNKW, pPOR_PAR
    };
    ASSERT( eType < PORTYPE_END, "GetPortionName: bad type" );
    return( ppNameArr[eType] );
#else
    return 0;
#endif
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

void SwLineLayout::DebugPortions( SvStream &rOs, const XubString &rTxt, //$ ostream
                                                const xub_StrLen nStart )
{
    SwLinePortion *pPortion = GetPortion();

    xub_StrLen nPos = 0;
    MSHORT nNr = 0;
    KSHORT nPrtWidth, nLastPrt;
    nPrtWidth = nLastPrt = 0;

    SwLinePortion::operator<<( rOs );
    rOs << '\"' << endl;

    while( pPortion )
    {
        DBG_LOOP;
        SwTxtPortion *pTxtPor = pPortion->InTxtGrp() ?
                                (SwTxtPortion *)pPortion : NULL ;
        ++nNr;
        nLastPrt = nPrtWidth;
        nPrtWidth += pPortion->PrtWidth();
        rOs << "\tNr:"  << nNr
            << " Pos:" << nPos
            << " Org:" << nLastPrt
            << endl;

        rOs << "\t";
        pPortion->operator<<( rOs );
        rOs << endl;
        nPos += pPortion->GetLen();
        pPortion = pPortion->GetPortion();
    }
}

#ifdef USED
CONSTCHAR( pRES_LNG_ALBANIAN, "ALBANIAN" );
CONSTCHAR( pRES_LNG_ARABIC, "ARABIC" );
CONSTCHAR( pRES_LNG_AUS_ENGLISH, "AUS_ENGLISH" );
CONSTCHAR( pRES_LNG_BAHASA, "BAHASA" );
CONSTCHAR( pRES_LNG_BELGIAN_DUTCH, "BELGIAN_DUTCH" );
CONSTCHAR( pRES_LNG_BELGIAN_FRENCH, "BELGIAN_FRENCH" );
CONSTCHAR( pRES_LNG_BRAZIL_PORT, "BRAZIL_PORT" );
CONSTCHAR( pRES_LNG_BULGARIAN, "BULGARIAN" );
CONSTCHAR( pRES_LNG_CANADA_FRENCH, "CANADA_FRENCH" );
CONSTCHAR( pRES_LNG_CAST_SPANISH, "CAST_SPANISH" );
CONSTCHAR( pRES_LNG_CATALAN, "CATALAN" );
CONSTCHAR( pRES_LNG_CROATO_SERBIAN, "CROATO_SERBIAN" );
CONSTCHAR( pRES_LNG_CZECH, "CZECH" );
CONSTCHAR( pRES_LNG_DANISH, "DANISH" );
CONSTCHAR( pRES_LNG_DUTCH, "DUTCH" );
CONSTCHAR( pRES_LNG_FINNISH, "FINNISH" );
CONSTCHAR( pRES_LNG_FRENCH, "FRENCH" );
CONSTCHAR( pRES_LNG_GERMAN, "GERMAN" );
CONSTCHAR( pRES_LNG_GREEK, "GREEK" );
CONSTCHAR( pRES_LNG_HEBREW, "HEBREW" );
CONSTCHAR( pRES_LNG_HUNGARIAN, "HUNGARIAN" );
CONSTCHAR( pRES_LNG_ICELANDIC, "ICELANDIC" );
CONSTCHAR( pRES_LNG_ITALIAN, "ITALIAN" );
CONSTCHAR( pRES_LNG_JAPANESE, "JAPANESE" );
CONSTCHAR( pRES_LNG_KOREAN, "KOREAN" );
CONSTCHAR( pRES_LNG_MEXICAN_SPANISH, "MEXICAN_SPANISH" );
CONSTCHAR( pRES_LNG_NORWEG_BOKMAL, "NORWEG_BOKMAL" );
CONSTCHAR( pRES_LNG_NORWEG_NYNORSK, "NORWEG_NYNORSK" );
CONSTCHAR( pRES_LNG_POLISH, "POLISH" );
CONSTCHAR( pRES_LNG_PORTUGUESE, "PORTUGUESE" );
CONSTCHAR( pRES_LNG_RHAETO_ROMANIC, "RHAETO_ROMANIC" );
CONSTCHAR( pRES_LNG_ROMANIAN, "ROMANIAN" );
CONSTCHAR( pRES_LNG_RUSSIAN, "RUSSIAN" );
CONSTCHAR( pRES_LNG_SERBO_CROATIAN, "SERBO_CROATIAN" );
CONSTCHAR( pRES_LNG_SIM_CHINESE, "SIM_CHINESE" );
CONSTCHAR( pRES_LNG_SLOVAKIAN, "SLOVAKIAN" );
CONSTCHAR( pRES_LNG_SWEDISH, "SWEDISH" );
CONSTCHAR( pRES_LNG_SWISS_FRENCH, "SWISS_FRENCH" );
CONSTCHAR( pRES_LNG_SWISS_GERMAN, "SWISS_GERMAN" );
CONSTCHAR( pRES_LNG_SWISS_ITALIAN, "SWISS_ITALIAN" );
CONSTCHAR( pRES_LNG_THAI, "THAI" );
CONSTCHAR( pRES_LNG_TRD_CHINESE, "TRD_CHINESE" );
CONSTCHAR( pRES_LNG_TURKISH, "TURKISH" );
CONSTCHAR( pRES_LNG_UK_ENGLISH, "UK_ENGLISH" );
CONSTCHAR( pRES_LNG_URDU, "URDU" );
CONSTCHAR( pRES_LNG_US_ENGLISH, "US_ENGLISH" );
CONSTCHAR( pRES_LNG_NOLANGUAGE, "NOLANGUAGE" );

const char *GetLangName( const MSHORT nLang )
{
    switch( nLang )
    {
        case 0x041c : return pRES_LNG_ALBANIAN;
        case 0x0401 : return pRES_LNG_ARABIC;
        case 0x0c09 : return pRES_LNG_AUS_ENGLISH;
        case 0x0421 : return pRES_LNG_BAHASA;
        case 0x0813 : return pRES_LNG_BELGIAN_DUTCH;
        case 0x080c : return pRES_LNG_BELGIAN_FRENCH;
        case 0x0416 : return pRES_LNG_BRAZIL_PORT;
        case 0x0402 : return pRES_LNG_BULGARIAN;
        case 0x0c0c : return pRES_LNG_CANADA_FRENCH;
        case 0x040a : return pRES_LNG_CAST_SPANISH;
        case 0x0403 : return pRES_LNG_CATALAN;
        case 0x041a : return pRES_LNG_CROATO_SERBIAN;
        case 0x0405 : return pRES_LNG_CZECH;
        case 0x0406 : return pRES_LNG_DANISH;
        case 0x0413 : return pRES_LNG_DUTCH;
        case 0x040b : return pRES_LNG_FINNISH;
        case 0x040c : return pRES_LNG_FRENCH;
        case 0x0407 : return pRES_LNG_GERMAN;
        case 0x0408 : return pRES_LNG_GREEK;
        case 0x040d : return pRES_LNG_HEBREW;
        case 0x040e : return pRES_LNG_HUNGARIAN;
        case 0x040f : return pRES_LNG_ICELANDIC;
        case 0x0410 : return pRES_LNG_ITALIAN;
        case 0x0411 : return pRES_LNG_JAPANESE;
        case 0x0412 : return pRES_LNG_KOREAN;
        case 0x080a : return pRES_LNG_MEXICAN_SPANISH;
        case 0x0414 : return pRES_LNG_NORWEG_BOKMAL;
        case 0x0814 : return pRES_LNG_NORWEG_NYNORSK;
        case 0x0415 : return pRES_LNG_POLISH;
        case 0x0816 : return pRES_LNG_PORTUGUESE;
        case 0x0417 : return pRES_LNG_RHAETO_ROMANIC;
        case 0x0418 : return pRES_LNG_ROMANIAN;
        case 0x0419 : return pRES_LNG_RUSSIAN;
        case 0x081a : return pRES_LNG_SERBO_CROATIAN;
        case 0x0804 : return pRES_LNG_SIM_CHINESE;
        case 0x041b : return pRES_LNG_SLOVAKIAN;
        case 0x041d : return pRES_LNG_SWEDISH;
        case 0x100c : return pRES_LNG_SWISS_FRENCH;
        case 0x0807 : return pRES_LNG_SWISS_GERMAN;
        case 0x0810 : return pRES_LNG_SWISS_ITALIAN;
        case 0x041e : return pRES_LNG_THAI;
        case 0x0404 : return pRES_LNG_TRD_CHINESE;
        case 0x041f : return pRES_LNG_TURKISH;
        case 0x0809 : return pRES_LNG_UK_ENGLISH;
        case 0x0420 : return pRES_LNG_URDU;
        case 0x0409 : return pRES_LNG_US_ENGLISH;
        default     : return pRES_LNG_NOLANGUAGE;
    }
}
#else

const char *GetLangName( const MSHORT nLang )
{
    return "???";
}
#endif

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
        CONSTCHAR( pTxt, " {DRAWINCNT" );
        rOs << pTxt;
        rOs << pClose;
    }
    else
    {
        CONSTCHAR( pTxt, " {FRM:" );
        rOs << pTxt;
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
    SwGrfNumPortion::operator<<( rOs );
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
    if( pFnt && nDropHeight )
    {
        rOs << " H:" << nDropHeight;
        rOs << " L:" << nLines;
        rOs <<" Fnt:" << pFnt->GetHeight();
        if( nX || nY )
            rOs << " [" << nX << '/' << nY << ']';
    }
    rOs << pClose;
    return rOs;
}

#endif /* DEBUG */


