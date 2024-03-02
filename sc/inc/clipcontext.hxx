/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "address.hxx"
#include "cellvalue.hxx"
#include "celltextattr.hxx"
#include "columnspanset.hxx"
#include "Sparkline.hxx"

#include <memory>
#include <vector>

class ScDocument;
class ScColumn;
class ScPatternAttr;
class ScPostIt;
class ScConditionalFormatList;

namespace sc {

struct ColumnBlockPosition;
class ColumnBlockPositionSet;

class ClipContextBase
{
    std::unique_ptr<ColumnBlockPositionSet> mpSet;

public:
    ClipContextBase() = delete;
    ClipContextBase(const ClipContextBase&) = delete;
    const ClipContextBase& operator=(const ClipContextBase&) = delete;
    ClipContextBase(ScDocument& rDoc);
    virtual ~ClipContextBase();

    ColumnBlockPosition* getBlockPosition(SCTAB nTab, SCCOL nCol);
    ColumnBlockPositionSet* getBlockPositionSet() { return mpSet.get(); }
};

class SAL_DLLPUBLIC_RTTI CopyFromClipContext final : public ClipContextBase
{
    /** Tracks modified formula group spans. */
    sc::ColumnSpanSet maListeningFormulaSpans;

    SCCOL mnDestCol1;
    SCCOL mnDestCol2;
    SCROW mnDestRow1;
    SCROW mnDestRow2;
    SCTAB mnTabStart;
    SCTAB mnTabEnd;
    ScDocument& mrDestDoc;
    ScDocument* mpRefUndoDoc;
    ScDocument* mpClipDoc;
    InsertDeleteFlags mnInsertFlag;
    InsertDeleteFlags mnDeleteFlag;

    std::vector<ScCellValue> maSingleCells;
    std::vector<sc::CellTextAttr> maSingleCellAttrs;
    std::vector<const ScPatternAttr*> maSinglePatterns;
    std::vector<const ScPostIt*> maSingleNotes;
    std::vector<std::shared_ptr<sc::Sparkline>> maSingleSparkline;

    ScConditionalFormatList* mpCondFormatList;
    bool mbAsLink:1;
    bool mbSkipEmptyCells:1;
    bool mbTableProtected:1;

public:

    struct Range
    {
        SCCOL mnCol1;
        SCCOL mnCol2;
        SCROW mnRow1;
        SCROW mnRow2;
    };

    CopyFromClipContext() = delete;
    SC_DLLPUBLIC CopyFromClipContext(ScDocument& rDoc,
        ScDocument* pRefUndoDoc, ScDocument* pClipDoc, InsertDeleteFlags nInsertFlag,
        bool bAsLink, bool bSkipAttrForEmptyCells);

    SC_DLLPUBLIC virtual ~CopyFromClipContext() override;

    SC_DLLPUBLIC void setTabRange(SCTAB nStart, SCTAB nEnd);

    SCTAB getTabStart() const;
    SCTAB getTabEnd() const;

    SC_DLLPUBLIC void setDestRange( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
    Range getDestRange() const;

    ScDocument* getUndoDoc();
    ScDocument* getClipDoc();
    ScDocument* getDestDoc() { return &mrDestDoc; }
    InsertDeleteFlags getInsertFlag() const;

    void setDeleteFlag( InsertDeleteFlags nFlag );
    InsertDeleteFlags getDeleteFlag() const;

    /**
     * Record a range of formula cells that need to start listening after the
     * copy-from-clip is complete.
     */
    void setListeningFormulaSpans( SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );

    /**
     * Have the formula cells in the recorded ranges start listening.
     */
    void startListeningFormulas();

    /**
     * Set the column size of a "single cell" row, which is used when copying
     * a single row of cells in a clip doc and pasting it into multiple
     * rows by replicating it.
     */
    void setSingleCellColumnSize( size_t nSize );

    ScCellValue& getSingleCell( size_t nColOffset );
    sc::CellTextAttr& getSingleCellAttr( size_t nColOffset );

    void setSingleCell( const ScAddress& rSrcPos, const ScColumn& rSrcCol );


    const ScPatternAttr* getSingleCellPattern( size_t nColOffset ) const;
    void setSingleCellPattern( size_t nColOffset, const ScPatternAttr* pAttr );

    const ScPostIt* getSingleCellNote( size_t nColOffset ) const;
    void setSingleCellNote( size_t nColOffset, const ScPostIt* pNote );

    std::shared_ptr<sc::Sparkline> const& getSingleSparkline(size_t nColOffset) const;
    void setSingleSparkline(size_t nColOffset, std::shared_ptr<sc::Sparkline> const& pSparkline);

    void setCondFormatList( ScConditionalFormatList* pCondFormatList );
    ScConditionalFormatList* getCondFormatList();

    void setTableProtected( bool b );
    bool isTableProtected() const;

    bool isAsLink() const;

    /**
     * Get the flag that indicates whether the "skip empty cells" paste option
     * is selected. When this option is selected, empty cells in the clipboard
     * document will not overwrite the corresponding non-empty cells in the
     * destination range.
     */
    bool isSkipEmptyCells() const;
    bool isCloneNotes() const;
    bool isCloneSparklines() const;
    bool isDateCell( const ScColumn& rCol, SCROW nRow ) const;
};

class CopyToClipContext final : public ClipContextBase
{
    bool mbKeepScenarioFlags:1;
    bool mbCopyChartRanges : 1 = false; // Copying ranges not included in selection:
                                        // only copy data, not cell attributes

public:
    CopyToClipContext(ScDocument& rDoc, bool bKeepScenarioFlags, bool bCopyChartRanges = false);
    virtual ~CopyToClipContext() override;

    bool isKeepScenarioFlags() const;
    bool isCopyChartRanges() const { return mbCopyChartRanges; }
};

class CopyToDocContext final : public ClipContextBase
{
    bool mbStartListening;

public:
    CopyToDocContext(ScDocument& rDoc);
    virtual ~CopyToDocContext() override;

    void setStartListening( bool b );
    bool isStartListening() const;
};

class MixDocContext final : public ClipContextBase
{
public:
    MixDocContext(ScDocument& rDoc);
    virtual ~MixDocContext() override;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
