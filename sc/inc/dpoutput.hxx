/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_DPOUTPUT_HXX
#define SC_DPOUTPUT_HXX

#include <com/sun/star/sheet/XDimensionsSupplier.hpp>
#include <com/sun/star/sheet/DataResult.hpp>
#include <com/sun/star/sheet/MemberResult.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/DataPilotOutputRangeType.hpp>

#include "global.hxx"
#include "address.hxx"

#include "dpfilteredcache.hxx"
#include "dptypes.hxx"

#include <vector>

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldFilter;
    struct DataPilotTablePositionData;
}}}}

class Rectangle;
class ScDocument;

struct ScDPOutLevelData;


struct ScDPGetPivotDataField
{
    rtl::OUString maFieldName;
    com::sun::star::sheet::GeneralFunction meFunction;

    bool   mbValIsStr;
    rtl::OUString maValStr;
    double mnValNum;

        ScDPGetPivotDataField() :
            meFunction( com::sun::star::sheet::GeneralFunction_NONE ),
            mbValIsStr( false ),
            mnValNum( 0.0 )
        {
        }
};



class ScDPOutput
{
private:
    ScDocument*             pDoc;
    com::sun::star::uno::Reference<
        com::sun::star::sheet::XDimensionsSupplier> xSource;
    ScAddress               aStartPos;
    ScDPOutLevelData*       pColFields;
    ScDPOutLevelData*       pRowFields;
    ScDPOutLevelData*       pPageFields;
    long                    nColFieldCount;
    long                    nRowFieldCount;
    long                    nPageFieldCount;
    com::sun::star::uno::Sequence<
        com::sun::star::uno::Sequence<
            com::sun::star::sheet::DataResult> > aData;
    rtl::OUString           aDataDescription;

    // Number format related parameters
    sal_uInt32*                 pColNumFmt;
    sal_uInt32*                 pRowNumFmt;
    long                    nColFmtCount;
    long                    nRowFmtCount;
    sal_uInt32                  nSingleNumFmt;

    // Output geometry related parameters
    long                    nColCount;
    long                    nRowCount;
    long                    nHeaderSize;
    SCCOL                   nTabStartCol;
    SCROW                   nTabStartRow;
    SCCOL                   nMemberStartCol;
    SCROW                   nMemberStartRow;
    SCCOL                   nDataStartCol;
    SCROW                   nDataStartRow;
    SCCOL                   nTabEndCol;
    SCROW                   nTabEndRow;
    bool                    bDoFilter:1;
    bool                    bResultsError:1;
    bool                    mbHasDataLayout:1;
    bool                    bSizesValid:1;
    bool                    bSizeOverflow:1;
    bool                    mbHeaderLayout:1;  // true : grid, false : standard

    void            DataCell( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                const com::sun::star::sheet::DataResult& rData );
    void            HeaderCell( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                const com::sun::star::sheet::MemberResult& rData,
                                bool bColHeader, long nLevel );

    void FieldCell(SCCOL nCol, SCROW nRow, SCTAB nTab, const ScDPOutLevelData& rData, bool bInTable);

    void            CalcSizes();

    /** Query which sub-area of the table the cell is in. See
        css.sheet.DataPilotTablePositionType for the interpretation of the
        return value. */
    sal_Int32       GetPositionType(const ScAddress& rPos);

public:
                    ScDPOutput( ScDocument* pD,
                                const com::sun::star::uno::Reference<
                                    com::sun::star::sheet::XDimensionsSupplier>& xSrc,
                                const ScAddress& rPos, bool bFilter );
                    ~ScDPOutput();

    void            SetPosition( const ScAddress& rPos );

    void            Output();           //! Refresh?
    ScRange         GetOutputRange( sal_Int32 nRegionType = ::com::sun::star::sheet::DataPilotOutputRangeType::WHOLE );
    long            GetHeaderRows();
    bool            HasError();         // range overflow or exception from source

    void            GetPositionData(const ScAddress& rPos, ::com::sun::star::sheet::DataPilotTablePositionData& rPosData);

    /** Get filtering criteria based on the position of the cell within data
        field region. */
    bool            GetDataResultPositionData(::std::vector< ::com::sun::star::sheet::DataPilotFieldFilter >& rFilters, const ScAddress& rPos);

    /**
     * @return true on success and stores the result in rTarget, or false if
     *         rFilters or rTarget describe something that is not visible.
     */
    bool GetPivotData( ScDPGetPivotDataField& rTarget,
                       const std::vector< ScDPGetPivotDataField >& rFilters );
    long            GetHeaderDim( const ScAddress& rPos, sal_uInt16& rOrient );
    bool GetHeaderDrag(
        const ScAddress& rPos, bool bMouseLeft, bool bMouseTop, long nDragDim,
        Rectangle& rPosRect, sal_uInt16& rOrient, long& rDimPos );
    bool IsFilterButton( const ScAddress& rPos );

    void GetMemberResultNames(ScDPUniqueStringSet& rNames, long nDimension);

    void            SetHeaderLayout(bool bUseGrid);
    bool            GetHeaderLayout() const;

    static void GetDataDimensionNames(
        rtl::OUString& rSourceName, rtl::OUString& rGivenName,
        const com::sun::star::uno::Reference<
            com::sun::star::uno::XInterface>& xDim );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
