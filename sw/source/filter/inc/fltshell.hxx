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

#include <deque>

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
#include <IDocumentRedlineAccess.hxx>

class SwTOXBase;
class SwFltShell;
class SwField;
class SwFieldType;
class Graphic;
class SwTableBox;
class SwDoc;
class SwPaM;

using namespace com::sun::star;

inline void SwFltClearFlag(ULONG& rFieldFlags, int no)
    { rFieldFlags &= ~(1L << no); }

inline void SwFltSetFlag(ULONG& rFieldFlags, int no)
    { rFieldFlags |= 1L << no; }

inline BOOL SwFltGetFlag(ULONG nFieldFlags, int no)
    { return (nFieldFlags & (1L << no)) != 0; }

// Stack-Eintrag fuer die Attribute Es werden immer Pointer auf neue Attribute uebergeben.
class SwFltStackEntry
{
public:
    SwNodeIndex nMkNode;
    SwNodeIndex nPtNode;
    SfxPoolItem * pAttr;// Format Attribute
    long nHandle;       // fuer verschachtelte Attrs, z.B. Bookmarks
    xub_StrLen nMkCntnt;// Nachbildung von Mark()
    xub_StrLen nPtCntnt;// Nachbildung von GetPoint()

    BOOL bOld;          // to mark Attributes *before* skipping field results
    BOOL bLocked;
    BOOL bCopied;
    BOOL bConsumedByField;

    SW_DLLPUBLIC SwFltStackEntry(const SwPosition & rStartPos, SfxPoolItem* pHt );
    SW_DLLPUBLIC SwFltStackEntry(const SwFltStackEntry& rEntry);
    SW_DLLPUBLIC ~SwFltStackEntry();

    void SetStartPos(const SwPosition & rStartPos);
    SW_DLLPUBLIC void SetEndPos(  const SwPosition & rEndPos);
    SW_DLLPUBLIC BOOL MakeRegion(SwDoc* pDoc, SwPaM& rRegion, BOOL bCheck );
};

class SW_DLLPUBLIC SwFltControlStack
{
    typedef std::deque<SwFltStackEntry*> Entries;
    typedef Entries::iterator myEIter;
    Entries maEntries;
    friend class SwFltShell;

    ULONG nFieldFlags;
    KeyCode aEmptyKeyCode; // fuer Bookmarks

protected:
    SwDoc* pDoc;
    BOOL bIsEndStack;

    void MoveAttrs( const SwPosition&  rPos );
    virtual void SetAttrInDoc(const SwPosition& rTmpPos, SwFltStackEntry* pEntry);

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

    SwFltControlStack(SwDoc* pDo, ULONG nFieldFl);
    virtual ~SwFltControlStack();

    BOOL IsFlagSet(Flags no) const  { return ::SwFltGetFlag(nFieldFlags, no);}

    void NewAttr(const SwPosition& rPos, const SfxPoolItem & rAttr );

    virtual void SetAttr(const SwPosition& rPos, USHORT nAttrId=0, BOOL bTstEnde=TRUE, long nHand = LONG_MAX, BOOL consumedByField=FALSE);

    void StealAttr(const SwPosition* pPos, USHORT nAttrId = 0);
    void MarkAllAttrsOld();
    void KillUnlockedAttrs(const SwPosition& pPos);
    SfxPoolItem* GetFmtStackAttr(USHORT nWhich, USHORT * pPos = 0);
    const SfxPoolItem* GetFmtAttr(const SwPosition& rPos, USHORT nWhich);
    void Delete(const SwPaM &rPam);

    Entries::size_type Count() { return maEntries.size(); }
    SwFltStackEntry* operator[](Entries::size_type nIndex)
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

    virtual void Modify (SfxPoolItem *pOld, SfxPoolItem *pNew);
};


class SW_DLLPUBLIC SwFltRedline : public SfxPoolItem
{
public:
    DateTime        aStamp;
    DateTime        aStampPrev;
    RedlineType_t   eType;
    RedlineType_t   eTypePrev;
    USHORT          nAutorNo;
    USHORT          nAutorNoPrev;

    SwFltRedline(RedlineType_t   eType_,
                 USHORT          nAutorNo_,
                 const DateTime& rStamp_,
                 RedlineType_t   eTypePrev_    = nsRedlineType_t::REDLINE_INSERT,
                 USHORT          nAutorNoPrev_ = USHRT_MAX,
                 const DateTime* pStampPrev_   = 0)
        : SfxPoolItem(RES_FLTR_REDLINE), aStamp(rStamp_), eType(eType_),
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
    BOOL bOnlyRef;          // "FRAGE"-Feld, Ref/Seitenrf oder nichts
    BOOL bRef;
    BOOL bPgRef;
public:
    SwFltBookmark( const String& rNa, const String& rVa,
                    long nHand, BOOL bOnlyR );
    SwFltBookmark( const SwFltBookmark& );
    // "pure virtual Methoden" vom SfxPoolItem
    virtual int operator==(const SfxPoolItem&) const;
    virtual SfxPoolItem* Clone(SfxItemPool* = 0) const;
    const String& GetName() const       { return aName; }
    const String& GetValSys() const     { return aVal; }
    BOOL IsOnlyRef() const              { return bOnlyRef; }
    BOOL IsRef() const                  { return bRef; }
    void SetRef(BOOL b = TRUE)          { bRef = b; }
    BOOL IsPgRef() const                { return bPgRef; }
    void SetPgRef(BOOL b = TRUE)        { bPgRef = b; }
    long GetHandle() const              { return nHandle; }
};

class SW_DLLPUBLIC SwFltTOX : public SfxPoolItem
{
    SwTOXBase* pTOXBase;
    USHORT nCols;
    BOOL bHadBreakItem; // there was a break item BEFORE insertion of the TOX
    BOOL bHadPageDescItem; // ...
public:
    SwFltTOX(SwTOXBase* pBase, USHORT _nCols = 0);
    SwFltTOX(const SwFltTOX&);
    // "pure virtual Methoden" vom SfxPoolItem
    virtual int operator==(const SfxPoolItem&) const;
    virtual SfxPoolItem* Clone(SfxItemPool* = 0) const;
    SwTOXBase* GetBase()            { return pTOXBase; }
    USHORT GetCols() const          { return nCols; }
    void SetHadBreakItem(    BOOL bVal ) { bHadBreakItem    = bVal; }
    void SetHadPageDescItem( BOOL bVal ) { bHadPageDescItem = bVal; }
    BOOL HadBreakItem()    const { return bHadBreakItem; }
    BOOL HadPageDescItem() const { return bHadPageDescItem; }
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
    SwFltEndStack(SwDoc* pDo, ULONG nFieldFl)
        :SwFltControlStack(pDo, nFieldFl)
    {
        bIsEndStack = TRUE;
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
    BOOL bFlyAbsPos;

    SwDoc& GetDoc()                 { return rDoc; }
    SfxItemSet* NewFlyDefaults();
    SwFltOutBase(SwDoc& rDocu);
    virtual ~SwFltOutBase();
public:
    virtual SwFltOutBase& operator << (const SfxPoolItem& rItem) = 0;

    virtual const SfxPoolItem& GetAttr(USHORT nWhich) = 0;
    virtual const SfxPoolItem& GetNodeOrStyAttr(USHORT nWhich) = 0;

    virtual const SfxPoolItem& GetCellAttr(USHORT nWhich);
    virtual BOOL BeginTable();
    virtual void NextTableCell();
    virtual void NextTableRow();
    virtual void SetTableWidth(SwTwips nW);
    virtual void SetTableOrient(sal_Int16 eOri);
    virtual void SetCellWidth(SwTwips nWidth, USHORT nCell);
    virtual void SetCellHeight(SwTwips nH);
    virtual void SetCellBorder(const SvxBoxItem& rFmtBox, USHORT nCell);
    virtual void SetCellSpace(USHORT nSp);
    virtual void DeleteCell(USHORT nCell);
    virtual void EndTable();

    virtual BOOL IsInFly() = 0;
    virtual void SetFlyFrmAttr(const SfxPoolItem& rAttr) = 0;
    virtual const SfxPoolItem& GetFlyFrmAttr(USHORT nWhich) = 0;
    virtual BOOL BeginFly( RndStdIds eAnchor, BOOL bAbsolutePos,
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
    USHORT usTableX;
    USHORT usTableY;
    BOOL bReadNoTbl;                // Keine Tabellen

    SwTableBox* GetBox(USHORT ny, USHORT nx = USHRT_MAX);
    BOOL SeekCell( short nRow, short nCol, BOOL bPam );
    void SplitTable();
public:
    SwFltOutDoc(SwDoc& rDocu, SwPaM* pP, SwFltControlStack& rStk,
        SwFltEndStack& rEStk)
    : SwFltOutBase(rDocu), rStack(rStk), rEndStack(rEStk), pPaM(pP),
          pFly(0), pTable(0), pTabSavedPos(0), pFlySavedPos(0),
          nTableWidth(0), bReadNoTbl(FALSE)
    {}

    void SetReadNoTable()           { bReadNoTbl = TRUE; }
    BOOL IsTableWidthSet() const    { return 0 != nTableWidth; }

    virtual SwFltOutBase& operator << (const SfxPoolItem& rItem);

    virtual const SfxPoolItem& GetAttr(USHORT nWhich);
    virtual const SfxPoolItem& GetNodeOrStyAttr(USHORT nWhich);

    BOOL IsInTable();
    virtual const SfxPoolItem& GetCellAttr(USHORT nWhich);
    virtual BOOL BeginTable();
    virtual void NextTableCell();
    virtual void NextTableRow();
    virtual void SetTableWidth(SwTwips nW);
    virtual void SetTableOrient(sal_Int16 eOri);
    virtual void SetCellWidth(SwTwips nWidth, USHORT nCell);
    virtual void SetCellHeight(SwTwips nH);
    virtual void SetCellBorder(const SvxBoxItem& rFmtBox, USHORT nCell);
    virtual void SetCellSpace(USHORT nSp);
    virtual void DeleteCell(USHORT nCell);
    virtual void EndTable();

    SwFrmFmt* MakeFly( RndStdIds eAnchor, SfxItemSet* pSet );
    virtual BOOL IsInFly();
    virtual void SetFlyFrmAttr(const SfxPoolItem& rAttr);
    virtual const SfxPoolItem& GetFlyFrmAttr(USHORT nWhich);
    virtual BOOL BeginFly( RndStdIds eAnchor, BOOL bAbsolutePos,
                               const SfxItemSet* pMoreAttrs = 0 );
    virtual void EndFly();
};

class SwFltFormatCollection : public SwFltOutBase
{
    SwTxtFmtColl* pColl;
    SfxItemSet* pFlyAttrs;      // Simulation der Flys in Styles
    BOOL bHasFly;
public:
    SwFltFormatCollection(SwDoc&, RES_POOL_COLLFMT_TYPE nType);
    SwFltFormatCollection(SwDoc&, const String& rName );
    ~SwFltFormatCollection() { if( pFlyAttrs ) delete pFlyAttrs; }

    void Reset()
    {
        // --> OD 2007-01-25 #i73790# - method renamed
        pColl->ResetAllFmtAttr();
        // <--
        pColl->SetAuto(FALSE); // nach Empfehlung JP
    }
    void Derived(SwTxtFmtColl* pBase)
        { pColl->SetDerivedFrom(pBase); }

//  SwTxtFmtColl* Search(String, CharSet eSrc);
    SwTxtFmtColl* GetColl()         { return pColl; }
    void SetHasFly()                { bHasFly = TRUE; }
    SfxItemSet* GetpFlyAttrs()      { return pFlyAttrs; }

    virtual SwFltOutBase& operator << (const SfxPoolItem& rItem);
    virtual const SfxPoolItem& GetAttr(USHORT nWhich);
    virtual const SfxPoolItem& GetNodeOrStyAttr(USHORT nWhich);

    virtual BOOL IsInFly();
    virtual void SetFlyFrmAttr(const SfxPoolItem& rAttr);
    virtual const SfxPoolItem& GetFlyFrmAttr(USHORT nWhich);
    virtual BOOL BeginFly( RndStdIds eAnchor, BOOL bAbsolutePos,
                               const SfxItemSet* pMoreAttrs = 0 );
    BOOL BeginStyleFly( SwFltOutDoc* pOutDoc );
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
    USHORT nAktStyle;               // zur Indizierung pStyleFlyTable
//
    SwFltControlStack aStack;
    SwFltEndStack aEndStack;
    SwPaM* pPaM;
//
    String sBaseURL;
    USHORT nPageDescOffset; // fuers update der pagedescs
    CharSet eSrcCharSet; // charset der quelle
    friend class SwFltControlStack;
    BOOL bNewDoc;
    BOOL bStdPD;
    BOOL bProtect;

public:
    SwFltShell(SwDoc* , SwPaM& , const String& rBaseURL, BOOL bNew, ULONG = 0);
    ~SwFltShell();

    SwDoc& GetDoc()                 { return *aStack.pDoc; }

    CharSet SetCharSet(CharSet eNew)    { CharSet eOld = eSrcCharSet;
                                          eSrcCharSet = eNew;
                                          return eOld;
                                        }
    void SetUseStdPageDesc()        { bStdPD = TRUE; }
    void SetProtect()               { bProtect = TRUE; }
    SwPageDesc* MakePageDesc(SwPageDesc* pFirstPageDesc = NULL);
    SwPageDesc& GetPageDesc()       { return *pCurrentPageDesc; }
    void NextTab()                  { (*this) << BYTE(0x09); }
    void NextLine()                 { (*this) << BYTE(0x0a); }
    void NextParagraph();
    void NextPage();
    void NextSection()      { pCurrentPageDesc = MakePageDesc(); }

    SwFltShell& AddGraphic( const String& rPicName );
    SwFltShell& AddError( const sal_Char* pErr );
    SwFltShell& EndItem( USHORT nId );
    SwFltShell& SetStyle( USHORT nStyle );

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
    BOOL IsInTable() {
        return ( pOut == pOutDoc ) ? pOutDoc->IsInTable() : 0; }
    const SfxPoolItem& GetCellAttr(USHORT nWhich) {
        return pOut->GetCellAttr(nWhich); }
    BOOL BeginTable() {
        BOOL b = pOut->BeginTable();
        if(b) eSubMode = Table;
        return b; }
    void NextTableCell() {
        pOut->NextTableCell(); }
    void NextTableRow() {
        pOut->NextTableRow(); }
    void SetTableWidth(SwTwips nW) {
        pOut->SetTableWidth(nW); }
    BOOL IsTableWidthSet() {
        return pOutDoc->IsTableWidthSet(); }
    void SetTableOrient(sal_Int16 eOri) {
        pOut->SetTableOrient(eOri); }
    void SetCellWidth(SwTwips nWidth, USHORT nCell = USHRT_MAX ) {
        pOut->SetCellWidth(nWidth, nCell); }
    void SetCellHeight(SwTwips nH) {
        pOut->SetCellHeight(nH); }
    void SetCellBorder(const SvxBoxItem& rFmtBox, USHORT nCell = USHRT_MAX ){
        pOut->SetCellBorder(rFmtBox, nCell); }
    void SetCellSpace(USHORT nSp) {
        pOut->SetCellSpace(nSp); }
    void DeleteCell(USHORT nCell = USHRT_MAX) {
        pOut->DeleteCell(nCell); }
    void EndTable() {
        pOut->EndTable(); }
// methoden zur verwaltung von Flys
    BOOL IsInFly() { return pOut->IsInFly(); }
    BOOL BeginFly( RndStdIds eAnchor = FLY_AT_PARA, BOOL bAbsolutePos = FALSE );
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
    void BeginStyle(USHORT nUserCode, RES_POOL_COLLFMT_TYPE aType)
    {
        OSL_ENSURE(nUserCode<sizeof(pColls)/sizeof(*pColls), "code out of bounds");
        OSL_ENSURE(pColls[nUserCode] == NULL, "user codes dublicate");
        if (eSubMode == Style)
            EndStyle();
        pOut = pColls[nUserCode] = new SwFltFormatCollection(GetDoc(), aType);
        nAktStyle = nUserCode;
        eSubMode = Style;
    }
    void BeginStyle( USHORT nUserCode, const String& rName )
    {
        OSL_ENSURE(nUserCode<sizeof(pColls)/sizeof(*pColls), "code out of bounds");
        OSL_ENSURE(pColls[nUserCode] == NULL, "user codes dublicate");
        if (eSubMode == Style)
            EndStyle();
        pOut = pColls[nUserCode] = new SwFltFormatCollection(GetDoc(), rName );
        nAktStyle = nUserCode;
        eSubMode = Style;
    }
    BOOL IsStyleImported(USHORT nUserCode)
        { return pColls[nUserCode] != 0; }
    void BaseStyle(USHORT nBased)
    {
        OSL_ENSURE(eSubMode == Style, "wrong state for style");
        OSL_ENSURE(pColls[nBased], "Style based on noexistent style" );
        if( eSubMode == Style && pColls[nBased]->GetColl() )
            ((SwFltFormatCollection*)pOut)->Derived(pColls[nBased]->GetColl());
    }
    void NextStyle(USHORT nWhich, USHORT nNext);

    void EndStyle()
    {
//   OSL_ENSURE(eSubMode == Style, "wrong state for style");
        nAktStyle = 0;
        pOut = pOutDoc;
        eSubMode = None;
    }

    BOOL IsFlagSet(SwFltControlStack::Flags no) const
        { return aStack.IsFlagSet(no); }
    void ConvertUStr( String& rInOut );
    String QuoteStr( const String& rIn );

    // folgende status kann die shell verwalten:
    const SfxPoolItem& GetNodeOrStyAttr(USHORT nWhich);
    const SfxPoolItem& GetAttr(USHORT nWhich);
    const SfxPoolItem& GetFlyFrmAttr(USHORT nWhich);
    SwFieldType* GetSysFldType(USHORT eWhich);
    BOOL GetWeightBold();
    BOOL GetPostureItalic();
    BOOL GetCrossedOut();
    BOOL GetContour();
    BOOL GetCaseKapitaelchen();
    BOOL GetCaseVersalien();

    const String& GetBaseURL() const { return sBaseURL; }
};

SW_DLLPUBLIC void UpdatePageDescs(SwDoc &rDoc, sal_uInt16 nInPageDescOffset);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
