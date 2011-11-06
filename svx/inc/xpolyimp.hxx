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



#ifndef _XPOLYIMP_HXX
#define _XPOLYIMP_HXX

#include <tools/gen.hxx>
#include <svx/xpoly.hxx>

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
    sal_uInt8*           pFlagAry;
    Point*          pOldPointAry;
    sal_Bool            bDeleteOldPoints;
    sal_uInt16          nSize;
    sal_uInt16          nResize;
    sal_uInt16          nPoints;
    sal_uInt16          nRefCount;

    ImpXPolygon( sal_uInt16 nInitSize = 16, sal_uInt16 nResize=16 );
    ImpXPolygon( const ImpXPolygon& rImpXPoly );
    ~ImpXPolygon();

    bool operator==(const ImpXPolygon& rImpXPoly) const;
    bool operator!=(const ImpXPolygon& rImpXPoly) const { return !operator==(rImpXPoly); }

    void CheckPointDelete()
    {
        if ( bDeleteOldPoints )
        {
            delete[] (char*)pOldPointAry;
            bDeleteOldPoints = sal_False;
        }
    }

    void Resize( sal_uInt16 nNewSize, sal_Bool bDeletePoints = sal_True );
    void InsertSpace( sal_uInt16 nPos, sal_uInt16 nCount );
    void Remove( sal_uInt16 nPos, sal_uInt16 nCount );
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
    sal_uInt16       nRefCount;

                 ImpXPolyPolygon( sal_uInt16 nInitSize = 16, sal_uInt16 nResize = 16 ) :
                    aXPolyList( 1024, nInitSize, nResize )
                    { nRefCount = 1; }
                ImpXPolyPolygon( const ImpXPolyPolygon& rImpXPolyPoly );
                ~ImpXPolyPolygon();

    bool operator==(const ImpXPolyPolygon& rImpXPolyPoly) const;
    bool operator!=(const ImpXPolyPolygon& rImpXPolyPoly) const { return !operator==(rImpXPolyPoly); }
};



#endif      // _XPOLYIMP_HXX
