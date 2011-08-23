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

#ifndef _XPOLYIMP_HXX
#define _XPOLYIMP_HXX

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _XPOLY_HXX
#include <bf_svx/xpoly.hxx>
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif
namespace binfilter {


/*************************************************************************
|*
|*    class ImpXPolygon
|*
|*    Beschreibung
|*    Ersterstellung       08.11.94
|*    Letzte Aenderung Joe 26.09.95
|*
*************************************************************************/
class ImpXPolygon
{
public:
    Point*          pPointAry;
    BYTE*           pFlagAry;
    Point*          pOldPointAry;
    BOOL            bDeleteOldPoints;
    USHORT          nSize;
    USHORT          nResize;
    USHORT          nPoints;
    USHORT          nRefCount;

    ImpXPolygon( USHORT nInitSize = 16, USHORT nResize=16 );
    ImpXPolygon( const ImpXPolygon& rImpXPoly );
    ~ImpXPolygon();

    FASTBOOL operator==(const ImpXPolygon& rImpXPoly) const;
    FASTBOOL operator!=(const ImpXPolygon& rImpXPoly) const { return !operator==(rImpXPoly); }

    void CheckPointDelete()
    {
        if ( bDeleteOldPoints )
        {
            delete[] (char*)pOldPointAry;
            bDeleteOldPoints = FALSE;
        }
    }

    void Resize( USHORT nNewSize, BOOL bDeletePoints = TRUE );
    void InsertSpace( USHORT nPos, USHORT nCount );
    void Remove( USHORT nPos, USHORT nCount );
};


/*************************************************************************
|*
|*    class ImpXPolyPolygon
|*
|*    Beschreibung
|*    Ersterstellung          08.11.94
|*    Letzte Aenderung  Joe 26-09-1995
|*
*************************************************************************/
DECLARE_LIST( XPolygonList, XPolygon* )//STRIP008 DECLARE_LIST( XPolygonList, XPolygon* );

class ImpXPolyPolygon
{
public:
    XPolygonList aXPolyList;
    USHORT       nRefCount;

                 ImpXPolyPolygon( USHORT nInitSize = 16, USHORT nResize = 16 ) :
                    aXPolyList( 1024, nInitSize, nResize )
                    { nRefCount = 1; }
                ImpXPolyPolygon( const ImpXPolyPolygon& rImpXPolyPoly );
                ~ImpXPolyPolygon();

};



}//end of namespace binfilter
#endif      // _XPOLYIMP_HXX
