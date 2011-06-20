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

struct WW8FlyPara
{                       // WinWord-Attribute
                        // Achtung: *Nicht* umsortieren, da Teile mit
                        // memcmp verglichen werden
    bool bVer67;
    sal_Int16 nSp26, nSp27;         // rohe Position
    sal_Int16 nSp45, nSp28;         // Breite / Hoehe
    sal_Int16 nLeMgn, nRiMgn, nUpMgn, nLoMgn;           // Raender
    sal_uInt8 nSp29;                 // rohe Bindung + Alignment
    sal_uInt8 nSp37;                 // Wrap-Mode ( 1 / 2; 0 = no Apo ? )
    WW8_BRC5 brc;               // Umrandung Top, Left, Bottom, Right, Between
    bool bBorderLines;          // Umrandungslinien
    bool bGrafApo;              // true: Dieser Rahmen dient allein dazu, die
                                // enthaltene Grafik anders als zeichengebunden
                                // zu positionieren
    bool mbVertSet;             // true if vertical positioning has been set
    sal_uInt8 nOrigSp29;

    WW8FlyPara(bool bIsVer67, const WW8FlyPara* pSrc = 0);
    bool operator==(const WW8FlyPara& rSrc) const;
    void Read(const sal_uInt8* pSprm29, WW8PLCFx_Cp_FKP* pPap);
    void ReadFull(const sal_uInt8* pSprm29, SwWW8ImplReader* pIo);
    void Read(const sal_uInt8* pSprm29, WW8RStyle* pStyle);
    void ApplyTabPos(const WW8_TablePos *pTabPos);
    bool IsEmpty() const;
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

class SwWW8StyInf
{
    String      sWWStyleName;
    sal_uInt16      nWWStyleId;
public:
    rtl_TextEncoding eLTRFontSrcCharSet;    // rtl_TextEncoding fuer den Font
    rtl_TextEncoding eRTLFontSrcCharSet;    // rtl_TextEncoding fuer den Font
    rtl_TextEncoding eCJKFontSrcCharSet;    // rtl_TextEncoding fuer den Font
    SwFmt*      pFmt;
    WW8FlyPara* pWWFly;
    SwNumRule*  pOutlineNumrule;
    long        nFilePos;
    sal_uInt16      nBase;
    sal_uInt16      nFollow;
    sal_uInt16      nLFOIndex;
    sal_uInt8        nListLevel;
    sal_uInt8        nOutlineLevel;      // falls Gliederungs-Style
    sal_uInt16  n81Flags;           // Fuer Bold, Italic, ...
    sal_uInt16  n81BiDiFlags;       // Fuer Bold, Italic, ...
    SvxLRSpaceItem maWordLR;
    bool bValid;            // leer oder Valid
    bool bImported;         // fuers rekursive Importieren
    bool bColl;             // true-> pFmt ist SwTxtFmtColl
    bool bImportSkipped;    // nur true bei !bNewDoc && vorh. Style
    bool bHasStyNumRule;    // true-> Benannter NumRule in Style
    bool bHasBrokenWW6List; // true-> WW8+ style has a WW7- list
    bool bListReleventIndentSet; //true if this style's indent has
                                 //been explicitly set, it's set to the value
                                 //of pFmt->GetItemState(RES_LR_SPACE, false)
                                 //if it was possible to get the ItemState
                                 //for L of the LR space independantly
    bool bParaAutoBefore;   // For Auto spacing before a paragraph
    bool bParaAutoAfter;    // For Auto Spacing after a paragraph

    SwWW8StyInf() :
        sWWStyleName( aEmptyStr ),
        nWWStyleId( 0 ),
        eLTRFontSrcCharSet(0),
        eRTLFontSrcCharSet(0),
        eCJKFontSrcCharSet(0),
        pFmt( 0 ),
        pWWFly( 0 ),
        pOutlineNumrule( 0 ),
        nFilePos( 0 ),
        nBase( 0 ),
        nFollow( 0 ),
        nLFOIndex( USHRT_MAX ),
        nListLevel(WW8ListManager::nMaxLevel),
        nOutlineLevel( MAXLEVEL ),
        n81Flags( 0 ),
        n81BiDiFlags(0),
        maWordLR( RES_LR_SPACE ),
        bValid(false),
        bImported(false),
        bColl(false),
        bImportSkipped(false),
        bHasStyNumRule(false),
        bHasBrokenWW6List(false),
        bListReleventIndentSet(false),
        bParaAutoBefore(false),
        bParaAutoAfter(false)

    {}

    ~SwWW8StyInf()
    {
        delete pWWFly;
    }

    void SetOrgWWIdent( const String& rName, const sal_uInt16 nId )
    {
        sWWStyleName = rName;
        nWWStyleId = nId;
    }
    sal_uInt16 GetWWStyleId() const { return nWWStyleId; }
    const String& GetOrgWWName() const
    {
        return sWWStyleName;
    }
    bool IsOutline() const
    {
        return (pFmt && (MAXLEVEL > nOutlineLevel));
    }
    bool IsOutlineNumbered() const
    {
        return pOutlineNumrule && IsOutline();
    }
    const SwNumRule* GetOutlineNumrule() const
    {
        return pOutlineNumrule;
    }
    CharSet GetCharSet() const;
    CharSet GetCJKCharSet() const;
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
