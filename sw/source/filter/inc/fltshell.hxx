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
#ifndef _FLTSHELL_HXX
#define _FLTSHELL_HXX

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <hintids.hxx>
#include <vcl/keycod.hxx>
#include <tools/datetime.hxx>
#include <editeng/brkitem.hxx>
#include <poolfmt.hxx>
#include <fmtornt.hxx>
#include <ndindex.hxx>
#include <pam.hxx>
#include <IDocumentRedlineAccess.hxx>

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_deque.hpp>

class SwTOXBase;
class SwFltShell;
class SwField;
class SwFieldType;
class Graphic;
class SwTableBox;
class SwDoc;
class SwPaM;

using namespace com::sun::star;

inline void SwFltClearFlag(sal_uLong& rFieldFlags, int no)
    { rFieldFlags &= ~(1L << no); }

inline void SwFltSetFlag(sal_uLong& rFieldFlags, int no)
    { rFieldFlags |= 1L << no; }

inline sal_Bool SwFltGetFlag(sal_uLong nFieldFlags, int no)
    { return (nFieldFlags & (1L << no)) != 0; }

//Subvert the Node/Content system to get positions which don't update as
//content is appended to them
struct SW_DLLPUBLIC SwFltPosition
{
public:
    SwNodeIndex m_nNode;
    xub_StrLen m_nCntnt;
public:
    SwFltPosition(const SwFltPosition &rOther)
        : m_nNode(rOther.m_nNode)
        , m_nCntnt(rOther.m_nCntnt)
    {
    }
    SwFltPosition &operator=(const SwFltPosition &rOther)
    {
        m_nNode = rOther.m_nNode;
        m_nCntnt = rOther.m_nCntnt;
        return *this;
    }
    bool operator==(const SwFltPosition &rOther) const
    {
        return (m_nCntnt == rOther.m_nCntnt &&
                m_nNode == rOther.m_nNode);
    }
    void SetPos(SwNodeIndex &rNode, sal_uInt16 nIdx)
    {
        m_nNode = rNode;
        m_nCntnt = nIdx;
    }
    //operators with SwPosition, where the node is hacked to the previous one,
    //and the offset to content is de-dynamic-ified
    SwFltPosition(const SwPosition &rPos)
        : m_nNode(rPos.nNode, -1)
        , m_nCntnt(rPos.nContent.GetIndex())
    {
    }
    void SetPos(const SwPosition &rPos)
    {
        m_nNode = rPos.nNode.GetIndex()-1;
        m_nCntnt = rPos.nContent.GetIndex();
    }
};

// Stack-Eintrag fuer die Attribute Es werden immer Pointer auf neue Attribute uebergeben.
class SwFltStackEntry : private ::boost::noncopyable
{
public:
    SwFltPosition m_aMkPos;
    SwFltPosition m_aPtPos;

    SfxPoolItem * pAttr;// Format Attribute

    sal_Bool bOld;          // to mark Attributes *before* skipping field results
    sal_Bool bOpen;     //Entry open, awaiting being closed
    sal_Bool bConsumedByField;

    SW_DLLPUBLIC SwFltStackEntry(const SwPosition & rStartPos, SfxPoolItem* pHt );
    SW_DLLPUBLIC ~SwFltStackEntry();

    void SetStartPos(const SwPosition & rStartPos);
    SW_DLLPUBLIC void SetEndPos(  const SwPosition & rEndPos);
    SW_DLLPUBLIC bool MakeRegion(SwDoc* pDoc, SwPaM& rRegion, bool bCheck) const;
    SW_DLLPUBLIC static bool MakeRegion(SwDoc* pDoc, SwPaM& rRegion,
        bool bCheck, const SwFltPosition &rMkPos, const SwFltPosition &rPtPos,
        sal_uInt16 nWhich=0);
};

class SW_DLLPUBLIC SwFltControlStack : private ::boost::noncopyable
{
    typedef boost::ptr_deque<SwFltStackEntry> Entries;
    typedef Entries::iterator myEIter;
    Entries maEntries;
    friend class SwFltShell;

    sal_uLong nFieldFlags;
    KeyCode aEmptyKeyCode; // fuer Bookmarks

protected:
    SwDoc* pDoc;
    sal_Bool bIsEndStack;

    void MoveAttrs( const SwPosition&  rPos );
    virtual void SetAttrInDoc(const SwPosition& rTmpPos, SwFltStackEntry& rEntry);

public:
    enum Flags
    {
        HYPO,
        TAGS_DO_ID,
        TAGS_VISIBLE,
        BOOK_TO_VAR_REF,
        BOOK_AND_REF,
        TAGS_IN_TEXT,
        ALLOW_FLD_CR,
        NO_FLD_CR,
        DONT_HARD_PROTECT
    };

    SwFltControlStack(SwDoc* pDo, sal_uLong nFieldFl);
    virtual ~SwFltControlStack();

    sal_Bool IsFlagSet(Flags no) const  { return ::SwFltGetFlag(nFieldFlags, no);}

    void NewAttr(const SwPosition& rPos, const SfxPoolItem & rAttr );

    virtual SwFltStackEntry* SetAttr(const SwPosition& rPos, sal_uInt16 nAttrId=0, sal_Bool bTstEnde=sal_True, long nHand = LONG_MAX, sal_Bool consumedByField=sal_False);

    void StealAttr(const SwNodeIndex& rNode, sal_uInt16 nAttrId = 0);
    void MarkAllAttrsOld();
    void KillUnlockedAttrs(const SwPosition& pPos);
    SfxPoolItem* GetFmtStackAttr(sal_uInt16 nWhich, sal_uInt16 * pPos = 0);
    const SfxPoolItem* GetFmtAttr(const SwPosition& rPos, sal_uInt16 nWhich);
    void Delete(const SwPaM &rPam);

    bool empty() const { return maEntries.empty(); }
    Entries::size_type size() const { return maEntries.size(); }
    SwFltStackEntry& operator[](Entries::size_type nIndex)
         { return maEntries[nIndex]; }
    void DeleteAndDestroy(Entries::size_type nCnt);
};

class SwFltAnchorClient;

class SW_DLLPUBLIC SwFltAnchor : public SfxPoolItem
{
    SwFrmFmt* pFrmFmt;
    SwFltAnchorClient * pClient;

public:
    SwFltAnchor(SwFrmFmt* pFlyFmt);
    SwFltAnchor(const SwFltAnchor&);
    virtual ~SwFltAnchor();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int operator==(const SfxPoolItem&) const;
    virtual SfxPoolItem* Clone(SfxItemPool* = 0) const;
    void SetFrmFmt(SwFrmFmt * _pFrmFmt);
    const SwFrmFmt* GetFrmFmt() const;
          SwFrmFmt* GetFrmFmt();
};

class SwFltAnchorClient : public SwClient
{
    SwFltAnchor * m_pFltAnchor;

public:
    SwFltAnchorClient(SwFltAnchor * pFltAnchor);

    virtual void Modify (const SfxPoolItem *pOld, const SfxPoolItem *pNew);
};


class SW_DLLPUBLIC SwFltRedline : public SfxPoolItem
{
public:
    DateTime        aStamp;
    DateTime        aStampPrev;
    RedlineType_t   eType;
    RedlineType_t   eTypePrev;
    sal_uInt16          nAutorNo;
    sal_uInt16          nAutorNoPrev;

    SwFltRedline(RedlineType_t   eType_,
                 sal_uInt16          nAutorNo_,
                 const DateTime& rStamp_,
                 RedlineType_t   eTypePrev_    = nsRedlineType_t::REDLINE_INSERT,
                 sal_uInt16          nAutorNoPrev_ = USHRT_MAX,
                 const DateTime* pStampPrev_   = 0)
        : SfxPoolItem(RES_FLTR_REDLINE), aStamp(rStamp_),
        aStampPrev( DateTime::EMPTY ),
        eType(eType_),
        eTypePrev(eTypePrev_), nAutorNo(nAutorNo_), nAutorNoPrev(nAutorNoPrev_)
    {
            if( pStampPrev_ )
                aStampPrev = *pStampPrev_;
    }

    SwFltRedline(const SwFltRedline& rCpy):
        SfxPoolItem(RES_FLTR_REDLINE),
        aStamp(         rCpy.aStamp       ),
        aStampPrev(     rCpy.aStampPrev   ),
        eType(          rCpy.eType        ),
        eTypePrev(      rCpy.eTypePrev    ),
        nAutorNo(       rCpy.nAutorNo     ),
        nAutorNoPrev(   rCpy.nAutorNoPrev )
        {}
    // "pure virtual Methoden" vom SfxPoolItem
    virtual int operator==(const SfxPoolItem& rItem) const;
    virtual SfxPoolItem* Clone(SfxItemPool* = 0) const;
};

class SW_DLLPUBLIC SwFltBookmark : public SfxPoolItem
{
    friend class SwFltShell;    // darf aName und aVal uebersetzen
    long nHandle;
    String aName;
    String aVal;
    sal_Bool bOnlyRef;          // "FRAGE"-Feld, Ref/Seitenrf oder nichts
    sal_Bool bRef;
    sal_Bool bPgRef;
public:
    SwFltBookmark( const String& rNa, const String& rVa,
                    long nHand, sal_Bool bOnlyR );
    SwFltBookmark( const SwFltBookmark& );
    // "pure virtual Methoden" vom SfxPoolItem
    virtual int operator==(const SfxPoolItem&) const;
    virtual SfxPoolItem* Clone(SfxItemPool* = 0) const;
    const String& GetName() const       { return aName; }
    const String& GetValSys() const     { return aVal; }
    sal_Bool IsOnlyRef() const              { return bOnlyRef; }
    sal_Bool IsRef() const                  { return bRef; }
    void SetRef(sal_Bool b = sal_True)          { bRef = b; }
    sal_Bool IsPgRef() const                { return bPgRef; }
    void SetPgRef(sal_Bool b = sal_True)        { bPgRef = b; }
    long GetHandle() const              { return nHandle; }
};

class SW_DLLPUBLIC SwFltTOX : public SfxPoolItem
{
    SwTOXBase* pTOXBase;
    sal_uInt16 nCols;
    sal_Bool bHadBreakItem; // there was a break item BEFORE insertion of the TOX
    sal_Bool bHadPageDescItem; // ...
public:
    SwFltTOX(SwTOXBase* pBase, sal_uInt16 _nCols = 0);
    SwFltTOX(const SwFltTOX&);
    // "pure virtual Methoden" vom SfxPoolItem
    virtual int operator==(const SfxPoolItem&) const;
    virtual SfxPoolItem* Clone(SfxItemPool* = 0) const;
    SwTOXBase* GetBase()            { return pTOXBase; }
    sal_uInt16 GetCols() const          { return nCols; }
    void SetHadBreakItem(    sal_Bool bVal ) { bHadBreakItem    = bVal; }
    void SetHadPageDescItem( sal_Bool bVal ) { bHadPageDescItem = bVal; }
    sal_Bool HadBreakItem()    const { return bHadBreakItem; }
    sal_Bool HadPageDescItem() const { return bHadPageDescItem; }
};

class SwFltSection : public SfxPoolItem
{
    SwSectionData * m_pSection;

public:
    SwFltSection( SwSectionData *const pSect );
    SwFltSection( const SwFltSection& );
    // "pure virtual Methoden" vom SfxPoolItem
    virtual int operator==(const SfxPoolItem&) const;
    virtual SfxPoolItem* Clone(SfxItemPool* = 0) const;
    SwSectionData * GetSectionData()    { return m_pSection; }
};
// Der WWEndStack verhaelt sich wie der WWControlStck, nur dass die Attribute
// auf ihm bis ans Ende des Dokuments gehortet werden, falls auf sie noch
// zugegriffen werden muss (z.B. Book/RefMarks, Index u.s.w.)
class SwFltEndStack : public SwFltControlStack
{
public:
    SwFltEndStack(SwDoc* pDo, sal_uLong nFieldFl)
        :SwFltControlStack(pDo, nFieldFl)
    {
        bIsEndStack = sal_True;
    }
};

///////////////////////////////////////////////////////////////////////////////////////////
//
// hier beginnen die fuer ww1-filter neu erstellten klassen. diese
// sollen eine simple oberflaeche fuer die komplexen strukturen des
// writers speziell fuer filter sein. soll etwas in den writer gegeben
// werden, soll ein << reichen. hierfuer sind neue typen erzeugt
// worden. ausserdem soll moeglich sein das objekt nach vielen
// zustaenden der momentanen formatierung zu fragen, sodasz diese der
// filter nicht selbst verwalten musz.
//
// den anfang macht eine vorlagen-oberklasse, die einfachen umgang mit
// formatvorlagen ermoeglicht:
//

class SwFltOutBase
{
    SwDoc& rDoc;
protected:
    RndStdIds eFlyAnchor;
    sal_Bool bFlyAbsPos;

    SwDoc& GetDoc()                 { return rDoc; }
    SfxItemSet* NewFlyDefaults();
    SwFltOutBase(SwDoc& rDocu);
    virtual ~SwFltOutBase();
public:
    virtual SwFltOutBase& operator << (const SfxPoolItem& rItem) = 0;

    virtual const SfxPoolItem& GetAttr(sal_uInt16 nWhich) = 0;
    virtual const SfxPoolItem& GetNodeOrStyAttr(sal_uInt16 nWhich) = 0;

    virtual const SfxPoolItem& GetCellAttr(sal_uInt16 nWhich);
    virtual sal_Bool BeginTable();
    virtual void NextTableCell();
    virtual void NextTableRow();
    virtual void SetTableWidth(SwTwips nW);
    virtual void SetTableOrient(sal_Int16 eOri);
    virtual void SetCellWidth(SwTwips nWidth, sal_uInt16 nCell);
    virtual void SetCellHeight(SwTwips nH);
    virtual void SetCellBorder(const SvxBoxItem& rFmtBox, sal_uInt16 nCell);
    virtual void SetCellSpace(sal_uInt16 nSp);
    virtual void DeleteCell(sal_uInt16 nCell);
    virtual void EndTable();

    virtual sal_Bool IsInFly() = 0;
    virtual void SetFlyFrmAttr(const SfxPoolItem& rAttr) = 0;
    virtual const SfxPoolItem& GetFlyFrmAttr(sal_uInt16 nWhich) = 0;
    virtual sal_Bool BeginFly( RndStdIds eAnchor, sal_Bool bAbsolutePos,
                               const SfxItemSet* pMoreAttrs = 0 );
    virtual void SetFlyAnchor( RndStdIds eAnchor );
    virtual void EndFly();
};

class SwFltOutDoc : public SwFltOutBase
{
    SwFltControlStack& rStack;
    SwFltEndStack& rEndStack;
    SwPaM* pPaM;
    SwFrmFmt* pFly;
// table items:
    const SwTable* pTable;
    SwPosition* pTabSavedPos; // set when in table
    SwPosition* pFlySavedPos; // set when in fly
    SwTwips nTableWidth;
    sal_uInt16 usTableX;
    sal_uInt16 usTableY;
    sal_Bool bReadNoTbl;                // Keine Tabellen

    SwTableBox* GetBox(sal_uInt16 ny, sal_uInt16 nx = USHRT_MAX);
    sal_Bool SeekCell( short nRow, short nCol, sal_Bool bPam );
    void SplitTable();
public:
    SwFltOutDoc(SwDoc& rDocu, SwPaM* pP, SwFltControlStack& rStk,
        SwFltEndStack& rEStk)
    : SwFltOutBase(rDocu), rStack(rStk), rEndStack(rEStk), pPaM(pP),
          pFly(0), pTable(0), pTabSavedPos(0), pFlySavedPos(0),
          nTableWidth(0), bReadNoTbl(sal_False)
    {}

    void SetReadNoTable()           { bReadNoTbl = sal_True; }
    sal_Bool IsTableWidthSet() const    { return 0 != nTableWidth; }

    virtual SwFltOutBase& operator << (const SfxPoolItem& rItem);

    virtual const SfxPoolItem& GetAttr(sal_uInt16 nWhich);
    virtual const SfxPoolItem& GetNodeOrStyAttr(sal_uInt16 nWhich);

    sal_Bool IsInTable();
    virtual const SfxPoolItem& GetCellAttr(sal_uInt16 nWhich);
    virtual sal_Bool BeginTable();
    virtual void NextTableCell();
    virtual void NextTableRow();
    virtual void SetTableWidth(SwTwips nW);
    virtual void SetTableOrient(sal_Int16 eOri);
    virtual void SetCellWidth(SwTwips nWidth, sal_uInt16 nCell);
    virtual void SetCellHeight(SwTwips nH);
    virtual void SetCellBorder(const SvxBoxItem& rFmtBox, sal_uInt16 nCell);
    virtual void SetCellSpace(sal_uInt16 nSp);
    virtual void DeleteCell(sal_uInt16 nCell);
    virtual void EndTable();

    SwFrmFmt* MakeFly( RndStdIds eAnchor, SfxItemSet* pSet );
    virtual sal_Bool IsInFly();
    virtual void SetFlyFrmAttr(const SfxPoolItem& rAttr);
    virtual const SfxPoolItem& GetFlyFrmAttr(sal_uInt16 nWhich);
    virtual sal_Bool BeginFly( RndStdIds eAnchor, sal_Bool bAbsolutePos,
                               const SfxItemSet* pMoreAttrs = 0 );
    virtual void EndFly();
};

class SwFltFormatCollection : public SwFltOutBase
{
    SwTxtFmtColl* pColl;
    SfxItemSet* pFlyAttrs;      // Simulation der Flys in Styles
    sal_Bool bHasFly;
public:
    SwFltFormatCollection(SwDoc&, RES_POOL_COLLFMT_TYPE nType);
    SwFltFormatCollection(SwDoc&, const String& rName );
    ~SwFltFormatCollection() { if( pFlyAttrs ) delete pFlyAttrs; }

    void Reset()
    {
        // #i73790# - method renamed
        pColl->ResetAllFmtAttr();
        pColl->SetAuto(sal_False); // nach Empfehlung JP
    }
    void Derived(SwTxtFmtColl* pBase)
        { pColl->SetDerivedFrom(pBase); }

//  SwTxtFmtColl* Search(String, CharSet eSrc);
    SwTxtFmtColl* GetColl()         { return pColl; }
    void SetHasFly()                { bHasFly = sal_True; }
    SfxItemSet* GetpFlyAttrs()      { return pFlyAttrs; }

    virtual SwFltOutBase& operator << (const SfxPoolItem& rItem);
    virtual const SfxPoolItem& GetAttr(sal_uInt16 nWhich);
    virtual const SfxPoolItem& GetNodeOrStyAttr(sal_uInt16 nWhich);

    virtual sal_Bool IsInFly();
    virtual void SetFlyFrmAttr(const SfxPoolItem& rAttr);
    virtual const SfxPoolItem& GetFlyFrmAttr(sal_uInt16 nWhich);
    virtual sal_Bool BeginFly( RndStdIds eAnchor, sal_Bool bAbsolutePos,
                               const SfxItemSet* pMoreAttrs = 0 );
    sal_Bool BeginStyleFly( SwFltOutDoc* pOutDoc );
    virtual void EndFly();
};

//
// dies nun ist die zauberhafteklasse: intention: alle eins nach dem
// andern hinein'pipe'n. wird eine besondere struktur eingelesen, wird
// eine klammer geoeffnet (BeginXxx) und nach beendigung geschlossen
// (EndXxx), wobei Xxx zB fuer Fusznoten, Kopf/Fuszzeilen oder
// Tabellen steht. Styles funktionieren auch so, haben aber den
// unterschied, keine buchstaben zu akzeptieren.
// beginnt ein neuer absatz oder aehnliches, wird NextXxx genutzt.
// hier ist moeglich, Tab, Zeilenumbruch, Absatzende, Seitenumbruch
// und Sektionsende einzufuegen.
//
class SwFltShell
{
    SwFltOutDoc* pOutDoc;
    SwFltFormatCollection* pColls[256];
    SwFltOutBase* pOut;

//  SwFltFormatCollection* pFormat; // set when in style-mode
    SwPageDesc* pCurrentPageDesc;
    SwPosition* pSavedPos; // set, when in footnote or header/footer -mode
#ifdef None
#undef None
#endif
    enum SubModes {
        None,
        Header,
        Footer,
        Footnote,
        Table,
        Fly,
        Style,
        Max
    } eSubMode;

// Fly items:
    sal_uInt16 nAktStyle;               // zur Indizierung pStyleFlyTable
//
    SwFltControlStack aStack;
    SwFltEndStack aEndStack;
    SwPaM* pPaM;
//
    String sBaseURL;
    sal_uInt16 nPageDescOffset; // fuers update der pagedescs
    CharSet eSrcCharSet; // charset der quelle
    friend class SwFltControlStack;
    sal_Bool bNewDoc;
    sal_Bool bStdPD;
    sal_Bool bProtect;

public:
    SwFltShell(SwDoc* , SwPaM& , const String& rBaseURL, sal_Bool bNew, sal_uLong = 0);
    ~SwFltShell();

    SwDoc& GetDoc()                 { return *aStack.pDoc; }

    CharSet SetCharSet(CharSet eNew)    { CharSet eOld = eSrcCharSet;
                                          eSrcCharSet = eNew;
                                          return eOld;
                                        }
    void SetUseStdPageDesc()        { bStdPD = sal_True; }
    void SetProtect()               { bProtect = sal_True; }
    SwPageDesc* MakePageDesc(SwPageDesc* pFirstPageDesc = NULL);
    SwPageDesc& GetPageDesc()       { return *pCurrentPageDesc; }
    void NextTab()                  { (*this) << sal_uInt8(0x09); }
    void NextLine()                 { (*this) << sal_uInt8(0x0a); }
    void NextParagraph();
    void NextPage();
    void NextSection()      { pCurrentPageDesc = MakePageDesc(); }

    SwFltShell& AddGraphic( const String& rPicName );
    SwFltShell& AddError( const sal_Char* pErr );
    SwFltShell& EndItem( sal_uInt16 nId );
    SwFltShell& SetStyle( sal_uInt16 nStyle );

    SwFltShell& operator << ( Graphic& );
    SwFltShell& operator << ( SwFltBookmark& aBook );
    void SetBookEnd(long nHandle);
    SwFltShell& operator << ( const String& );  // Vorsicht: CHARSET_ANSI
    SwFltShell& operator << ( const sal_Unicode );
    SwFltShell& operator << ( const SwField& );
    SwFltShell& operator << ( const SfxPoolItem& rItem )
        { *pOut << rItem; return *this; }

//  SwFltShell& operator >> (SfxPoolItem&);
// methode zum beenden einer sub-sektion, zB Fusznote etc
    void End()                      { eSubMode = None; }
// methoden zur verwaltung von Header/Footer
    void BeginHeader(SwPageDesc* =NULL);
    void BeginFooter(SwPageDesc* =NULL);
    void EndHeaderFooter();
// methoden zur verwaltung von FootNotes
    void BeginFootnote();
    void EndFootnote();
// methoden zur verwaltung von Tabellen
    sal_Bool IsInTable() {
        return ( pOut == pOutDoc ) ? pOutDoc->IsInTable() : 0; }
    const SfxPoolItem& GetCellAttr(sal_uInt16 nWhich) {
        return pOut->GetCellAttr(nWhich); }
    sal_Bool BeginTable() {
        sal_Bool b = pOut->BeginTable();
        if(b) eSubMode = Table;
        return b; }
    void NextTableCell() {
        pOut->NextTableCell(); }
    void NextTableRow() {
        pOut->NextTableRow(); }
    void SetTableWidth(SwTwips nW) {
        pOut->SetTableWidth(nW); }
    sal_Bool IsTableWidthSet() {
        return pOutDoc->IsTableWidthSet(); }
    void SetTableOrient(sal_Int16 eOri) {
        pOut->SetTableOrient(eOri); }
    void SetCellWidth(SwTwips nWidth, sal_uInt16 nCell = USHRT_MAX ) {
        pOut->SetCellWidth(nWidth, nCell); }
    void SetCellHeight(SwTwips nH) {
        pOut->SetCellHeight(nH); }
    void SetCellBorder(const SvxBoxItem& rFmtBox, sal_uInt16 nCell = USHRT_MAX ){
        pOut->SetCellBorder(rFmtBox, nCell); }
    void SetCellSpace(sal_uInt16 nSp) {
        pOut->SetCellSpace(nSp); }
    void DeleteCell(sal_uInt16 nCell = USHRT_MAX) {
        pOut->DeleteCell(nCell); }
    void EndTable() {
        pOut->EndTable(); }
// methoden zur verwaltung von Flys
    sal_Bool IsInFly() { return pOut->IsInFly(); }
    sal_Bool BeginFly( RndStdIds eAnchor = FLY_AT_PARA, sal_Bool bAbsolutePos = sal_False );
    void SetFlyAnchor( RndStdIds eAnchor )
        { pOut->SetFlyAnchor( eAnchor ); }
    void SetFlyXPos( short nXPos, sal_Int16 eHRel = com::sun::star::text::RelOrientation::FRAME,
                     sal_Int16 eHAlign = com::sun::star::text::HoriOrientation::NONE );
    void SetFlyYPos( short nYPos, sal_Int16 eVRel = com::sun::star::text::RelOrientation::FRAME,
                     sal_Int16 eVAlign = com::sun::star::text::VertOrientation::NONE );
    void SetFlyFrmAttr(const SfxPoolItem& rAttr){
        pOut->SetFlyFrmAttr( rAttr ); }
    void EndFly();
// methoden zur verwaltung von styles:
    void BeginStyle(sal_uInt16 nUserCode, RES_POOL_COLLFMT_TYPE aType)
    {
        OSL_ENSURE(nUserCode<sizeof(pColls)/sizeof(*pColls), "code out of bounds");
        OSL_ENSURE(pColls[nUserCode] == NULL, "user codes dublicate");
        if (eSubMode == Style)
            EndStyle();
        pOut = pColls[nUserCode] = new SwFltFormatCollection(GetDoc(), aType);
        nAktStyle = nUserCode;
        eSubMode = Style;
    }
    void BeginStyle( sal_uInt16 nUserCode, const String& rName )
    {
        OSL_ENSURE(nUserCode<sizeof(pColls)/sizeof(*pColls), "code out of bounds");
        OSL_ENSURE(pColls[nUserCode] == NULL, "user codes dublicate");
        if (eSubMode == Style)
            EndStyle();
        pOut = pColls[nUserCode] = new SwFltFormatCollection(GetDoc(), rName );
        nAktStyle = nUserCode;
        eSubMode = Style;
    }
    sal_Bool IsStyleImported(sal_uInt16 nUserCode)
        { return pColls[nUserCode] != 0; }
    void BaseStyle(sal_uInt16 nBased)
    {
        OSL_ENSURE(eSubMode == Style, "wrong state for style");
        OSL_ENSURE(pColls[nBased], "Style based on noexistent style" );
        if( eSubMode == Style && pColls[nBased]->GetColl() )
            ((SwFltFormatCollection*)pOut)->Derived(pColls[nBased]->GetColl());
    }
    void NextStyle(sal_uInt16 nWhich, sal_uInt16 nNext);

    void EndStyle()
    {
//   OSL_ENSURE(eSubMode == Style, "wrong state for style");
        nAktStyle = 0;
        pOut = pOutDoc;
        eSubMode = None;
    }

    sal_Bool IsFlagSet(SwFltControlStack::Flags no) const
        { return aStack.IsFlagSet(no); }
    void ConvertUStr( String& rInOut );
    OUString QuoteStr( const OUString& rIn );

    // folgende status kann die shell verwalten:
    const SfxPoolItem& GetNodeOrStyAttr(sal_uInt16 nWhich);
    const SfxPoolItem& GetAttr(sal_uInt16 nWhich);
    const SfxPoolItem& GetFlyFrmAttr(sal_uInt16 nWhich);
    SwFieldType* GetSysFldType(sal_uInt16 eWhich);
    sal_Bool GetWeightBold();
    sal_Bool GetPostureItalic();
    sal_Bool GetCrossedOut();
    sal_Bool GetContour();
    sal_Bool GetCaseKapitaelchen();
    sal_Bool GetCaseVersalien();

    const String& GetBaseURL() const { return sBaseURL; }
};

SW_DLLPUBLIC void UpdatePageDescs(SwDoc &rDoc, sal_uInt16 nInPageDescOffset);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
