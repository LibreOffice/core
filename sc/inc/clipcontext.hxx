/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_CLIPCONTEXT_HXX
#define INCLUDED_SC_INC_CLIPCONTEXT_HXX

#include "address.hxx"
#include "cellvalue.hxx"
#include <celltextattr.hxx>

#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

class ScDocument;
class ScColumn;
class ScPatternAttr;
class ScPostIt;
class ScConditionalFormatList;

namespace sc {

struct ColumnBlockPosition;
class ColumnBlockPositionSet;

class ClipContextBase : boost::noncopyable
{
    boost::scoped_ptr<ColumnBlockPositionSet> mpSet;

    ClipContextBase(); // disabled

public:
    ClipContextBase(ScDocument& rDoc);
    virtual ~ClipContextBase();

    ColumnBlockPosition* getBlockPosition(SCTAB nTab, SCCOL nCol);
};

class CopyFromClipContext : public ClipContextBase
{
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

    ScConditionalFormatList* mpCondFormatList;
    bool mbAsLink:1;
    bool mbSkipAttrForEmptyCells:1;
    bool mbCloneNotes:1;
    bool mbTableProtected:1;

    CopyFromClipContext(); // disabled

public:

    struct Range
    {
        SCCOL mnCol1;
        SCCOL mnCol2;
        SCROW mnRow1;
        SCROW mnRow2;
    };

    CopyFromClipContext(ScDocument& rDoc,
        ScDocument* pRefUndoDoc, ScDocument* pClipDoc, InsertDeleteFlags nInsertFlag,
        bool bAsLink, bool bSkipAttrForEmptyCells);

    virtual ~CopyFromClipContext();

    void setTabRange(SCTAB nStart, SCTAB nEnd);

    SCTAB getTabStart() const;
    SCTAB getTabEnd() const;

    void setDestRange( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
    Range getDestRange() const;

    ScDocument* getUndoDoc();
    ScDocument* getClipDoc();
    InsertDeleteFlags getInsertFlag() const;

    void setDeleteFlag( InsertDeleteFlags nFlag );
    InsertDeleteFlags getDeleteFlag() const;

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

    void setCondFormatList( ScConditionalFormatList* pCondFormatList );
    ScConditionalFormatList* getCondFormatList();

    void setTableProtected( bool b );
    bool isTableProtected() const;

    bool isAsLink() const;
    bool isSkipAttrForEmptyCells() const;
    bool isCloneNotes() const;
    bool isDateCell( const ScColumn& rCol, SCROW nRow ) const;
};

class CopyToClipContext : public ClipContextBase
{
    bool mbKeepScenarioFlags:1;
    bool mbCloneNotes:1;

    CopyToClipContext(); // disabled

public:
    CopyToClipContext(ScDocument& rDoc, bool bKeepScenarioFlags, bool bCloneNotes);
    virtual ~CopyToClipContext();

    bool isKeepScenarioFlags() const;
    bool isCloneNotes() const;
};

class CopyToDocContext : public ClipContextBase
{
    bool mbStartListening;

public:
    CopyToDocContext(ScDocument& rDoc);
    virtual ~CopyToDocContext();

    void setStartListening( bool b );
    bool isStartListening() const;
};

class MixDocContext : public ClipContextBase
{
public:
    MixDocContext(ScDocument& rDoc);
    virtual ~MixDocContext();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
