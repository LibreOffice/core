/*************************************************************************
 *
 *  $RCSfile: swparrtf.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:56 $
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
#ifndef _SWPARRTF_HXX
#define _SWPARRTF_HXX

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_BOOLS
#include <svtools/svstdarr.hxx>
#endif
#ifndef _SVXRTF_HXX //autogen
#include <svx/svxrtf.hxx>
#endif

#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif

class Font;
class Graphic;

class SwNoTxtNode;
class Size;
class SwPaM;
class SwDoc;
class SwPageDesc;
class SwTxtFmtColl;
class SwTableNode;
class SwCharFmt;
class SwNumFmt;
class SwNumRule;
class SwRTFParser;
class SvxFontItem;
class SwRelNumRuleSpaces;
class SwNodeNum;
class SwTxtNode;
struct SvxRTFPictureType;


class SwNodeIdx : public SvxNodeIdx
{
    SwNodeIndex aIdx;
public:
    SwNodeIdx( const SwNodeIndex& rIdx ) : aIdx( rIdx ) {}
    virtual ULONG   GetIdx() const;
    virtual SvxNodeIdx* Clone() const;
};

class SwxPosition : public SvxPosition
{
    SwPaM* pPam;
public:
    SwxPosition( SwPaM* pCrsr ) : pPam( pCrsr ) {}

    virtual ULONG   GetNodeIdx() const;
    virtual xub_StrLen GetCntIdx() const;

    // erzeuge von sich selbst eine Kopie
    virtual SvxPosition* Clone() const;
    // erzeuge vom NodeIndex eine Kopie
    virtual SvxNodeIdx* MakeNodeIdx() const;
};


// zum zwischenspeichern der Flys:
struct SwFlySave
{
    SfxItemSet  aFlySet;
    SwNodeIndex nSttNd, nEndNd;
    SwTwips     nPageWidth;
    xub_StrLen  nEndCnt;
    USHORT      nDropLines, nDropAnchor;

    SwFlySave( const SwPaM& rPam, SfxItemSet& rSet );
    int IsEqualFly( const SwPaM& rPos, SfxItemSet& rSet );
    void SetFlySize( const SwTableNode& rTblNd );
};

struct SwListEntry
{
    long nListId, nListTemplateId, nListNo;
    USHORT nListDocPos;
    BOOL bRuleUsed;

    SwListEntry()
        : nListId( 0 ), nListTemplateId( 0 ), nListNo( 0 ), nListDocPos( 0 ),
        bRuleUsed( FALSE )
    {}
    SwListEntry( long nLstId, long nLstTmplId, USHORT nLstDocPos )
        : nListId( nLstId ), nListTemplateId( nLstTmplId ), nListNo( 0 ),
        nListDocPos( nLstDocPos ), bRuleUsed( FALSE )
    {}

    void Clear() { nListId = nListTemplateId = nListNo = 0, nListDocPos = 0;
                    bRuleUsed = FALSE; }
};

DECLARE_TABLE( SwRTFStyleTbl, SwTxtFmtColl* )
DECLARE_TABLE( SwRTFCharStyleTbl, SwCharFmt* )
typedef SwFlySave* SwFlySavePtr;
SV_DECL_PTRARR_DEL( SwFlySaveArr, SwFlySavePtr, 0, 20 )
SV_DECL_VARARR( SwListArr, SwListEntry, 0, 20 )


class SwRTFParser : public SvxRTFParser
{
    SwRTFStyleTbl aTxtCollTbl;
    SwRTFCharStyleTbl aCharFmtTbl;
    SwFlySaveArr aFlyArr;               // Flys als Letzes im Doc setzen
    SvBools aMergeBoxes;                // Flags fuer gemergte Zellen
    SwListArr aListArr;
    SvPtrarr aTblFmts;

    SfxItemSet* pGrfAttrSet;
    SwTableNode* pTableNode, *pOldTblNd; // fuers Lesen von Tabellen: akt. Tab
    SwNodeIndex* pSttNdIdx;
    SwNodeIndex* pRegionEndIdx;
    SwDoc*  pDoc;
    SwPaM*  pPam;               // SwPosition duerfte doch reichen, oder ??
    SwRelNumRuleSpaces* pRelNumRule;    // Liste aller benannten NumRules

    USHORT nAktPageDesc, nAktFirstPageDesc;
    USHORT nAktBox;                     // akt. Box
    USHORT nInsTblRow;          // beim nach \row kein \pard -> neue Line anlegen
    USHORT nNewNumSectDef;      // jeder SectionWechsel kann neue Rules definieren

    BOOL bSwPageDesc : 1;
    BOOL bReadSwFly : 1;        // lese Swg-Fly (wichtig fuer Bitmaps!)
    BOOL bReadNoTbl : 1;        // verhinder Tabelle in Tabelle/FootNote
    BOOL bFootnoteAutoNum : 1;  // automatische Numerierung ?
    BOOL bStyleTabValid : 1;    // Styles schon erzeugt ?
    BOOL bInPgDscTbl : 1;       // beim PageDescTbl lesen
    BOOL bNewNumList : 1;       // Word 7.0 NumList gelesen, 6.0 ueberspringen
    BOOL bFirstContinue: 1;     // 1.Call ins Continue
    BOOL bFirstDocControl: 1;   // 1.Call of ReadDocControl


    virtual void InsertPara();
    virtual void InsertText();
    virtual void MovePos( int bForward = TRUE );
    virtual void SetEndPrevPara( SvxNodeIdx*& rpNodePos, xub_StrLen& rCntPos );

    void CheckInsNewTblLine();

    // setze RTF-Werte auf Swg-Werte
    void SetSwgValues( SfxItemSet& rSet );

    virtual void ReadInfo( const sal_Char* pChkForVerNo = 0 );

    void ReadListLevel( SwNumRule& rRule, BYTE nLvl );
    void ReadListTable();
    void ReadListOverrideTable();
    void ReadNumSecLevel( int nToken );
    SwNumRule* GetNumRuleOfListNo( long nListNo,
                                    BOOL bRemoveFromList = FALSE );
    void RemoveUnusedNumRule( SwNumRule* );
    void RemoveUnusedNumRules();
    const Font* FindFontOfItem( const SvxFontItem& rItem ) const;

    // 3 Methoden zum Aufbauen der Styles
    SwTxtFmtColl* MakeColl( const String&, USHORT nPos, BYTE nOutlineLevel,
                            int& rbCollExist );
    SwCharFmt* MakeCharFmt( const String& rName, USHORT nPos,
                            int& rbCollExist );
    void SetStyleAttr( SfxItemSet& rCollSet,
                        const SfxItemSet& rStyleSet,
                        const SfxItemSet& rDerivedSet );
    SwTxtFmtColl* MakeStyle( USHORT nNo, const SvxRTFStyleType& rStyle );
    SwCharFmt* MakeCharStyle( USHORT nNo, const SvxRTFStyleType& rStyle );
    void MakeStyleTab();

    int MakeFieldInst( String& rFieldStr );
    SwPageDesc* _MakeNewPageDesc( int bFirst );

    // einlesen/einfuegen von Bitmaps
    void InsPicture( const String& rNm,
                    const Graphic* = 0, const SvxRTFPictureType* = 0 );
    void _SetPictureSize( const SwNoTxtNode& rNd, const SwNodeIndex& rAnchor,
                            SfxItemSet& rSet, const SvxRTFPictureType* = 0  );

    void SetFlysInDoc();
    void GetPageSize( Size& rSize );        // Groesse der PagePrintArea

    // fuers Einlesen von Tabellen
    void GotoNextBox();
    void NewTblLine();

    void DelLastNode();         // loesche den letzten Node (Tabelle/Fly/Ftn/..)

    void AddNumRule( SwNumRule* pRule );
    void SetNumRelSpaces();
    void SetOultineRelSpaces( const SwNodeIndex& rStt,
                                const SwNodeIndex& rEnd );
    void SetNumLSpace( SwTxtNode& rNd, const SwNodeNum& rNum,
                                const SwNumRule& rRule );

protected:
    // wird fuer jedes Token gerufen, das in CallParser erkannt wird
    virtual void NextToken( int nToken );

//  virtual void ReadUnknownData();
    virtual void ReadBitmapData();
#ifdef READ_OLE_OBJECT
    virtual void ReadOLEData();
#endif
    virtual void SetAttrInDoc( SvxRTFItemStackType &rSet );
    // fuer Tokens, die im ReadAttr nicht ausgewertet werden
    virtual void UnknownAttrToken( int nToken, SfxItemSet* pSet );

    void ReadPrtData();
    void ReadField();
    void ReadHeaderFooter( int nToken, SwPageDesc* pPageDesc = 0 );
    void ReadDocControls( int nToken );
    void ReadSectControls( int nToken );
    void ReadFly( int nToken, SfxItemSet* pFillSet = 0 );
    void ReadTable( int nToken );
    void ReadPageDescTbl();

    virtual ~SwRTFParser();

public:
    SwRTFParser( SwDoc* pD, const SwPaM& rCrsr, SvStream& rIn,
                        int bReadNewDoc = TRUE );

    virtual SvParserState CallParser(); // Aufruf des Parsers
    virtual int IsEndPara( SvxNodeIdx* pNd, xub_StrLen nCnt ) const;

    // fuers asynchrone lesen aus dem SvStream
//  virtual void SaveState( int nToken );
//  virtual void RestoreState();
    virtual void Continue( int nToken );
};

/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.45  2000/09/18 16:04:51  willem.vandorp
      OpenOffice header added.

      Revision 1.44  2000/09/09 12:53:29  willem.vandorp
      Header and footer replaced

      Revision 1.43  2000/05/09 17:23:05  jp
      Changes for Unicode

      Revision 1.42  2000/04/14 14:00:41  khz
      Task #70451# use CalculateFlySize() for WW frames with 'auto'width

      Revision 1.41  2000/03/14 09:22:33  jp
      Bug #73941#: remove unused numrules, share override rules

      Revision 1.40  1999/08/03 17:43:08  JP
      Bug #68028#,#68026#,#67983#,#67982#: changes for import of PageDesc from interleave/WP7/Aplixware


      Rev 1.39   03 Aug 1999 19:43:08   JP
   Bug #68028#,#68026#,#67983#,#67982#: changes for import of PageDesc from interleave/WP7/Aplixware

      Rev 1.38   19 Apr 1999 12:48:50   JP
   Vorbereitungen fuer OLE-Objecte in RTF lesen (anstoss ist Bug #60140#)

      Rev 1.37   18 Mar 1999 09:51:50   JP
   Task #63049#: Numerierung mit rel. Einzuegen

      Rev 1.36   16 Mar 1999 23:20:00   JP
   Task #63049#: Einzuege bei NumRules relativ

      Rev 1.35   05 Mar 1999 14:27:58   JP
   Bug #57749#: spaltige Bereiche einlesen - jeder Bereich hat eigene NumRules

      Rev 1.34   03 Mar 1999 15:17:24   JP
   Bug #57749#: spaltige Bereiche einlesen

      Rev 1.33   24 Feb 1999 19:51:32   JP
   Task #62314#: Asynchrone DDE-Links/-Requests

      Rev 1.32   21 Sep 1998 17:16:14   JP
   DropCaps unterstuetzen

      Rev 1.31   13 Aug 1998 10:33:24   JP
   Bug #53525#: Tabellen - an den Kanten keine doppelte Umrandung mehr

      Rev 1.30   27 May 1998 22:26:16   JP
   Bug #50585#: ListOverrideTabelle korrekt einlesen

      Rev 1.29   20 Apr 1998 17:43:20   JP
   neu: Numerierung lesen/schreiben

      Rev 1.28   06 Apr 1998 14:46:30   JP
   Erweiterungen fuer Numerierungen (97-Vers.)

      Rev 1.27   03 Apr 1998 18:51:52   JP
   RTF-Parser um neue Tokens erweitert

      Rev 1.26   11 Feb 1998 14:07:20   JP
   kleinere Bugs behoben bei Rahmen & Tabellen

      Rev 1.25   26 Jan 1998 20:54:50   JP
   Bug #45958#: Continue - die gesplitteten Nodes richtig zusammenfassen, neu: Zeichenvorlagen lesen

      Rev 1.24   09 Oct 1997 14:27:52   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.23   01 Sep 1997 20:09:02   JP
   Bug #43126#: Flag fuers Einfuegen in Numerierungen

      Rev 1.22   11 Aug 1997 13:59:42   OM
   Headerfile-Umstellung

      Rev 1.21   27 Jun 1997 11:34:36   JP
   Bug #40723#: erst pard oder neue Ebene nach einem row hebt die Tabelle auf

      Rev 1.20   23 Oct 1996 12:42:24   JP
   SVMEM -> SVSTDARR Umstellung

      Rev 1.19   04 Jul 1996 15:02:30   JP
   Tabellen einlesen korrigiert

      Rev 1.18   28 Jun 1996 15:07:26   MA
   includes

      Rev 1.17   18 Jun 1996 16:35:38   JP
   ReadFly: erweitert um optionalen Itemset erweitert(FlyInFly lesen)

      Rev 1.16   12 Jun 1996 16:49:36   JP
   PageDesc: Header/Footer richtig einlesen

      Rev 1.15   08 May 1996 18:33:20   JP
   Erweiterung fuers asynchrone Laden

      Rev 1.14   25 Apr 1996 13:57:16   MIB
   Verschiebung SvHTMLParser in Gooedies

      Rev 1.13   25 Apr 1996 09:24:30   JP
   SwPictureType in svx exportiert

      Rev 1.12   12 Feb 1996 12:45:50   JP
   StyleTabelle auch aufbauen wenn sie gebraucht wird

      Rev 1.11   13 Dec 1995 10:35:58   MA
   opt: Bessere Defaults fuer Arrays

      Rev 1.10   30 Oct 1995 21:18:58   JP
   intbl refernziert auch vorher eingelesene Tabellen

      Rev 1.9   11 Oct 1995 20:31:48   HJS
   umbenennung

      Rev 1.8   23 Feb 1995 09:37:06   JP
   Methode RTFPardPlain wird nicht mehr benoetigt

      Rev 1.7   08 Feb 1995 09:49:14   JP
   alten RTF-Parser entfernt, Sw_RTF -> SwRTF

      Rev 1.6   18 Jan 1995 19:53:48   JP
   Footnote: beachte fuehrende Zeichen

      Rev 1.5   15 Jan 1995 20:34:12   JP
   Einfuegen in bestehendes Doc: Vorlagen richtig behandeln

      Rev 1.4   11 Jan 1995 19:45:30   JP
   RTF-Reader fertiggestellt

      Rev 1.3   09 Jan 1995 08:25:12   JP
   aus rtffly.cxx SwFlySave public gemacht

      Rev 1.2   06 Jan 1995 12:08:28   JP
   lesen von Tabellen zugefuegt

      Rev 1.1   23 Dec 1994 08:39:28   JP
   forward deklaration zugefuegt

      Rev 1.0   22 Dec 1994 17:38:36   JP
   Initial revision.

*************************************************************************/

#endif
    //_SWPARRTF_HXX

