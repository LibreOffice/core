/*************************************************************************
 *
 *  $RCSfile: XMLExportIterator.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: sab $ $Date: 2000-11-16 18:14:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SC_XMLEXPORTITERATOR_HXX
#define _SC_XMLEXPORTITERATOR_HXX

#ifndef __SGI_STL_VECTOR
#include <stl/vector>
#endif

#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEET_HPP_
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#endif

#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLRANGEADDRESS_HPP_
#include <com/sun/star/table/CellRangeAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_XCELL_HPP_
#include <com/sun/star/table/XCell.hpp>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef SC_DETFUNC_HXX
#include "detfunc.hxx"
#endif
#ifndef SC_DETDATA_HXX
#include "detdata.hxx"
#endif

class   ScHorizontalCellIterator;
struct  ScMyCell;
class   ScXMLExport;

//==============================================================================

class ScMyIteratorBase
{
protected:
    virtual sal_Bool            GetFirstAddress( ::com::sun::star::table::CellAddress& rCellAddress ) = 0;

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
    sal_Int32   nIndex;
};

typedef std::vector<ScMyShape>  ScMyShapeVec;

class ScMyShapesContainer : ScMyIteratorBase
{
private:
    ScMyShapeVec                aShapeVec;
protected:
    virtual sal_Bool            GetFirstAddress( ::com::sun::star::table::CellAddress& rCellAddress );
public:
                                ScMyShapesContainer();
    virtual                     ~ScMyShapesContainer();

                                ScMyIteratorBase::UpdateAddress;
    void                        AddNewShape(const ScMyShape& aShape);
    virtual void                SetCellData( ScMyCell& rMyCell );
    virtual void                Sort();
};

//==============================================================================

struct ScMyMergedRange
{
    com::sun::star::table::CellRangeAddress aCellRange;
    sal_Int32                   nRows;
    sal_Bool                    bIsFirst;
};

typedef std::vector<ScMyMergedRange>    ScMyMergedRangeVec;

class ScMyMergedRangesContainer : ScMyIteratorBase
{
private:
    ScMyMergedRangeVec          aRangeVec;
protected:
    virtual sal_Bool            GetFirstAddress( ::com::sun::star::table::CellAddress& rCellAddress );
public:
                                ScMyMergedRangesContainer();
    virtual                     ~ScMyMergedRangesContainer();
    void                        AddRange(const com::sun::star::table::CellRangeAddress aMergedRange);

                                ScMyIteratorBase::UpdateAddress;
    virtual void                SetCellData( ScMyCell& rMyCell );
    virtual void                Sort(); // + remove doublets
};

//==============================================================================

struct ScMyAreaLink
{
    ::rtl::OUString             sFilter;
    ::rtl::OUString             sFilterOptions;
    ::rtl::OUString             sURL;
    ::rtl::OUString             sSourceStr;
    ::com::sun::star::table::CellRangeAddress aDestRange;

    inline sal_Int32            GetColCount() const { return aDestRange.EndColumn - aDestRange.StartColumn + 1; }
    inline sal_Int32            GetRowCount() const { return aDestRange.EndRow - aDestRange.StartRow + 1; }

    sal_Bool                    Compare( const ScMyAreaLink& rAreaLink ) const;
};

typedef ::std::vector< ScMyAreaLink > ScMyAreaLinkVec;

class ScMyAreaLinksContainer : ScMyIteratorBase
{
private:
    ScMyAreaLinkVec             aAreaLinkVec;
protected:
    virtual sal_Bool            GetFirstAddress( ::com::sun::star::table::CellAddress& rCellAddress );
public:
                                ScMyAreaLinksContainer();
    virtual                     ~ScMyAreaLinksContainer();

    inline void                 AddNewAreaLink( const ScMyAreaLink& rAreaLink )
                                    { aAreaLinkVec.push_back( rAreaLink ); }

                                ScMyIteratorBase::UpdateAddress;
    virtual void                SetCellData( ScMyCell& rMyCell );
    virtual void                Sort();
};

//==============================================================================

typedef std::vector<com::sun::star::table::CellRangeAddress> ScMyEmptyDatabaseRangeVec;

class ScMyEmptyDatabaseRangesContainer : ScMyIteratorBase
{
private:
    ScMyEmptyDatabaseRangeVec   aDatabaseVec;
protected:
    virtual sal_Bool            GetFirstAddress( ::com::sun::star::table::CellAddress& rCellAddress );
public:
                                ScMyEmptyDatabaseRangesContainer();
    virtual                     ~ScMyEmptyDatabaseRangesContainer();
    void                        AddNewEmptyDatabaseRange(const com::sun::star::table::CellRangeAddress& aCellRangeAddress);

                                ScMyIteratorBase::UpdateAddress;
    virtual void                SetCellData( ScMyCell& rMyCell );
    virtual void                Sort();
};

//==============================================================================

struct ScMyDetectiveObj
{
    ::com::sun::star::table::CellAddress        aPosition;
    ::com::sun::star::table::CellRangeAddress   aSourceRange;
    ScDetectiveObjType                          eObjType;
    sal_Bool                                    bHasError;
};

typedef ::std::vector< ScMyDetectiveObj > ScMyDetectiveObjVec;

class ScMyDetectiveObjContainer : ScMyIteratorBase
{
private:
    ScMyDetectiveObjVec         aDetectiveObjVec;
protected:
    virtual sal_Bool            GetFirstAddress( ::com::sun::star::table::CellAddress& rCellAddress );
public:
                                ScMyDetectiveObjContainer();
    virtual                     ~ScMyDetectiveObjContainer();

    void                        AddObject(
                                    ScDetectiveObjType eObjType,
                                    const ScAddress& rPosition,
                                    const ScRange& rSourceRange,
                                    sal_Bool bHasError );

                                ScMyIteratorBase::UpdateAddress;
    virtual void                SetCellData( ScMyCell& rMyCell );
    virtual void                Sort();
};

//==============================================================================

struct ScMyDetectiveOp
{
    ::com::sun::star::table::CellAddress    aPosition;
    ScDetOpType                             eOpType;
    sal_Int32                               nIndex;
};

typedef ::std::vector< ScMyDetectiveOp > ScMyDetectiveOpVec;

class ScMyDetectiveOpContainer : ScMyIteratorBase
{
private:
    ScMyDetectiveOpVec          aDetectiveOpVec;
protected:
    virtual sal_Bool            GetFirstAddress( ::com::sun::star::table::CellAddress& rCellAddress );
public:
                                ScMyDetectiveOpContainer();
    virtual                     ~ScMyDetectiveOpContainer();

    void                        AddOperation( ScDetOpType eOpType, const ScAddress& rPosition );

                                ScMyIteratorBase::UpdateAddress;
    virtual void                SetCellData( ScMyCell& rMyCell );
    virtual void                Sort();
};

//==============================================================================

// contains data to export for the current cell position
struct ScMyCell
{
    com::sun::star::uno::Reference<com::sun::star::table::XCell> xCell;
    com::sun::star::table::CellAddress      aCellAddress;
    com::sun::star::table::CellRangeAddress aMergeRange;
    com::sun::star::table::CellRangeAddress aMatrixRange;

    ScMyAreaLink                aAreaLink;
    ScMyShapeVec                aShapeVec;
    ScMyDetectiveObjVec         aDetectiveObjVec;
    ScMyDetectiveOpVec          aDetectiveOpVec;

    sal_Bool                    bHasShape;
    sal_Bool                    bIsMergedBase;
    sal_Bool                    bIsCovered;
    sal_Bool                    bHasAreaLink;
    sal_Bool                    bHasEmptyDatabase;
    sal_Bool                    bHasDetectiveObj;
    sal_Bool                    bHasDetectiveOp;

    sal_Bool                    bIsMatrixBase;
    sal_Bool                    bIsMatrixCovered;
    sal_Bool                    bHasAnnotation;

                                ScMyCell();
                                ~ScMyCell();
};

//==============================================================================

class ScMyNotEmptyCellsIterator
{
    com::sun::star::uno::Reference<com::sun::star::sheet::XSpreadsheet> xTable;

    ScMyShapesContainer*                pShapes;
    ScMyEmptyDatabaseRangesContainer*   pEmptyDatabaseRanges;
    ScMyMergedRangesContainer*          pMergedRanges;
    ScMyAreaLinksContainer*             pAreaLinks;
    ScMyDetectiveObjContainer*          pDetectiveObj;
    ScMyDetectiveOpContainer*           pDetectiveOp;

    ScXMLExport&                rExport;
    ScHorizontalCellIterator*   pCellItr;

    sal_uInt16                  nCellCol;
    sal_uInt16                  nCellRow;
    sal_Int16                   nCurrentTable;

    void                        UpdateAddress( ::com::sun::star::table::CellAddress& rAddress );
    void                        SetCellData( ScMyCell& rMyCell, ::com::sun::star::table::CellAddress& rAddress );

    void                        SetMatrixCellData( ScMyCell& rMyCell );
    void                        HasAnnotation( ScMyCell& aCell );
public:
                                ScMyNotEmptyCellsIterator(ScXMLExport& rExport);
                                ~ScMyNotEmptyCellsIterator();

    inline void                 SetShapes(ScMyShapesContainer* pNewShapes)
                                    { pShapes = pNewShapes; }
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

    void                        SetCurrentTable(const sal_Int32 nTable);

    sal_Bool                    GetNext(ScMyCell& aCell);
};

#endif

