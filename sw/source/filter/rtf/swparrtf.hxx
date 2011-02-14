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
/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef _SWPARRTF_HXX
#define _SWPARRTF_HXX

#ifndef __SGI_STL_DEQUE
#include <deque>
#endif
#ifndef __SGI_STL_VECTOR
#include <vector>
#endif

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_BOOLS
#include <svl/svstdarr.hxx>
#endif
#include <editeng/svxrtf.hxx>
#include <editeng/numitem.hxx>
#include <editeng/boxitem.hxx>
#include <redline.hxx>

#include <fltshell.hxx>         // fuer den Attribut Stack
#include <ndindex.hxx>
#include "../inc/msfilter.hxx"
#include <svx/svdobj.hxx>


extern void GetLineIndex(SvxBoxItem &rBox, short nLineThickness, short nSpace, sal_uInt8 nCol, short nIdx,
    sal_uInt16 nOOIndex, sal_uInt16 nWWIndex, short *pSize);

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
class SwNumRule;
class SwRTFParser;
class SvxFontItem;
class SwRelNumRuleSpaces;
class SwNodeNum;
class SwTxtNode;
struct SvxRTFPictureType;

class RtfReader: public Reader
{
    virtual sal_uLong Read( SwDoc &, const String& rBaseURL, SwPaM &,const String &);
public:
    virtual sal_uLong Read( SvStream* pStrm, SwDoc &, const String& rBaseURL, SwPaM &);
};

class SwNodeIdx : public SvxNodeIdx
{
    SwNodeIndex aIdx;
public:
    SwNodeIdx( const SwNodeIndex& rIdx ) : aIdx( rIdx ) {}
    virtual sal_uLong   GetIdx() const;
    virtual SvxNodeIdx* Clone() const;
};

class BookmarkPosition
{
public:
    SwNodeIndex maMkNode;
    xub_StrLen mnMkCntnt;
    BookmarkPosition(const SwPaM &rPaM);
    BookmarkPosition(const BookmarkPosition &rEntry);

    bool operator==(const BookmarkPosition);
private:
    //No assignment
    BookmarkPosition& operator=(const BookmarkPosition&);

};

class SwxPosition : public SvxPosition
{
    SwPaM* pPam;
public:
    SwxPosition( SwPaM* pCrsr ) : pPam( pCrsr ) {}

    virtual sal_uLong   GetNodeIdx() const;
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
    xub_StrLen  nEndCnt;
    SwTwips     nPageWidth;
    sal_uInt16      nDropLines, nDropAnchor;

    SwFlySave( const SwPaM& rPam, SfxItemSet& rSet );
    int IsEqualFly( const SwPaM& rPos, SfxItemSet& rSet );
    void SetFlySize( const SwTableNode& rTblNd );
};

struct SwListEntry
{
    long nListId, nListTemplateId, nListNo;
    sal_uInt16 nListDocPos;
    sal_Bool bRuleUsed;

    SwListEntry()
        : nListId( 0 ), nListTemplateId( 0 ), nListNo( 0 ), nListDocPos( 0 ),
        bRuleUsed( sal_False )
    {}
    SwListEntry( long nLstId, long nLstTmplId, sal_uInt16 nLstDocPos )
        : nListId( nLstId ), nListTemplateId( nLstTmplId ), nListNo( 0 ),
        nListDocPos( nLstDocPos ), bRuleUsed( sal_False )
    {}

    void Clear() { nListId = nListTemplateId = nListNo = 0, nListDocPos = 0;
                    bRuleUsed = sal_False; }
};

DECLARE_TABLE( SwRTFStyleTbl, SwTxtFmtColl* )
DECLARE_TABLE( SwRTFCharStyleTbl, SwCharFmt* )
typedef SwFlySave* SwFlySavePtr;
SV_DECL_PTRARR_DEL( SwFlySaveArr, SwFlySavePtr, 0, 20 )
typedef std::deque< SwListEntry > SwListArr;

struct DocPageInformation
{
    SvxBoxItem maBox;
    long mnPaperw;
    long mnPaperh;
    long mnMargl;
    long mnMargr;
    long mnMargt;
    long mnMargb;
    long mnGutter;
    long mnPgnStart;
    bool mbFacingp;
    bool mbLandscape;
    bool mbRTLdoc;
    DocPageInformation();
};

struct SectPageInformation
{
    std::vector<long> maColumns;
    SvxBoxItem maBox;
    SvxNumberType maNumType;
    SwPageDesc *mpTitlePageHdFt;
    SwPageDesc *mpPageHdFt;
    long mnPgwsxn;
    long mnPghsxn;
    long mnMarglsxn;
    long mnMargrsxn;
    long mnMargtsxn;
    long mnMargbsxn;
    long mnGutterxsn;
    long mnHeadery;
    long mnFootery;
    long mnPgnStarts;
    long mnCols;
    long mnColsx;
    long mnStextflow;
    int mnBkc;
    bool mbLndscpsxn;
    bool mbTitlepg;
    bool mbFacpgsxn;
    bool mbRTLsection;
    bool mbPgnrestart;
    bool mbTitlePageHdFtUsed;
    bool mbPageHdFtUsed;
    SectPageInformation(const DocPageInformation &rDoc);
    SectPageInformation(const SectPageInformation &rSect);
};

class rtfSection
{
public:
    rtfSection(const SwPosition &rPos,
        const SectPageInformation &rPageInfo);
    SwNodeIndex maStart;
    SectPageInformation maPageInfo;
    SwSection *mpSection;
    SwPageDesc *mpTitlePage;
    SwPageDesc *mpPage;

    bool IsContinous() const { return maPageInfo.mnBkc == 0; }
    long NoCols() const { return maPageInfo.mnCols; }
    long StandardColSeperation() const { return maPageInfo.mnColsx; }
    bool HasTitlePage() const { return maPageInfo.mbTitlepg; }
    long PageStartAt() const { return maPageInfo.mnPgnStarts; }
    bool PageRestartNo() const { return maPageInfo.mbPgnrestart; }
    bool IsBiDi() const { return maPageInfo.mbRTLsection; }
    long GetPageWidth() const { return maPageInfo.mnPgwsxn; }
    long GetPageHeight() const { return maPageInfo.mnPghsxn; }
    long GetPageLeft() const { return maPageInfo.mnMarglsxn; }
    long GetPageRight() const { return maPageInfo.mnMargrsxn; }
    bool IsLandScape() const { return maPageInfo.mbLndscpsxn; }
};

class rtfSections
{
private:
    SwRTFParser &mrReader;
    std::deque<rtfSection> maSegments;
    typedef std::deque<rtfSection>::iterator mySegIter;
    typedef std::deque<rtfSection>::reverse_iterator mySegrIter;

    struct wwULSpaceData
    {
        bool bHasHeader, bHasFooter;
        short nSwHLo, nHdUL, nSwFUp, nFtUL, nSwUp,  nSwLo;
        wwULSpaceData() : bHasHeader(false), bHasFooter(false) {}
    };

    void SetSegmentToPageDesc(const rtfSection &rSection, bool bTitlePage,
        bool bIgnoreCols);
    SwSectionFmt *InsertSection(SwPaM& rMyPaM, rtfSection &rSection);
    void SetPage(SwPageDesc &rInPageDesc, SwFrmFmt &rFmt,
        const rtfSection &rSection, bool bIgnoreCols);
    void GetPageULData(const rtfSection &rSection, bool bFirst,
        wwULSpaceData& rData);
    void SetPageULSpaceItems(SwFrmFmt &rFmt, wwULSpaceData& rData);
    bool SetCols(SwFrmFmt &rFmt, const rtfSection &rSection,
        sal_uInt16 nNettoWidth);
    void SetHdFt(rtfSection &rSection);
    void CopyFrom(const SwPageDesc &rFrom, SwPageDesc &rDest);
    void MoveFrom(SwPageDesc &rFrom, SwPageDesc &rDest);
public:
    bool empty() const { return maSegments.empty(); }
    int size() const { return maSegments.size(); }
    void push_back(const rtfSection &rSect);
    void pop_back() { maSegments.pop_back(); }
    rtfSection& back() { return maSegments.back(); }
    const rtfSection& back() const { return maSegments.back(); }
    void InsertSegments(bool bIsNewDoc);
    rtfSections(SwRTFParser &rReader) : mrReader(rReader) {}
    std::vector<sal_uInt16> maDummyPageNos;
    typedef std::vector<sal_uInt16>::reverse_iterator myrDummyIter;
    void PrependedInlineNode(const SwPosition &rPos,
        const SwNode &rNode);
};




class SwRTFParser : public SvxRTFParser
{
    /*
     Knows which writer style a given word style should be imported as.
    */
    sw::util::ParaStyleMapper maParaStyleMapper;
    sw::util::CharStyleMapper maCharStyleMapper;

    std::vector<String> aRevTbl;

    friend class rtfSections;
    DocPageInformation maPageDefaults;
    rtfSections maSegments;

    sw::util::InsertedTablesManager maInsertedTables;
    SwRTFStyleTbl aTxtCollTbl;
    SwRTFCharStyleTbl aCharFmtTbl;
    SwFlySaveArr aFlyArr;               // Flys als Letzes im Doc setzen
    SvBools aMergeBoxes;                // Flags fuer gemergte Zellen
    SwListArr aListArr;
    SvPtrarr aTblFmts;
    SvPtrarr aRubyCharFmts;
    BookmarkPosition* mpBookmarkStart;
    sw::util::RedlineStack *mpRedlineStack;
    sw::util::AuthorInfos* pAuthorInfos;

    SfxItemSet* pGrfAttrSet;
    SwTableNode* pTableNode, *pOldTblNd; // fuers Lesen von Tabellen: akt. Tab
    SwNodeIndex* pSttNdIdx;
    SwNodeIndex* pRegionEndIdx;
    SwDoc*  pDoc;
    SwPaM*  pPam;               // SwPosition duerfte doch reichen, oder ??
    SwRelNumRuleSpaces* pRelNumRule;    // Liste aller benannten NumRules

    String sNestedFieldStr;
    SwFltRedline *pRedlineInsert;
    SwFltRedline *pRedlineDelete;

    String sBaseURL;

    sal_uInt16 nAktPageDesc, nAktFirstPageDesc;
    sal_uInt16 m_nCurrentBox;
    sal_uInt16 nInsTblRow;      // beim nach \row kein \pard -> neue Line anlegen
    sal_uInt16 nNewNumSectDef;  // jeder SectionWechsel kann neue Rules definieren
    sal_uInt16 nRowsToRepeat;

    bool bSwPageDesc;
    bool bReadSwFly;        // lese Swg-Fly (wichtig fuer Bitmaps!)
    // --> OD 2008-12-22 #i83368#
    bool mbReadCellWhileReadSwFly;
    // <--
    bool mbReadNoTbl;       // verhinder Tabelle in Tabelle/FootNote
    bool mbIsFootnote;
    bool bFootnoteAutoNum;  // automatische Numerierung ?
    bool bStyleTabValid;    // Styles schon erzeugt ?
    bool bInPgDscTbl;       // beim PageDescTbl lesen
    bool bNewNumList;       // Word 7.0 NumList gelesen, 6.0 ueberspringen
    bool bFirstContinue;    // 1.Call ins Continue
    bool bContainsPara;     // If there is no paragraph in the section
    bool bContainsTablePara;     // If there is an table in this section
    bool bForceNewTable;     // Forces a beginning of a new table
    bool bNestedField;
    bool bTrowdRead;         // True, iff an \trowd definition was read after the last \row

    int nReadFlyDepth;

    int nZOrder;
    /*
     #i9243#
     In a footnote tables are not possible (for some obscure reason!)
    */
    bool CantUseTables() const { return mbReadNoTbl || mbIsFootnote; }

    virtual void InsertPara();
    virtual void InsertText();
    virtual void MovePos( int bForward = sal_True );
    virtual void SetEndPrevPara( SvxNodeIdx*& rpNodePos, xub_StrLen& rCntPos );
    void EnterEnvironment();
    void LeaveEnvironment();

    SwFmtPageDesc* GetCurrentPageDesc(SwPaM *pPam);
    void CheckInsNewTblLine();

    // setze RTF-Werte auf Swg-Werte
    void SetSwgValues( SfxItemSet& rSet );

    virtual void ReadInfo( const sal_Char* pChkForVerNo = 0 );

    void ReadUserProperties();

    void ReadListLevel( SwNumRule& rRule, sal_uInt8 nLvl );
    void SetBorderLine(SvxBoxItem& rBox, sal_uInt16 nLine);
    void ReadListTable();
    sal_uInt16 ReadRevTbl();
    void ReadShpRslt();
    void ReadShpTxt(String &shpTxt);
    void ReadDrawingObject();
    void InsertShpObject(SdrObject* pStroke, int nZOrder);
    void ReadShapeObject();
    void ReadListOverrideTable();
    SwNumRule *ReadNumSecLevel( int nToken );
    SwNumRule* GetNumRuleOfListNo( long nListNo,
                                    sal_Bool bRemoveFromList = sal_False );
    void RemoveUnusedNumRule( SwNumRule* );
    void RemoveUnusedNumRules();
    const Font* FindFontOfItem( const SvxFontItem& rItem ) const;

    // 3 Methoden zum Aufbauen der Styles
    SwTxtFmtColl* MakeColl( const String&, sal_uInt16 nPos, sal_uInt8 nOutlineLevel,
                            bool& rbCollExist );
    SwCharFmt* MakeCharFmt( const String& rName, sal_uInt16 nPos,
                            int& rbCollExist );
    void SetStyleAttr( SfxItemSet& rCollSet,
                        const SfxItemSet& rStyleSet,
                        const SfxItemSet& rDerivedSet );
    SwTxtFmtColl* MakeStyle( sal_uInt16 nNo, const SvxRTFStyleType& rStyle );
    SwCharFmt* MakeCharStyle( sal_uInt16 nNo, const SvxRTFStyleType& rStyle );
    void MakeStyleTab();

    int MakeFieldInst( String& rFieldStr );

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
    long GetSafePos(long nPos);

protected:
    // wird fuer jedes Token gerufen, das in CallParser erkannt wird
    virtual void NextToken( int nToken );

//  virtual void ReadUnknownData();
    virtual void ReadBitmapData();
#ifdef READ_OLE_OBJECT
    virtual void ReadOLEData();
#endif
    virtual void SetAttrInDoc( SvxRTFItemStackType &rSet );
    virtual bool UncompressableStackEntry(const SvxRTFItemStackType &rSet) const;
    // fuer Tokens, die im ReadAttr nicht ausgewertet werden
    virtual void UnknownAttrToken( int nToken, SfxItemSet* pSet );

    void ReadPrtData();
    void ReadField();
    void ReadXEField();
    void ReadHeaderFooter( int nToken, SwPageDesc* pPageDesc = 0 );
    void ReadDocControls( int nToken );
    void SetPageInformationAsDefault(const DocPageInformation &rInfo);
    void ReadSectControls( int nToken );
    void DoHairyWriterPageDesc(int nToken);
    void ReadFly( int nToken, SfxItemSet* pFillSet = 0 );
    void ReadTable( int nToken );
    void ReadPageDescTbl();
    void SkipPageDescTbl();
    bool IsBorderToken(int nToken);

    const String& GetBaseURL() const { return sBaseURL;}

    virtual ~SwRTFParser();

public:
    SwRTFParser( SwDoc* pD,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::document::XDocumentProperties> i_xDocProps,
        const SwPaM& rCrsr, SvStream& rIn,
        const String& rBaseURL,
        int bReadNewDoc = sal_True );

    virtual SvParserState CallParser(); // Aufruf des Parsers
    virtual int IsEndPara( SvxNodeIdx* pNd, xub_StrLen nCnt ) const;

    // fuers asynchrone lesen aus dem SvStream
//  virtual void SaveState( int nToken );
//  virtual void RestoreState();
    virtual void Continue( int nToken );
};


#endif

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
