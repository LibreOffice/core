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

#include <vcl/keycodes.hxx>
#include <tools/string.hxx>
#include <tools/gen.hxx>

#include <editeng/swafopt.hxx>

SvxSwAutoFmtFlags::SvxSwAutoFmtFlags()
    : aBulletFont( OUString("StarSymbol"),
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
    bAutoCmpltAppendBlanc = sal_False;

    bAutoCmpltShowAsTip =
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

    nAutoCmpltWordLen = 8;
    nAutoCmpltListLen = 1000;
    m_pAutoCompleteList = 0;
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
    m_pAutoCompleteList = rAFFlags.m_pAutoCompleteList;
    pSmartTagMgr = rAFFlags.pSmartTagMgr;
    nAutoCmpltExpandKey = rAFFlags.nAutoCmpltExpandKey;

    nAutoCmpltWordLen = rAFFlags.nAutoCmpltWordLen;
    nAutoCmpltListLen = rAFFlags.nAutoCmpltListLen;

    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
