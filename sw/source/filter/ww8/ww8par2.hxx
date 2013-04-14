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

#ifndef _WW8PAR2_HXX
#define _WW8PAR2_HXX

#include <swtypes.hxx>  // enum RndStdIds
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <editeng/lrspitem.hxx>

#include "ww8scan.hxx"  // class WW8Style
#include "ww8par.hxx"   // WW8_BRC5

class WW8RStyle;

class WW8DupProperties
{
public:
    WW8DupProperties(SwDoc &rDoc, SwWW8FltControlStack *pStk);
    void Insert(const SwPosition &rPos);
private:
    //No copying
    WW8DupProperties(const WW8DupProperties&);
    WW8DupProperties& operator=(const WW8DupProperties&);
    SwWW8FltControlStack* pCtrlStck;
    SfxItemSet aChrSet,aParSet;
};


struct WW8SwFlyPara
{
    SwFlyFrmFmt* pFlyFmt;

                // 1. Teil: daraus abgeleitete Sw-Attribute
    sal_Int16 nXPos, nYPos;         // Position
    sal_Int16 nLeMgn, nRiMgn;       // Raender
    sal_Int16 nUpMgn, nLoMgn;       // Raender
    sal_Int16 nWidth, nHeight;      // Groesse
    sal_Int16 nNettoWidth;

    SwFrmSize eHeightFix;       // Hoehe Fix oder Min
    RndStdIds eAnchor;          // Bindung
    short eHRel;     // Seite oder Seitenrand
    short eVRel;     // Seite oder Seitenrand
    sal_Int16 eVAlign;       // Oben, unten, mittig
    sal_Int16 eHAlign;       // links, rechts, mittig
    SwSurround eSurround;       // Wrap-Mode

    sal_uInt8 nXBind, nYBind;        // relativ zu was gebunden

                // 2.Teil: sich waehrend des Einlesens ergebende AEnderungen
    long nNewNettoWidth;
    SwPosition* pMainTextPos;   // um nach Apo in Haupttext zurueckzukehren
    sal_uInt16 nLineSpace;          // LineSpace in tw fuer Graf-Apos
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
    SwWW8ImplReader* pIo;   // Parser-Klasse
    SvStream* pStStrm;      // Input-File

    SwNumRule* pStyRule;    // Bullets und Aufzaehlungen in Styles

    sal_uInt8* pParaSprms;           // alle ParaSprms des UPX falls UPX.Papx
    sal_uInt16 nSprmsLen;           // Laenge davon

    sal_uInt8 nWwNumLevel;           // fuer Bullets und Aufzaehlungen in Styles

    bool bTxtColChanged;
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

    //No copying
    WW8RStyle(const WW8RStyle&);
    WW8RStyle& operator=(const WW8RStyle&);
public:
    WW8RStyle( WW8Fib& rFib, SwWW8ImplReader* pI );
    void Import();
    void PostProcessStyles();
    const sal_uInt8* HasParaSprm( sal_uInt16 nId ) const;
};

class WW8FlySet: public SfxItemSet
{
private:
    //No copying
    const WW8FlySet& operator=(const WW8FlySet&);
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
