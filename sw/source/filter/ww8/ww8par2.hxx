/*************************************************************************
 *
 *  $RCSfile: ww8par2.hxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: cmc $ $Date: 2002-06-27 16:04:27 $
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

#ifndef _WW8PAR2_HXX
#define _WW8PAR2_HXX

#ifndef SWTYPES_HXX
#include <swtypes.hxx>  // enum RndStdIds
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif

#ifndef WW8SCAN_HXX
#include "ww8scan.hxx"  // class WW8Style
#endif
#ifndef WW8PAR_HXX
#include "ww8par.hxx"   // WW8_BRC5
#endif

class SwCharFmt;
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
    BOOL bVer67;
    INT16 nSp26, nSp27;         // rohe Position
    INT16 nSp45, nSp28;         // Breite / Hoehe
    INT16 nLeMgn, nRiMgn, nUpMgn, nLoMgn;           // Raender
    BYTE nSp29;                 // rohe Bindung + Alignment
    BYTE nSp37;                 // Wrap-Mode ( 1 / 2; 0 = no Apo ? )
    WW8_BRC5 brc;               // Umrandung Top, Left, Bottom, Right, Between
    BOOL bBorderLines;          // Umrandungslinien
    BOOL bGrafApo;              // TRUE: Dieser Rahmen dient allein dazu, die
                                // enthaltene Grafik anders als zeichengebunden
                                // zu positionieren

    WW8FlyPara( BOOL bIsVer67, const WW8FlyPara* pSrc = 0 );
    int operator == ( const WW8FlyPara& rSrc ) const;
    BOOL Read( const BYTE* pSprm29, WW8PLCFx_Cp_FKP* pPap );
    BOOL ReadFull( const BYTE* pSprm29, SwWW8ImplReader* pIo );
    BOOL Read( const BYTE* pSprm29, WW8RStyle* pStyle );
    void ApplyTabPos(WW8_TablePos *pTabPos, const BYTE *pSprm29);
};

struct WW8SwFlyPara
{
    SwFlyFrmFmt* pFlyFmt;

                // 1. Teil: daraus abgeleitete Sw-Attribute
    INT16 nXPos, nYPos;         // Position
    INT16 nLeMgn, nRiMgn;       // Raender
    INT16 nUpMgn, nLoMgn;       // Raender
    INT16 nWidth, nHeight;      // Groesse
    INT16 nNettoWidth;
    INT16 nInnerMgn;            // Rand innen

    SwFrmSize eHeightFix;       // Hoehe Fix oder Min
    RndStdIds eAnchor;          // Bindung
    SwRelationOrient eHRel;     // Seite oder Seitenrand
    SwRelationOrient eVRel;     // Seite oder Seitenrand
    SwVertOrient eVAlign;       // Oben, unten, mittig
    SwHoriOrient eHAlign;       // links, rechts, mittig
    SwSurround eSurround;       // Wrap-Mode

    BYTE nXBind, nYBind;        // relativ zu was gebunden

                // 2.Teil: sich waehrend des Einlesens ergebende AEnderungen
    long nNewNettoWidth;
    SwPosition* pMainTextPos;   // um nach Apo in Haupttext zurueckzukehren
    USHORT nLineSpace;          // LineSpace in tw fuer Graf-Apos
    BOOL bAutoWidth;
    BOOL bToggelPos;

    WW8SwFlyPara(SwPaM& rPaM, SwWW8ImplReader& rIo, WW8FlyPara& rWW,
        short nPgLeft, short nPgWidth, INT32 nIniFlyDx, INT32 nIniFlyDy);

    void BoxUpWidth( long nWidth );
    const SwPosition* GetMainTextPos() const    // Fuer PageDesc aus Apo
                { return pMainTextPos; };
    SwWW8FltAnchorStack *pOldAnchorStck;
};

class SwWW8StyInf
{
    String      sWWStyleName;
    USHORT      nWWStyleId;
public:
    SwFmt*      pFmt;
    WW8FlyPara* pWWFly;
    SwNumRule*  pOutlineNumrule;
    long        nFilePos;
    short       nLeftParaMgn;
    short       nTxtFirstLineOfst;
    USHORT      nBase;
    USHORT      nFollow;
    USHORT      nLFOIndex;
    rtl_TextEncoding eFontSrcCharSet;   // rtl_TextEncoding fuer den Font
    BYTE        nListLevel;
    BYTE        nOutlineLevel;      // falls Gliederungs-Style
    BYTE        n81Flags;           // Fuer Bold, Italic, ...
    BOOL        bValid          :1; // leer oder Valid
    BOOL        bImported       :1; // fuers rekursive Importieren
    BOOL        bColl           :1; // TRUE -> pFmt ist SwTxtFmtColl
    BOOL        bImportSkipped  :1; // nur TRUE bei !bNewDoc && vorh. Style
    BOOL        bHasStyNumRule  :1; // TRUE -> Benannter NumRule in Style
    BOOL        bHasBrokenWW6List :1;   //TRUE -> WW8+ style has a WW7- list

    SwWW8StyInf() :
        sWWStyleName( aEmptyStr ),
        nWWStyleId( 0 ),
        pFmt( 0 ),
        pWWFly( 0 ),
        pOutlineNumrule( 0 ),
        nFilePos( 0 ),
        nLeftParaMgn( 0 ),
        nTxtFirstLineOfst( 0 ),
        nBase( 0 ),
        nFollow( 0 ),
        nLFOIndex( USHRT_MAX ),
        eFontSrcCharSet( 0 ),
        nListLevel( nWW8MaxListLevel ),
        nOutlineLevel( MAXLEVEL ),
        n81Flags( 0 ),
        bValid( FALSE ),
        bImported( FALSE ),
        bColl( FALSE ),
        bImportSkipped( FALSE ),
        bHasStyNumRule( FALSE ),
        bHasBrokenWW6List( FALSE )
    {}

    ~SwWW8StyInf()
    {
        delete pWWFly;
    }

    void SetOrgWWIdent( const String& rName, const USHORT nId )
    {
        sWWStyleName = rName;
        nWWStyleId = nId;
    }
    const USHORT GetWWStyleId() const { return nWWStyleId; }
    const String& GetOrgWWName() const
    {
        return sWWStyleName;
    }
};


class WW8RStyle: public WW8Style
{
friend class SwWW8ImplReader;
    wwSprmParser maSprmParser;
    SwWW8ImplReader* pIo;   // Parser-Klasse
    SvStream* pStStrm;      // Input-File

    SwNumRule* pStyRule;    // Bullets und Aufzaehlungen in Styles

    BYTE* pParaSprms;           // alle ParaSprms des UPX falls UPX.Papx
    USHORT nSprmsLen;           // Laenge davon

    BYTE nWwNumLevel;           // fuer Bullets und Aufzaehlungen in Styles

    BOOL bTxtColChanged : 1;
    BOOL bFontChanged   : 1;    // Fuer Simulation Default-Font
    BOOL bCJKFontChanged: 1;    // Fuer Simulation Default-Font
    BOOL bFSizeChanged  : 1;    // Fuer Simulation Default-FontSize
    BOOL bWidowsChanged : 1;    // Fuer Simulation Default-Widows / Orphans

    void ImportSprms( long nPosFc, short nLen, BOOL bPap );
    void ImportGrupx( short nLen, BOOL bPara, BOOL bOdd );
    short ImportUPX( short nLen, BOOL bPAP, BOOL bOdd );


    SwCharFmt* SearchCharFmt( const String& rName );
    SwCharFmt* MakeNewCharFmt( WW8_STD* pStd, const String& rName );
    SwCharFmt* MakeOrGetCharFmt( BOOL* pbStyExist, WW8_STD* pStd,
        const String& rName );

    SwTxtFmtColl* SearchFmtColl( const String& rName );
    SwTxtFmtColl* MakeNewFmtColl( WW8_STD* pStd, const String& rName );
    SwTxtFmtColl* MakeOrGetFmtColl( BOOL* pbStyExist, WW8_STD* pStd,
        const String& rName );

    void Set1StyleDefaults();
    void Import1Style(USHORT nNr);
    void RecursiveReg(USHORT nNr);
    void ScanStyles();

    //No copying
    WW8RStyle(const WW8RStyle&);
    WW8RStyle& operator=(const WW8RStyle&);
public:
    WW8RStyle( WW8Fib& rFib, SwWW8ImplReader* pI );
    void Import();
    void PostProcessStyles();
    const BYTE* HasParaSprm( USHORT nId ) const;
};

class WW8FlySet: public SfxItemSet
{
private:
    //No copying
    const WW8FlySet& operator=(const WW8FlySet&);
public:
    WW8FlySet( SwWW8ImplReader& rReader, const WW8FlyPara* pFW,
        const WW8SwFlyPara* pFS, BOOL bGraf );
    WW8FlySet( SwWW8ImplReader& rReader, const SwPaM* pPaM, const WW8_PIC& rPic,
        long nWidth, long nHeight );
};

enum WW8LvlType {WW8_None, WW8_Outline, WW8_Numbering, WW8_Sequence, WW8_Pause};

WW8LvlType GetNumType(BYTE nWwLevelNo);

#define STI_USER 0x0FFE

#endif
