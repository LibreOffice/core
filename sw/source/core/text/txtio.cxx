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


#ifdef DBG_UTIL

#include "viewsh.hxx"
#include "viewopt.hxx"
#include "txtatr.hxx"
#include "txtfrm.hxx"
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

SvStream &operator<<( SvStream &rOs, const SwpHints & ) //$ ostream
{
    rOs << " {HINTS:";
    rOs << '}';
    return rOs;
}

#if OSL_DEBUG_LEVEL < 2

static void Error()
{
    SAL_WARN( "sw", "txtio: No debug version" );
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

#else
#include <limits.h>
#include <stdlib.h>
#include "swtypes.hxx"      // ZTCCONST
#include "swfont.hxx"     // SwDropPortion

#define CONSTCHAR( name, string ) static const sal_Char name[] = string

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
    OSL_ENSURE( ePrep < PREP_END, "GetPrepName: unknown PrepareHint" );
    return( ppNameArr[ePrep] );
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
        rOs << " Fnt:" << static_cast<sal_Int32>(pPart->GetFont().GetHeight());
        if( nX || nY )
            rOs << " [" << nX << '/' << nY << ']';
    }
    rOs << pClose;
    return rOs;
}

#endif

#endif /* DBG_UTIL */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
