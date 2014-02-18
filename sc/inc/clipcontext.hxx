/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_CLIPCONTEXT_HXX
#define SC_CLIPCONTEXT_HXX

#include "address.hxx"
#include <cellvalue.hxx>

#include <vector>
#include <boost/unordered_map.hpp>
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
    ScDocument* mpRefUndoDoc;
    ScDocument* mpClipDoc;
    sal_uInt16 mnInsertFlag;
    sal_uInt16 mnDeleteFlag;
    ScCellValue maSingleCell;
    ScConditionalFormatList* mpCondFormatList;
    const ScPatternAttr* mpSinglePattern;
    const ScPostIt* mpSingleNote;
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
        ScDocument* pRefUndoDoc, ScDocument* pClipDoc, sal_uInt16 nInsertFlag,
        bool bAsLink, bool bSkipAttrForEmptyCells);

    virtual ~CopyFromClipContext();

    void setTabRange(SCTAB nStart, SCTAB nEnd);

    SCTAB getTabStart() const;
    SCTAB getTabEnd() const;

    void setDestRange( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
    Range getDestRange() const;

    ScDocument* getUndoDoc();
    ScDocument* getClipDoc();
    sal_uInt16 getInsertFlag() const;

    void setDeleteFlag( sal_uInt16 nFlag );
    sal_uInt16 getDeleteFlag() const;

    ScCellValue& getSingleCell();

    void setCondFormatList( ScConditionalFormatList* pCondFormatList );
    ScConditionalFormatList* getCondFormatList();

    const ScPatternAttr* getSingleCellPattern() const;
    void setSingleCellPattern( const ScPatternAttr* pAttr );

    const ScPostIt* getSingleCellNote() const;
    void setSingleCellNote( const ScPostIt* pNote );

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
public:
    CopyToDocContext(ScDocument& rDoc);
    virtual ~CopyToDocContext();
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
