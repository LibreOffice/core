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
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/sheet/XSheetAnnotation.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <global.hxx>
#include <detfunc.hxx>
#include <detdata.hxx>
#include <postit.hxx>
#include <cellvalue.hxx>

#include <memory>

class   ScHorizontalCellIterator;
struct  ScMyCell;
class   ScXMLExport;
class   ScFormatRangeStyles;

class ScMyIteratorBase
{
protected:
    virtual bool                GetFirstAddress( ScAddress& rCellAddress ) = 0;

public:
                                ScMyIteratorBase();
    virtual                     ~ScMyIteratorBase();

    ScMyIteratorBase(ScMyIteratorBase const &) = default;
    ScMyIteratorBase(ScMyIteratorBase &&) = default;
    ScMyIteratorBase & operator =(ScMyIteratorBase const &) = default;
    ScMyIteratorBase & operator =(ScMyIteratorBase &&) = default;

    virtual void                SetCellData( ScMyCell& rMyCell ) = 0;
    virtual void                Sort() = 0;

    void                        UpdateAddress( ScAddress& rCellAddress );
};

struct ScMyShape
{
    ScAddress       aAddress;
    ScAddress       aEndAddress;
    sal_Int32       nEndX;
    sal_Int32       nEndY;
    bool            bResizeWithCell;
    css::uno::Reference<css::drawing::XShape> xShape;

    bool operator<(const ScMyShape& aShape) const;
};

typedef std::list<ScMyShape>    ScMyShapeList;

class ScMyShapesContainer : public ScMyIteratorBase
{
private:
    ScMyShapeList               aShapeList;
protected:
    virtual bool                GetFirstAddress( ScAddress& rCellAddress ) override;
public:
                                ScMyShapesContainer();
    virtual                     ~ScMyShapesContainer() override;

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

class ScMyNoteShapesContainer : public ScMyIteratorBase
{
private:
    ScMyNoteShapeList           aNoteShapeList;
protected:
    virtual bool                GetFirstAddress( ScAddress& rCellAddress ) override;
public:
                                ScMyNoteShapesContainer();
    virtual                     ~ScMyNoteShapesContainer() override;

    using ScMyIteratorBase::UpdateAddress;
    void                        AddNewNote(const ScMyNoteShape& aNote);
    const ScMyNoteShapeList&    GetNotes() const { return aNoteShapeList; }
    virtual void                SetCellData( ScMyCell& rMyCell ) override;
    virtual void                Sort() override;
    void                        SkipTable(SCTAB nSkip);
};

struct ScMyMergedRange
{
    ScRange                     aCellRange;
    sal_Int32                   nRows;
    bool                        bIsFirst;
    bool                        operator<(const ScMyMergedRange& aRange) const;
};

typedef std::list<ScMyMergedRange>  ScMyMergedRangeList;

class ScMyMergedRangesContainer : public ScMyIteratorBase
{
private:
    ScMyMergedRangeList         aRangeList;
protected:
    virtual bool                GetFirstAddress( ScAddress& rCellAddress ) override;
public:
                                ScMyMergedRangesContainer();
    virtual                     ~ScMyMergedRangesContainer() override;
    void                        AddRange(const ScRange& rMergedRange);

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
    ScRange                     aDestRange;
    sal_Int32                   nRefresh;

    ScMyAreaLink() : nRefresh( 0 ) {}

    sal_Int32            GetColCount() const { return aDestRange.aEnd.Col() - aDestRange.aStart.Col() + 1; }
    sal_Int32            GetRowCount() const { return aDestRange.aEnd.Row() - aDestRange.aStart.Col() + 1; }

    bool                        Compare( const ScMyAreaLink& rAreaLink ) const;
    bool                        operator<(const ScMyAreaLink& rAreaLink ) const;
};

typedef ::std::list< ScMyAreaLink > ScMyAreaLinkList;

class ScMyAreaLinksContainer : public ScMyIteratorBase
{
private:
    ScMyAreaLinkList            aAreaLinkList;
protected:
    virtual bool                GetFirstAddress( ScAddress& rCellAddress ) override;
public:
                                ScMyAreaLinksContainer();
    virtual                     ~ScMyAreaLinksContainer() override;

    void                 AddNewAreaLink( const ScMyAreaLink& rAreaLink )
                                    { aAreaLinkList.push_back( rAreaLink ); }

                                using ScMyIteratorBase::UpdateAddress;
    virtual void                SetCellData( ScMyCell& rMyCell ) override;
    virtual void                Sort() override;
    void                        SkipTable(SCTAB nSkip);
};

typedef std::list<ScRange> ScMyEmptyDatabaseRangeList;

class ScMyEmptyDatabaseRangesContainer : public ScMyIteratorBase
{
private:
    ScMyEmptyDatabaseRangeList  aDatabaseList;
protected:
    virtual bool                GetFirstAddress( ScAddress& rCellAddress ) override;
public:
                                ScMyEmptyDatabaseRangesContainer();
    virtual                     ~ScMyEmptyDatabaseRangesContainer() override;

    ScMyEmptyDatabaseRangesContainer(ScMyEmptyDatabaseRangesContainer const &) = default;
    ScMyEmptyDatabaseRangesContainer(ScMyEmptyDatabaseRangesContainer &&) = default;
    ScMyEmptyDatabaseRangesContainer & operator =(ScMyEmptyDatabaseRangesContainer const &) = default;
    ScMyEmptyDatabaseRangesContainer & operator =(ScMyEmptyDatabaseRangesContainer &&) = default;

    void                        AddNewEmptyDatabaseRange(const css::table::CellRangeAddress& aCellRangeAddress);

                                using ScMyIteratorBase::UpdateAddress;
    virtual void                SetCellData( ScMyCell& rMyCell ) override;
    virtual void                Sort() override;
    void                        SkipTable(SCTAB nSkip);
};

struct ScMyDetectiveObj
{
    ScAddress                      aPosition;
    ScRange                        aSourceRange;
    ScDetectiveObjType             eObjType;
    bool                           bHasError;
    bool operator<(const ScMyDetectiveObj& rDetObj) const;
};

typedef ::std::list< ScMyDetectiveObj > ScMyDetectiveObjList;
typedef ::std::vector< ScMyDetectiveObj > ScMyDetectiveObjVec;

class ScMyDetectiveObjContainer : public ScMyIteratorBase
{
private:
    ScMyDetectiveObjList        aDetectiveObjList;
protected:
    virtual bool                GetFirstAddress( ScAddress& rCellAddress ) override;
public:
                                ScMyDetectiveObjContainer();
    virtual                     ~ScMyDetectiveObjContainer() override;

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
    ScAddress                  aPosition;
    ScDetOpType                eOpType;
    sal_Int32                  nIndex;
    bool operator<(const ScMyDetectiveOp& rDetOp) const;
};

typedef ::std::list< ScMyDetectiveOp > ScMyDetectiveOpList;
typedef ::std::vector< ScMyDetectiveOp > ScMyDetectiveOpVec;

class ScMyDetectiveOpContainer : public ScMyIteratorBase
{
private:
    ScMyDetectiveOpList         aDetectiveOpList;
protected:
    virtual bool                GetFirstAddress( ScAddress& rCellAddress ) override;
public:
                                ScMyDetectiveOpContainer();
    virtual                     ~ScMyDetectiveOpContainer() override;

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

    ScAddress                   aCellAddress;
    ScRange                     aMergeRange;
    ScRange                     aMatrixRange;

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
};

class ScMyNotEmptyCellsIterator
{
    ScMyNotEmptyCellsIterator(const ScMyNotEmptyCellsIterator&) = delete;
    const ScMyNotEmptyCellsIterator& operator=(const ScMyNotEmptyCellsIterator&) = delete;

    css::uno::Reference<css::sheet::XSpreadsheet> xTable;
    css::uno::Reference<css::table::XCellRange> xCellRange;
    ScAddress                           aLastAddress;

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

    void                        UpdateAddress( ScAddress& rAddress );
    void SetCellData( ScMyCell& rMyCell, const ScAddress& rAddress );

    void                        HasAnnotation( ScMyCell& aCell );
public:
    explicit                    ScMyNotEmptyCellsIterator(ScXMLExport& rExport);
                                ~ScMyNotEmptyCellsIterator();

    void                        Clear();

    void                 SetShapes(ScMyShapesContainer* pNewShapes)
                                    { pShapes = pNewShapes; }
    void                 SetNoteShapes(ScMyNoteShapesContainer* pNewNoteShapes)
                                    { pNoteShapes = pNewNoteShapes; }
    void                 SetEmptyDatabaseRanges(ScMyEmptyDatabaseRangesContainer* pNewEmptyDatabaseRanges)
                                    { pEmptyDatabaseRanges = pNewEmptyDatabaseRanges; }
    void                 SetMergedRanges(ScMyMergedRangesContainer* pNewMergedRanges)
                                    { pMergedRanges = pNewMergedRanges; }
    void                 SetAreaLinks(ScMyAreaLinksContainer* pNewAreaLinks)
                                    { pAreaLinks = pNewAreaLinks; }
    void                 SetDetectiveObj(ScMyDetectiveObjContainer* pNewDetectiveObj)
                                    { pDetectiveObj = pNewDetectiveObj; }
    void                 SetDetectiveOp(ScMyDetectiveOpContainer* pNewDetectiveOp)
                                    { pDetectiveOp = pNewDetectiveOp; }

    void                        SetCurrentTable(const SCTAB nTable,
                                    const css::uno::Reference<css::sheet::XSpreadsheet>& rxTable);
    void                        SkipTable(SCTAB nSkip);

    bool                        GetNext(ScMyCell& aCell, ScFormatRangeStyles* pCellStyles);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
