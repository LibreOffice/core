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

#ifndef SC_XMLEXPORTITERATOR_HXX
#define SC_XMLEXPORTITERATOR_HXX

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
#include <boost/scoped_ptr.hpp>

class   ScHorizontalCellIterator;
struct  ScMyCell;
class   ScXMLExport;
class   ScFormatRangeStyles;

//==============================================================================

class ScMyIteratorBase
{
protected:
    virtual bool                GetFirstAddress( ::com::sun::star::table::CellAddress& rCellAddress ) = 0;

public:
                                ScMyIteratorBase();
    virtual                     ~ScMyIteratorBase();

    virtual void                SetCellData( ScMyCell& rMyCell ) = 0;
    virtual void                Sort() = 0;

    virtual void                UpdateAddress( ::com::sun::star::table::CellAddress& rCellAddress );
};

//==============================================================================

struct ScMyShape
{
    ScAddress   aAddress;
    ScAddress   aEndAddress;
    sal_Int32       nEndX;
    sal_Int32       nEndY;
    com::sun::star::uno::Reference<com::sun::star::drawing::XShape> xShape;

    bool operator<(const ScMyShape& aShape) const;
};

typedef std::list<ScMyShape>    ScMyShapeList;

class ScMyShapesContainer : ScMyIteratorBase
{
private:
    ScMyShapeList               aShapeList;
protected:
    virtual bool                GetFirstAddress( ::com::sun::star::table::CellAddress& rCellAddress );
public:
                                ScMyShapesContainer();
    virtual                     ~ScMyShapesContainer();

                                using ScMyIteratorBase::UpdateAddress;
    void                        AddNewShape(const ScMyShape& aShape);
    bool                        HasShapes() { return !aShapeList.empty(); }
    const ScMyShapeList*        GetShapes() const { return &aShapeList; }
    virtual void                SetCellData( ScMyCell& rMyCell );
    virtual void                Sort();
    void                        SkipTable(SCTAB nSkip);
};

struct ScMyNoteShape
{
    com::sun::star::uno::Reference<com::sun::star::drawing::XShape> xShape;
    ScAddress aPos;

    bool operator<(const ScMyNoteShape& aNote) const;
};

typedef std::list<ScMyNoteShape>    ScMyNoteShapeList;

class ScMyNoteShapesContainer : ScMyIteratorBase
{
private:
    ScMyNoteShapeList           aNoteShapeList;
protected:
    virtual bool                GetFirstAddress( ::com::sun::star::table::CellAddress& rCellAddress );
public:
                                ScMyNoteShapesContainer();
    virtual                     ~ScMyNoteShapesContainer();

                                using ScMyIteratorBase::UpdateAddress;
    void                        AddNewNote(const ScMyNoteShape& aNote);
    bool                        HasNotes() { return !aNoteShapeList.empty(); }
    const ScMyNoteShapeList*    GetNotes() const { return &aNoteShapeList; }
    virtual void                SetCellData( ScMyCell& rMyCell );
    virtual void                Sort();
    void                        SkipTable(SCTAB nSkip);
};

//==============================================================================

struct ScMyMergedRange
{
    com::sun::star::table::CellRangeAddress aCellRange;
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
    virtual bool                GetFirstAddress( ::com::sun::star::table::CellAddress& rCellAddress );
public:
                                ScMyMergedRangesContainer();
    virtual                     ~ScMyMergedRangesContainer();
    void                        AddRange(const com::sun::star::table::CellRangeAddress aMergedRange);

                                using ScMyIteratorBase::UpdateAddress;
    virtual void                SetCellData( ScMyCell& rMyCell );
    virtual void                Sort(); // + remove doublets
    void                        SkipTable(SCTAB nSkip);
};

//==============================================================================

struct ScMyAreaLink
{
    OUString             sFilter;
    OUString             sFilterOptions;
    OUString             sURL;
    OUString             sSourceStr;
    ::com::sun::star::table::CellRangeAddress aDestRange;
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
    virtual bool                GetFirstAddress( ::com::sun::star::table::CellAddress& rCellAddress );
public:
                                ScMyAreaLinksContainer();
    virtual                     ~ScMyAreaLinksContainer();

    inline void                 AddNewAreaLink( const ScMyAreaLink& rAreaLink )
                                    { aAreaLinkList.push_back( rAreaLink ); }

                                using ScMyIteratorBase::UpdateAddress;
    virtual void                SetCellData( ScMyCell& rMyCell );
    virtual void                Sort();
    void                        SkipTable(SCTAB nSkip);
};

//==============================================================================

struct ScMyCellRangeAddress : com::sun::star::table::CellRangeAddress
{
    ScMyCellRangeAddress(const com::sun::star::table::CellRangeAddress& rRange);
    bool                        operator<(const ScMyCellRangeAddress& rCellRangeAddress ) const;
};

typedef std::list<ScMyCellRangeAddress> ScMyEmptyDatabaseRangeList;

class ScMyEmptyDatabaseRangesContainer : ScMyIteratorBase
{
private:
    ScMyEmptyDatabaseRangeList  aDatabaseList;
protected:
    virtual bool                GetFirstAddress( ::com::sun::star::table::CellAddress& rCellAddress );
public:
                                ScMyEmptyDatabaseRangesContainer();
    virtual                     ~ScMyEmptyDatabaseRangesContainer();
    void                        AddNewEmptyDatabaseRange(const com::sun::star::table::CellRangeAddress& aCellRangeAddress);

                                using ScMyIteratorBase::UpdateAddress;
    virtual void                SetCellData( ScMyCell& rMyCell );
    virtual void                Sort();
    void                        SkipTable(SCTAB nSkip);
};

//==============================================================================

struct ScMyDetectiveObj
{
    ::com::sun::star::table::CellAddress        aPosition;
    ::com::sun::star::table::CellRangeAddress   aSourceRange;
    ScDetectiveObjType                          eObjType;
    bool                                        bHasError;
    bool operator<(const ScMyDetectiveObj& rDetObj) const;
};

typedef ::std::list< ScMyDetectiveObj > ScMyDetectiveObjList;
typedef ::std::vector< ScMyDetectiveObj > ScMyDetectiveObjVec;

class ScMyDetectiveObjContainer : ScMyIteratorBase
{
private:
    ScMyDetectiveObjList        aDetectiveObjList;
protected:
    virtual bool                GetFirstAddress( ::com::sun::star::table::CellAddress& rCellAddress );
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
    virtual void                SetCellData( ScMyCell& rMyCell );
    virtual void                Sort();
    void                        SkipTable(SCTAB nSkip);
};

//==============================================================================

struct ScMyDetectiveOp
{
    ::com::sun::star::table::CellAddress    aPosition;
    ScDetOpType                             eOpType;
    sal_Int32                               nIndex;
    bool operator<(const ScMyDetectiveOp& rDetOp) const;
};

typedef ::std::list< ScMyDetectiveOp > ScMyDetectiveOpList;
typedef ::std::vector< ScMyDetectiveOp > ScMyDetectiveOpVec;

class ScMyDetectiveOpContainer : ScMyIteratorBase
{
private:
    ScMyDetectiveOpList         aDetectiveOpList;
protected:
    virtual bool                GetFirstAddress( ::com::sun::star::table::CellAddress& rCellAddress );
public:
                                ScMyDetectiveOpContainer();
    virtual                     ~ScMyDetectiveOpContainer();

    void                        AddOperation( ScDetOpType eOpType, const ScAddress& rPosition, sal_uInt32 nIndex );

                                using ScMyIteratorBase::UpdateAddress;
    virtual void                SetCellData( ScMyCell& rMyCell );
    virtual void                Sort();
    void                        SkipTable(SCTAB nSkip);
};

//==============================================================================

// contains data to export for the current cell position
struct ScMyCell
{
    com::sun::star::uno::Reference<com::sun::star::sheet::XSheetAnnotation> xAnnotation;
    com::sun::star::uno::Reference<com::sun::star::drawing::XShape> xNoteShape;
    com::sun::star::table::CellAddress      aCellAddress;
    com::sun::star::table::CellRangeAddress aMergeRange;
    com::sun::star::table::CellRangeAddress aMatrixRange;

    OUString               sStringValue;
    OUString               sAnnotationText;

    ScMyAreaLink                aAreaLink;
    ScMyShapeList               aShapeList;
    ScMyDetectiveObjVec         aDetectiveObjVec;
    ScMyDetectiveOpVec          aDetectiveOpVec;

    double                      fValue;
    sal_Int32                   nValidationIndex;
    sal_Int32                   nStyleIndex;
    sal_Int32                   nNumberFormat;
    com::sun::star::table::CellContentType  nType;

    ScRefCellValue              maBaseCell;

    bool                        bIsAutoStyle;

    bool                        bHasShape;
    bool                        bIsMergedBase;
    bool                        bIsCovered;
    bool                        bHasAreaLink;
    bool                        bHasEmptyDatabase;
    bool                        bHasDetectiveObj;
    bool                        bHasDetectiveOp;

    bool                        bIsEditCell;
    bool                        bKnowWhetherIsEditCell;
    bool                        bHasStringValue;
    bool                        bHasDoubleValue;
    bool                        bHasXText;

    bool                        bIsMatrixBase;
    bool                        bIsMatrixCovered;
    bool                        bHasAnnotation;

                                ScMyCell();
                                ~ScMyCell();
};

//==============================================================================

struct ScMyExportAnnotation
{
    com::sun::star::uno::Reference<com::sun::star::sheet::XSheetAnnotation> xAnnotation;
    com::sun::star::table::CellAddress      aCellAddress;
    bool operator<(const ScMyExportAnnotation& rAnno) const;
};

struct ScNoteExportData
{
    SCROW nRow;
    SCCOL nCol;
    ScPostIt* pNote;

    bool operator<(const ScNoteExportData& r) const
    {
        if(nRow < r.nRow)
            return true;
        else if(nRow > r.nRow)
            return false;
        else
        {
            if(nCol < r.nCol)
                return true;
            else
                return false;
        }
    }
};

typedef ::std::list< ScMyExportAnnotation > ScMyExportAnnotationList;
typedef ::std::set< ScNoteExportData > ScMyNoteExportDataList;

class ScMyNotEmptyCellsIterator : boost::noncopyable
{
    com::sun::star::uno::Reference<com::sun::star::sheet::XSpreadsheet> xTable;
    com::sun::star::uno::Reference<com::sun::star::table::XCellRange> xCellRange;
    com::sun::star::table::CellAddress  aLastAddress;
    ScMyExportAnnotationList            aAnnotations;

    ScMyShapesContainer*                pShapes;
    ScMyNoteShapesContainer*            pNoteShapes;
    ScMyEmptyDatabaseRangesContainer*   pEmptyDatabaseRanges;
    ScMyMergedRangesContainer*          pMergedRanges;
    ScMyAreaLinksContainer*             pAreaLinks;
    ScMyDetectiveObjContainer*          pDetectiveObj;
    ScMyDetectiveOpContainer*           pDetectiveOp;
    ScMyNoteExportDataList              maNoteExportList;
    ScMyNoteExportDataList::iterator  maNoteExportListItr;

    ScXMLExport&                rExport;
    boost::scoped_ptr<ScHorizontalCellIterator> mpCellItr;

    SCCOL                       nCellCol;
    SCROW                       nCellRow;
    SCTAB                       nCurrentTable;

    void                        UpdateAddress( ::com::sun::star::table::CellAddress& rAddress );
    void                        SetCellData( ScMyCell& rMyCell, ::com::sun::star::table::CellAddress& rAddress );

    void                        SetMatrixCellData( ScMyCell& rMyCell );
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
                                    com::sun::star::uno::Reference<com::sun::star::sheet::XSpreadsheet>& rxTable);
    void                        SkipTable(SCTAB nSkip);

    bool                        GetNext(ScMyCell& aCell, ScFormatRangeStyles* pCellStyles);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
