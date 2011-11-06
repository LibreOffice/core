/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_DPOUTPUT_HXX
#define SC_DPOUTPUT_HXX

#include <com/sun/star/sheet/XDimensionsSupplier.hpp>
#include <com/sun/star/sheet/DataResult.hpp>
#include <com/sun/star/sheet/MemberResult.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/DataPilotOutputRangeType.hpp>

#include "global.hxx"
#include "address.hxx"

#include "dpcachetable.hxx"
#include <vector>

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldFilter;
    struct DataPilotTablePositionData;
}}}}

class Rectangle;
class SvStream;
class ScDocument;
class ScStrCollection;

struct ScDPOutLevelData;


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
    sal_Bool                    bDoFilter;
    ScDPOutLevelData*       pColFields;
    ScDPOutLevelData*       pRowFields;
    ScDPOutLevelData*       pPageFields;
    long                    nColFieldCount;
    long                    nRowFieldCount;
    long                    nPageFieldCount;
    com::sun::star::uno::Sequence<
        com::sun::star::uno::Sequence<
            com::sun::star::sheet::DataResult> > aData;
    sal_Bool                    bResultsError;
    bool                    mbHasDataLayout;
    String                  aDataDescription;

    // Number format related parameters
    sal_uInt32*                 pColNumFmt;
    sal_uInt32*                 pRowNumFmt;
    long                    nColFmtCount;
    long                    nRowFmtCount;
    sal_uInt32                  nSingleNumFmt;

    // Output geometry related parameters
    sal_Bool                    bSizesValid;
    sal_Bool                    bSizeOverflow;
    long                    nColCount;
    long                    nRowCount;
    long                    nHeaderSize;
    bool                    mbHeaderLayout;  // sal_True : grid, sal_False : standard
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
                                sal_Bool bColHeader, long nLevel );
    void            FieldCell( SCCOL nCol, SCROW nRow, SCTAB nTab, const String& rCaption,
                               bool bInTable, bool bPopup, bool bHasHiddenMember );
    void            CalcSizes();

    /** Query which sub-area of the table the cell is in. See
        css.sheet.DataPilotTablePositionType for the interpretation of the
        return value. */
    sal_Int32       GetPositionType(const ScAddress& rPos);

public:
                    ScDPOutput( ScDocument* pD,
                                const com::sun::star::uno::Reference<
                                    com::sun::star::sheet::XDimensionsSupplier>& xSrc,
                                const ScAddress& rPos, sal_Bool bFilter );
                    ~ScDPOutput();

    void            SetPosition( const ScAddress& rPos );

    void            Output();           //! Refresh?
    ScRange         GetOutputRange( sal_Int32 nRegionType = ::com::sun::star::sheet::DataPilotOutputRangeType::WHOLE );
    long            GetHeaderRows();
    sal_Bool            HasError();         // range overflow or exception from source

    void            GetPositionData(const ScAddress& rPos, ::com::sun::star::sheet::DataPilotTablePositionData& rPosData);

    /** Get filtering criteria based on the position of the cell within data
        field region. */
    bool            GetDataResultPositionData(::std::vector< ::com::sun::star::sheet::DataPilotFieldFilter >& rFilters, const ScAddress& rPos);

    sal_Bool            GetPivotData( ScDPGetPivotDataField& rTarget, /* returns result */
                                  const std::vector< ScDPGetPivotDataField >& rFilters );
    long            GetHeaderDim( const ScAddress& rPos, sal_uInt16& rOrient );
    sal_Bool            GetHeaderDrag( const ScAddress& rPos, sal_Bool bMouseLeft, sal_Bool bMouseTop,
                                    long nDragDim,
                                    Rectangle& rPosRect, sal_uInt16& rOrient, long& rDimPos );
    sal_Bool            IsFilterButton( const ScAddress& rPos );

    void            GetMemberResultNames( ScStrCollection& rNames, long nDimension );

    void            SetHeaderLayout(bool bUseGrid);
    bool            GetHeaderLayout() const;

    static void     GetDataDimensionNames( String& rSourceName, String& rGivenName,
                                           const com::sun::star::uno::Reference<
                                               com::sun::star::uno::XInterface>& xDim );
};


#endif

