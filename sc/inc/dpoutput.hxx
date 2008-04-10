/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dpoutput.hxx,v $
 * $Revision: 1.10 $
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
#include "global.hxx"
#include "address.hxx"

#include <vector>

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldFilter;
}}}}

class Rectangle;
class SvStream;
class ScDocument;
class StrCollection;

struct ScDPOutLevelData;




struct ScDPPositionData
{
    long    nDimension;
    long    nHierarchy;
    long    nLevel;
    long    nFlags;
    String  aMemberName;

    ScDPPositionData() { nDimension = nHierarchy = nLevel = -1; nFlags = 0; }   // invalid
};

struct ScDPGetPivotDataField
{
    String maFieldName;
    com::sun::star::sheet::GeneralFunction meFunction;

    bool   mbValIsStr;
    String maValStr;
    double mnValNum;

        ScDPGetPivotDataField() :
            meFunction( com::sun::star::sheet::GeneralFunction_NONE ),
            mbValIsStr( false ),
            mnValNum( 0.0 )
        {
        }
};



class ScDPOutput            //! name???
{
private:
    //! use impl-object?
    ScDocument*             pDoc;
    com::sun::star::uno::Reference<
        com::sun::star::sheet::XDimensionsSupplier> xSource;
    ScAddress               aStartPos;
    BOOL                    bDoFilter;
    ScDPOutLevelData*       pColFields;
    ScDPOutLevelData*       pRowFields;
    ScDPOutLevelData*       pPageFields;
    long                    nColFieldCount;
    long                    nRowFieldCount;
    long                    nPageFieldCount;
    com::sun::star::uno::Sequence<
        com::sun::star::uno::Sequence<
            com::sun::star::sheet::DataResult> > aData;
    BOOL                    bResultsError;
    String                  aDataDescription;
    UINT32*                 pColNumFmt;
    UINT32*                 pRowNumFmt;
    long                    nColFmtCount;
    long                    nRowFmtCount;
    UINT32                  nSingleNumFmt;

    BOOL                    bSizesValid;
    BOOL                    bSizeOverflow;
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

    void            DataCell( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                const com::sun::star::sheet::DataResult& rData );
    void            HeaderCell( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                const com::sun::star::sheet::MemberResult& rData,
                                BOOL bColHeader, long nLevel );
    void            FieldCell( SCCOL nCol, SCROW nRow, SCTAB nTab, const String& rCaption,
                                BOOL bFrame = TRUE );
    void            CalcSizes();

public:
                    ScDPOutput( ScDocument* pD,
                                const com::sun::star::uno::Reference<
                                    com::sun::star::sheet::XDimensionsSupplier>& xSrc,
                                const ScAddress& rPos, BOOL bFilter );
                    ~ScDPOutput();

    void            SetPosition( const ScAddress& rPos );

    void            Output();           //! Refresh?
    ScRange         GetOutputRange();
    long            GetHeaderRows();
    BOOL            HasError();         // range overflow or exception from source

    void            GetPositionData( ScDPPositionData& rData, const ScAddress& rPos );

    /** Get filtering criteria based on the position of the cell within data
        field region. */
    bool            GetDataFieldPositionData(::std::vector< ::com::sun::star::sheet::DataPilotFieldFilter >& rFilters, const ScAddress& rPos);

    BOOL            GetPivotData( ScDPGetPivotDataField& rTarget, /* returns result */
                                  const std::vector< ScDPGetPivotDataField >& rFilters );
    long            GetHeaderDim( const ScAddress& rPos, USHORT& rOrient );
    BOOL            GetHeaderDrag( const ScAddress& rPos, BOOL bMouseLeft, BOOL bMouseTop,
                                    long nDragDim,
                                    Rectangle& rPosRect, USHORT& rOrient, long& rDimPos );
    BOOL            IsFilterButton( const ScAddress& rPos );

    void            GetMemberResultNames( StrCollection& rNames, long nDimension );

    static void     GetDataDimensionNames( String& rSourceName, String& rGivenName,
                                           const com::sun::star::uno::Reference<
                                               com::sun::star::uno::XInterface>& xDim );
};


#endif

