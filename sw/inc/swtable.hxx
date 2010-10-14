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
#ifndef _SWTABLE_HXX
#define _SWTABLE_HXX
#include <tools/mempool.hxx>
#include <tools/ref.hxx>
#include <svl/svarray.hxx>
#include <tblenum.hxx>
#include <swtypes.hxx>
#include <calbck.hxx>
#include <swrect.hxx>
#ifndef DBG_UTIL
#include <node.hxx>         // fuer StartNode->GetMyIndex
#else
class SwStartNode;
#include <memory>
#include <boost/noncopyable.hpp>
#endif

class Color;
class SwFrmFmt;
class SwTableFmt;
class SwTableLineFmt;
class SwTableBoxFmt;
class SwHTMLTableLayout;
class SwTableLine;
class SwTableBox;
class SwTableNode;
class SwTabCols;
class SwDoc;
class SwSelBoxes;
class SwTblCalcPara;
class SwChartLines;
struct SwPosition;
class SwNodeIndex;
class SwNode;
class SfxPoolItem;
class SwUndoTblMerge;
class SwUndo;
class SwPaM;
class SwTableBox_Impl;
class SwUndoTblCpyTbl;
class SwBoxSelection;
struct SwSaveRowSpan;
struct Parm;

#ifndef SW_DECL_SWSERVEROBJECT_DEFINED
#define SW_DECL_SWSERVEROBJECT_DEFINED
SV_DECL_REF( SwServerObject )
#endif

SV_DECL_PTRARR_DEL(SwTableLines, SwTableLine*, 10, 20)
SV_DECL_PTRARR_DEL(SwTableBoxes, SwTableBox*, 25, 50)

// speicher die Inhaltstragenden Box-Pointer zusaetzlich in einem
// sortierten Array (fuers rechnen in der Tabelle)
typedef SwTableBox* SwTableBoxPtr;
SV_DECL_PTRARR_SORT( SwTableSortBoxes, SwTableBoxPtr, 25, 50 )
typedef SwTableLine* SwTableLinePtr;

class SW_DLLPUBLIC SwTable: public SwClient          //Client vom FrmFmt
{
    using SwClient::IsModifyLocked;

protected:
    SwTableLines aLines;
    SwTableSortBoxes aSortCntBoxes;
    SwServerObjectRef refObj;   // falls DataServer -> Pointer gesetzt

    SwHTMLTableLayout *pHTMLLayout;

    // Usually, the table node of a SwTable can be accessed by getting a box
    // out of aSortCntBoxes, which know their SwStartNode. But in some rare
    // cases, we need to know the table node of a SwTable, before the table
    // boxes have been build (SwTableNode::MakeCopy with tables in tables).
    SwTableNode* pTableNode;

//SOLL das fuer jede Tabelle einstellbar sein?
    TblChgMode  eTblChgMode;

    USHORT      nGrfsThatResize;    // Anzahl der Grfs, die beim HTML-Import
                                    // noch ein Resize der Tbl. anstossen
    USHORT      nRowsToRepeat;      // number of rows to repeat on every page

    BOOL        bModifyLocked   :1;
    BOOL        bNewModel       :1; // FALSE: old SubTableModel; TRUE: new RowSpanModel
#ifdef DBG_UTIL
    bool bDontChangeModel;  // This is set by functions (like Merge()) to forbid a laet model change
#endif

    BOOL IsModifyLocked(){ return bModifyLocked;}

public:
    enum SearchType
    {
        SEARCH_NONE, // Default: expand to rectangle
        SEARCH_ROW, // row selection
        SEARCH_COL  // column selection
    };

    TYPEINFO();

    // single argument ctors shall be explicit.
    explicit SwTable( SwTableFmt* );
    virtual ~SwTable();

    // @@@ public copy ctor, but no copy assignment?
    SwTable( const SwTable& rTable );       // kein Copy der Lines !!
private:
    // @@@ public copy ctor, but no copy assignment?
    SwTable & operator= (const SwTable &);
    // no default ctor.
    SwTable();
    BOOL OldMerge( SwDoc*, const SwSelBoxes&, SwTableBox*, SwUndoTblMerge* );
    BOOL OldSplitRow( SwDoc*, const SwSelBoxes&, USHORT, BOOL );
    BOOL NewMerge( SwDoc*, const SwSelBoxes&, const SwSelBoxes& rMerged,
                   SwTableBox*, SwUndoTblMerge* );
    BOOL NewSplitRow( SwDoc*, const SwSelBoxes&, USHORT, BOOL );
    SwBoxSelection* CollectBoxSelection( const SwPaM& rPam ) const;
    void InsertSpannedRow( SwDoc* pDoc, USHORT nIdx, USHORT nCnt );
    BOOL _InsertRow( SwDoc*, const SwSelBoxes&, USHORT nCnt, BOOL bBehind );
    BOOL NewInsertCol( SwDoc*, const SwSelBoxes& rBoxes, USHORT nCnt, BOOL );
    void _FindSuperfluousRows( SwSelBoxes& rBoxes, SwTableLine*, SwTableLine* );
    void AdjustWidths( const long nOld, const long nNew );
    void NewSetTabCols( Parm &rP, const SwTabCols &rNew, const SwTabCols &rOld,
                        const SwTableBox *pStart, BOOL bCurRowOnly );

public:

    SwHTMLTableLayout *GetHTMLTableLayout() { return pHTMLLayout; }
    const SwHTMLTableLayout *GetHTMLTableLayout() const { return pHTMLLayout; }
    void SetHTMLTableLayout( SwHTMLTableLayout *p );    //Eigentumsuebergang!

    USHORT IncGrfsThatResize() { return ++nGrfsThatResize; }
    USHORT DecGrfsThatResize() { return nGrfsThatResize ? --nGrfsThatResize : 0; }

    void LockModify()   { bModifyLocked = TRUE; }   //Muessen _immer_ paarig
    void UnlockModify() { bModifyLocked = FALSE;}   //benutzt werden!

    void SetTableModel( BOOL bNew ){ bNewModel = bNew; }
    BOOL IsNewModel() const { return bNewModel; }

    USHORT GetRowsToRepeat() const { return Min( GetTabLines().Count(), nRowsToRepeat ); }
    USHORT _GetRowsToRepeat() const { return nRowsToRepeat; }
    void SetRowsToRepeat( USHORT nNumOfRows ) { nRowsToRepeat = nNumOfRows; }

    bool IsHeadline( const SwTableLine& rLine ) const;

          SwTableLines &GetTabLines() { return aLines; }
    const SwTableLines &GetTabLines() const { return aLines; }

    SwFrmFmt* GetFrmFmt()       { return (SwFrmFmt*)pRegisteredIn; }
    SwFrmFmt* GetFrmFmt() const { return (SwFrmFmt*)pRegisteredIn; }

    virtual void Modify( SfxPoolItem* pOld, SfxPoolItem* pNew );

    void GetTabCols( SwTabCols &rToFill, const SwTableBox *pStart,
                     BOOL bHidden = FALSE, BOOL bCurRowOnly = FALSE ) const;
    void SetTabCols( const SwTabCols &rNew, const SwTabCols &rOld,
                     const SwTableBox *pStart, BOOL bCurRowOnly );

// The following functions are for new table model only...
    void CreateSelection(  const SwPaM& rPam, SwSelBoxes& rBoxes,
        const SearchType eSearchType, bool bProtect ) const;
    void CreateSelection( const SwNode* pStart, const SwNode* pEnd,
        SwSelBoxes& rBoxes, const SearchType eSearchType, bool bProtect ) const;
    void ExpandSelection( SwSelBoxes& rBoxes ) const;
    // When a table is splitted into two tables, the row spans which overlaps
    // the split have to be corrected and stored for undo
    // SwSavRowSpan is the structure needed by Undo to undo the split operation
    // CleanUpRowSpan corrects the (top of the) second table and delviers the structure
    // for Undo
    SwSaveRowSpan* CleanUpTopRowSpan( USHORT nSplitLine );
    // RestoreRowSpan is called by Undo to restore the old row span values
    void RestoreRowSpan( const SwSaveRowSpan& );
    // CleanUpBottomRowSpan corrects the overhanging row spans at the end of the first table
    void CleanUpBottomRowSpan( USHORT nDelLines );


// The following functions are "pseudo-virtual", i.e. they are different for old and new table model
// It's not allowed to change the table model after the first call of one of these functions.

    BOOL Merge( SwDoc* pDoc, const SwSelBoxes& rBoxes, const SwSelBoxes& rMerged,
                SwTableBox* pMergeBox, SwUndoTblMerge* pUndo = 0 )
    {
#ifdef DBG_UTIL
        bDontChangeModel = true;
#endif
        return bNewModel ? NewMerge( pDoc, rBoxes, rMerged, pMergeBox, pUndo ) :
                           OldMerge( pDoc, rBoxes, pMergeBox, pUndo );
    }
    BOOL SplitRow( SwDoc* pDoc, const SwSelBoxes& rBoxes, USHORT nCnt=1,
                   BOOL bSameHeight = FALSE )
    {
#ifdef DBG_UTIL
        bDontChangeModel = true;
#endif
        return bNewModel ? NewSplitRow( pDoc, rBoxes, nCnt, bSameHeight ) :
                           OldSplitRow( pDoc, rBoxes, nCnt, bSameHeight );
    }
    bool PrepareMerge( const SwPaM& rPam, SwSelBoxes& rBoxes,
        SwSelBoxes& rMerged, SwTableBox** ppMergeBox, SwUndoTblMerge* pUndo );
    void ExpandColumnSelection( SwSelBoxes& rBoxes, long &rMin, long &rMax ) const;
    void PrepareDeleteCol( long nMin, long nMax );

    BOOL InsertCol( SwDoc*, const SwSelBoxes& rBoxes,
                    USHORT nCnt = 1, BOOL bBehind = TRUE );
    BOOL InsertRow( SwDoc*, const SwSelBoxes& rBoxes,
                    USHORT nCnt = 1, BOOL bBehind = TRUE );
    BOOL AppendRow( SwDoc* pDoc, USHORT nCnt = 1 );
    void PrepareDelBoxes( const SwSelBoxes& rBoxes );
    BOOL DeleteSel( SwDoc*, const SwSelBoxes& rBoxes, const SwSelBoxes* pMerged,
        SwUndo* pUndo, const BOOL bDelMakeFrms, const BOOL bCorrBorder );
    BOOL SplitCol( SwDoc* pDoc, const SwSelBoxes& rBoxes, USHORT nCnt=1 );
    BOOL Merge( const SwSelBoxes& rBoxes,
                SwTableBox* pMergeBox, SwUndoTblMerge* = 0 );

    void FindSuperfluousRows( SwSelBoxes& rBoxes )
        { _FindSuperfluousRows( rBoxes, 0, 0 ); }
    void CheckRowSpan( SwTableLinePtr &rpLine, bool bUp ) const;

          SwTableSortBoxes& GetTabSortBoxes()       { return aSortCntBoxes; }
    const SwTableSortBoxes& GetTabSortBoxes() const { return aSortCntBoxes; }

        // lese die 1. Nummer und loesche sie aus dem String
        // (wird von GetTblBox und SwTblFld benutzt)
    // --> OD 2007-08-03 #i80314#
    // add 3rd parameter in order to control validation check on <rStr>
    static USHORT _GetBoxNum( String& rStr,
                              BOOL bFirst = FALSE,
                              const bool bPerformValidCheck = false );
    // <--
        // suche die Inhaltstragende Box mit dem Namen
    // --> OD 2007-08-03 #i80314#
    // add 2nd parameter in order to control validation check in called method
    // <_GetBoxNum(..)>
    const SwTableBox* GetTblBox( const String& rName,
                                 const bool bPerformValidCheck = false ) const;
    // <--
        // kopiere die selektierten Boxen in ein anderes Dokument.
    BOOL MakeCopy( SwDoc*, const SwPosition&, const SwSelBoxes&,
                    BOOL bCpyNds = TRUE, BOOL bCpyName = FALSE ) const;
        // kopiere die Tabelle in diese. (die Logik steht im TBLRWCL.CXX)
    BOOL InsTable( const SwTable& rCpyTbl, const SwNodeIndex&,
                    SwUndoTblCpyTbl* pUndo = 0 );
    BOOL InsTable( const SwTable& rCpyTbl, const SwSelBoxes&,
                    SwUndoTblCpyTbl* pUndo = 0 );
    BOOL InsNewTable( const SwTable& rCpyTbl, const SwSelBoxes&,
                      SwUndoTblCpyTbl* pUndo );
        // kopiere die Headline (mit Inhalt!) der Tabelle in eine andere
    BOOL CopyHeadlineIntoTable( SwTableNode& rTblNd );

        // erfrage die Box, dessen Start-Index auf nBoxStt steht
          SwTableBox* GetTblBox( ULONG nSttIdx );
    const SwTableBox* GetTblBox( ULONG nSttIdx ) const
                        {   return ((SwTable*)this)->GetTblBox( nSttIdx );  }

    // returnt TRUE wenn sich in der Tabelle Verschachtelungen befinden
    BOOL IsTblComplex() const;

    //returnt TRUE wenn die Tabelle oder Selektion ausgeglichen ist
    BOOL IsTblComplexForChart( const String& rSel,
                                SwChartLines* pGetCLines = 0  ) const;

    // suche alle Inhaltstragenden-Boxen der Grundline in der diese Box
    // steht. rBoxes auch als Return-Wert, um es gleich weiter zu benutzen
    //JP 31.01.97: bToTop = TRUE -> hoch bis zur Grundline,
    //                      FALSE-> sonst nur die Line der Box
    SwSelBoxes& SelLineFromBox( const SwTableBox* pBox,
                            SwSelBoxes& rBoxes, BOOL bToTop = TRUE ) const;
        // erfrage vom Client Informationen
    virtual BOOL GetInfo( SfxPoolItem& ) const;

        // suche im Format nach der angemeldeten Tabelle
    static SwTable * FindTable( SwFrmFmt const*const pFmt );

        // Struktur ein wenig aufraeumen
    void GCLines();

    // returns the table node via aSortCntBoxes or pTableNode
    SwTableNode* GetTableNode() const;
    void SetTableNode( SwTableNode* pNode ) { pTableNode = pNode; }

        // Daten Server-Methoden
    void SetRefObject( SwServerObject* );
    const SwServerObject* GetObject() const     {  return &refObj; }
          SwServerObject* GetObject()           {  return &refObj; }

    //Daten fuer das Chart fuellen.
    void UpdateCharts() const;

    TblChgMode GetTblChgMode() const        { return eTblChgMode; }
    void SetTblChgMode( TblChgMode eMode )  { eTblChgMode = eMode; }

    BOOL SetColWidth( SwTableBox& rAktBox, USHORT eType,
                        SwTwips nAbsDiff, SwTwips nRelDiff, SwUndo** ppUndo );
    BOOL SetRowHeight( SwTableBox& rAktBox, USHORT eType,
                        SwTwips nAbsDiff, SwTwips nRelDiff, SwUndo** ppUndo );
#ifdef DBG_UTIL
    void CheckConsistency() const;
#endif
};

class SW_DLLPUBLIC SwTableLine: public SwClient     // Client vom FrmFmt
{
    SwTableBoxes aBoxes;
    SwTableBox *pUpper;

public:
    TYPEINFO();

    SwTableLine() : pUpper(0) {}

    SwTableLine( SwTableLineFmt*, USHORT nBoxes, SwTableBox *pUp );
    virtual ~SwTableLine();

          SwTableBoxes &GetTabBoxes() { return aBoxes; }
    const SwTableBoxes &GetTabBoxes() const { return aBoxes; }

          SwTableBox *GetUpper() { return pUpper; }
    const SwTableBox *GetUpper() const { return pUpper; }
    void SetUpper( SwTableBox *pNew ) { pUpper = pNew; }


    SwFrmFmt* GetFrmFmt()       { return (SwFrmFmt*)pRegisteredIn; }
    SwFrmFmt* GetFrmFmt() const { return (SwFrmFmt*)pRegisteredIn; }

    //Macht ein eingenes FrmFmt wenn noch mehr Lines von ihm abhaengen.
    SwFrmFmt* ClaimFrmFmt();
    void ChgFrmFmt( SwTableLineFmt* pNewFmt );

    // suche nach der naechsten/vorherigen Box mit Inhalt
    SwTableBox* FindNextBox( const SwTable&, const SwTableBox* =0,
                            BOOL bOvrTblLns=TRUE ) const;
    SwTableBox* FindPreviousBox( const SwTable&, const SwTableBox* =0,
                            BOOL bOvrTblLns=TRUE ) const;

    SwTwips GetTableLineHeight( bool& bLayoutAvailable ) const;

    bool hasSoftPageBreak() const;
};

class SW_DLLPUBLIC SwTableBox: public SwClient      //Client vom FrmFmt
{
    friend class SwNodes;           // um den Index umzusetzen !
    friend void DelBoxNode(SwTableSortBoxes&);  // um den StartNode* zu loeschen !
    friend class SwXMLTableContext;

    //nicht (mehr) implementiert.
    SwTableBox( const SwTableBox & );
    SwTableBox &operator=( const SwTableBox &); //gibts nicht.

    SwTableLines aLines;
    const SwStartNode * pSttNd;
    SwTableLine *pUpper;
    SwTableBox_Impl* pImpl;

    // falls das Format schon Formeln/Values enthaelt, muss ein neues
    // fuer die neue Box erzeugt werden.
    SwTableBoxFmt* CheckBoxFmt( SwTableBoxFmt* );

public:
    TYPEINFO();

    SwTableBox() : pSttNd(0), pUpper(0), pImpl(0) {}

    SwTableBox( SwTableBoxFmt*, USHORT nLines, SwTableLine *pUp = 0 );
    SwTableBox( SwTableBoxFmt*, const SwStartNode&, SwTableLine *pUp = 0 );
    SwTableBox( SwTableBoxFmt*, const SwNodeIndex&, SwTableLine *pUp = 0 );
    virtual ~SwTableBox();

          SwTableLines &GetTabLines() { return aLines; }
    const SwTableLines &GetTabLines() const { return aLines; }

          SwTableLine *GetUpper() { return pUpper; }
    const SwTableLine *GetUpper() const { return pUpper; }
    void SetUpper( SwTableLine *pNew ) { pUpper = pNew; }

    SwFrmFmt* GetFrmFmt()       { return (SwFrmFmt*)pRegisteredIn; }
    SwFrmFmt* GetFrmFmt() const { return (SwFrmFmt*)pRegisteredIn; }

    //Macht ein eingenes FrmFmt wenn noch mehr Boxen von ihm abhaengen.
    SwFrmFmt* ClaimFrmFmt();
    void ChgFrmFmt( SwTableBoxFmt *pNewFmt );

    const SwStartNode *GetSttNd() const { return pSttNd; }
    ULONG GetSttIdx() const
#ifndef DBG_UTIL
        { return pSttNd ? pSttNd->GetIndex() : 0; }
#else
        ;
#endif

    // suche nach der naechsten/vorherigen Box mit Inhalt
    SwTableBox* FindNextBox( const SwTable&, const SwTableBox* =0,
                            BOOL bOvrTblLns=TRUE ) const;
    SwTableBox* FindPreviousBox( const SwTable&, const SwTableBox* =0,
                            BOOL bOvrTblLns=TRUE ) const;
    // gebe den Namen dieser Box zurueck. Dieser wird dynamisch bestimmt
    // und ergibt sich aus der Position in den Lines/Boxen/Tabelle
    String GetName() const;
    // gebe den "Wert" der Box zurueck (fuers rechnen in der Tabelle)
    double GetValue( SwTblCalcPara& rPara ) const;

    BOOL IsInHeadline( const SwTable* pTbl = 0 ) const;

    // enthaelt die Box Inhalt, der als Nummer formatiert werden kann?
    BOOL HasNumCntnt( double& rNum, sal_uInt32& rFmtIndex,
                    BOOL& rIsEmptyTxtNd ) const;
    ULONG IsValidNumTxtNd( BOOL bCheckAttr = TRUE ) const;
    // teste ob der BoxInhalt mit der Nummer uebereinstimmt, wenn eine
    // Tabellenformel gesetzt ist. (fuers Redo des Change vom NumFormat!)
    BOOL IsNumberChanged() const;

    // ist das eine FormelBox oder eine Box mit numerischen Inhalt (AutoSum)
    // Was es ist, besagt der ReturnWert - die WhichId des Attributes
    // Leere Boxen haben den ReturnWert USHRT_MAX !!
    USHORT IsFormulaOrValueBox() const;

    // Loading of a document requires an actualisation of cells with values
    void ActualiseValueBox();

    DECL_FIXEDMEMPOOL_NEWDEL(SwTableBox)

    // zugriff auf interne Daten - z.Z. benutzt fuer den NumFormatter
    inline const Color* GetSaveUserColor()  const;
    inline const Color* GetSaveNumFmtColor() const;
    inline void SetSaveUserColor(const Color* p );
    inline void SetSaveNumFmtColor( const Color* p );

    long getRowSpan() const;
    void setRowSpan( long nNewRowSpan );
    bool getDummyFlag() const;
    void setDummyFlag( bool bDummy );

    SwTableBox& FindStartOfRowSpan( const SwTable&, USHORT nMaxStep = USHRT_MAX );
    const SwTableBox& FindStartOfRowSpan( const SwTable& rTable,
        USHORT nMaxStep = USHRT_MAX ) const
        { return const_cast<SwTableBox*>(this)->FindStartOfRowSpan( rTable, nMaxStep ); }

    SwTableBox& FindEndOfRowSpan( const SwTable&, USHORT nMaxStep = USHRT_MAX );
    const SwTableBox& FindEndOfRowSpan( const SwTable& rTable,
        USHORT nMaxStep = USHRT_MAX ) const
        { return const_cast<SwTableBox*>(this)->FindEndOfRowSpan( rTable, nMaxStep ); }
};

class SwCellFrm;
class SW_DLLPUBLIC SwTableCellInfo : public ::boost::noncopyable
{
    struct Impl;
    ::std::auto_ptr<Impl> m_pImpl;

    const SwCellFrm * getCellFrm() const ;

public:
    SwTableCellInfo(const SwTable * pTable);
    ~SwTableCellInfo();

    bool getNext();
    SwRect getRect() const;
    const SwTableBox * getTableBox() const;
};

#endif  //_SWTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
