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

#include <editeng/swafopt.hxx>
#include <tools/gen.hxx>
#include <vcl/keycodes.hxx>

SvxSwAutoFormatFlags::SvxSwAutoFormatFlags()
    : aBulletFont( u"OpenSymbol"_ustr, Size( 0, 14 ) )
{
    bAutoCorrect =
    bCapitalStartSentence =
    bCapitalStartWord =
    bChgEnumNum =
    bAddNonBrkSpace =
    bChgOrdinalNumber =
    bTransliterateRTL =
    bChgAngleQuotes =
    bChgToEnEmDash =
    bChgWeightUnderl =
    bSetINetAttr =
    bSetDOIAttr =
    bAFormatDelSpacesAtSttEnd =
    bAFormatDelSpacesBetweenLines =
    bAFormatByInpDelSpacesAtSttEnd =
    bAFormatByInpDelSpacesBetweenLines = true;

    bChgUserColl =
    bReplaceStyles =
    bDelEmptyNode =
    bWithRedlining =
    bAutoCmpltEndless =
    bSetNumRuleAfterSpace =
    bAutoCmpltAppendBlank = false;

    bAutoCmpltShowAsTip =
    bSetBorder =
    bCreateTable =
    bSetNumRule =
    bAFormatByInput =
    bRightMargin =
    bAutoCompleteWords =
    bAutoCmpltCollectWords =
    bAutoCmpltKeepList = true;

    nRightMargin = 50;      // default 50%
    nAutoCmpltExpandKey = KEY_RETURN;

    aBulletFont.SetCharSet( RTL_TEXTENCODING_SYMBOL );
    aBulletFont.SetFamily( FAMILY_DONTKNOW );
    aBulletFont.SetPitch( PITCH_DONTKNOW );
    aBulletFont.SetWeight( WEIGHT_DONTKNOW );
    aBulletFont.SetTransparent( true );

    cBullet = 0x2022;
    cByInputBullet = cBullet;
    aByInputBulletFont = aBulletFont;

    nAutoCmpltWordLen = 8;
    nAutoCmpltListLen = 1000;
    m_pAutoCompleteList = nullptr;
    pSmartTagMgr = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
