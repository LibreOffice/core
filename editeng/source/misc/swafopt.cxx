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
#include "precompiled_editeng.hxx"
#include <vcl/keycodes.hxx>
#include <tools/string.hxx>

#include <editeng/swafopt.hxx>

SvxSwAutoFmtFlags::SvxSwAutoFmtFlags()
    : aBulletFont( String::CreateFromAscii(
                        RTL_CONSTASCII_STRINGPARAM( "StarSymbol" )),
                    Size( 0, 14 ) )
{
    bAutoCorrect =
    bCptlSttSntnc =
    bCptlSttWrd =
    bChkFontAttr =
    bChgUserColl =
    bChgEnumNum =
    bAddNonBrkSpace =
    bChgOrdinalNumber =
    bChgToEnEmDash =
    bChgWeightUnderl =
    bSetINetAttr =
    bAFmtDelSpacesAtSttEnd =
    bAFmtDelSpacesBetweenLines =
    bAFmtByInpDelSpacesAtSttEnd =
    bAFmtByInpDelSpacesBetweenLines =
    bDummy = sal_True;

    bReplaceStyles =
    bDelEmptyNode =
    bWithRedlining =
    bAutoCmpltEndless =
    bAutoCmpltAppendBlanc =
    bAutoCmpltShowAsTip = sal_False;

    bSetBorder =
    bCreateTable =
    bSetNumRule =
    bAFmtByInput =
    bRightMargin =
    bAutoCompleteWords =
    bAutoCmpltCollectWords =
    bAutoCmpltKeepList = sal_True;

    bDummy6 = bDummy7 = bDummy8 =
         sal_False;

    nRightMargin = 50;      // dflt. 50 %
    nAutoCmpltExpandKey = KEY_RETURN;

    aBulletFont.SetCharSet( RTL_TEXTENCODING_SYMBOL );
    aBulletFont.SetFamily( FAMILY_DONTKNOW );
    aBulletFont.SetPitch( PITCH_DONTKNOW );
    aBulletFont.SetWeight( WEIGHT_DONTKNOW );
    aBulletFont.SetTransparent( sal_True );

    cBullet = 0x2022;
    cByInputBullet = cBullet;
    aByInputBulletFont = aBulletFont;

    nAutoCmpltWordLen = 10;
    nAutoCmpltListLen = 500;
    pAutoCmpltList = 0;
    pSmartTagMgr = 0;
}


SvxSwAutoFmtFlags& SvxSwAutoFmtFlags::operator=( const SvxSwAutoFmtFlags& rAFFlags )
{
    bAutoCorrect = rAFFlags.bAutoCorrect;
    bCptlSttSntnc = rAFFlags.bCptlSttSntnc;
    bCptlSttWrd = rAFFlags.bCptlSttWrd;
    bChkFontAttr = rAFFlags.bChkFontAttr;

    bChgUserColl = rAFFlags.bChgUserColl;
    bChgEnumNum = rAFFlags.bChgEnumNum;
    bDelEmptyNode = rAFFlags.bDelEmptyNode;
    bSetNumRule = rAFFlags.bSetNumRule;
    bAFmtByInput = rAFFlags.bAFmtByInput;

    bAddNonBrkSpace = rAFFlags.bAddNonBrkSpace;
    bChgOrdinalNumber = rAFFlags.bChgOrdinalNumber;
    bChgToEnEmDash = rAFFlags.bChgToEnEmDash;
    bChgWeightUnderl = rAFFlags.bChgWeightUnderl;
    bSetINetAttr = rAFFlags.bSetINetAttr;
    bSetBorder = rAFFlags.bSetBorder;
    bCreateTable = rAFFlags.bCreateTable;
    bReplaceStyles = rAFFlags.bReplaceStyles;
    bAFmtDelSpacesAtSttEnd = rAFFlags.bAFmtDelSpacesAtSttEnd;
    bAFmtDelSpacesBetweenLines = rAFFlags.bAFmtDelSpacesBetweenLines;
    bAFmtByInpDelSpacesAtSttEnd = rAFFlags.bAFmtByInpDelSpacesAtSttEnd;
    bAFmtByInpDelSpacesBetweenLines = rAFFlags.bAFmtByInpDelSpacesBetweenLines;

    bDummy = rAFFlags.bDummy;

    bDummy6 = rAFFlags.bDummy6;
    bDummy7 = rAFFlags.bDummy7;
    bDummy8 = rAFFlags.bDummy8;

    bWithRedlining = rAFFlags.bWithRedlining;

    bRightMargin = rAFFlags.bRightMargin;
    nRightMargin = rAFFlags.nRightMargin;

    cBullet = rAFFlags.cBullet;
    aBulletFont = rAFFlags.aBulletFont;

    cByInputBullet = rAFFlags.cByInputBullet;
    aByInputBulletFont = rAFFlags.aByInputBulletFont;

    bAutoCompleteWords = rAFFlags.bAutoCompleteWords;
    bAutoCmpltCollectWords = rAFFlags.bAutoCmpltCollectWords;
    bAutoCmpltKeepList = rAFFlags.bAutoCmpltKeepList;
    bAutoCmpltEndless = rAFFlags.bAutoCmpltEndless;
    bAutoCmpltAppendBlanc = rAFFlags.bAutoCmpltAppendBlanc;
    bAutoCmpltShowAsTip = rAFFlags.bAutoCmpltShowAsTip;
    pAutoCmpltList = rAFFlags.pAutoCmpltList;
    pSmartTagMgr = rAFFlags.pSmartTagMgr;
    nAutoCmpltExpandKey = rAFFlags.nAutoCmpltExpandKey;

    nAutoCmpltWordLen = rAFFlags.nAutoCmpltWordLen;
    nAutoCmpltListLen = rAFFlags.nAutoCmpltListLen;

    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
