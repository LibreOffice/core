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
#ifndef INCLUDED_SW_INC_SWTABLE_HXX
#define INCLUDED_SW_INC_SWTABLE_HXX

#include <tools/mempool.hxx>
#include <tools/ref.hxx>
#include "tblenum.hxx"
#include "swtypes.hxx"
#include "calbck.hxx"
#include "swrect.hxx"
#include "swtblfmt.hxx"

#include <memory>
#include <vector>
#include <algorithm>
#include <o3tl/sorted_vector.hxx>

class SwStartNode;
class SwFormat;
class Color;
class SwHTMLTableLayout;
class SwTableLine;
class SwTableBox;
class SwTableNode;
class SwTabCols;
class SwDoc;
class SwSelBoxes;
class SwTableCalcPara;
struct SwPosition;
class SwNodeIndex;
class SwNode;
class SfxPoolItem;
class SwUndoTableMerge;
class SwUndo;
class SwPaM;
class SwUndoTableCpyTable;
class SwBoxSelection;
struct SwSaveRowSpan;
struct Parm;
class SwServerObject;

void sw_GetTableBoxColStr( sal_uInt16 nCol, OUString& rNm );

class SwTableLines
{
    std::vector<SwTableLine*> m_vLines;

public:
    typedef std::vector<SwTableLine*>::size_type size_type;
    typedef std::vector<SwTableLine*>::iterator iterator;
    typedef std::vector<SwTableLine*>::const_iterator const_iterator;

    // free's any remaining child objects
    ~SwTableLines();

    bool empty() const { return m_vLines.empty(); }
    size_type size() const { return m_vLines.size(); }
    iterator begin() { return m_vLines.begin(); }
    const_iterator begin() const { return m_vLines.begin(); }
    iterator end() { return m_vLines.end(); }
    const_iterator end() const { return m_vLines.end(); }
    SwTableLine* front() const { return m_vLines.front(); }
    SwTableLine* back() const { return m_vLines.back(); }
    void clear() { m_vLines.clear(); }
    iterator erase( iterator aIt ) { return m_vLines.erase( aIt ); }
    iterator erase( iterator aFirst, iterator aLast ) { return m_vLines.erase( aFirst, aLast ); }
    iterator insert( iterator aIt, SwTableLine* pLine ) { return m_vLines.insert( aIt, pLine ); }
    template<typename TInputIterator>
    void insert( iterator aIt, TInputIterator aFirst, TInputIterator aLast )
    {
        m_vLines.insert( aIt, aFirst, aLast );
    }
    void push_back( SwTableLine* pLine ) { m_vLines.push_back( pLine ); }
    void reserve( size_type nSize ) { m_vLines.reserve( nSize ); }
    SwTableLine*& operator[]( size_type nPos ) { return m_vLines[ nPos ]; }
    SwTableLine* operator[]( size_type nPos ) const { return m_vLines[ nPos ]; }

    // return USHRT_MAX if not found, else index of position
    sal_uInt16 GetPos(const SwTableLine* pBox) const
    {
        const_iterator it = std::find(begin(), end(), pBox);
        return it == end() ? USHRT_MAX : it - begin();
    }
};

using SwTableBoxes = std::vector<SwTableBox*>;

// Save content-bearing box-pointers additionally in a sorted array
// (for calculation in table).
class SwTableSortBoxes : public o3tl::sorted_vector<SwTableBox*> {};

/// SwTable is one table in the document model, containing rows (which contain cells).
class SW_DLLPUBLIC SwTable: public SwClient          //Client of FrameFormat.
{

protected:
    SwTableLines m_aLines;
    SwTableSortBoxes m_TabSortContentBoxes;
    tools::SvRef<SwServerObject> m_xRefObj;   // In case DataServer -> pointer is set.

    std::shared_ptr<SwHTMLTableLayout> m_xHTMLLayout;

    // Usually, the table node of a SwTable can be accessed by getting a box
    // out of m_TabSortContentBoxes, which know their SwStartNode. But in some rare
    // cases, we need to know the table node of a SwTable, before the table
    // boxes have been build (SwTableNode::MakeCopy with tables in tables).
    SwTableNode* m_pTableNode;

    // Should that be adjustable for every table?
    TableChgMode  m_eTableChgMode;

    sal_uInt16      m_nGraphicsThatResize;    // Count of Grfs that initiate a resize of table
                                        // at HTML-import.
    sal_uInt16      m_nRowsToRepeat;      // Number of rows to repeat on every page.

    /// Name of the table style to be applied on this table.
    OUString maTableStyleName;

    bool        m_bModifyLocked   :1;
    bool        m_bNewModel       :1; // false: old SubTableModel; true: new RowSpanModel

    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew ) override;

public:
    enum SearchType
    {
        SEARCH_NONE, // Default: expand to rectangle
        SEARCH_ROW, // row selection
        SEARCH_COL  // column selection
    };


    explicit SwTable();
    virtual ~SwTable() override;

    // @@@ public copy ctor, but no copy assignment?
    SwTable( const SwTable& rTable );       // no copy of the lines !!
private:
    // @@@ public copy ctor, but no copy assignment?
    SwTable & operator= (const SwTable &) = delete;
    bool OldMerge( SwDoc*, const SwSelBoxes&, SwTableBox*, SwUndoTableMerge* );
    bool OldSplitRow( SwDoc*, const SwSelBoxes&, sal_uInt16, bool );
    bool NewMerge( SwDoc*, const SwSelBoxes&, const SwSelBoxes& rMerged,
                   SwUndoTableMerge* );
    bool NewSplitRow( SwDoc*, const SwSelBoxes&, sal_uInt16, bool );
    std::unique_ptr<SwBoxSelection> CollectBoxSelection( const SwPaM& rPam ) const;
    void InsertSpannedRow( SwDoc* pDoc, sal_uInt16 nIdx, sal_uInt16 nCnt );
    bool InsertRow_( SwDoc*, const SwSelBoxes&, sal_uInt16 nCnt, bool bBehind );
    bool NewInsertCol( SwDoc*, const SwSelBoxes& rBoxes, sal_uInt16 nCnt, bool );
    void FindSuperfluousRows_( SwSelBoxes& rBoxes, SwTableLine*, SwTableLine* );
    void AdjustWidths( const long nOld, const long nNew );
    void NewSetTabCols( Parm &rP, const SwTabCols &rNew, const SwTabCols &rOld,
                        const SwTableBox *pStart, bool bCurRowOnly );

public:

    SwHTMLTableLayout *GetHTMLTableLayout() { return m_xHTMLLayout.get(); }
    const SwHTMLTableLayout *GetHTMLTableLayout() const { return m_xHTMLLayout.get(); }
    void SetHTMLTableLayout(std::shared_ptr<SwHTMLTableLayout> const& r);    //Change of property!

    sal_uInt16 IncGrfsThatResize() { return ++m_nGraphicsThatResize; }
    sal_uInt16 DecGrfsThatResize() { return m_nGraphicsThatResize ? --m_nGraphicsThatResize : 0; }

    void LockModify()   { m_bModifyLocked = true; }   // Must be used always
    void UnlockModify() { m_bModifyLocked = false;}   // in pairs!

    void SetTableModel( bool bNew ){ m_bNewModel = bNew; }
    bool IsNewModel() const { return m_bNewModel; }

    /// Return the table style name of this table.
    const OUString& GetTableStyleName() const { return maTableStyleName; }

    /// Set the new table style name for this table.
    void SetTableStyleName(const OUString& rName) { maTableStyleName = rName; }

    sal_uInt16 GetRowsToRepeat() const { return std::min( static_cast<sal_uInt16>(GetTabLines().size()), m_nRowsToRepeat ); }
    void SetRowsToRepeat( sal_uInt16 nNumOfRows ) { m_nRowsToRepeat = nNumOfRows; }

    bool IsHeadline( const SwTableLine& rLine ) const;

          SwTableLines &GetTabLines() { return m_aLines; }
    const SwTableLines &GetTabLines() const { return m_aLines; }

    SwTableFormat* GetFrameFormat()       { return static_cast<SwTableFormat*>(GetRegisteredIn()); }
    SwTableFormat* GetFrameFormat() const { return const_cast<SwTableFormat*>(static_cast<const SwTableFormat*>(GetRegisteredIn())); }

    void GetTabCols( SwTabCols &rToFill, const SwTableBox *pStart,
                     bool bHidden = false, bool bCurRowOnly = false ) const;
    void SetTabCols( const SwTabCols &rNew, const SwTabCols &rOld,
                     const SwTableBox *pStart, bool bCurRowOnly );

// The following functions are for new table model only...
    void CreateSelection(  const SwPaM& rPam, SwSelBoxes& rBoxes,
        const SearchType eSearchType, bool bProtect ) const;
    void CreateSelection( const SwNode* pStart, const SwNode* pEnd,
        SwSelBoxes& rBoxes, const SearchType eSearchType, bool bProtect ) const;
    void ExpandSelection( SwSelBoxes& rBoxes ) const;
    // When a table is splitted into two tables, the row spans which overlaps
    // the split have to be corrected and stored for undo
    // SwSavRowSpan is the structure needed by Undo to undo the split operation
    // CleanUpRowSpan corrects the (top of the) second table and delivers the structure
    // for Undo
    std::unique_ptr<SwSaveRowSpan> CleanUpTopRowSpan( sal_uInt16 nSplitLine );
    // RestoreRowSpan is called by Undo to restore the old row span values
    void RestoreRowSpan( const SwSaveRowSpan& );
    // CleanUpBottomRowSpan corrects the overhanging row spans at the end of the first table
    void CleanUpBottomRowSpan( sal_uInt16 nDelLines );

// The following functions are "pseudo-virtual", i.e. they are different for old and new table model
// It's not allowed to change the table model after the first call of one of these functions.

    bool Merge( SwDoc* pDoc, const SwSelBoxes& rBoxes, const SwSelBoxes& rMerged,
                SwTableBox* pMergeBox, SwUndoTableMerge* pUndo )
    {
        return m_bNewModel ? NewMerge( pDoc, rBoxes, rMerged, pUndo ) :
                             OldMerge( pDoc, rBoxes, pMergeBox, pUndo );
    }
    bool SplitRow( SwDoc* pDoc, const SwSelBoxes& rBoxes, sal_uInt16 nCnt,
                   bool bSameHeight )
    {
        return m_bNewModel ? NewSplitRow( pDoc, rBoxes, nCnt, bSameHeight ) :
                           OldSplitRow( pDoc, rBoxes, nCnt, bSameHeight );
    }
    bool PrepareMerge( const SwPaM& rPam, SwSelBoxes& rBoxes,
        SwSelBoxes& rMerged, SwTableBox** ppMergeBox, SwUndoTableMerge* pUndo );
    void ExpandColumnSelection( SwSelBoxes& rBoxes, long &rMin, long &rMax ) const;
    void PrepareDeleteCol( long nMin, long nMax );

    bool InsertCol( SwDoc*, const SwSelBoxes& rBoxes,
                    sal_uInt16 nCnt, bool bBehind );
    bool InsertRow( SwDoc*, const SwSelBoxes& rBoxes,
                    sal_uInt16 nCnt, bool bBehind );
    void PrepareDelBoxes( const SwSelBoxes& rBoxes );
    bool DeleteSel( SwDoc*, const SwSelBoxes& rBoxes, const SwSelBoxes* pMerged,
        SwUndo* pUndo, const bool bDelMakeFrames, const bool bCorrBorder );
    bool SplitCol( SwDoc* pDoc, const SwSelBoxes& rBoxes, sal_uInt16 nCnt );

    void FindSuperfluousRows( SwSelBoxes& rBoxes )
        { FindSuperfluousRows_( rBoxes, nullptr, nullptr ); }
    void CheckRowSpan( SwTableLine* &rpLine, bool bUp ) const;

          SwTableSortBoxes& GetTabSortBoxes()       { return m_TabSortContentBoxes; }
    const SwTableSortBoxes& GetTabSortBoxes() const { return m_TabSortContentBoxes; }

    // Read 1st number and delete it from string (used by GetTableBox and SwTableField).

    // #i80314#
    // add 3rd parameter in order to control validation check on <rStr>
    static sal_uInt16 GetBoxNum( OUString& rStr,
                              bool bFirst = false,
                              const bool bPerformValidCheck = false );

    // Search content-bearing box with that name.

    // #i80314#
    // add 2nd parameter in order to control validation check in called method
    // <GetBoxNum(..)>
    const SwTableBox* GetTableBox( const OUString& rName,
                                 const bool bPerformValidCheck = false ) const;
    // Copy selected boxes to another document.
    bool MakeCopy( SwDoc*, const SwPosition&, const SwSelBoxes&,
                    bool bCpyName = false ) const;
    // Copy table in this
    bool InsTable( const SwTable& rCpyTable, const SwNodeIndex&,
                    SwUndoTableCpyTable* pUndo );
    bool InsTable( const SwTable& rCpyTable, const SwSelBoxes&,
                    SwUndoTableCpyTable* pUndo );
    bool InsNewTable( const SwTable& rCpyTable, const SwSelBoxes&,
                      SwUndoTableCpyTable* pUndo );
    // Copy headline of table (with content!) into an other one.
    void CopyHeadlineIntoTable( SwTableNode& rTableNd );

    // Get box, whose start index is set on nBoxStt.
          SwTableBox* GetTableBox( sal_uLong nSttIdx );
    const SwTableBox* GetTableBox( sal_uLong nSttIdx ) const
                        {   return const_cast<SwTable*>(this)->GetTableBox( nSttIdx );  }

    // Returns true if table contains nestings.
    bool IsTableComplex() const;

    // Returns true if table or selection is balanced.
    bool IsTableComplexForChart( const OUString& rSel ) const;

    // Search all content-bearing boxes of the base line on which this box stands.
    // rBoxes as a return value for immediate use.
    // bToTop = true -> up to base line, false-> else only line of box.
    static SwSelBoxes& SelLineFromBox( const SwTableBox* pBox,
                            SwSelBoxes& rBoxes, bool bToTop = true );

    // Get information from client.
    virtual bool GetInfo( SfxPoolItem& ) const override;

    // Search in format for registered table.
    static SwTable * FindTable( SwFrameFormat const*const pFormat );

    // Clean up structure a bit.
    void GCLines();

    // Returns the table node via m_TabSortContentBoxes or pTableNode.
    SwTableNode* GetTableNode() const;
    void SetTableNode( SwTableNode* pNode ) { m_pTableNode = pNode; }

    // Data server methods.
    void SetRefObject( SwServerObject* );
    const SwServerObject* GetObject() const     {  return m_xRefObj.get(); }
          SwServerObject* GetObject()           {  return m_xRefObj.get(); }

    // Fill data for chart.
    void UpdateCharts() const;

    TableChgMode GetTableChgMode() const        { return m_eTableChgMode; }
    void SetTableChgMode( TableChgMode eMode )  { m_eTableChgMode = eMode; }

    bool SetColWidth( SwTableBox& rCurrentBox, TableChgWidthHeightType eType,
                        SwTwips nAbsDiff, SwTwips nRelDiff, SwUndo** ppUndo );
    bool SetRowHeight( SwTableBox& rCurrentBox, TableChgWidthHeightType eType,
                        SwTwips nAbsDiff, SwTwips nRelDiff, SwUndo** ppUndo );
    void RegisterToFormat( SwFormat& rFormat );
#ifdef DBG_UTIL
    void CheckConsistency() const;
#endif

    bool HasLayout() const;
};

/// SwTableLine is one table row in the document model.
class SW_DLLPUBLIC SwTableLine: public SwClient     // Client of FrameFormat.
{
    SwTableBoxes m_aBoxes;
    SwTableBox *m_pUpper;

public:

    SwTableLine( SwTableLineFormat*, sal_uInt16 nBoxes, SwTableBox *pUp );
    virtual ~SwTableLine() override;

          SwTableBoxes &GetTabBoxes() { return m_aBoxes; }
    const SwTableBoxes &GetTabBoxes() const { return m_aBoxes; }
    sal_uInt16 GetBoxPos(const SwTableBox* pBox) const
    {
        SwTableBoxes::const_iterator it = std::find(m_aBoxes.begin(), m_aBoxes.end(), pBox);
        return it == m_aBoxes.end() ? USHRT_MAX : it - m_aBoxes.begin();
    }

          SwTableBox *GetUpper() { return m_pUpper; }
    const SwTableBox *GetUpper() const { return m_pUpper; }
    void SetUpper( SwTableBox *pNew ) { m_pUpper = pNew; }

    SwFrameFormat* GetFrameFormat()       { return static_cast<SwFrameFormat*>(GetRegisteredIn()); }
    SwFrameFormat* GetFrameFormat() const { return const_cast<SwFrameFormat*>(static_cast<const SwFrameFormat*>(GetRegisteredIn())); }

    // Creates a own FrameFormat if more lines depend on it.
    SwFrameFormat* ClaimFrameFormat();
    void ChgFrameFormat( SwTableLineFormat* pNewFormat );

    // Search next/previous box with content.
    SwTableBox* FindNextBox( const SwTable&, const SwTableBox* =nullptr,
                            bool bOvrTableLns=true ) const;
    SwTableBox* FindPreviousBox( const SwTable&, const SwTableBox* =nullptr,
                            bool bOvrTableLns=true ) const;

    SwTwips GetTableLineHeight( bool& bLayoutAvailable ) const;

    bool hasSoftPageBreak() const;
    void RegisterToFormat( SwFormat& rFormat );
};

/// SwTableBox is one table cell in the document model.
class SW_DLLPUBLIC SwTableBox: public SwClient      //Client of FrameFormat.
{
    friend class SwNodes;           // Transpose index.
    friend void DelBoxNode(SwTableSortBoxes const &);  // Delete StartNode* !
    friend class SwXMLTableContext;

    SwTableBox( const SwTableBox & ) = delete;
    SwTableBox &operator=( const SwTableBox &) = delete;

    SwTableLines m_aLines;
    const SwStartNode * m_pStartNode;
    SwTableLine *m_pUpper;

    std::unique_ptr<Color> mpUserColor;
    std::unique_ptr<Color> mpNumFormatColor;
    long mnRowSpan;
    bool mbDummyFlag;

    /// Do we contain any direct formatting?
    bool mbDirectFormatting;

    // In case Format contains formulas/values already,
    // a new one must be created for the new box.
    static SwTableBoxFormat* CheckBoxFormat( SwTableBoxFormat* );

public:

    SwTableBox( SwTableBoxFormat*, sal_uInt16 nLines, SwTableLine *pUp );
    SwTableBox( SwTableBoxFormat*, const SwStartNode&, SwTableLine *pUp );
    SwTableBox( SwTableBoxFormat*, const SwNodeIndex&, SwTableLine *pUp );
    virtual ~SwTableBox() override;

          SwTableLines &GetTabLines() { return m_aLines; }
    const SwTableLines &GetTabLines() const { return m_aLines; }

          SwTableLine *GetUpper() { return m_pUpper; }
    const SwTableLine *GetUpper() const { return m_pUpper; }
    void SetUpper( SwTableLine *pNew ) { m_pUpper = pNew; }

    SwFrameFormat* GetFrameFormat()       { return static_cast<SwFrameFormat*>(GetRegisteredIn()); }
    SwFrameFormat* GetFrameFormat() const { return const_cast<SwFrameFormat*>(static_cast<const SwFrameFormat*>(GetRegisteredIn())); }

    /// Set that this table box contains formatting that is not set by the table style.
    void SetDirectFormatting(bool bDirect) { mbDirectFormatting = bDirect; }

    /// Do we contain any direct formatting (ie. something not affected by the table style)?
    bool HasDirectFormatting() const { return mbDirectFormatting; }

    // Creates its own FrameFormat if more boxes depend on it.
    SwFrameFormat* ClaimFrameFormat();
    void ChgFrameFormat( SwTableBoxFormat *pNewFormat );

    void RemoveFromTable();
    const SwStartNode *GetSttNd() const { return m_pStartNode; }
    sal_uLong GetSttIdx() const;

    // Search next/previous box with content.
    SwTableBox* FindNextBox( const SwTable&, const SwTableBox*,
                            bool bOvrTableLns=true ) const;
    SwTableBox* FindPreviousBox( const SwTable&, const SwTableBox* ) const;
    // Return name of this box. It is determined dynamically and
    // is calculated from the position in the lines/boxes/table.
    OUString GetName() const;
    // Return "value" of box (for calculating in table).
    double GetValue( SwTableCalcPara& rPara ) const;

    // Computes "coordinates" of a box, used to computed selection
    // width or height when inserting cols or rows
    Point GetCoordinates() const;

    bool IsInHeadline( const SwTable* pTable ) const;

    // Contains box contents, that can be formatted as a number?
    bool HasNumContent( double& rNum, sal_uInt32& rFormatIndex,
                    bool& rIsEmptyTextNd ) const;
    sal_uLong IsValidNumTextNd( bool bCheckAttr = true ) const;
    // If a table formula is set, test if box contents is congruent with number.
    // (For Redo of change of NumFormat!).
    bool IsNumberChanged() const;

    // Is that a formula box or a box with numeric contents (AutoSum)?
    // What it is indicated by the return value - the WhichId of the attribute.
    // Empty boxes have the return value USHRT_MAX !!
    sal_uInt16 IsFormulaOrValueBox() const;

    // Loading of a document requires an actualization of cells with values
    void ActualiseValueBox();

    DECL_FIXEDMEMPOOL_NEWDEL(SwTableBox)

    // Access on internal data - currently used for the NumFormatter.
    inline const Color* GetSaveUserColor()  const;
    inline const Color* GetSaveNumFormatColor() const;
    inline void SetSaveUserColor(const Color* p );
    inline void SetSaveNumFormatColor( const Color* p );

    long getRowSpan() const;
    void setRowSpan( long nNewRowSpan );
    bool getDummyFlag() const;
    void setDummyFlag( bool bDummy );

    SwTableBox& FindStartOfRowSpan( const SwTable&, sal_uInt16 nMaxStep = USHRT_MAX );
    const SwTableBox& FindStartOfRowSpan( const SwTable& rTable,
        sal_uInt16 nMaxStep = USHRT_MAX ) const
        { return const_cast<SwTableBox*>(this)->FindStartOfRowSpan( rTable, nMaxStep ); }

    SwTableBox& FindEndOfRowSpan( const SwTable&, sal_uInt16 nMaxStep );
    const SwTableBox& FindEndOfRowSpan( const SwTable& rTable,
        sal_uInt16 nMaxStep ) const
        { return const_cast<SwTableBox*>(this)->FindEndOfRowSpan( rTable, nMaxStep ); }
    void RegisterToFormat( SwFormat& rFormat ) ;
};

class SwCellFrame;
class SW_DLLPUBLIC SwTableCellInfo
{
    struct Impl;
    std::unique_ptr<Impl> m_pImpl;

    const SwCellFrame * getCellFrame() const;

    SwTableCellInfo(SwTableCellInfo const&) = delete;
    SwTableCellInfo& operator=(SwTableCellInfo const&) = delete;

public:
    SwTableCellInfo(const SwTable * pTable);
    ~SwTableCellInfo();

    bool getNext();
    SwRect getRect() const;
    const SwTableBox * getTableBox() const;
};

#endif // INCLUDED_SW_INC_SWTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
