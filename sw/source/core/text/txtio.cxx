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

IMPL_OUTOP( SwTextPortion )
IMPL_OUTOP( SwLinePortion )
IMPL_OUTOP( SwBreakPortion )
IMPL_OUTOP( SwGluePortion )
IMPL_OUTOP( SwFieldPortion )
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
IMPL_OUTOP( SwFootnotePortion )
IMPL_OUTOP( SwFootnoteNumPortion )
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

#else
#include <limits.h>
#include <stdlib.h>
#include "swtypes.hxx"
#include "swfont.hxx"

#define CONSTCHAR( name, string ) static const sal_Char name[] = string

CONSTCHAR( pClose, "} " );

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

SvStream &SwLinePortion::operator<<( SvStream &rOs ) const //$ ostream
{
    rOs.WriteCharPtr(" {");
    rOs.WriteCharPtr("L:").WriteInt32(nLineLength);
    rOs.WriteCharPtr(" H:").WriteUInt16(Height());
    rOs.WriteCharPtr(" W:").WriteUInt16(PrtWidth());
    rOs.WriteCharPtr(" A:").WriteUInt32(nAscent);
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwTextPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {TXT:" );
    rOs.WriteCharPtr(pText);
    SwLinePortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwTmpEndPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {END:" );
    rOs.WriteCharPtr(pText);
    SwLinePortion::operator<<( rOs );
    if( PrtWidth() )
        rOs.WriteCharPtr("(view)");
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwBreakPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {BREAK:" );
    rOs.WriteCharPtr(pText);
    SwLinePortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwKernPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {KERN:" );
    rOs.WriteCharPtr(pText);
    SwLinePortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwArrowPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {ARROW:" );
    rOs.WriteCharPtr(pText);
    SwLinePortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwMultiPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {MULTI:" );
    rOs.WriteCharPtr(pText);
    SwLinePortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwCombinedPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {COMBINED:" );
    rOs.WriteCharPtr(pText);
    SwLinePortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwLineLayout::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {LINE:" );
    rOs.WriteCharPtr(pText);
    SwLinePortion::operator<<( rOs );
    SwLinePortion *pPos = GetPortion();
    while( pPos )
    {
        rOs.WriteCharPtr("\t");
        pPos->operator<<( rOs );
        pPos = pPos->GetPortion();
    }
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwGluePortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {GLUE:" );
    rOs.WriteCharPtr(pText);
    SwLinePortion::operator<<( rOs );
    rOs.WriteCharPtr(" F:").WriteUInt16(GetFixWidth());
    rOs.WriteCharPtr(" G:").WriteInt16(GetPrtGlue());
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwFixPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {FIX:" );
    rOs.WriteCharPtr(pText);
    SwGluePortion::operator<<( rOs );
    rOs.WriteCharPtr(" Fix:").WriteUInt16(nFix);
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwFlyPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {FLY:" );
    rOs.WriteCharPtr(pText);
    SwFixPortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwMarginPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {MAR:" );
    rOs.WriteCharPtr(pText);
    SwGluePortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwFlyCntPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {FLYCNT:" );
    rOs.WriteCharPtr(pText);
    SwLinePortion::operator<<( rOs );
    if( bDraw )
    {
        CONSTCHAR( pText2, " {DRAWINCNT" );
        rOs.WriteCharPtr(pText2);
        rOs.WriteCharPtr(pClose);
    }
    else
    {
        CONSTCHAR( pText2, " {FRM:" );
        rOs.WriteCharPtr(pText2);
        rOs.WriteCharPtr(" {FRM:");
        WriteSwRect(rOs, GetFlyFrame()->Frame()).WriteCharPtr(pClose);
        rOs.WriteCharPtr(" {PRT:");
        WriteSwRect(rOs, GetFlyFrame()->Prt()).WriteCharPtr(pClose);
        rOs.WriteCharPtr(pClose);
    }
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwExpandPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {EXP:" );
    rOs.WriteCharPtr(pText);
    SwLinePortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwFootnotePortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {FTN:" );
    rOs.WriteCharPtr(pText);
    SwExpandPortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwFootnoteNumPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {FTNNUM:" );
    rOs.WriteCharPtr(pText);
    SwNumberPortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwNumberPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {NUMBER:" );
    rOs.WriteCharPtr(pText);
    SwExpandPortion::operator<<( rOs );
    rOs.WriteCharPtr(" Exp:\"").WriteChar('\"');
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwBulletPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {BULLET:" );
    rOs.WriteCharPtr(pText);
    SwNumberPortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwGrfNumPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {GRFNUM:" );
    rOs.WriteCharPtr(pText);
    SwNumberPortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwHiddenPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {Hidden:" );
    rOs.WriteCharPtr(pText);
    SwFieldPortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwToxPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {TOX:" );
    rOs.WriteCharPtr(pText);
    SwTextPortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwRefPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {Ref:" );
    rOs.WriteCharPtr(pText);
    SwTextPortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwIsoToxPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {ISOTOX:" );
    rOs.WriteCharPtr(pText);
    SwToxPortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwIsoRefPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {ISOREF:" );
    rOs.WriteCharPtr(pText);
    SwRefPortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwHyphPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {HYPH:" );
    rOs.WriteCharPtr(pText);
    SwExpandPortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwHyphStrPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {HYPHSTR:" );
    rOs.WriteCharPtr(pText);
    SwExpandPortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwSoftHyphPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {SOFTHYPH:" );
    rOs.WriteCharPtr(pText);
    SwHyphPortion::operator<<( rOs );
    rOs.WriteCharPtr(IsExpand() ? " on" : " off");
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwSoftHyphStrPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {SOFTHYPHSTR:" );
    rOs.WriteCharPtr(pText);
    SwHyphStrPortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwBlankPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {BLANK:" );
    rOs.WriteCharPtr(pText);
    SwExpandPortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwFieldPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {FLD:" );
    rOs.WriteCharPtr(pText);
    SwLinePortion::operator<<( rOs );
    if( IsFollow() )
        rOs.WriteCharPtr(" F!");
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwPostItsPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {POSTITS" );
    rOs.WriteCharPtr(pText);
    SwLinePortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwTabPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {TAB" );
    rOs.WriteCharPtr(pText);
    SwFixPortion::operator<<( rOs );
    rOs.WriteCharPtr(" T:").WriteUInt16(nTabPos);
    if( IsFilled() )
        rOs.WriteCharPtr(" \"").WriteChar(cFill).WriteChar('\"');
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwTabLeftPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {TABLEFT" );
    rOs.WriteCharPtr(pText);
    SwTabPortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwTabRightPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {TABRIGHT" );
    rOs.WriteCharPtr(pText);
    SwTabPortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwTabCenterPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {TABCENTER" );
    rOs.WriteCharPtr(pText);
    SwTabPortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwTabDecimalPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {TABDECIMAL" );
    rOs.WriteCharPtr(pText);
    SwTabPortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwParaPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {PAR" );
    rOs.WriteCharPtr(pText);
    SwLineLayout::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwHolePortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {HOLE" );
    rOs.WriteCharPtr(pText);
    SwLinePortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwQuoVadisPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {QUOVADIS" );
    rOs.WriteCharPtr(pText);
    SwFieldPortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwErgoSumPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {ERGOSUM" );
    rOs.WriteCharPtr(pText);
    SwFieldPortion::operator<<( rOs );
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &operator<<( SvStream &rOs, const SwTextSizeInfo &rInf ) //$ ostream
{
    CONSTCHAR( pText, " {SIZEINFO:" );
    rOs.WriteCharPtr(pText);
    rOs.WriteChar(' ').WriteCharPtr(rInf.OnWin() ? "WIN:" : "PRT:");
    rOs.WriteCharPtr(" Idx:").WriteInt32(rInf.GetIdx());
    rOs.WriteCharPtr(" Len:").WriteInt32(rInf.GetLen());
    rOs.WriteCharPtr(pClose);
    return rOs;
}

SvStream &SwDropPortion::operator<<( SvStream &rOs ) const //$ ostream
{
    CONSTCHAR( pText, " {DROP:" );
    rOs.WriteCharPtr(pText);
    SwTextPortion::operator<<( rOs );
    if( pPart && nDropHeight )
    {
        rOs.WriteCharPtr(" H:").WriteUInt16(nDropHeight);
        rOs.WriteCharPtr(" L:").WriteUInt16(nLines);
        rOs.WriteCharPtr(" Fnt:").WriteInt32(pPart->GetFont().GetHeight());
        if( nX || nY )
            rOs.WriteCharPtr(" [").WriteInt16(nX).WriteChar('/').WriteInt16(nY).WriteChar(']');
    }
    rOs.WriteCharPtr(pClose);
    return rOs;
}

#endif

#endif /* DBG_UTIL */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
