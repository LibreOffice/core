/*************************************************************************
 *
 *  $RCSfile: swparrtf.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: os $ $Date: 2002-12-10 14:30:10 $
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

#ifndef __SGI_STL_MAP
#include <map>
#endif

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
    ::std::map<SwTableNode *, SwNodeIndex *> maTables;
    SwRTFStyleTbl aTxtCollTbl;
    SwRTFCharStyleTbl aCharFmtTbl;
    SwFlySaveArr aFlyArr;               // Flys als Letzes im Doc setzen
    SvBools aMergeBoxes;                // Flags fuer gemergte Zellen
    SwListArr aListArr;
    SvPtrarr aTblFmts;
    SvPtrarr aRubyCharFmts;

    SfxItemSet* pGrfAttrSet;
    SwTableNode* pTableNode, *pOldTblNd; // fuers Lesen von Tabellen: akt. Tab
    SwNodeIndex* pSttNdIdx;
    SwNodeIndex* pRegionEndIdx;
    SwDoc*  pDoc;
    SwPaM*  pPam;               // SwPosition duerfte doch reichen, oder ??
    SwRelNumRuleSpaces* pRelNumRule;    // Liste aller benannten NumRules

    USHORT nAktPageDesc, nAktFirstPageDesc;
    USHORT nAktBox;         // akt. Box
    USHORT nInsTblRow;      // beim nach \row kein \pard -> neue Line anlegen
    USHORT nNewNumSectDef;  // jeder SectionWechsel kann neue Rules definieren

    bool bSwPageDesc;
    bool bReadSwFly;        // lese Swg-Fly (wichtig fuer Bitmaps!)
    bool mbReadNoTbl;       // verhinder Tabelle in Tabelle/FootNote
    bool mbIsFootnote;
    bool bFootnoteAutoNum;  // automatische Numerierung ?
    bool bStyleTabValid;    // Styles schon erzeugt ?
    bool bInPgDscTbl;       // beim PageDescTbl lesen
    bool bNewNumList;       // Word 7.0 NumList gelesen, 6.0 ueberspringen
    bool bFirstContinue;    // 1.Call ins Continue
    bool bFirstDocControl;  // 1.Call of ReadDocControl

    /*
     #i9243#
     In a footnote tables are not possible (for some obscure reason!)
    */
    bool CantUseTables() const { return mbReadNoTbl || mbIsFootnote; }

    virtual void InsertPara();
    virtual void InsertText();
    virtual void MovePos( int bForward = TRUE );
    virtual void SetEndPrevPara( SvxNodeIdx*& rpNodePos, xub_StrLen& rCntPos );

    SwFmtPageDesc* GetCurrentPageDesc(SwPaM *pPam);
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


#endif
    //_SWPARRTF_HXX

