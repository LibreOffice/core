/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xpolyimp.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:00:02 $
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

#ifndef _XPOLYIMP_HXX
#define _XPOLYIMP_HXX

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _XPOLY_HXX
#include <xpoly.hxx>
#endif

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

    bool operator==(const ImpXPolygon& rImpXPoly) const;
    bool operator!=(const ImpXPolygon& rImpXPoly) const { return !operator==(rImpXPoly); }

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
DECLARE_LIST( XPolygonList, XPolygon* )

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

    bool operator==(const ImpXPolyPolygon& rImpXPolyPoly) const;
    bool operator!=(const ImpXPolyPolygon& rImpXPolyPoly) const { return !operator==(rImpXPolyPoly); }
};



#endif      // _XPOLYIMP_HXX
