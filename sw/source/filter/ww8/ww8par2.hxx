/*************************************************************************
 *
 *  $RCSfile: ww8par2.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:58 $
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
 #include "swtypes.hxx" // enum RndStdIds
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

class SwCharFmt;

#ifndef WW8PAR_HXX
#include <ww8par.hxx>   // WW8_BRC5
#endif

#ifndef WW8SCAN_HXX
#include <ww8scan.hxx>  // class WW8Style
#endif

class WW8RStyle;

struct WW8FlyPara
{                       // WinWord-Attribute
                        // Achtung: *Nicht* umsortieren, da Teile mit
                        // memcmp verglichen werden
    BOOL bVer67;
    INT16 nSp26, nSp27;         // rohe Position
    INT16 nSp45, nSp28;         // Breite / Hoehe
    INT16 nSp48, nSp49;         // Raender
    BYTE nSp29;                 // rohe Bindung + Alignment
    BYTE nSp37;                 // Wrap-Mode ( 1 / 2; 0 = no Apo ? )
    WW8_BRC5 brc;               // Umrandung Top, Left, Bottom, Right, Between
    BOOL bBorderLines;          // Umrandungslinien
    BOOL bBorder;               // Umrandungs-/Schatten-Struct vorhanden
    BOOL bDropCap;              // Rahmen als DropCap
    BOOL bGrafApo;              // TRUE: Dieser Rahmen dient allein dazu, die
                                // enthaltene Grafik anders als zeichengebunden
                                // zu positionieren

    WW8FlyPara( BOOL bIsVer67, const WW8FlyPara* pSrc = 0 );
    int operator == ( const WW8FlyPara& rSrc ) const;
    BOOL Read( BYTE* pSprm29, WW8PLCFx_Cp_FKP* pPap );
    BOOL ReadFull( BYTE* pSprm29, SwWW8ImplReader* pIo );
    BOOL Read( BYTE* pSprm29, WW8RStyle* pStyle );
};

struct WW8SwFlyPara
{
    SwFrmFmt* pFlyFmt;

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


    WW8SwFlyPara( SwPaM& rPaM, SwWW8ImplReader& rIo, WW8FlyPara& rWW,
                  short nPgTop,
                  short nPgLeft,
                  short nPgWidth, INT32 nIniFlyDx, INT32 nIniFlyDy );

    void BoxUpWidth( long nWidth );
    const SwPosition* GetMainTextPos() const    // Fuer PageDesc aus Apo
                { return pMainTextPos; };
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
        bHasStyNumRule( FALSE )
    {}

    ~SwWW8StyInf()
    {
        delete( pWWFly );
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
    SwWW8ImplReader* pIo;   // Parser-Klasse
    SvStream* pStStrm;      // Input-File

    SwNumRule* pStyRule;    // Bullets und Aufzaehlungen in Styles

    BYTE* pParaSprms;           // alle ParaSprms des UPX falls UPX.Papx
    short nSprmsLen;            // Laenge davon

    BYTE nWwNumLevel;           // fuer Bullets und Aufzaehlungen in Styles

    BOOL bFontChanged   : 1;    // Fuer Simulation Default-Font
    BOOL bFSizeChanged  : 1;    // Fuer Simulation Default-FontSize
    BOOL bWidowsChanged : 1;    // Fuer Simulation Default-Widows / Orphans

    void ImportSprms( long nPosFc, short nLen, BOOL bPap );
    void ImportGrupx( short nLen, BOOL bPara );
    short ImportUPX( short nLen, BOOL bPAP );


    SwCharFmt* SearchCharFmt( const String& rName );
    SwCharFmt* MakeNewCharFmt( WW8_STD* pStd, const String& rName );
    SwCharFmt* MakeOrGetCharFmt( BOOL* pbStyExist, WW8_STD* pStd, const String& rName );

    SwTxtFmtColl* SearchFmtColl( const String& rName );
    SwTxtFmtColl* MakeNewFmtColl( WW8_STD* pStd, const String& rName );
    SwTxtFmtColl* MakeOrGetFmtColl( BOOL* pbStyExist, WW8_STD* pStd, const String& rName );

    void Set1StyleDefaults();
    void Import1Style( USHORT nNr );
    void ScanStyles();
public:
    WW8RStyle( WW8Fib& rFib, SwWW8ImplReader* pI );
    void Import();
    void RegisterNumFmts();
    BYTE* HasParaSprm( USHORT nId ) const;
};

class WW8FlySet: public SfxItemSet
{
public:
    WW8FlySet( SwWW8ImplReader& rReader, /*const*/ WW8FlyPara* pFW,
              /*const*/ WW8SwFlyPara* pFS, BOOL bGraf );
    WW8FlySet( SwWW8ImplReader& rReader, const SwPaM* pPaM,
              const WW8_PIC& rPic, long nWidth, long nHeight );
};

enum WW8LvlType { WW8_None, WW8_Outline, WW8_Numbering, WW8_Sequence, WW8_Pause };

inline WW8LvlType GetNumType( BYTE nWwLevelNo )
{
    WW8LvlType nRet = WW8_None;
    if( nWwLevelNo == 12 )
        nRet = WW8_Pause;
    else if( nWwLevelNo == 10 )
        nRet = WW8_Numbering;
    else if( nWwLevelNo == 11 )
        nRet = WW8_Sequence;
    else if( nWwLevelNo > 0 && nWwLevelNo <= 9 )
        nRet = WW8_Outline;
    return nRet;
}

#define STI_USER 0xffe

/*************************************************************************
      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/ww8par2.hxx,v 1.1.1.1 2000-09-18 17:14:58 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.22  2000/09/18 16:05:00  willem.vandorp
      OpenOffice header added.

      Revision 1.21  2000/05/16 12:03:56  jp
      Changes for unicode

      Revision 1.20  2000/05/16 11:14:53  khz
      Unicode code-conversion

      Revision 1.19  2000/04/12 11:06:17  khz
      Task #74260# import FLY_PAGE-frames in Header/Footer as FLY_AT_CNTNT

      Revision 1.18  1999/12/09 16:46:32  khz
      Task #69180# allow Tabs if right of 1st-line-start OR right of paragraph margin

      Revision 1.17  1999/12/09 14:52:30  khz
      Task #67145# Don't set NumRule if OutlineNumrule is set on Style

      Revision 1.16  1999/12/01 14:35:48  khz
      Task #68488# Graphics in Sections with more than one Column

      Revision 1.15  1999/11/05 17:04:13  khz
      import new TOX_CONTENT and TOX_INDEX features (4)

      Revision 1.14  1999/11/05 10:22:15  khz
      import new TOX_CONTENT and TOX_INDEX features (3)

      Revision 1.13  1999/11/02 15:59:42  khz
      import new TOX_CONTENT and TOX_INDEX features (2)

      Revision 1.12  1999/04/28 21:03:30  KHZ
      Task #65245# Breite eines Rahmens bei WW \'automatisch\' muss PRTAREA sein


      Rev 1.11   28 Apr 1999 23:03:30   KHZ
   Task #65245# Breite eines Rahmens bei WW 'automatisch' muss PRTAREA sein

      Rev 1.10   28 Jan 1999 17:10:42   KHZ
   Task #60715# in SdrTextObj verankerte Grafiken und OLEs (4)

      Rev 1.9   25 Jan 1999 10:24:58   KHZ
   Task #60715# in Textobjekt verankerte Grafik als Grafik importieren

      Rev 1.8   08 Dec 1998 18:26:52   KHZ
   Task #59580# TOC-Import

      Rev 1.7   04 Dec 1998 17:56:40   JP
   Bug #59619#: falls ein Rahmen eine Umrandung hat, darf der Absatz keine bekommen

      Rev 1.6   19 Nov 1998 22:34:46   JP
   Task #59476#: Umstellungen fuers korrekte lesen von Umrandungen/Schatten

      Rev 1.5   15 Sep 1998 12:58:24   HJS
   '#56161#

      Rev 1.4   28 Jul 1998 11:02:06   KHZ
   Task #52607# nummerierte Listen (Teil 1)

      Rev 1.3   14 Jul 1998 16:33:04   KHZ
   Task #52607# Absatz- und Zeichen-Styles

      Rev 1.2   30 Jun 1998 21:33:24   KHZ
   Header/Footer/Footnotes weitgehend ok

      Rev 1.1   16 Jun 1998 18:34:56   KHZ
   DaSi-Stand

      Rev 1.0   16 Jun 1998 10:57:16   KHZ
   Initial revision.

*************************************************************************/
#endif


