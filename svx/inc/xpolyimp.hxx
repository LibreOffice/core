/*************************************************************************
 *
 *  $RCSfile: xpolyimp.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:05 $
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

#ifndef _XPOLYIMP_HXX
#define _XPOLYIMP_HXX

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _XPOLY_HXX
#include <xpoly.hxx>
#endif

#pragma hdrstop


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

    void CheckPointDelete() { if ( bDeleteOldPoints )
                              { delete pOldPointAry;
                                bDeleteOldPoints = FALSE; } }
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
DECLARE_LIST( XPolygonList, XPolygon* );

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

    FASTBOOL operator==(const ImpXPolyPolygon& rImpXPolyPoly) const;
    FASTBOOL operator!=(const ImpXPolyPolygon& rImpXPolyPoly) const { return !operator==(rImpXPolyPoly); }
};



#endif      // _XPOLYIMP_HXX
