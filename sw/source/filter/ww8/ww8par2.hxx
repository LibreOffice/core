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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_WW8PAR2_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_WW8PAR2_HXX

#include <swtypes.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <editeng/lrspitem.hxx>

#include "ww8scan.hxx"
#include "ww8par.hxx"

class WW8RStyle;

class WW8DupProperties
{
public:
    WW8DupProperties(SwDoc &rDoc, SwWW8FltControlStack *pStk);
    void Insert(const SwPosition &rPos);
private:
    WW8DupProperties(const WW8DupProperties&) = delete;
    WW8DupProperties& operator=(const WW8DupProperties&) = delete;
    SwWW8FltControlStack* pCtrlStck;
    SfxItemSet aChrSet,aParSet;
};

struct WW8SwFlyPara
{
    SwFlyFrameFormat* pFlyFormat;

                // part 1: directly derived Sw attributes
    sal_Int16 nXPos, nYPos;         // Position
    sal_Int16 nLeMgn, nRiMgn;       // borders
    sal_Int16 nUpMgn, nLoMgn;       // borders
    sal_Int16 nWidth, nHeight;      // size
    sal_Int16 nNetWidth;

    SwFrmSize eHeightFix;       // height fixed or min
    RndStdIds eAnchor;          // binding
    short eHRel;     // page or page border
    short eVRel;     // page or page border
    sal_Int16 eVAlign;       // up, down, centered
    sal_Int16 eHAlign;       // left, right, centered
    SwSurround eSurround;       // wrap mode

    sal_uInt8 nXBind, nYBind;        // bound relative to what

                // part 2: changes found during reading
    long nNewNetWidth;
    SwPosition* pMainTextPos;   // to return to main text after apo
    sal_uInt16 nLineSpace;          // line space in tw for graf apos
    bool bAutoWidth;
    bool bToggelPos;

    // add parameter <nWWPgTop> - WW8's page top margin
    WW8SwFlyPara( SwPaM& rPaM,
                  SwWW8ImplReader& rIo,
                  WW8FlyPara& rWW,
                  const sal_uInt32 nWWPgTop,
                  const sal_uInt32 nPgLeft,
                  const sal_uInt32 nPgWidth,
                  const sal_Int32 nIniFlyDx,
                  const sal_Int32 nIniFlyDy );

    void BoxUpWidth( long nWidth );
    SwWW8FltAnchorStack *pOldAnchorStck;
};

class WW8RStyle: public WW8Style
{
friend class SwWW8ImplReader;
    wwSprmParser maSprmParser;
    SwWW8ImplReader* pIo;   // parser class
    SvStream* pStStrm;      // input file

    SwNumRule* pStyRule;    // bullets and enumerations in styles

    sal_uInt8* pParaSprms;           // all ParaSprms of the UPX if UPX.Papx
    sal_uInt16 nSprmsLen;           // its length

    sal_uInt8 nWwNumLevel;           // for bullets and enumerations in styles

    bool bTextColChanged;
    bool bFontChanged;      // For Simulating Default-Font
    bool bCJKFontChanged;   // For Simulating Default-CJK Font
    bool bCTLFontChanged;   // For Simulating Default-CTL Font
    bool bFSizeChanged;     // For Simulating Default-FontSize
    bool bFCTLSizeChanged;  // For Simulating Default-CTL FontSize
    bool bWidowsChanged;    // For Simulating Default-Widows / Orphans

    void ImportSprms(sal_Size nPosFc, short nLen, bool bPap);
    void ImportSprms(sal_uInt8 *pSprms, short nLen, bool bPap);
    void ImportGrupx(short nLen, bool bPara, bool bOdd);
    short ImportUPX(short nLen, bool bPAP, bool bOdd);

    void Set1StyleDefaults();
    void Import1Style(sal_uInt16 nNr);
    void RecursiveReg(sal_uInt16 nNr);

    void ImportStyles();

    void ImportNewFormatStyles();
    void ScanStyles();
    void ImportOldFormatStyles();

    bool PrepareStyle(SwWW8StyInf &rSI, ww::sti eSti, sal_uInt16 nThisStyle, sal_uInt16 nNextStyle);
    void PostStyle(SwWW8StyInf &rSI, bool bOldNoImp);

    WW8RStyle(const WW8RStyle&) = delete;
    WW8RStyle& operator=(const WW8RStyle&) = delete;
public:
    WW8RStyle( WW8Fib& rFib, SwWW8ImplReader* pI );
    void Import();
    void PostProcessStyles();
    const sal_uInt8* HasParaSprm( sal_uInt16 nId ) const;
};

class WW8FlySet: public SfxItemSet
{
private:
    const WW8FlySet& operator=(const WW8FlySet&) = delete;
    void Init(const SwWW8ImplReader& rReader, const SwPaM* pPaM);
public:
    WW8FlySet(SwWW8ImplReader& rReader, const WW8FlyPara* pFW,
        const WW8SwFlyPara* pFS, bool bGraf);
    WW8FlySet(SwWW8ImplReader& rReader, const SwPaM* pPaM, const WW8_PIC& rPic,
        long nWidth, long nHeight);
};

enum WW8LvlType {WW8_None, WW8_Outline, WW8_Numbering, WW8_Sequence, WW8_Pause};

WW8LvlType GetNumType(sal_uInt8 nWwLevelNo);
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
