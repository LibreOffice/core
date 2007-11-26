/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dpoutput.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 15:18:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_DPOUTPUT_HXX
#define SC_DPOUTPUT_HXX

#ifndef _COM_SUN_STAR_SHEET_XDIMENSIONSSUPPLIER_HPP_
#include <com/sun/star/sheet/XDimensionsSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_SHEET_DATARESULT_HPP_
#include <com/sun/star/sheet/DataResult.hpp>
#endif

#ifndef _COM_SUN_STAR_SHEET_MEMBERRESULT_HPP_
#include <com/sun/star/sheet/MemberResult.hpp>
#endif

#ifndef _COM_SUN_STAR_SHEET_GENERALFUNCTION_HPP_
#include <com/sun/star/sheet/GeneralFunction.hpp>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

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

