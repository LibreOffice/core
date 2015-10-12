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

#include <undobj.hxx>
#include <set>
#include <swtypes.hxx>
#include <itabenum.hxx>
#include <memory>
#include <vector>
#include <boost/noncopyable.hpp>

class SfxItemSet;

struct SwSaveRowSpan;
class _SaveTable;
class SwDDEFieldType;
class SwUndoSaveSections;
class SwUndoMoves;
class SwUndoDelete;
class SwSelBoxes;
class SwTableToTextSaves;
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
    SwDDEFieldType* pDDEFieldType;
    std::vector<sal_uInt16> *pColWidth;
    SwRedlineData*  pRedlData;
    SwTableAutoFormat* pAutoFormat;
    sal_uLong nSttNode;
    sal_uInt16 nRows, nCols;
    sal_uInt16 nAdjust;

public:
    SwUndoInsTable( const SwPosition&, sal_uInt16 nCols, sal_uInt16 nRows,
                    sal_uInt16 eAdjust, const SwInsertTableOptions& rInsTableOpts,
                    const SwTableAutoFormat* pTAFormat, const std::vector<sal_uInt16> *pColArr,
                  const OUString & rName);

    virtual ~SwUndoInsTable();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    virtual SwRewriter GetRewriter() const override;
};

class SwUndoTextToTable : public SwUndo, public SwUndRng
{
    OUString sTableNm;
    SwInsertTableOptions aInsTableOpts;
    std::vector<sal_uLong>* pDelBoxes;
    SwTableAutoFormat* pAutoFormat;
    SwHistory* pHistory;
    sal_Unicode cTrenner;
    sal_uInt16 nAdjust;
    bool bSplitEnd : 1;

public:
    SwUndoTextToTable( const SwPaM&, const SwInsertTableOptions&, sal_Unicode,
                    sal_uInt16,
                    const SwTableAutoFormat* pAFormat );

    virtual ~SwUndoTextToTable();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    SwHistory& GetHistory(); // will be created if necessary
    void AddFillBox( const SwTableBox& rBox );
};

class SwUndoTableToText : public SwUndo
{
    OUString sTableNm;
    SwDDEFieldType* pDDEFieldType;
    _SaveTable* pTableSave;
    SwTableToTextSaves* m_pBoxSaves;
    SwHistory* pHistory;
    sal_uLong nSttNd, nEndNd;
    sal_Unicode cTrenner;
    sal_uInt16 nHdlnRpt;
    bool bCheckNumFormat : 1;

public:
    SwUndoTableToText( const SwTable& rTable, sal_Unicode cCh );

    virtual ~SwUndoTableToText();

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
    _SaveTable* pSaveTable;
    bool bClearTabCol : 1;

public:
    SwUndoAttrTable( const SwTableNode& rTableNd, bool bClearTabCols = false );

    virtual ~SwUndoAttrTable();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
};

class SwUndoTableNumFormat;

class SwUndoTableAutoFormat : public SwUndo
{
    sal_uLong nSttNode;
    _SaveTable* pSaveTable;
    std::vector< std::shared_ptr<SwUndoTableNumFormat> > m_Undos;
    bool bSaveContentAttr;
    sal_uInt16 m_nRepeatHeading;

    void UndoRedo(bool const bUndo, ::sw::UndoRedoContext & rContext);

public:
    SwUndoTableAutoFormat( const SwTableNode& rTableNd, const SwTableAutoFormat& );

    virtual ~SwUndoTableAutoFormat();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    void SaveBoxContent( const SwTableBox& rBox );
};

class SwUndoTableNdsChg : public SwUndo, private boost::noncopyable
{
    _SaveTable* pSaveTable;
    std::set<sal_uLong> m_Boxes;
    struct _BoxMove
    {
        sal_uLong index;    ///< Index of this box.
        bool      hasMoved; ///< Has this box been moved already.
        _BoxMove(sal_uLong idx, bool moved=false) : index(idx), hasMoved(moved) {};
        bool operator<(const _BoxMove& other) const { return index < other.index; };
    };
    std::unique_ptr< std::set<_BoxMove> > pNewSttNds;
    std::unique_ptr<SwUndoSaveSections> m_pDelSects;
    long nMin, nMax;        // for redo of delete column
    sal_uLong nSttNode, nCurrBox;
    sal_uInt16 nCount, nRelDiff, nAbsDiff, nSetColType;
    bool bFlag;
    bool bSameHeight;                   // only used for SplitRow

public:
    SwUndoTableNdsChg( SwUndoId UndoId,
                    const SwSelBoxes& rBoxes,
                    const SwTableNode& rTableNd,
                    long nMn, long nMx,
                    sal_uInt16 nCnt, bool bFlg, bool bSameHeight );

    // for SetColWidth
    SwUndoTableNdsChg( SwUndoId UndoId, const SwSelBoxes& rBoxes,
                    const SwTableNode& rTableNd );

    virtual ~SwUndoTableNdsChg();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    void SaveNewBoxes( const SwTableNode& rTableNd, const SwTableSortBoxes& rOld );
    void SaveNewBoxes( const SwTableNode& rTableNd, const SwTableSortBoxes& rOld,
                       const SwSelBoxes& rBoxes, const std::vector<sal_uLong> &rNodeCnts );
    void SaveSection( SwStartNode* pSttNd );
    void ReNewBoxes( const SwSelBoxes& rBoxes );

    void SetColWidthParam( sal_uLong nBoxIdx, sal_uInt16 nMode, sal_uInt16 nType,
                            SwTwips nAbsDif, SwTwips nRelDif )
    {
        nCurrBox = nBoxIdx;
        nCount = nMode;
        nSetColType = nType;
        nAbsDiff = (sal_uInt16)nAbsDif;
        nRelDiff = (sal_uInt16)nRelDif;
    }
};

class SwUndoTableMerge : public SwUndo, private SwUndRng
{
    sal_uLong nTableNode;
    _SaveTable* pSaveTable;
    std::set<sal_uLong> m_Boxes;
    std::vector<sal_uLong> aNewSttNds;
    SwUndoMoves* m_pMoves;
    SwHistory* pHistory;

public:
    SwUndoTableMerge( const SwPaM& rTableSel );

    virtual ~SwUndoTableMerge();

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
    SfxItemSet *pBoxSet;
    SwHistory* pHistory;
    OUString aStr, aNewFormula;

    sal_uLong nFormatIdx, nNewFormatIdx;
    double fNum, fNewNum;
    sal_uLong nNode;
    sal_uLong nNdPos;

    bool bNewFormat : 1;
    bool bNewFormula : 1;
    bool bNewValue : 1;

public:
    SwUndoTableNumFormat( const SwTableBox& rBox, const SfxItemSet* pNewSet = 0 );

    virtual ~SwUndoTableNumFormat();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    void SetNumFormat( sal_uLong nNewNumFormatIdx, const double& rNewNumber )
            { nFormatIdx = nNewNumFormatIdx; fNum = rNewNumber; }
    void SetBox( const SwTableBox& rBox );
};

class SwUndoTableCpyTable_Entries;

class SwUndoTableCpyTable : public SwUndo
{
    SwUndoTableCpyTable_Entries* m_pArr;
    SwUndoTableNdsChg* pInsRowUndo;

    //b6341295: When redlining is active, PrepareRedline has to create the
    //redlining attributes for the new and the old table cell content
    static SwUndo* PrepareRedline( SwDoc* pDoc, const SwTableBox& rBox,
                const SwPosition& rPos, bool& rJoin, bool bRedo );

public:
    SwUndoTableCpyTable();

    virtual ~SwUndoTableCpyTable();

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
    SwUndoDelete* pDel;
    sal_uLong nTableNode;

public:
    SwUndoCpyTable();

    virtual ~SwUndoCpyTable();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    void SetTableSttIdx( sal_uLong nIdx )           { nTableNode = nIdx; }
};

class SwUndoSplitTable : public SwUndo
{
    sal_uLong nTableNode, nOffset;
    SwSaveRowSpan* mpSaveRowSpan; // stores row span values at the splitting row
    _SaveTable* pSavTable;
    SwHistory* pHistory;
    sal_uInt16 nMode, nFormulaEnd;
    bool bCalcNewSize;

public:
    SwUndoSplitTable( const SwTableNode& rTableNd, SwSaveRowSpan* pRowSp,
            sal_uInt16 nMode, bool bCalcNewSize );

    virtual ~SwUndoSplitTable();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    void SetTableNodeOffset( sal_uLong nIdx )     { nOffset = nIdx - nTableNode; }
    SwHistory* GetHistory()                 { return pHistory; }
    void SaveFormula( SwHistory& rHistory );
};

class SwUndoMergeTable : public SwUndo
{
    OUString aName;
    sal_uLong nTableNode;
    _SaveTable* pSavTable, *pSavHdl;
    SwHistory* pHistory;
    sal_uInt16 nMode;
    bool bWithPrev;

public:
    SwUndoMergeTable( const SwTableNode& rTableNd, const SwTableNode& rDelTableNd,
                    bool bWithPrev, sal_uInt16 nMode );

    virtual ~SwUndoMergeTable();

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

void InsertSort( std::vector<sal_uInt16>& rArr, sal_uInt16 nIdx, sal_uInt16* pInsPos = 0 );

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDOTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
