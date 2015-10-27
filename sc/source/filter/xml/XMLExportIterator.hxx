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

#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLEXPORTITERATOR_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLEXPORTITERATOR_HXX

#include <vector>
#include <list>
#include <set>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/sheet/XSheetAnnotation.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include "global.hxx"
#include "detfunc.hxx"
#include "detdata.hxx"
#include "postit.hxx"
#include "cellvalue.hxx"

#include <boost/noncopyable.hpp>
#include <memory>

class   ScHorizontalCellIterator;
struct  ScMyCell;
class   ScXMLExport;
class   ScFormatRangeStyles;

class ScMyIteratorBase
{
protected:
    virtual bool                GetFirstAddress( css::table::CellAddress& rCellAddress ) = 0;

public:
                                ScMyIteratorBase();
    virtual                     ~ScMyIteratorBase();

    virtual void                SetCellData( ScMyCell& rMyCell ) = 0;
    virtual void                Sort() = 0;

    void                        UpdateAddress( css::table::CellAddress& rCellAddress );
};

struct ScMyShape
{
    ScAddress   aAddress;
    ScAddress   aEndAddress;
    sal_Int32       nEndX;
    sal_Int32       nEndY;
    css::uno::Reference<css::drawing::XShape> xShape;

    bool operator<(const ScMyShape& aShape) const;
};

typedef std::list<ScMyShape>    ScMyShapeList;

class ScMyShapesContainer : ScMyIteratorBase
{
private:
    ScMyShapeList               aShapeList;
protected:
    virtual bool                GetFirstAddress( css::table::CellAddress& rCellAddress ) override;
public:
                                ScMyShapesContainer();
    virtual                     ~ScMyShapesContainer();

                                using ScMyIteratorBase::UpdateAddress;
    void                        AddNewShape(const ScMyShape& aShape);
    bool                        HasShapes() { return !aShapeList.empty(); }
    const ScMyShapeList&        GetShapes() const { return aShapeList; }
    virtual void                SetCellData( ScMyCell& rMyCell ) override;
    virtual void                Sort() override;
    void                        SkipTable(SCTAB nSkip);
};

struct ScMyNoteShape
{
    css::uno::Reference<css::drawing::XShape> xShape;
    ScAddress aPos;

    bool operator<(const ScMyNoteShape& aNote) const;
};

typedef std::list<ScMyNoteShape>    ScMyNoteShapeList;

class ScMyNoteShapesContainer : ScMyIteratorBase
{
private:
    ScMyNoteShapeList           aNoteShapeList;
protected:
    virtual bool                GetFirstAddress( css::table::CellAddress& rCellAddress ) override;
public:
                                ScMyNoteShapesContainer();
    virtual                     ~ScMyNoteShapesContainer();

    using ScMyIteratorBase::UpdateAddress;
    void                        AddNewNote(const ScMyNoteShape& aNote);
    const ScMyNoteShapeList&    GetNotes() const { return aNoteShapeList; }
    virtual void                SetCellData( ScMyCell& rMyCell ) override;
    virtual void                Sort() override;
    void                        SkipTable(SCTAB nSkip);
};

struct ScMyMergedRange
{
    css::table::CellRangeAddress aCellRange;
    sal_Int32                   nRows;
    bool                        bIsFirst;
    bool                        operator<(const ScMyMergedRange& aRange) const;
};

typedef std::list<ScMyMergedRange>  ScMyMergedRangeList;

class ScMyMergedRangesContainer : ScMyIteratorBase
{
private:
    ScMyMergedRangeList         aRangeList;
protected:
    virtual bool                GetFirstAddress( css::table::CellAddress& rCellAddress ) override;
public:
                                ScMyMergedRangesContainer();
    virtual                     ~ScMyMergedRangesContainer();
    void                        AddRange(const css::table::CellRangeAddress& rMergedRange);

                                using ScMyIteratorBase::UpdateAddress;
    virtual void                SetCellData( ScMyCell& rMyCell ) override;
    virtual void                Sort() override; // + remove doublets
    void                        SkipTable(SCTAB nSkip);
};

struct ScMyAreaLink
{
    OUString             sFilter;
    OUString             sFilterOptions;
    OUString             sURL;
    OUString             sSourceStr;
    css::table::CellRangeAddress aDestRange;
    sal_Int32                   nRefresh;

    inline                      ScMyAreaLink() : nRefresh( 0 ) {}

    inline sal_Int32            GetColCount() const { return aDestRange.EndColumn - aDestRange.StartColumn + 1; }
    inline sal_Int32            GetRowCount() const { return aDestRange.EndRow - aDestRange.StartRow + 1; }

    bool                        Compare( const ScMyAreaLink& rAreaLink ) const;
    bool                        operator<(const ScMyAreaLink& rAreaLink ) const;
};

typedef ::std::list< ScMyAreaLink > ScMyAreaLinkList;

class ScMyAreaLinksContainer : ScMyIteratorBase
{
private:
    ScMyAreaLinkList            aAreaLinkList;
protected:
    virtual bool                GetFirstAddress( css::table::CellAddress& rCellAddress ) override;
public:
                                ScMyAreaLinksContainer();
    virtual                     ~ScMyAreaLinksContainer();

    inline void                 AddNewAreaLink( const ScMyAreaLink& rAreaLink )
                                    { aAreaLinkList.push_back( rAreaLink ); }

                                using ScMyIteratorBase::UpdateAddress;
    virtual void                SetCellData( ScMyCell& rMyCell ) override;
    virtual void                Sort() override;
    void                        SkipTable(SCTAB nSkip);
};

struct ScMyCellRangeAddress : css::table::CellRangeAddress
{
    ScMyCellRangeAddress(const css::table::CellRangeAddress& rRange);
    bool                        operator<(const ScMyCellRangeAddress& rCellRangeAddress ) const;
};

typedef std::list<ScMyCellRangeAddress> ScMyEmptyDatabaseRangeList;

class ScMyEmptyDatabaseRangesContainer : ScMyIteratorBase
{
private:
    ScMyEmptyDatabaseRangeList  aDatabaseList;
protected:
    virtual bool                GetFirstAddress( css::table::CellAddress& rCellAddress ) override;
public:
                                ScMyEmptyDatabaseRangesContainer();
    virtual                     ~ScMyEmptyDatabaseRangesContainer();
    void                        AddNewEmptyDatabaseRange(const css::table::CellRangeAddress& aCellRangeAddress);

                                using ScMyIteratorBase::UpdateAddress;
    virtual void                SetCellData( ScMyCell& rMyCell ) override;
    virtual void                Sort() override;
    void                        SkipTable(SCTAB nSkip);
};

struct ScMyDetectiveObj
{
    css::table::CellAddress        aPosition;
    css::table::CellRangeAddress   aSourceRange;
    ScDetectiveObjType             eObjType;
    bool                           bHasError;
    bool operator<(const ScMyDetectiveObj& rDetObj) const;
};

typedef ::std::list< ScMyDetectiveObj > ScMyDetectiveObjList;
typedef ::std::vector< ScMyDetectiveObj > ScMyDetectiveObjVec;

class ScMyDetectiveObjContainer : ScMyIteratorBase
{
private:
    ScMyDetectiveObjList        aDetectiveObjList;
protected:
    virtual bool                GetFirstAddress( css::table::CellAddress& rCellAddress ) override;
public:
                                ScMyDetectiveObjContainer();
    virtual                     ~ScMyDetectiveObjContainer();

    void                        AddObject(
                                    ScDetectiveObjType eObjType,
                                    const SCTAB nSheet,
                                    const ScAddress& rPosition,
                                    const ScRange& rSourceRange,
                                    bool bHasError );

                                using ScMyIteratorBase::UpdateAddress;
    virtual void                SetCellData( ScMyCell& rMyCell ) override;
    virtual void                Sort() override;
    void                        SkipTable(SCTAB nSkip);
};

struct ScMyDetectiveOp
{
    css::table::CellAddress    aPosition;
    ScDetOpType                eOpType;
    sal_Int32                  nIndex;
    bool operator<(const ScMyDetectiveOp& rDetOp) const;
};

typedef ::std::list< ScMyDetectiveOp > ScMyDetectiveOpList;
typedef ::std::vector< ScMyDetectiveOp > ScMyDetectiveOpVec;

class ScMyDetectiveOpContainer : ScMyIteratorBase
{
private:
    ScMyDetectiveOpList         aDetectiveOpList;
protected:
    virtual bool                GetFirstAddress( css::table::CellAddress& rCellAddress ) override;
public:
                                ScMyDetectiveOpContainer();
    virtual                     ~ScMyDetectiveOpContainer();

    void                        AddOperation( ScDetOpType eOpType, const ScAddress& rPosition, sal_uInt32 nIndex );

                                using ScMyIteratorBase::UpdateAddress;
    virtual void                SetCellData( ScMyCell& rMyCell ) override;
    virtual void                Sort() override;
    void                        SkipTable(SCTAB nSkip);
};

// contains data to export for the current cell position
struct ScMyCell
{
    ScAddress maCellAddress; /// Use this instead of the UNO one.

    css::table::CellAddress      aCellAddress;
    css::table::CellRangeAddress aMergeRange;
    css::table::CellRangeAddress aMatrixRange;

    ScMyAreaLink                aAreaLink;
    ScMyShapeList               aShapeList;
    ScMyDetectiveObjVec         aDetectiveObjVec;
    ScMyDetectiveOpVec          aDetectiveOpVec;

    ScPostIt*                   pNote;

    sal_Int32                   nValidationIndex;
    sal_Int32                   nStyleIndex;
    sal_Int32                   nNumberFormat;
    css::table::CellContentType nType;

    ScRefCellValue              maBaseCell;

    bool                        bIsAutoStyle;

    bool                        bHasShape;
    bool                        bIsMergedBase;
    bool                        bIsCovered;
    bool                        bHasAreaLink;
    bool                        bHasEmptyDatabase;
    bool                        bHasDetectiveObj;
    bool                        bHasDetectiveOp;

    bool                        bIsMatrixBase;
    bool                        bIsMatrixCovered;
    bool                        bHasAnnotation;

                                ScMyCell();
                                ~ScMyCell();
};

class ScMyNotEmptyCellsIterator : boost::noncopyable
{
    css::uno::Reference<css::sheet::XSpreadsheet> xTable;
    css::uno::Reference<css::table::XCellRange> xCellRange;
    css::table::CellAddress             aLastAddress;

    ScMyShapesContainer*                pShapes;
    ScMyNoteShapesContainer*            pNoteShapes;
    ScMyEmptyDatabaseRangesContainer*   pEmptyDatabaseRanges;
    ScMyMergedRangesContainer*          pMergedRanges;
    ScMyAreaLinksContainer*             pAreaLinks;
    ScMyDetectiveObjContainer*          pDetectiveObj;
    ScMyDetectiveOpContainer*           pDetectiveOp;

    ScXMLExport&                rExport;
    std::unique_ptr<ScHorizontalCellIterator> mpCellItr;

    SCCOL                       nCellCol;
    SCROW                       nCellRow;
    SCTAB                       nCurrentTable;

    void                        UpdateAddress( css::table::CellAddress& rAddress );
    void SetCellData( ScMyCell& rMyCell, const css::table::CellAddress& rAddress );

    void                        HasAnnotation( ScMyCell& aCell );
public:
                                ScMyNotEmptyCellsIterator(ScXMLExport& rExport);
                                ~ScMyNotEmptyCellsIterator();

    void                        Clear();

    inline void                 SetShapes(ScMyShapesContainer* pNewShapes)
                                    { pShapes = pNewShapes; }
    inline void                 SetNoteShapes(ScMyNoteShapesContainer* pNewNoteShapes)
                                    { pNoteShapes = pNewNoteShapes; }
    inline void                 SetEmptyDatabaseRanges(ScMyEmptyDatabaseRangesContainer* pNewEmptyDatabaseRanges)
                                    { pEmptyDatabaseRanges = pNewEmptyDatabaseRanges; }
    inline void                 SetMergedRanges(ScMyMergedRangesContainer* pNewMergedRanges)
                                    { pMergedRanges = pNewMergedRanges; }
    inline void                 SetAreaLinks(ScMyAreaLinksContainer* pNewAreaLinks)
                                    { pAreaLinks = pNewAreaLinks; }
    inline void                 SetDetectiveObj(ScMyDetectiveObjContainer* pNewDetectiveObj)
                                    { pDetectiveObj = pNewDetectiveObj; }
    inline void                 SetDetectiveOp(ScMyDetectiveOpContainer* pNewDetectiveOp)
                                    { pDetectiveOp = pNewDetectiveOp; }

    void                        SetCurrentTable(const SCTAB nTable,
                                    css::uno::Reference<css::sheet::XSpreadsheet>& rxTable);
    void                        SkipTable(SCTAB nSkip);

    bool                        GetNext(ScMyCell& aCell, ScFormatRangeStyles* pCellStyles);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
