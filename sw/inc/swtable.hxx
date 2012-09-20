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
#include <tblenum.hxx>
#include <swtypes.hxx>
#include <calbck.hxx>
#include <swrect.hxx>

#include <memory>
#include <boost/noncopyable.hpp>
#include <vector>
#include <algorithm>
#include <o3tl/sorted_vector.hxx>

class SwStartNode;
class SwFmt;
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

class SwTableLines : public std::vector<SwTableLine*> {
public:
    // free's any remaining child objects
    ~SwTableLines();

    // return USHRT_MAX if not found, else index of position
    sal_uInt16 GetPos(const SwTableLine* pBox) const
    {
        const_iterator it = std::find(begin(), end(), pBox);
        return it == end() ? USHRT_MAX : it - begin();
    }
};

class SwTableBoxes : public std::vector<SwTableBox*> {
public:
    // return USHRT_MAX if not found, else index of position
    sal_uInt16 GetPos(const SwTableBox* pBox) const
    {
        const_iterator it = std::find(begin(), end(), pBox);
        return it == end() ? USHRT_MAX : it - begin();
    }
};

// Save content-bearing box-pointers additionally in a sorted array
// (for calculation in table).
class SwTableSortBoxes : public o3tl::sorted_vector<SwTableBox*> {};

class SW_DLLPUBLIC SwTable: public SwClient          //Client of FrmFmt.
{


protected:
    SwTableLines aLines;
    SwTableSortBoxes m_TabSortContentBoxes;
    SwServerObjectRef refObj;   // In case DataServer -> pointer is set.

    SwHTMLTableLayout *pHTMLLayout;

    // Usually, the table node of a SwTable can be accessed by getting a box
    // out of m_TabSortContentBoxes, which know their SwStartNode. But in some rare
    // cases, we need to know the table node of a SwTable, before the table
    // boxes have been build (SwTableNode::MakeCopy with tables in tables).
    SwTableNode* pTableNode;

    // Should that be adjustable for every table?
    TblChgMode  eTblChgMode;

    sal_uInt16      nGrfsThatResize;    // Count of Grfs that initiate a resize of table
                                        // at HTML-import.
    sal_uInt16      nRowsToRepeat;      // Number of rows to repeat on every page.

    sal_Bool        bModifyLocked   :1;
    sal_Bool        bNewModel       :1; // sal_False: old SubTableModel; sal_True: new RowSpanModel
#ifdef DBG_UTIL
    /// This is set by functions (like Merge()) to forbid a late model change.
    bool m_bDontChangeModel;
#endif

    sal_Bool IsModifyLocked(){ return bModifyLocked;}

   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew );

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
    sal_Bool OldMerge( SwDoc*, const SwSelBoxes&, SwTableBox*, SwUndoTblMerge* );
    sal_Bool OldSplitRow( SwDoc*, const SwSelBoxes&, sal_uInt16, sal_Bool );
    sal_Bool NewMerge( SwDoc*, const SwSelBoxes&, const SwSelBoxes& rMerged,
                   SwTableBox*, SwUndoTblMerge* );
    sal_Bool NewSplitRow( SwDoc*, const SwSelBoxes&, sal_uInt16, sal_Bool );
    SwBoxSelection* CollectBoxSelection( const SwPaM& rPam ) const;
    void InsertSpannedRow( SwDoc* pDoc, sal_uInt16 nIdx, sal_uInt16 nCnt );
    sal_Bool _InsertRow( SwDoc*, const SwSelBoxes&, sal_uInt16 nCnt, sal_Bool bBehind );
    sal_Bool NewInsertCol( SwDoc*, const SwSelBoxes& rBoxes, sal_uInt16 nCnt, sal_Bool );
    void _FindSuperfluousRows( SwSelBoxes& rBoxes, SwTableLine*, SwTableLine* );
    void AdjustWidths( const long nOld, const long nNew );
    void NewSetTabCols( Parm &rP, const SwTabCols &rNew, const SwTabCols &rOld,
                        const SwTableBox *pStart, sal_Bool bCurRowOnly );

public:

    SwHTMLTableLayout *GetHTMLTableLayout() { return pHTMLLayout; }
    const SwHTMLTableLayout *GetHTMLTableLayout() const { return pHTMLLayout; }
    void SetHTMLTableLayout( SwHTMLTableLayout *p );    //Change of property!

    sal_uInt16 IncGrfsThatResize() { return ++nGrfsThatResize; }
    sal_uInt16 DecGrfsThatResize() { return nGrfsThatResize ? --nGrfsThatResize : 0; }

    void LockModify()   { bModifyLocked = sal_True; }   // Must be used always
    void UnlockModify() { bModifyLocked = sal_False;}   // in pairs!

    void SetTableModel( sal_Bool bNew ){ bNewModel = bNew; }
    sal_Bool IsNewModel() const { return bNewModel; }

    sal_uInt16 GetRowsToRepeat() const { return Min( (sal_uInt16)GetTabLines().size(), nRowsToRepeat ); }
    sal_uInt16 _GetRowsToRepeat() const { return nRowsToRepeat; }
    void SetRowsToRepeat( sal_uInt16 nNumOfRows ) { nRowsToRepeat = nNumOfRows; }

    bool IsHeadline( const SwTableLine& rLine ) const;

          SwTableLines &GetTabLines() { return aLines; }
    const SwTableLines &GetTabLines() const { return aLines; }

    SwFrmFmt* GetFrmFmt()       { return (SwFrmFmt*)GetRegisteredIn(); }
    SwFrmFmt* GetFrmFmt() const { return (SwFrmFmt*)GetRegisteredIn(); }
    SwTableFmt* GetTableFmt() const { return (SwTableFmt*)GetRegisteredIn(); }

    void GetTabCols( SwTabCols &rToFill, const SwTableBox *pStart,
                     sal_Bool bHidden = sal_False, sal_Bool bCurRowOnly = sal_False ) const;
    void SetTabCols( const SwTabCols &rNew, const SwTabCols &rOld,
                     const SwTableBox *pStart, sal_Bool bCurRowOnly );

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
    SwSaveRowSpan* CleanUpTopRowSpan( sal_uInt16 nSplitLine );
    // RestoreRowSpan is called by Undo to restore the old row span values
    void RestoreRowSpan( const SwSaveRowSpan& );
    // CleanUpBottomRowSpan corrects the overhanging row spans at the end of the first table
    void CleanUpBottomRowSpan( sal_uInt16 nDelLines );


// The following functions are "pseudo-virtual", i.e. they are different for old and new table model
// It's not allowed to change the table model after the first call of one of these functions.

    sal_Bool Merge( SwDoc* pDoc, const SwSelBoxes& rBoxes, const SwSelBoxes& rMerged,
                SwTableBox* pMergeBox, SwUndoTblMerge* pUndo = 0 )
    {
#ifdef DBG_UTIL
        m_bDontChangeModel = true;
#endif
        return bNewModel ? NewMerge( pDoc, rBoxes, rMerged, pMergeBox, pUndo ) :
                           OldMerge( pDoc, rBoxes, pMergeBox, pUndo );
    }
    sal_Bool SplitRow( SwDoc* pDoc, const SwSelBoxes& rBoxes, sal_uInt16 nCnt=1,
                   sal_Bool bSameHeight = sal_False )
    {
#ifdef DBG_UTIL
        m_bDontChangeModel = true;
#endif
        return bNewModel ? NewSplitRow( pDoc, rBoxes, nCnt, bSameHeight ) :
                           OldSplitRow( pDoc, rBoxes, nCnt, bSameHeight );
    }
    bool PrepareMerge( const SwPaM& rPam, SwSelBoxes& rBoxes,
        SwSelBoxes& rMerged, SwTableBox** ppMergeBox, SwUndoTblMerge* pUndo );
    void ExpandColumnSelection( SwSelBoxes& rBoxes, long &rMin, long &rMax ) const;
    void PrepareDeleteCol( long nMin, long nMax );

    sal_Bool InsertCol( SwDoc*, const SwSelBoxes& rBoxes,
                    sal_uInt16 nCnt = 1, sal_Bool bBehind = sal_True );
    sal_Bool InsertRow( SwDoc*, const SwSelBoxes& rBoxes,
                    sal_uInt16 nCnt = 1, sal_Bool bBehind = sal_True );
    sal_Bool AppendRow( SwDoc* pDoc, sal_uInt16 nCnt = 1 );
    void PrepareDelBoxes( const SwSelBoxes& rBoxes );
    sal_Bool DeleteSel( SwDoc*, const SwSelBoxes& rBoxes, const SwSelBoxes* pMerged,
        SwUndo* pUndo, const sal_Bool bDelMakeFrms, const sal_Bool bCorrBorder );
    sal_Bool SplitCol( SwDoc* pDoc, const SwSelBoxes& rBoxes, sal_uInt16 nCnt=1 );
    sal_Bool Merge( const SwSelBoxes& rBoxes,
                SwTableBox* pMergeBox, SwUndoTblMerge* = 0 );

    void FindSuperfluousRows( SwSelBoxes& rBoxes )
        { _FindSuperfluousRows( rBoxes, 0, 0 ); }
    void CheckRowSpan( SwTableLine* &rpLine, bool bUp ) const;

          SwTableSortBoxes& GetTabSortBoxes()       { return m_TabSortContentBoxes; }
    const SwTableSortBoxes& GetTabSortBoxes() const { return m_TabSortContentBoxes; }

    // Read 1st number and delete it from string (used by GetTblBox and SwTblFld).

    // #i80314#
    // add 3rd parameter in order to control validation check on <rStr>
    static sal_uInt16 _GetBoxNum( String& rStr,
                              sal_Bool bFirst = sal_False,
                              const bool bPerformValidCheck = false );

    // Search content-bearing box with that name.

    // #i80314#
    // add 2nd parameter in order to control validation check in called method
    // <_GetBoxNum(..)>
    const SwTableBox* GetTblBox( const String& rName,
                                 const bool bPerformValidCheck = false ) const;
    // Copy selected boxes to another document.
    sal_Bool MakeCopy( SwDoc*, const SwPosition&, const SwSelBoxes&,
                    sal_Bool bCpyNds = sal_True, sal_Bool bCpyName = sal_False ) const;
    // Copy table in this (implemented in TBLRWCL.CXX).
    sal_Bool InsTable( const SwTable& rCpyTbl, const SwNodeIndex&,
                    SwUndoTblCpyTbl* pUndo = 0 );
    sal_Bool InsTable( const SwTable& rCpyTbl, const SwSelBoxes&,
                    SwUndoTblCpyTbl* pUndo = 0 );
    sal_Bool InsNewTable( const SwTable& rCpyTbl, const SwSelBoxes&,
                      SwUndoTblCpyTbl* pUndo );
    // Copy headline of table (with content!) into an other one.
    sal_Bool CopyHeadlineIntoTable( SwTableNode& rTblNd );

    // Get box, whose start index is set on nBoxStt.
          SwTableBox* GetTblBox( sal_uLong nSttIdx );
    const SwTableBox* GetTblBox( sal_uLong nSttIdx ) const
                        {   return ((SwTable*)this)->GetTblBox( nSttIdx );  }

    // Returns sal_True if table contains nestings.
    sal_Bool IsTblComplex() const;

    // Returns sal_True if table or selection is balanced.
    sal_Bool IsTblComplexForChart( const String& rSel ) const;

    // Search all content-bearing boxes of the base line on which this box stands.
    // rBoxes as a return value for immediate use.
    // steht. rBoxes auch als Return-Wert, um es gleich weiter zu benutzen
    // bToTop = sal_True -> up to base line, sal_False-> else only line of box.
    SwSelBoxes& SelLineFromBox( const SwTableBox* pBox,
                            SwSelBoxes& rBoxes, sal_Bool bToTop = sal_True ) const;

    // Get information from client.
    virtual sal_Bool GetInfo( SfxPoolItem& ) const;

    // Search in format for registered table.
    static SwTable * FindTable( SwFrmFmt const*const pFmt );

    // Clean up structure a bit.
    void GCLines();

    // Returns the table node via m_TabSortContentBoxes or pTableNode.
    SwTableNode* GetTableNode() const;
    void SetTableNode( SwTableNode* pNode ) { pTableNode = pNode; }

    // Data server methods.
    void SetRefObject( SwServerObject* );
    const SwServerObject* GetObject() const     {  return &refObj; }
          SwServerObject* GetObject()           {  return &refObj; }

    // Fill data for chart.
    void UpdateCharts() const;

    TblChgMode GetTblChgMode() const        { return eTblChgMode; }
    void SetTblChgMode( TblChgMode eMode )  { eTblChgMode = eMode; }

    sal_Bool SetColWidth( SwTableBox& rAktBox, sal_uInt16 eType,
                        SwTwips nAbsDiff, SwTwips nRelDiff, SwUndo** ppUndo );
    sal_Bool SetRowHeight( SwTableBox& rAktBox, sal_uInt16 eType,
                        SwTwips nAbsDiff, SwTwips nRelDiff, SwUndo** ppUndo );
    void RegisterToFormat( SwFmt& rFmt );
#ifdef DBG_UTIL
    void CheckConsistency() const;
#endif

    bool HasLayout() const;
};

class SW_DLLPUBLIC SwTableLine: public SwClient     // Client of FrmFmt.
{
    SwTableBoxes aBoxes;
    SwTableBox *pUpper;

public:
    TYPEINFO();

    SwTableLine() : pUpper(0) {}

    SwTableLine( SwTableLineFmt*, sal_uInt16 nBoxes, SwTableBox *pUp );
    virtual ~SwTableLine();

          SwTableBoxes &GetTabBoxes() { return aBoxes; }
    const SwTableBoxes &GetTabBoxes() const { return aBoxes; }

          SwTableBox *GetUpper() { return pUpper; }
    const SwTableBox *GetUpper() const { return pUpper; }
    void SetUpper( SwTableBox *pNew ) { pUpper = pNew; }


    SwFrmFmt* GetFrmFmt()       { return (SwFrmFmt*)GetRegisteredIn(); }
    SwFrmFmt* GetFrmFmt() const { return (SwFrmFmt*)GetRegisteredIn(); }

    // Creates a own FrmFmt if more lines depend on it.
    SwFrmFmt* ClaimFrmFmt();
    void ChgFrmFmt( SwTableLineFmt* pNewFmt );

    // Search next/previous box with content.
    SwTableBox* FindNextBox( const SwTable&, const SwTableBox* =0,
                            sal_Bool bOvrTblLns=sal_True ) const;
    SwTableBox* FindPreviousBox( const SwTable&, const SwTableBox* =0,
                            sal_Bool bOvrTblLns=sal_True ) const;

    SwTwips GetTableLineHeight( bool& bLayoutAvailable ) const;

    bool hasSoftPageBreak() const;
    void RegisterToFormat( SwFmt& rFmt );
};

class SW_DLLPUBLIC SwTableBox: public SwClient      //Client of FrmFmt.
{
    friend class SwNodes;           // Transpose index.
    friend void DelBoxNode(SwTableSortBoxes&);  // Delete StartNode* !
    friend class SwXMLTableContext;

    // Not implemented (any more).
    SwTableBox( const SwTableBox & );
    SwTableBox &operator=( const SwTableBox &); // Does not exist.

    SwTableLines aLines;
    const SwStartNode * pSttNd;
    SwTableLine *pUpper;
    SwTableBox_Impl* pImpl;

    // In case Format contains formulas/values already,
    // a new one must be created for the new box.
    SwTableBoxFmt* CheckBoxFmt( SwTableBoxFmt* );

public:
    TYPEINFO();

    SwTableBox() : pSttNd(0), pUpper(0), pImpl(0) {}

    SwTableBox( SwTableBoxFmt*, sal_uInt16 nLines, SwTableLine *pUp = 0 );
    SwTableBox( SwTableBoxFmt*, const SwStartNode&, SwTableLine *pUp = 0 );
    SwTableBox( SwTableBoxFmt*, const SwNodeIndex&, SwTableLine *pUp = 0 );
    virtual ~SwTableBox();

          SwTableLines &GetTabLines() { return aLines; }
    const SwTableLines &GetTabLines() const { return aLines; }

          SwTableLine *GetUpper() { return pUpper; }
    const SwTableLine *GetUpper() const { return pUpper; }
    void SetUpper( SwTableLine *pNew ) { pUpper = pNew; }

    SwFrmFmt* GetFrmFmt()       { return (SwFrmFmt*)GetRegisteredIn(); }
    SwFrmFmt* GetFrmFmt() const { return (SwFrmFmt*)GetRegisteredIn(); }

    // Creates its own FrmFmt if more boxes depend on it.
    SwFrmFmt* ClaimFrmFmt();
    void ChgFrmFmt( SwTableBoxFmt *pNewFmt );

    void RemoveFromTable();
    const SwStartNode *GetSttNd() const { return pSttNd; }
    sal_uLong GetSttIdx() const;

    // Search next/previous box with content.
    SwTableBox* FindNextBox( const SwTable&, const SwTableBox* =0,
                            sal_Bool bOvrTblLns=sal_True ) const;
    SwTableBox* FindPreviousBox( const SwTable&, const SwTableBox* =0,
                            sal_Bool bOvrTblLns=sal_True ) const;
    // Return name of this box. It is determined dynamically and
    // is calculated from the position in the lines/boxes/table.
    String GetName() const;
    // Return "value" of box (for calculating in table).
    double GetValue( SwTblCalcPara& rPara ) const;

    sal_Bool IsInHeadline( const SwTable* pTbl = 0 ) const;

    // Contains box contents, that can be formated as a number?
    sal_Bool HasNumCntnt( double& rNum, sal_uInt32& rFmtIndex,
                    sal_Bool& rIsEmptyTxtNd ) const;
    sal_uLong IsValidNumTxtNd( sal_Bool bCheckAttr = sal_True ) const;
    // If a table formula is set, test if box contents is congruent with number.
    // (For Redo of change of NumFormat!).
    sal_Bool IsNumberChanged() const;

    // Is that a formula box or a box with numeric contents (AutoSum)?
    // What it is is indicated by the return value - the WhichId of the attribute.
    // Empty boxes have the return value USHRT_MAX !!
    sal_uInt16 IsFormulaOrValueBox() const;

    // Loading of a document requires an actualisation of cells with values
    void ActualiseValueBox();

    DECL_FIXEDMEMPOOL_NEWDEL(SwTableBox)

    // Access on internal data - currently used for the NumFormatter.
    inline const Color* GetSaveUserColor()  const;
    inline const Color* GetSaveNumFmtColor() const;
    inline void SetSaveUserColor(const Color* p );
    inline void SetSaveNumFmtColor( const Color* p );

    long getRowSpan() const;
    void setRowSpan( long nNewRowSpan );
    bool getDummyFlag() const;
    void setDummyFlag( bool bDummy );

    SwTableBox& FindStartOfRowSpan( const SwTable&, sal_uInt16 nMaxStep = USHRT_MAX );
    const SwTableBox& FindStartOfRowSpan( const SwTable& rTable,
        sal_uInt16 nMaxStep = USHRT_MAX ) const
        { return const_cast<SwTableBox*>(this)->FindStartOfRowSpan( rTable, nMaxStep ); }

    SwTableBox& FindEndOfRowSpan( const SwTable&, sal_uInt16 nMaxStep = USHRT_MAX );
    const SwTableBox& FindEndOfRowSpan( const SwTable& rTable,
        sal_uInt16 nMaxStep = USHRT_MAX ) const
        { return const_cast<SwTableBox*>(this)->FindEndOfRowSpan( rTable, nMaxStep ); }
    void RegisterToFormat( SwFmt& rFmt ) ;
    void ForgetFrmFmt();
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
