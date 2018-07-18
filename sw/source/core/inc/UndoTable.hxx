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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNDOTABLE_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNDOTABLE_HXX

#include <o3tl/deleter.hxx>
#include <ndarr.hxx>
#include <undobj.hxx>
#include <set>
#include <swtypes.hxx>
#include <itabenum.hxx>
#include <tblenum.hxx>
#include <memory>
#include <vector>

class SfxItemSet;

struct SwSaveRowSpan;
class SaveTable;
class SwDDEFieldType;
class SwUndoDelete;
class SwSelBoxes;
class SwTable;
class SwTableBox;
class SwStartNode;
class SwTableNode;
class SwTableAutoFormat;
class SwTableSortBoxes;

class SwUndoInsTable : public SwUndo
{
    OUString sTableNm;
    SwInsertTableOptions aInsTableOpts;
    std::unique_ptr<SwDDEFieldType> pDDEFieldType;
    std::unique_ptr<std::vector<sal_uInt16>> pColWidth;
    std::unique_ptr<SwRedlineData>  pRedlData;
    std::unique_ptr<SwTableAutoFormat> pAutoFormat;
    sal_uLong nSttNode;
    sal_uInt16 nRows, nCols;
    sal_uInt16 nAdjust;

public:
    SwUndoInsTable( const SwPosition&, sal_uInt16 nCols, sal_uInt16 nRows,
                    sal_uInt16 eAdjust, const SwInsertTableOptions& rInsTableOpts,
                    const SwTableAutoFormat* pTAFormat, const std::vector<sal_uInt16> *pColArr,
                  const OUString & rName);

    virtual ~SwUndoInsTable() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    virtual SwRewriter GetRewriter() const override;
};

class SwUndoTextToTable : public SwUndo, public SwUndRng
{
    OUString sTableNm;
    SwInsertTableOptions aInsTableOpts;
    std::vector<sal_uLong> mvDelBoxes;
    std::unique_ptr<SwTableAutoFormat> pAutoFormat;
    SwHistory* pHistory;
    sal_Unicode cTrenner;
    sal_uInt16 nAdjust;
    bool bSplitEnd : 1;

public:
    SwUndoTextToTable( const SwPaM&, const SwInsertTableOptions&, sal_Unicode,
                    sal_uInt16,
                    const SwTableAutoFormat* pAFormat );

    virtual ~SwUndoTextToTable() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    SwHistory& GetHistory(); // will be created if necessary
    void AddFillBox( const SwTableBox& rBox );
};

class SwUndoTableToText : public SwUndo
{
    OUString sTableNm;
    std::unique_ptr<SwDDEFieldType> pDDEFieldType;
    std::unique_ptr<SaveTable> pTableSave;
    SwTableToTextSaves m_vBoxSaves;
    std::unique_ptr<SwHistory> pHistory;
    sal_uLong nSttNd, nEndNd;
    sal_Unicode cTrenner;
    sal_uInt16 nHdlnRpt;
    bool bCheckNumFormat : 1;

public:
    SwUndoTableToText( const SwTable& rTable, sal_Unicode cCh );

    virtual ~SwUndoTableToText() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    void SetRange( const SwNodeRange& );
    void AddBoxPos( SwDoc& rDoc, sal_uLong nNdIdx, sal_uLong nEndIdx,
                    sal_Int32 nContentIdx = SAL_MAX_INT32);
};

class SwUndoAttrTable : public SwUndo
{
    sal_uLong nSttNode;
    std::unique_ptr<SaveTable> pSaveTable;
    bool bClearTabCol : 1;

public:
    SwUndoAttrTable( const SwTableNode& rTableNd, bool bClearTabCols = false );

    virtual ~SwUndoAttrTable() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
};

class SwUndoTableNumFormat;

class SwUndoTableAutoFormat : public SwUndo
{
    OUString m_TableStyleName;
    sal_uLong nSttNode;
    std::unique_ptr<SaveTable> pSaveTable;
    std::vector< std::shared_ptr<SwUndoTableNumFormat> > m_Undos;
    bool bSaveContentAttr;
    sal_uInt16 m_nRepeatHeading;

    void UndoRedo(bool const bUndo, ::sw::UndoRedoContext & rContext);

public:
    SwUndoTableAutoFormat( const SwTableNode& rTableNd, const SwTableAutoFormat& );

    virtual ~SwUndoTableAutoFormat() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    void SaveBoxContent( const SwTableBox& rBox );
};

using SwUndoSaveSections = std::vector<std::unique_ptr<SwUndoSaveSection, o3tl::default_delete<SwUndoSaveSection>>>;

class SwUndoTableNdsChg : public SwUndo
{
    std::unique_ptr<SaveTable> pSaveTable;
    std::set<sal_uLong> m_Boxes;
    struct BoxMove
    {
        sal_uLong index;    ///< Index of this box.
        bool      hasMoved; ///< Has this box been moved already.
        BoxMove(sal_uLong idx, bool moved=false) : index(idx), hasMoved(moved) {};
        bool operator<(const BoxMove& other) const { return index < other.index; };
    };
    std::unique_ptr< std::set<BoxMove> > pNewSttNds;
    std::unique_ptr<SwUndoSaveSections> m_pDelSects;
    long nMin, nMax;        // for redo of delete column
    sal_uLong nSttNode, nCurrBox;
    sal_uInt16 nCount, nRelDiff, nAbsDiff;
    TableChgWidthHeightType nSetColType;
    bool bFlag;
    bool bSameHeight;                   // only used for SplitRow

    SwUndoTableNdsChg(SwUndoTableNdsChg const&) = delete;
    SwUndoTableNdsChg& operator=(SwUndoTableNdsChg const&) = delete;

public:
    SwUndoTableNdsChg( SwUndoId UndoId,
                    const SwSelBoxes& rBoxes,
                    const SwTableNode& rTableNd,
                    long nMn, long nMx,
                    sal_uInt16 nCnt, bool bFlg, bool bSameHeight );

    // for SetColWidth
    SwUndoTableNdsChg( SwUndoId UndoId, const SwSelBoxes& rBoxes,
                    const SwTableNode& rTableNd );

    virtual ~SwUndoTableNdsChg() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    void SaveNewBoxes( const SwTableNode& rTableNd, const SwTableSortBoxes& rOld );
    void SaveNewBoxes( const SwTableNode& rTableNd, const SwTableSortBoxes& rOld,
                       const SwSelBoxes& rBoxes, const std::vector<sal_uLong> &rNodeCnts );
    void SaveSection( SwStartNode* pSttNd );
    void ReNewBoxes( const SwSelBoxes& rBoxes );

    void SetColWidthParam( sal_uLong nBoxIdx, sal_uInt16 nMode, TableChgWidthHeightType nType,
                            SwTwips nAbsDif, SwTwips nRelDif )
    {
        nCurrBox = nBoxIdx;
        nCount = nMode;
        nSetColType = nType;
        nAbsDiff = static_cast<sal_uInt16>(nAbsDif);
        nRelDiff = static_cast<sal_uInt16>(nRelDif);
    }
};

class SwUndoMove;

class SwUndoTableMerge : public SwUndo, private SwUndRng
{
    sal_uLong nTableNode;
    std::unique_ptr<SaveTable> pSaveTable;
    std::set<sal_uLong> m_Boxes;
    std::vector<sal_uLong> aNewSttNds;
    std::vector<std::unique_ptr<SwUndoMove>> m_vMoves;
    std::unique_ptr<SwHistory> pHistory;

public:
    SwUndoTableMerge( const SwPaM& rTableSel );

    virtual ~SwUndoTableMerge() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    void MoveBoxContent( SwDoc* pDoc, SwNodeRange& rRg, SwNodeIndex& rPos );

    void SetSelBoxes( const SwSelBoxes& rBoxes );

    void AddNewBox( sal_uLong nSttNdIdx )
        { aNewSttNds.push_back( nSttNdIdx ); }

    void SaveCollection( const SwTableBox& rBox );
};

class SwUndoTableNumFormat : public SwUndo
{
    std::unique_ptr<SfxItemSet> m_pBoxSet;
    std::unique_ptr<SwHistory> m_pHistory;
    OUString m_aStr, m_aNewFormula;

    sal_uLong m_nFormatIdx, m_nNewFormatIdx;
    double m_fNum, m_fNewNum;
    sal_uLong m_nNode;
    sal_uLong m_nNodePos;

    bool m_bNewFormat : 1;
    bool m_bNewFormula : 1;
    bool m_bNewValue : 1;

public:
    SwUndoTableNumFormat( const SwTableBox& rBox, const SfxItemSet* pNewSet = nullptr );

    virtual ~SwUndoTableNumFormat() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    void SetNumFormat( sal_uLong nNewNumFormatIdx, const double& rNewNumber )
            { m_nFormatIdx = nNewNumFormatIdx; m_fNum = rNewNumber; }
    void SetBox( const SwTableBox& rBox );
};

struct UndoTableCpyTable_Entry;

class SwUndoTableCpyTable : public SwUndo
{
    std::vector<std::unique_ptr<UndoTableCpyTable_Entry>> m_vArr;
    std::unique_ptr<SwUndoTableNdsChg> pInsRowUndo;

    //b6341295: When redlining is active, PrepareRedline has to create the
    //redlining attributes for the new and the old table cell content
    static std::unique_ptr<SwUndo> PrepareRedline( SwDoc* pDoc, const SwTableBox& rBox,
                const SwPosition& rPos, bool& rJoin, bool bRedo );

public:
    SwUndoTableCpyTable(const SwDoc* pDoc);

    virtual ~SwUndoTableCpyTable() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    void AddBoxBefore( const SwTableBox& rBox, bool bDelContent );
    void AddBoxAfter( const SwTableBox& rBox, const SwNodeIndex& rIdx,
                bool bDelContent );

    bool IsEmpty() const;
    bool InsertRow( SwTable& rTable, const SwSelBoxes& rBoxes, sal_uInt16 nCnt );
};

class SwUndoCpyTable : public SwUndo
{
    std::unique_ptr<SwUndoDelete> pDel;
    sal_uLong nTableNode;

public:
    SwUndoCpyTable(const SwDoc* pDoc);

    virtual ~SwUndoCpyTable() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    void SetTableSttIdx( sal_uLong nIdx )           { nTableNode = nIdx; }
};

class SwUndoSplitTable : public SwUndo
{
    sal_uLong nTableNode, nOffset;
    std::unique_ptr<SwSaveRowSpan> mpSaveRowSpan; // stores row span values at the splitting row
    std::unique_ptr<SaveTable> pSavTable;
    std::unique_ptr<SwHistory> pHistory;
    SplitTable_HeadlineOption nMode;
    sal_uInt16 nFormulaEnd;
    bool bCalcNewSize;

public:
    SwUndoSplitTable( const SwTableNode& rTableNd, std::unique_ptr<SwSaveRowSpan> pRowSp,
            SplitTable_HeadlineOption nMode, bool bCalcNewSize );

    virtual ~SwUndoSplitTable() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    void SetTableNodeOffset( sal_uLong nIdx )     { nOffset = nIdx - nTableNode; }
    SwHistory* GetHistory()                 { return pHistory.get(); }
    void SaveFormula( SwHistory& rHistory );
};

class SwUndoMergeTable : public SwUndo
{
    OUString aName;
    sal_uLong nTableNode;
    SaveTable* pSavTable, *pSavHdl;
    SwHistory* pHistory;
    sal_uInt16 nMode;
    bool bWithPrev;

public:
    SwUndoMergeTable( const SwTableNode& rTableNd, const SwTableNode& rDelTableNd,
                    bool bWithPrev, sal_uInt16 nMode );

    virtual ~SwUndoMergeTable() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    void SaveFormula( SwHistory& rHistory );
};

class SwUndoTableHeadline : public SwUndo
{
    sal_uLong nTableNd;
    sal_uInt16 nOldHeadline;
    sal_uInt16 nNewHeadline;

public:
    SwUndoTableHeadline( const SwTable&, sal_uInt16 nOldHdl,  sal_uInt16 nNewHdl );

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;
};

void InsertSort( std::vector<sal_uInt16>& rArr, sal_uInt16 nIdx );

class SwUndoTableStyleMake : public SwUndo
{
    OUString m_sName;
    std::unique_ptr<SwTableAutoFormat> m_pAutoFormat;
public:
    SwUndoTableStyleMake(const OUString& rName, const SwDoc* pDoc);

    virtual ~SwUndoTableStyleMake() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    virtual SwRewriter GetRewriter() const override;
};

class SwUndoTableStyleDelete : public SwUndo
{
    std::unique_ptr<SwTableAutoFormat> m_pAutoFormat;
    std::vector<SwTable*> m_rAffectedTables;
public:
    SwUndoTableStyleDelete(std::unique_ptr<SwTableAutoFormat> pAutoFormat, const std::vector<SwTable*>& rAffectedTables, const SwDoc* pDoc);

    virtual ~SwUndoTableStyleDelete() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    virtual SwRewriter GetRewriter() const override;
};

class SwUndoTableStyleUpdate : public SwUndo
{
    std::unique_ptr<SwTableAutoFormat> m_pOldFormat, m_pNewFormat;
public:
    SwUndoTableStyleUpdate(const SwTableAutoFormat& rNewFormat, const SwTableAutoFormat& rOldFormat, const SwDoc* pDoc);

    virtual ~SwUndoTableStyleUpdate() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    virtual SwRewriter GetRewriter() const override;
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDOTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
