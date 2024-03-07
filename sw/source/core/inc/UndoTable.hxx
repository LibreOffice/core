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

#pragma once

#include <o3tl/deleter.hxx>
#include <tools/long.hxx>
#include <tools/solar.h>
#include <ndarr.hxx>
#include <undobj.hxx>
#include <set>
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
class SwContentNode;
class SwCursorShell;

namespace sw {

void NotifyTableCollapsedParagraph(const SwContentNode* pNode, SwCursorShell *const pShell);

}

/// Implements undo/redo for Table -> Insert Table.
class SwUndoInsTable final : public SwUndo
{
    OUString m_sTableName;
    SwInsertTableOptions m_aInsTableOptions;
    std::unique_ptr<SwDDEFieldType> m_pDDEFieldType;
    std::optional<std::vector<sal_uInt16>> m_oColumnWidth;
    std::unique_ptr<SwRedlineData>  m_pRedlineData;
    std::unique_ptr<SwTableAutoFormat> m_pAutoFormat;
    SwNodeOffset m_nStartNode;
    sal_uInt16 m_nRows, m_nColumns;
    sal_uInt16 const m_nAdjust;

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

class SwUndoTextToTable final : public SwUndo, public SwUndRng
{
    OUString m_sTableName;
    SwInsertTableOptions m_aInsertTableOpts;
    std::vector<SwNodeOffset> mvDelBoxes;
    std::unique_ptr<SwTableAutoFormat> m_pAutoFormat;
    SwHistory* m_pHistory;
    sal_Unicode m_cSeparator;
    sal_uInt16 m_nAdjust;
    bool m_bSplitEnd : 1;

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

class SwUndoTableToText final : public SwUndo
{
    OUString m_sTableName;
    std::unique_ptr<SwDDEFieldType> m_pDDEFieldType;
    std::unique_ptr<SaveTable> m_pTableSave;
    SwTableToTextSaves m_vBoxSaves;
    std::unique_ptr<SwHistory> m_pHistory;
    SwNodeOffset m_nStartNode, m_nEndNode;
    sal_Unicode m_cSeparator;
    sal_uInt16 m_nHeadlineRepeat;
    bool m_bCheckNumFormat : 1;

public:
    SwUndoTableToText( const SwTable& rTable, sal_Unicode cCh );

    virtual ~SwUndoTableToText() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    void SetRange( const SwNodeRange& );
    void AddBoxPos( SwDoc& rDoc, SwNodeOffset nNdIdx, SwNodeOffset nEndIdx,
                    sal_Int32 nContentIdx = SAL_MAX_INT32);
};

class SwUndoAttrTable final : public SwUndo
{
    SwNodeOffset m_nStartNode;
    std::unique_ptr<SaveTable> m_pSaveTable;
    bool m_bClearTableCol : 1;

public:
    SwUndoAttrTable( const SwTableNode& rTableNd, bool bClearTabCols = false );

    virtual ~SwUndoAttrTable() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
};

class SwUndoTableNumFormat;

class SwUndoTableAutoFormat final : public SwUndo
{
    OUString m_TableStyleName;
    SwNodeOffset m_nStartNode;
    std::unique_ptr<SaveTable> m_pSaveTable;
    std::vector< std::shared_ptr<SwUndoTableNumFormat> > m_Undos;
    bool m_bSaveContentAttr;
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

class SwUndoTableNdsChg final : public SwUndo
{
    std::unique_ptr<SaveTable> m_pSaveTable;
    std::set<SwNodeOffset> m_Boxes;
    struct BoxMove
    {
        SwNodeOffset index;    ///< Index of this box.
        bool      hasMoved; ///< Has this box been moved already.
        BoxMove(SwNodeOffset idx, bool moved=false) : index(idx), hasMoved(moved) {};
        bool operator<(const BoxMove& other) const { return index < other.index; };
    };
    std::optional< std::set<BoxMove> > m_xNewSttNds;
    std::unique_ptr<SwUndoSaveSections> m_pDelSects;
    tools::Long m_nMin, m_nMax;        // for redo of delete column
    SwNodeOffset m_nSttNode;
    sal_uInt16 m_nCount;
    bool m_bFlag;
    bool m_bSameHeight;                   // only used for SplitRow

    SwUndoTableNdsChg(SwUndoTableNdsChg const&) = delete;
    SwUndoTableNdsChg& operator=(SwUndoTableNdsChg const&) = delete;

public:
    SwUndoTableNdsChg( SwUndoId UndoId,
                    const SwSelBoxes& rBoxes,
                    const SwTableNode& rTableNd,
                    tools::Long nMn, tools::Long nMx,
                    sal_uInt16 nCnt, bool bFlg, bool bSameHeight );

    virtual ~SwUndoTableNdsChg() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    void SaveNewBoxes( const SwTableNode& rTableNd, const SwTableSortBoxes& rOld );
    void SaveNewBoxes( const SwTableNode& rTableNd, const SwTableSortBoxes& rOld,
                       const SwSelBoxes& rBoxes, const std::vector<SwNodeOffset> &rNodeCnts );
    void SaveSection( SwStartNode* pSttNd );
    void ReNewBoxes( const SwSelBoxes& rBoxes );

};

class SwUndoMove;

class SwUndoTableMerge final : public SwUndo, private SwUndRng
{
    SwNodeOffset m_nTableNode;
    std::unique_ptr<SaveTable> m_pSaveTable;
    std::set<SwNodeOffset> m_Boxes;
    std::vector<SwNodeOffset> m_aNewStartNodes;
    std::vector<std::unique_ptr<SwUndoMove>> m_vMoves;
    std::unique_ptr<SwHistory> m_pHistory;

public:
    SwUndoTableMerge( const SwPaM& rTableSel );

    virtual ~SwUndoTableMerge() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    void MoveBoxContent( SwDoc& rDoc, SwNodeRange& rRg, SwNode& rPos );

    void SetSelBoxes( const SwSelBoxes& rBoxes );

    void AddNewBox( SwNodeOffset nSttNdIdx )
        { m_aNewStartNodes.push_back( nSttNdIdx ); }

    void SaveCollection( const SwTableBox& rBox );
};

class SwUndoTableNumFormat final : public SwUndo
{
    std::unique_ptr<SfxItemSet> m_pBoxSet;
    std::unique_ptr<SwHistory> m_pHistory;
    OUString m_aStr, m_aNewFormula;

    sal_uLong m_nFormatIdx, m_nNewFormatIdx;
    double m_fNum, m_fNewNum;
    SwNodeOffset m_nNode;
    SwNodeOffset m_nNodePos;

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

class SwUndoTableCpyTable final : public SwUndo
{
    std::vector<std::unique_ptr<UndoTableCpyTable_Entry>> m_vArr;
    std::unique_ptr<SwUndoTableNdsChg> m_pInsRowUndo;

    //b6341295: When redlining is active, PrepareRedline has to create the
    //redlining attributes for the new and the old table cell content
    static std::unique_ptr<SwUndo> PrepareRedline( SwDoc* pDoc, const SwTableBox& rBox,
                SwPosition& rPos, bool& rJoin, bool bRedo );

public:
    SwUndoTableCpyTable(const SwDoc& rDoc);

    virtual ~SwUndoTableCpyTable() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    void AddBoxBefore( const SwTableBox& rBox, bool bDelContent );
    void AddBoxAfter( const SwTableBox& rBox, const SwNodeIndex& rIdx,
                bool bDelContent );

    bool IsEmpty() const;
    bool InsertRow( SwTable& rTable, const SwSelBoxes& rBoxes, sal_uInt16 nCnt );
    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

class SwUndoCpyTable final : public SwUndo
{
    std::unique_ptr<SwUndoDelete> m_pDelete;
    SwNodeOffset m_nTableNode;

public:
    SwUndoCpyTable(const SwDoc& rDoc);

    virtual ~SwUndoCpyTable() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    void SetTableSttIdx( SwNodeOffset nIdx )           { m_nTableNode = nIdx; }
};

class SwUndoSplitTable final : public SwUndo
{
    SwNodeOffset m_nTableNode, m_nOffset;
    std::unique_ptr<SwSaveRowSpan> mpSaveRowSpan; // stores row span values at the splitting row
    std::unique_ptr<SaveTable> m_pSavedTable;
    std::unique_ptr<SwHistory> m_pHistory;
    SplitTable_HeadlineOption const m_nMode;
    sal_uInt16 m_nFormulaEnd;
    bool m_bCalcNewSize;

public:
    SwUndoSplitTable( const SwTableNode& rTableNd, std::unique_ptr<SwSaveRowSpan> pRowSp,
            SplitTable_HeadlineOption nMode, bool bCalcNewSize );

    virtual ~SwUndoSplitTable() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    void SetTableNodeOffset( SwNodeOffset nIdx )     { m_nOffset = nIdx - m_nTableNode; }
    SwHistory* GetHistory()                 { return m_pHistory.get(); }
    void SaveFormula( SwHistory& rHistory );
};

class SwUndoMergeTable final : public SwUndo
{
    OUString m_aName;
    SwNodeOffset m_nTableNode;
    std::unique_ptr<SaveTable> m_pSaveTable, m_pSaveHdl;
    std::unique_ptr<SwHistory> m_pHistory;
    bool m_bWithPrev;

public:
    SwUndoMergeTable( const SwTableNode& rTableNd, const SwTableNode& rDelTableNd,
                    bool bWithPrev );

    virtual ~SwUndoMergeTable() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    void SaveFormula( SwHistory& rHistory );
};

class SwUndoTableHeadline final : public SwUndo
{
    SwNodeOffset m_nTableNode;
    sal_uInt16 m_nOldHeadline;
    sal_uInt16 m_nNewHeadline;

public:
    SwUndoTableHeadline( const SwTable&, sal_uInt16 nOldHdl,  sal_uInt16 nNewHdl );

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;
};

void InsertSort( std::vector<sal_uInt16>& rArr, sal_uInt16 nIdx );

class SwUndoTableStyleMake final : public SwUndo
{
    OUString m_sName;
    std::unique_ptr<SwTableAutoFormat> m_pAutoFormat;
public:
    SwUndoTableStyleMake(OUString aName, const SwDoc& rDoc);

    virtual ~SwUndoTableStyleMake() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    virtual SwRewriter GetRewriter() const override;
};

class SwUndoTableStyleDelete final : public SwUndo
{
    std::unique_ptr<SwTableAutoFormat> m_pAutoFormat;
    std::vector<SwTable*> m_rAffectedTables;
public:
    SwUndoTableStyleDelete(std::unique_ptr<SwTableAutoFormat> pAutoFormat, std::vector<SwTable*>&& rAffectedTables, const SwDoc& rDoc);

    virtual ~SwUndoTableStyleDelete() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    virtual SwRewriter GetRewriter() const override;
};

class SwUndoTableStyleUpdate final : public SwUndo
{
    std::unique_ptr<SwTableAutoFormat> m_pOldFormat, m_pNewFormat;
public:
    SwUndoTableStyleUpdate(const SwTableAutoFormat& rNewFormat, const SwTableAutoFormat& rOldFormat, const SwDoc& rDoc);

    virtual ~SwUndoTableStyleUpdate() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    virtual SwRewriter GetRewriter() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
