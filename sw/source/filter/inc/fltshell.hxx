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
#ifndef INCLUDED_SW_SOURCE_FILTER_INC_FLTSHELL_HXX
#define INCLUDED_SW_SOURCE_FILTER_INC_FLTSHELL_HXX

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <hintids.hxx>
#include <vcl/keycod.hxx>
#include <tools/datetime.hxx>
#include <editeng/formatbreakitem.hxx>
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

inline void SwFltClearFlag(sal_uLong& rFieldFlags, int no)
    { rFieldFlags &= ~(1L << no); }

inline void SwFltSetFlag(sal_uLong& rFieldFlags, int no)
    { rFieldFlags |= 1L << no; }

inline bool SwFltGetFlag(sal_uLong nFieldFlags, int no)
    { return (nFieldFlags & (1L << no)) != 0; }

//Subvert the Node/Content system to get positions which don't update as
//content is appended to them
struct SW_DLLPUBLIC SwFltPosition
{
public:
    SwNodeIndex m_nNode;
    sal_Int32 m_nCntnt;
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

    bool bOld;          // to mark Attributes *before* skipping field results
    bool bOpen;     //Entry open, awaiting being closed
    bool bConsumedByField;

    sal_Int32 mnStartCP;
    sal_Int32 mnEndCP;
    bool bIsParaEnd;

    SW_DLLPUBLIC SwFltStackEntry(const SwPosition & rStartPos, SfxPoolItem* pHt );
    SW_DLLPUBLIC ~SwFltStackEntry();

    void SetStartPos(const SwPosition & rStartPos);
    SW_DLLPUBLIC void SetEndPos(  const SwPosition & rEndPos);
    SW_DLLPUBLIC bool MakeRegion(SwDoc* pDoc, SwPaM& rRegion, bool bCheck) const;
    SW_DLLPUBLIC static bool MakeRegion(SwDoc* pDoc, SwPaM& rRegion,
        bool bCheck, const SwFltPosition &rMkPos, const SwFltPosition &rPtPos, bool bIsParaEnd=false,
        sal_uInt16 nWhich=0);

    void SetStartCP(sal_Int32 nCP) {mnStartCP = nCP;}
    void SetEndCP(sal_Int32 nCP) {mnEndCP = nCP;}
    sal_Int32 GetStartCP() const {return mnStartCP;}
    sal_Int32 GetEndCP() const {return mnEndCP;}
    bool IsAbleMakeRegion();
    bool IsParaEnd(){ return bIsParaEnd;}
    void SetIsParaEnd(bool bArg){ bIsParaEnd = bArg;}
};

class SW_DLLPUBLIC SwFltControlStack : private ::boost::noncopyable
{
    typedef boost::ptr_deque<SwFltStackEntry> Entries;
    typedef Entries::iterator myEIter;
    Entries maEntries;
    friend class SwFltShell;

    sal_uLong nFieldFlags;
    KeyCode aEmptyKeyCode; // fuer Bookmarks

private:
    bool bHasSdOD;
    bool bSdODChecked;

protected:
    SwDoc* pDoc;
    bool bIsEndStack;

    void MoveAttrs( const SwPosition&  rPos );
    virtual void SetAttrInDoc(const SwPosition& rTmpPos, SwFltStackEntry& rEntry);
    virtual sal_Int32 GetCurrAttrCP() const {return -1;}
    virtual bool IsParaEndInCPs(sal_Int32 nStart,sal_Int32 nEnd,bool bSdOD=true) const;

    //Clear the para end position recorded in reader intermittently for the least impact on loading performance
    virtual void ClearParaEndPosition(){};
    virtual bool CheckSdOD(sal_Int32 nStart,sal_Int32 nEnd);
    bool HasSdOD();

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

    bool IsFlagSet(Flags no) const  { return ::SwFltGetFlag(nFieldFlags, no);}

    void NewAttr(const SwPosition& rPos, const SfxPoolItem & rAttr );

    virtual SwFltStackEntry* SetAttr(const SwPosition& rPos, sal_uInt16 nAttrId=0, bool bTstEnde=true, long nHand = LONG_MAX, bool consumedByField=false);

    void StealAttr(const SwNodeIndex& rNode, sal_uInt16 nAttrId = 0);
    void MarkAllAttrsOld();
    void KillUnlockedAttrs(const SwPosition& pPos);
    SfxPoolItem* GetFmtStackAttr(sal_uInt16 nWhich, sal_uInt16 * pPos = 0);
    const SfxPoolItem* GetOpenStackAttr(const SwPosition& rPos, sal_uInt16 nWhich);
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
    virtual bool operator==(const SfxPoolItem&) const SAL_OVERRIDE;
    virtual SfxPoolItem* Clone(SfxItemPool* = 0) const SAL_OVERRIDE;
    void SetFrmFmt(SwFrmFmt * _pFrmFmt);
    const SwFrmFmt* GetFrmFmt() const { return pFrmFmt;}
          SwFrmFmt* GetFrmFmt() { return pFrmFmt;}
};

class SwFltAnchorClient : public SwClient
{
    SwFltAnchor * m_pFltAnchor;

public:
    SwFltAnchorClient(SwFltAnchor * pFltAnchor);

    virtual void Modify (const SfxPoolItem *pOld, const SfxPoolItem *pNew) SAL_OVERRIDE;
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
    virtual bool operator==(const SfxPoolItem& rItem) const SAL_OVERRIDE;
    virtual SfxPoolItem* Clone(SfxItemPool* = 0) const SAL_OVERRIDE;
};

class SW_DLLPUBLIC SwFltBookmark : public SfxPoolItem
{
private:
    friend class SwFltShell;    // darf aName und aVal uebersetzen

    long mnHandle;
    OUString maName;
    OUString maVal;
    bool mbIsTOCBookmark;

public:
    SwFltBookmark( const OUString& rNa,
                   const OUString& rVa,
                   long nHand,
                   const bool bIsTOCBookmark = false );
    SwFltBookmark( const SwFltBookmark& );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual bool operator==(const SfxPoolItem&) const SAL_OVERRIDE;
    virtual SfxPoolItem* Clone(SfxItemPool* = 0) const SAL_OVERRIDE;

    long GetHandle() const              { return mnHandle; }
    const OUString& GetName() const       { return maName; }
    const OUString& GetValSys() const     { return maVal; }
    bool IsTOCBookmark() const
    {
        return mbIsTOCBookmark;
    }
};

class SW_DLLPUBLIC SwFltTOX : public SfxPoolItem
{
    SwTOXBase* pTOXBase;
    sal_uInt16 nCols;
    bool bHadBreakItem; // there was a break item BEFORE insertion of the TOX
    bool bHadPageDescItem;
public:
    SwFltTOX(SwTOXBase* pBase, sal_uInt16 _nCols = 0);
    SwFltTOX(const SwFltTOX&);
    // "pure virtual Methoden" vom SfxPoolItem
    virtual bool operator==(const SfxPoolItem&) const SAL_OVERRIDE;
    virtual SfxPoolItem* Clone(SfxItemPool* = 0) const SAL_OVERRIDE;
    SwTOXBase* GetBase()            { return pTOXBase; }
    sal_uInt16 GetCols() const          { return nCols; }
    void SetHadBreakItem(    bool bVal ) { bHadBreakItem    = bVal; }
    void SetHadPageDescItem( bool bVal ) { bHadPageDescItem = bVal; }
    bool HadBreakItem()    const { return bHadBreakItem; }
    bool HadPageDescItem() const { return bHadPageDescItem; }
};

class SwFltSection : public SfxPoolItem
{
    SwSectionData * m_pSection;

public:
    SwFltSection( SwSectionData *const pSect );
    SwFltSection( const SwFltSection& );
    // "pure virtual Methoden" vom SfxPoolItem
    virtual bool operator==(const SfxPoolItem&) const SAL_OVERRIDE;
    virtual SfxPoolItem* Clone(SfxItemPool* = 0) const SAL_OVERRIDE;
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
        bIsEndStack = true;
    }
};

// hier beginnen die fuer ww1-filter neu erstellten klassen. diese
// sollen eine simple oberflaeche fuer die komplexen strukturen des
// writers speziell fuer filter sein. soll etwas in den writer gegeben
// werden, soll ein << reichen. hierfuer sind neue typen erzeugt
// worden. ausserdem soll moeglich sein das objekt nach vielen
// zustaenden der momentanen formatierung zu fragen, sodasz diese der
// filter nicht selbst verwalten musz.

// den anfang macht eine vorlagen-oberklasse, die einfachen umgang mit
// formatvorlagen ermoeglicht:

class SwFltOutBase
{
    SwDoc& rDoc;
protected:
    RndStdIds eFlyAnchor;
    bool bFlyAbsPos;

    SwDoc& GetDoc()                 { return rDoc; }
    SfxItemSet* NewFlyDefaults();
    SwFltOutBase(SwDoc& rDocu);
    virtual ~SwFltOutBase();
public:
    virtual SwFltOutBase& operator << (const SfxPoolItem& rItem) = 0;

    virtual const SfxPoolItem& GetAttr(sal_uInt16 nWhich) = 0;
    virtual const SfxPoolItem& GetNodeOrStyAttr(sal_uInt16 nWhich) = 0;

    virtual const SfxPoolItem& GetCellAttr(sal_uInt16 nWhich);
    virtual bool BeginTable();
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

    virtual bool IsInFly() = 0;
    virtual void SetFlyFrmAttr(const SfxPoolItem& rAttr) = 0;
    virtual const SfxPoolItem& GetFlyFrmAttr(sal_uInt16 nWhich) = 0;
    virtual bool BeginFly( RndStdIds eAnchor, bool bAbsolutePos,
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
    bool bReadNoTbl;                // Keine Tabellen

    SwTableBox* GetBox(sal_uInt16 ny, sal_uInt16 nx = USHRT_MAX);
    bool SeekCell( short nRow, short nCol, bool bPam );
    void SplitTable();
public:
    SwFltOutDoc(SwDoc& rDocu, SwPaM* pP, SwFltControlStack& rStk,
        SwFltEndStack& rEStk)
        : SwFltOutBase(rDocu)
        , rStack(rStk)
        , rEndStack(rEStk)
        , pPaM(pP)
        , pFly(0)
        , pTable(0)
        , pTabSavedPos(0)
        , pFlySavedPos(0)
        , nTableWidth(0)
        , usTableX(0)
        , usTableY(0)
        , bReadNoTbl(false)
    {
    }

    void SetReadNoTable()           { bReadNoTbl = true; }
    bool IsTableWidthSet() const    { return 0 != nTableWidth; }

    virtual SwFltOutBase& operator << (const SfxPoolItem& rItem) SAL_OVERRIDE;

    virtual const SfxPoolItem& GetAttr(sal_uInt16 nWhich) SAL_OVERRIDE;
    virtual const SfxPoolItem& GetNodeOrStyAttr(sal_uInt16 nWhich) SAL_OVERRIDE;

    bool IsInTable();
    virtual const SfxPoolItem& GetCellAttr(sal_uInt16 nWhich) SAL_OVERRIDE;
    virtual bool BeginTable() SAL_OVERRIDE;
    virtual void NextTableCell() SAL_OVERRIDE;
    virtual void NextTableRow() SAL_OVERRIDE;
    virtual void SetTableWidth(SwTwips nW) SAL_OVERRIDE;
    virtual void SetTableOrient(sal_Int16 eOri) SAL_OVERRIDE;
    virtual void SetCellWidth(SwTwips nWidth, sal_uInt16 nCell) SAL_OVERRIDE;
    virtual void SetCellHeight(SwTwips nH) SAL_OVERRIDE;
    virtual void SetCellBorder(const SvxBoxItem& rFmtBox, sal_uInt16 nCell) SAL_OVERRIDE;
    virtual void SetCellSpace(sal_uInt16 nSp) SAL_OVERRIDE;
    virtual void DeleteCell(sal_uInt16 nCell) SAL_OVERRIDE;
    virtual void EndTable() SAL_OVERRIDE;

    SwFrmFmt* MakeFly( RndStdIds eAnchor, SfxItemSet* pSet );
    virtual bool IsInFly() SAL_OVERRIDE;
    virtual void SetFlyFrmAttr(const SfxPoolItem& rAttr) SAL_OVERRIDE;
    virtual const SfxPoolItem& GetFlyFrmAttr(sal_uInt16 nWhich) SAL_OVERRIDE;
    virtual bool BeginFly( RndStdIds eAnchor, bool bAbsolutePos,
                               const SfxItemSet* pMoreAttrs = 0 ) SAL_OVERRIDE;
    virtual void EndFly() SAL_OVERRIDE;
};

class SwFltFormatCollection : public SwFltOutBase
{
    SwTxtFmtColl* pColl;
    SfxItemSet* pFlyAttrs;      // Simulation der Flys in Styles
    bool bHasFly;
public:
    SwFltFormatCollection(SwDoc&, RES_POOL_COLLFMT_TYPE nType);
    SwFltFormatCollection(SwDoc&, const OUString& rName );
    virtual ~SwFltFormatCollection() { if( pFlyAttrs ) delete pFlyAttrs; }

    void Reset()
    {
        // #i73790# - method renamed
        pColl->ResetAllFmtAttr();
        pColl->SetAuto(false); // nach Empfehlung JP
    }
    void Derived(SwTxtFmtColl* pBase)
        { pColl->SetDerivedFrom(pBase); }

//  SwTxtFmtColl* Search(String, CharSet eSrc);
    SwTxtFmtColl* GetColl()         { return pColl; }
    void SetHasFly()                { bHasFly = true; }
    SfxItemSet* GetpFlyAttrs()      { return pFlyAttrs; }

    virtual SwFltOutBase& operator << (const SfxPoolItem& rItem) SAL_OVERRIDE;
    virtual const SfxPoolItem& GetAttr(sal_uInt16 nWhich) SAL_OVERRIDE;
    virtual const SfxPoolItem& GetNodeOrStyAttr(sal_uInt16 nWhich) SAL_OVERRIDE;

    virtual bool IsInFly() SAL_OVERRIDE;
    virtual void SetFlyFrmAttr(const SfxPoolItem& rAttr) SAL_OVERRIDE;
    virtual const SfxPoolItem& GetFlyFrmAttr(sal_uInt16 nWhich) SAL_OVERRIDE;
    virtual bool BeginFly( RndStdIds eAnchor, bool bAbsolutePos,
                               const SfxItemSet* pMoreAttrs = 0 ) SAL_OVERRIDE;
    bool BeginStyleFly( SwFltOutDoc* pOutDoc );
    virtual void EndFly() SAL_OVERRIDE;
};

// dies nun ist die zauberhafteklasse: intention: alle eins nach dem
// andern hinein'pipe'n. wird eine besondere struktur eingelesen, wird
// eine klammer geoeffnet (BeginXxx) und nach beendigung geschlossen
// (EndXxx), wobei Xxx zB fuer Fusznoten, Kopf/Fuszzeilen oder
// Tabellen steht. Styles funktionieren auch so, haben aber den
// unterschied, keine buchstaben zu akzeptieren.
// beginnt ein neuer absatz oder aehnliches, wird NextXxx genutzt.
// hier ist moeglich, Tab, Zeilenumbruch, Absatzende, Seitenumbruch
// und Sektionsende einzufuegen.

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

    SwFltControlStack aStack;
    SwFltEndStack aEndStack;
    SwPaM* pPaM;

    OUString sBaseURL;
    sal_uInt16 nPageDescOffset; // fuers update der pagedescs
    rtl_TextEncoding eSrcCharSet; // charset der quelle
    friend class SwFltControlStack;
    bool bNewDoc;
    bool bStdPD;
    bool bProtect;

public:
    SwFltShell(SwDoc* , SwPaM& , const OUString& rBaseURL, bool bNew, sal_uLong = 0);
    ~SwFltShell();

    SwDoc& GetDoc()                 { return *aStack.pDoc; }

    rtl_TextEncoding SetCharSet(rtl_TextEncoding eNew)    { rtl_TextEncoding eOld = eSrcCharSet;
                                          eSrcCharSet = eNew;
                                          return eOld;
                                        }
    void SetUseStdPageDesc()        { bStdPD = true; }
    void SetProtect()               { bProtect = true; }
    SwPageDesc* MakePageDesc(SwPageDesc* pFirstPageDesc = NULL);
    SwPageDesc& GetPageDesc()       { return *pCurrentPageDesc; }
    void NextTab()                  { (*this) << sal_uInt8(0x09); }
    void NextLine()                 { (*this) << sal_uInt8(0x0a); }
    void NextParagraph();
    void NextPage();
    void NextSection()      { pCurrentPageDesc = MakePageDesc(); }

    SwFltShell& AddGraphic( const OUString& rPicName );
    SwFltShell& AddError( const sal_Char* pErr );
    SwFltShell& EndItem( sal_uInt16 nId );
    SwFltShell& SetStyle( sal_uInt16 nStyle );

    SwFltShell& operator << ( Graphic& );
    SwFltShell& operator << ( SwFltBookmark& aBook );
    void SetBookEnd(long nHandle);
    SwFltShell& operator << ( const OUString& );  // Vorsicht: CHARSET_ANSI
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
    bool IsInTable() {
        return ( pOut == pOutDoc ) && pOutDoc->IsInTable(); }
    const SfxPoolItem& GetCellAttr(sal_uInt16 nWhich) {
        return pOut->GetCellAttr(nWhich); }
    bool BeginTable() {
        bool b = pOut->BeginTable();
        if(b) eSubMode = Table;
        return b; }
    void NextTableCell() {
        pOut->NextTableCell(); }
    void NextTableRow() {
        pOut->NextTableRow(); }
    void SetTableWidth(SwTwips nW) {
        pOut->SetTableWidth(nW); }
    bool IsTableWidthSet() {
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
    bool IsInFly() { return pOut->IsInFly(); }
    bool BeginFly( RndStdIds eAnchor = FLY_AT_PARA, bool bAbsolutePos = false );
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
    void BeginStyle( sal_uInt16 nUserCode, const OUString& rName )
    {
        OSL_ENSURE(nUserCode<sizeof(pColls)/sizeof(*pColls), "code out of bounds");
        OSL_ENSURE(pColls[nUserCode] == NULL, "user codes dublicate");
        if (eSubMode == Style)
            EndStyle();
        pOut = pColls[nUserCode] = new SwFltFormatCollection(GetDoc(), rName );
        nAktStyle = nUserCode;
        eSubMode = Style;
    }
    bool IsStyleImported(sal_uInt16 nUserCode)
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

    bool IsFlagSet(SwFltControlStack::Flags no) const
        { return aStack.IsFlagSet(no); }
    OUString ConvertUStr(const OUString& rInOut);
    OUString QuoteStr( const OUString& rIn );

    // folgende status kann die shell verwalten:
    const SfxPoolItem& GetNodeOrStyAttr(sal_uInt16 nWhich);
    const SfxPoolItem& GetAttr(sal_uInt16 nWhich);
    const SfxPoolItem& GetFlyFrmAttr(sal_uInt16 nWhich);
    SwFieldType* GetSysFldType(sal_uInt16 eWhich);
    bool GetWeightBold();
    bool GetPostureItalic();
    bool GetCrossedOut();
    bool GetContour();
    bool GetCaseKapitaelchen();
    bool GetCaseVersalien();

    const OUString& GetBaseURL() const { return sBaseURL; }
};

SW_DLLPUBLIC void UpdatePageDescs(SwDoc &rDoc, sal_uInt16 nInPageDescOffset);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
