/*************************************************************************
 *
 *  $RCSfile: dpoutput.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:48 $
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

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

class Rectangle;
class SvStream;
class ScDocument;

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
    String                  aDataDescription;
    UINT32*                 pColNumFmt;
    UINT32*                 pRowNumFmt;
    long                    nColFmtCount;
    long                    nRowFmtCount;

    BOOL                    bSizesValid;
    long                    nColCount;
    long                    nRowCount;
    long                    nHeaderSize;
    USHORT                  nTabStartCol;
    USHORT                  nTabStartRow;
    USHORT                  nMemberStartCol;
    USHORT                  nMemberStartRow;
    USHORT                  nDataStartCol;
    USHORT                  nDataStartRow;
    USHORT                  nTabEndCol;
    USHORT                  nTabEndRow;

    void            DataCell( USHORT nCol, USHORT nRow, USHORT nTab,
                                const com::sun::star::sheet::DataResult& rData );
    void            HeaderCell( USHORT nCol, USHORT nRow, USHORT nTab,
                                const com::sun::star::sheet::MemberResult& rData,
                                BOOL bColHeader, long nLevel );
    void            FieldCell( USHORT nCol, USHORT nRow, USHORT nTab, const String& rCaption );
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

    void            GetPositionData( ScDPPositionData& rData, const ScAddress& rPos );
    long            GetHeaderDim( const ScAddress& rPos );
    BOOL            GetHeaderDrag( const ScAddress& rPos, BOOL bMouseLeft, BOOL bMouseTop,
                                    long nDragDim,
                                    Rectangle& rPosRect, USHORT& rOrient, long& rDimPos );
    BOOL            IsFilterButton( const ScAddress& rPos );
};


#endif

