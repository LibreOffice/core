/*************************************************************************
 *
 *  $RCSfile: fltshell.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:53 $
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
#ifndef _FLTSHELL_HXX
#define _FLTSHELL_HXX

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _KEYCOD_HXX //autogen
#include <vcl/keycod.hxx>
#endif
#ifndef _DATETIME_HXX //autogen
#include <tools/datetime.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif


#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif

class SwFlyFrmFmt;
class SwTOXBase;
class SwNumRule;
class SwFltShell;
class SwFmtFld;
class SwField;
class Graphic;
class SwTableLines;
class SwTableBoxes;
class SwTableBox;
class SwTableLine;
class International;
class DateTime;


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

    SwFltStackEntry(const SwPosition & rStartPos, SfxPoolItem* pHt );
    SwFltStackEntry(const SwFltStackEntry& rEntry);
    ~SwFltStackEntry();

    void SetStartPos(const SwPosition & rStartPos);
    void SetEndPos(  const SwPosition & rEndPos);
    BOOL MakeRegion(SwDoc* pDoc, SwPaM& rRegion, BOOL bCheck );
};

// ein Stack fuer die gesamten Text-Attribute
typedef SwFltStackEntry* SwFltStackEntryPtr;

SV_DECL_PTRARR(SwFltControlStackEntries,SwFltStackEntryPtr,5,10)

class SwFltControlStack : public SwFltControlStackEntries
{
    friend SwFltShell;

    ULONG nFieldFlags;
    KeyCode aEmptyKeyCode; // fuer Bookmarks

    BOOL IsAttrOpen(USHORT nAttrId);

    void MoveAttrs( const SwPosition&  rPos );

protected:
    SwDoc* pDoc;
    BOOL bIsEndStack;

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
    ~SwFltControlStack();

    BOOL IsFlagSet(Flags no) const  { return ::SwFltGetFlag(nFieldFlags, no);}

    void NewAttr(const SwPosition& rPos, const SfxPoolItem & rAttr );
//  BOOL SetAttr(const SwPosition& rPos, USHORT nAttrId=0, BOOL bTstEnde=TRUE);
                // SetAttr mit Handle fuer verschachtelte Attrs, z.B. Bookmarks
    BOOL SetAttr(const SwPosition& rPos, USHORT nAttrId=0, BOOL bTstEnde=TRUE,
                 long nHand = LONG_MAX);

    void StealAttr(const SwPosition* pPos, USHORT nAttrId = 0);
    void MoveAttrsToNextNode( const SwNodeIndex& rNdIdx );
    void MarkAllAttrsOld();
    void KillUnlockedAttrs(const SwPosition& pPos);
    SfxPoolItem* GetFmtStackAttr(USHORT nWhich, USHORT * pPos = 0);
    const SfxPoolItem* GetOpenStackAttr(const SwPosition& rPos, USHORT nWhich);
    const SfxPoolItem* GetFmtAttr(const SwPosition& rPos, USHORT nWhich);
};

class SwFltAnchor : public SfxPoolItem
{
    SwFrmFmt* pFrmFmt;
public:
    SwFltAnchor(SwFrmFmt* pFlyFmt);
    SwFltAnchor(const SwFltAnchor&);
    // "pure virtual Methoden" vom SfxPoolItem
    virtual int operator==(const SfxPoolItem&) const;
    virtual SfxPoolItem* Clone(SfxItemPool* = 0) const;
    const SwFrmFmt* GetFrmFmt() const       { return pFrmFmt; }
          SwFrmFmt* GetFrmFmt()             { return pFrmFmt; }
};

class SwFltRedline : public SfxPoolItem
{
public:
    DateTime        aStamp;
    DateTime        aStampPrev;
    SwRedlineType   eType;
    SwRedlineType   eTypePrev;
    USHORT          nAutorNo;
    USHORT          nAutorNoPrev;

    SwFltRedline(SwRedlineType   eType_,
                 USHORT          nAutorNo_,
                 const DateTime& rStamp_,
                 SwRedlineType   eTypePrev_    = REDLINE_INSERT,
                 USHORT          nAutorNoPrev_ = USHRT_MAX,
                 const DateTime* pStampPrev_   = 0)
        : eType( eType_ ), nAutorNo( nAutorNo_ ), aStamp( rStamp_ ),
        eTypePrev( eTypePrev_ ), nAutorNoPrev( nAutorNoPrev_ )
    {
            if( pStampPrev_ )
                aStampPrev = *pStampPrev_;
    }

    SwFltRedline(const SwFltRedline& rCpy):
        SfxPoolItem(RES_FLTR_REDLINE),
        eType(          rCpy.eType        ),
        nAutorNo(       rCpy.nAutorNo     ),
        aStamp(         rCpy.aStamp       ),
        eTypePrev(      rCpy.eTypePrev    ),
        nAutorNoPrev(   rCpy.nAutorNoPrev ),
        aStampPrev(     rCpy.aStampPrev   )
        {}
    // "pure virtual Methoden" vom SfxPoolItem
    virtual int operator==(const SfxPoolItem& rItem) const;
    virtual SfxPoolItem* Clone(SfxItemPool* = 0) const;
};

class SwFltBookmark : public SfxPoolItem
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

class SwFltTOX : public SfxPoolItem
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
    SwSection* pSection;
public:
    SwFltSection( SwSection* pSect );
    SwFltSection( const SwFltSection& );
    // "pure virtual Methoden" vom SfxPoolItem
    virtual int operator==(const SfxPoolItem&) const;
    virtual SfxPoolItem* Clone(SfxItemPool* = 0) const;
    SwSection* GetSection()         { return pSection; }
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

    void SetBookRef( const String& rName, BOOL bPgRef = FALSE );
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

    SwFltOutBase(SwDoc& rDocu)
        : rDoc(rDocu), bFlyAbsPos(FALSE), eFlyAnchor(FLY_AT_CNTNT)
    {}

public:
    virtual SwFltOutBase& operator << (const SfxPoolItem& rItem) = 0;

    virtual const SfxPoolItem& GetAttr(USHORT nWhich) = 0;
    virtual const SfxPoolItem& GetNodeOrStyAttr(USHORT nWhich) = 0;

    virtual const SfxPoolItem& GetCellAttr(USHORT nWhich);
    virtual BOOL BeginTable();
    virtual void NextTableCell();
    virtual void NextTableRow();
    virtual void SetTableWidth(SwTwips nW);
    virtual void SetTableOrient(SwHoriOrient eOri);
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
    SwFltOutDoc(SwDoc& rDocu, SwPaM* pP, SwFltControlStack& rStk, SwFltEndStack& rEStk)
         :SwFltOutBase(rDocu), pPaM(pP), rStack(rStk), rEndStack(rEStk),
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
    virtual void SetTableOrient(SwHoriOrient eOri);
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
        pColl->ResetAllAttr();
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
    USHORT nPageDescOffset; // fuers update der pagedescs
    CharSet eSrcCharSet; // charset der quelle
    friend SwFltControlStack;
    BOOL bNewDoc;
    BOOL bStdPD;
    BOOL bProtect;

public:
    SwFltShell(SwDoc* , SwPaM& , BOOL bNew, ULONG = 0);
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
    void NextParagraph()    {   GetDoc().AppendTxtNode(*pPaM->GetPoint()); }
    void NextPage()         { NextParagraph();
                                  GetDoc().Insert(*pPaM,
                                    SvxFmtBreakItem(SVX_BREAK_PAGE_BEFORE));
                            }
    void NextSection()      { pCurrentPageDesc = MakePageDesc(); }

    SwFltShell& AddGraphic( const String& rPicName );
    SwFltShell& AddError( const sal_Char* pErr );
    SwFltShell& AddLinkedSection( const String& rFileName );
    SwFltShell& EndItem( USHORT nId );
    SwFltShell& SetStyle( USHORT nStyle );

    SwFltShell& operator << ( Graphic& );
    SwFltShell& operator << ( SwFltBookmark& aBook );
    void SetBookEnd(long nHandle);
    SwFltShell& operator << ( const SwFltTOX& );
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
    void SetTableOrient(SwHoriOrient eOri) {
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
    BOOL BeginFly( RndStdIds eAnchor = FLY_AT_CNTNT, BOOL bAbsolutePos = FALSE );
    void SetFlyAnchor( RndStdIds eAnchor )
        { pOut->SetFlyAnchor( eAnchor ); }
    void SetFlyXPos( short nXPos,SwRelationOrient eHRel = FRAME,
                     SwHoriOrient eHAlign = HORI_NONE );
    void SetFlyYPos( short nYPos, SwRelationOrient eVRel = FRAME,
                     SwVertOrient eVAlign = VERT_NONE );
    void SetFlyFrmAttr(const SfxPoolItem& rAttr){
        pOut->SetFlyFrmAttr( rAttr ); }
    void EndFly();
// methoden zur verwaltung von styles:
    void BeginStyle(USHORT nUserCode, RES_POOL_COLLFMT_TYPE aType)
    {
        ASSERT(nUserCode<sizeof(pColls)/sizeof(*pColls), "code out of bounds");
        ASSERT(pColls[nUserCode] == NULL, "user codes dublicate");
        if (eSubMode == Style)
            EndStyle();
        pOut = pColls[nUserCode] = new SwFltFormatCollection(GetDoc(), aType);
        nAktStyle = nUserCode;
        eSubMode = Style;
    }
    void BeginStyle( USHORT nUserCode, const String& rName )
    {
        ASSERT(nUserCode<sizeof(pColls)/sizeof(*pColls), "code out of bounds");
        ASSERT(pColls[nUserCode] == NULL, "user codes dublicate");
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
        ASSERT(eSubMode == Style, "wrong state for style");
        ASSERT(pColls[nBased], "Style based on noexistent style" );
        if( eSubMode == Style && pColls[nBased]->GetColl() )
            ((SwFltFormatCollection*)pOut)->Derived(pColls[nBased]->GetColl());
    }
    void NextStyle(USHORT nWhich, USHORT nNext);

    void EndStyle()
    {
//      ASSERT(eSubMode == Style, "wrong state for style");
        nAktStyle = 0;
        pOut = pOutDoc;
        eSubMode = None;
    }

    BOOL IsFlagSet(SwFltControlStack::Flags no) const
        { return aStack.IsFlagSet(no); }
    void ConvertUStr( String& rInOut );
    void ConvertLStr( String& rInOut );
    String QuoteStr( const String& rIn );
// folgende status kann die shell verwalten:
    const SfxPoolItem& GetNodeOrStyAttr(USHORT nWhich)
        { return pOut->GetNodeOrStyAttr( nWhich ); }
    const SfxPoolItem& GetAttr(USHORT nWhich)
        { return pOut->GetAttr( nWhich ); }
    const SfxPoolItem& GetFlyFrmAttr(USHORT nWhich)
        { return pOut->GetFlyFrmAttr( nWhich ); }
    SwFieldType *GetSysFldType(USHORT eWhich)
        { return GetDoc().GetSysFldType(eWhich); }
    BOOL GetWeightBold();
    BOOL GetPostureItalic();
    BOOL GetCrossedOut();
    BOOL GetContour();
    BOOL GetCaseKapitaelchen();
    BOOL GetCaseVersalien();
};


#endif
