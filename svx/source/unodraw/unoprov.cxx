/*************************************************************************
 *
 *  $RCSfile: unoprov.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2000-11-17 10:14:03 $
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

#define _SVX_USE_UNOGLOBALS_

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#include "unotext.hxx"
#include "unoshprp.hxx"
#include "svdobj.hxx"
#include "globl3d.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;

// Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
SfxItemPropertyMap* ImplGetSvxShapePropertyMap()
{
    static SfxItemPropertyMap aShapePropertyMap_Impl[] =
    {
        EDGERADIUS_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        AUTOSHAPE_PROPERTIES
        {0,0,0,0,0}
    };

    return aShapePropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxConnectorPropertyMap()
{
    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    static SfxItemPropertyMap aConnectorPropertyMap_Impl[] =
    {
        SPECIAL_CONNECTOR_PROPERTIES
        EDGERADIUS_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        {0,0,0,0,0}
    };

    return aConnectorPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxDimensioningPropertyMap()
{
    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    static SfxItemPropertyMap aDimensioningPropertyMap_Impl[] =
    {
        SPECIAL_DIMENSIONING_PROPERTIES
        EDGERADIUS_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        {0,0,0,0,0}
    };

    return aDimensioningPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxCirclePropertyMap()
{
    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    static SfxItemPropertyMap aCirclePropertyMap_Impl[] =
    {
        SPECIAL_CIRCLE_PROPERTIES
        EDGERADIUS_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        AUTOSHAPE_PROPERTIES
        {0,0,0,0,0}
    };

    return aCirclePropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxPolyPolygonPropertyMap()
{
    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    static SfxItemPropertyMap aPolyPolygonPropertyMap_Impl[] =
    {
        SPECIAL_POLYGON_PROPERTIES
        SPECIAL_POLYPOLYGON_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        AUTOSHAPE_PROPERTIES
        {0,0,0,0,0}
    };

    return aPolyPolygonPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxPolyPolygonBezierPropertyMap()
{
    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    static SfxItemPropertyMap aPolyPolygonBezierPropertyMap_Impl[] =
    {
        SPECIAL_POLYGON_PROPERTIES
        SPECIAL_POLYPOLYGONBEZIER_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        AUTOSHAPE_PROPERTIES
        {0,0,0,0,0}
    };

    return aPolyPolygonBezierPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxGraphicObjectPropertyMap()
{
    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    static SfxItemPropertyMap aGraphicObjectPropertyMap_Impl[] =
    {
        SPECIAL_GRAPHOBJ_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        { MAP_CHAR_LEN("IsMirrored"),               OWN_ATTR_MIRRORED           , &::getCppuBooleanType(), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        {0,0,0,0,0}
    };

    return aGraphicObjectPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvx3DSceneObjectPropertyMap()
{
    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    static SfxItemPropertyMap a3DSceneObjectPropertyMap_Impl[] =
    {
        SPECIAL_3DSCENEOBJECT_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        {0,0,0,0,0}
    };

    return a3DSceneObjectPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvx3DCubeObjectPropertyMap()
{
    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    static SfxItemPropertyMap a3DCubeObjectPropertyMap_Impl[] =
    {
        SPECIAL_3DCUBEOBJECT_PROPERTIES
        MISC_3D_OBJ_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        {0,0,0,0,0}
    };

    return a3DCubeObjectPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvx3DSphereObjectPropertyMap()
{
    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    static SfxItemPropertyMap a3DSphereObjectPropertyMap_Impl[] =
    {
        SPECIAL_3DSPHEREOBJECT_PROPERTIES
        MISC_3D_OBJ_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        {0,0,0,0,0}
    };
    return a3DSphereObjectPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvx3DLatheObjectPropertyMap()
{
    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    static SfxItemPropertyMap a3DLatheObjectPropertyMap_Impl[] =
    {
        SPECIAL_3DLATHEOBJECT_PROPERTIES
        SPECIAL_3DBACKSCALE_PROPERTIES
        MISC_3D_OBJ_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        {0,0,0,0,0}
    };

    return a3DLatheObjectPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvx3DExtrudeObjectPropertyMap()
{
    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    static SfxItemPropertyMap a3DExtrudeObjectPropertyMap_Impl[] =
    {
        SPECIAL_3DEXTRUDEOBJECT_PROPERTIES
        SPECIAL_3DBACKSCALE_PROPERTIES
        MISC_3D_OBJ_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        {0,0,0,0,0}
    };

    return a3DExtrudeObjectPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvx3DPolygonObjectPropertyMap()
{
    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    static SfxItemPropertyMap a3DPolygonObjectPropertyMap_Impl[] =
    {
        SPECIAL_3DPOLYGONOBJECT_PROPERTIES
        MISC_3D_OBJ_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        {0,0,0,0,0}
    };

    return a3DPolygonObjectPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxAllPropertyMap()
{
    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    static SfxItemPropertyMap aAllPropertyMap_Impl[] =
    {
        SHADOW_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        FILL_PROPERTIES
        EDGERADIUS_PROPERTIES
        TEXT_PROPERTIES
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SPECIAL_CONNECTOR_PROPERTIES
        SPECIAL_DIMENSIONING_PROPERTIES
        SPECIAL_CIRCLE_PROPERTIES
        SPECIAL_POLYGON_PROPERTIES
        SPECIAL_POLYPOLYGON_PROPERTIES
        SPECIAL_POLYPOLYGONBEZIER_PROPERTIES
        SPECIAL_GRAPHOBJ_PROPERTIES
        SPECIAL_3DSCENEOBJECT_PROPERTIES
        MISC_3D_OBJ_PROPERTIES
        SPECIAL_3DCUBEOBJECT_PROPERTIES
        SPECIAL_3DSPHEREOBJECT_PROPERTIES
        SPECIAL_3DLATHEOBJECT_PROPERTIES
        SPECIAL_3DEXTRUDEOBJECT_PROPERTIES
        SPECIAL_3DBACKSCALE_PROPERTIES
        SPECIAL_3DPOLYGONOBJECT_PROPERTIES
        AUTOSHAPE_PROPERTIES
        {0,0,0,0,0}
    };

    return aAllPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxGroupPropertyMap()
{
    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    static SfxItemPropertyMap aGroupPropertyMap_Impl[] =
    {
        SVX_UNOEDIT_PARA_PROPERTIES,
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_SIMPLE_FONT_PROPERTIES,
        SHADOW_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        FILL_PROPERTIES
        EDGERADIUS_PROPERTIES
        TEXT_PROPERTIES
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SPECIAL_CONNECTOR_PROPERTIES
        SPECIAL_DIMENSIONING_PROPERTIES
        SPECIAL_CIRCLE_PROPERTIES
        SPECIAL_POLYGON_PROPERTIES
        SPECIAL_POLYPOLYGON_PROPERTIES
        SPECIAL_POLYPOLYGONBEZIER_PROPERTIES
        SPECIAL_GRAPHOBJ_PROPERTIES
        SPECIAL_3DSCENEOBJECT_PROPERTIES
        MISC_3D_OBJ_PROPERTIES
        AUTOSHAPE_PROPERTIES
        {0,0,0,0,0}
    };

    return aGroupPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxOle2PropertyMap()
{
    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    static SfxItemPropertyMap aOle2PropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERID),  SDRATTR_LAYERID             , &::getCppuType((const sal_Int16*)0),      ::com::sun::star::beans::PropertyAttribute::READONLY,   0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERNAME),SDRATTR_LAYERNAME           , &::getCppuType((const ::rtl::OUString*)0),    ::com::sun::star::beans::PropertyAttribute::READONLY,   0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP), OWN_ATTR_LDBITMAP           , &::getCppuType(( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME),   OWN_ATTR_LDNAME             , &::getCppuType(( const ::rtl::OUString*)0),   ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_OLE2_METAFILE),     OWN_ATTR_METAFILE           , SEQTYPE(::getCppuType((::com::sun::star::uno::Sequence<sal_Int8>*)0)), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("Model"),                    OWN_ATTR_OLEMODEL           , &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("OriginalSize"),             OWN_ATTR_OLESIZE            , &::getCppuType(( const ::com::sun::star::awt::Size*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("CLSID"),                    OWN_ATTR_CLSID              , &::getCppuType(( const ::rtl::OUString*)0), 0, 0 },
        {0,0,0,0,0}
    };

    return aOle2PropertyMap_Impl;
}

// ---------------------------------------------------------------------

SvxUnoPropertyMapProvider aSvxMapProvider;

UHashMapEntry pSdrShapeIdentifierMap[] =
{
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("RectangleShape"),        OBJ_RECT ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("EllipseShape"),          OBJ_CIRC ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("ControlShape"),          OBJ_UNO  ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("ConnectorShape"),        OBJ_EDGE ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("MeasureShape"),          OBJ_MEASURE ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("LineShape"),             OBJ_LINE ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("PolyPolygonShape"),      OBJ_POLY ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("PolyLineShape"),         OBJ_PLIN ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("OpenBezierShape"),       OBJ_PATHLINE ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("ClosedBezierShape"),     OBJ_PATHFILL ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("OpenFreeHandShape"),     OBJ_FREELINE ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("ClosedFreeHandShape"),   OBJ_FREEFILL ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("PolyPolygonPathShape"),  OBJ_PATHPOLY ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("PolyLinePathShape"),     OBJ_PATHPLIN ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("GraphicObjectShape"),    OBJ_GRAF ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("GroupShape"),            OBJ_GRUP ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("TextShape"),             OBJ_TEXT ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("OLE2Shape"),             OBJ_OLE2 ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("PageShape"),             OBJ_PAGE ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("FrameShape"),            OBJ_FRAME ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("CaptionShape"),          OBJ_CAPTION ),

    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("Shape3DSceneObject"),    E3D_POLYSCENE_ID  | E3D_INVENTOR_FLAG ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("Shape3DCubeObject"),     E3D_CUBEOBJ_ID    | E3D_INVENTOR_FLAG ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("Shape3DSphereObject"),   E3D_SPHEREOBJ_ID  | E3D_INVENTOR_FLAG ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("Shape3DLatheObject"),    E3D_LATHEOBJ_ID   | E3D_INVENTOR_FLAG ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("Shape3DExtrudeObject"),  E3D_EXTRUDEOBJ_ID | E3D_INVENTOR_FLAG ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("Shape3DPolygonObject"),  E3D_POLYGONOBJ_ID | E3D_INVENTOR_FLAG ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM(""),  0 )
};

// ---------------------------------------------------------------------

UHashMap aSdrShapeIdentifierMap( pSdrShapeIdentifierMap );

/***********************************************************************
* class UHashMap                                                       *
***********************************************************************/

UHashMap::UHashMap( UHashMapEntry* pMap )
{
    while( pMap->aIdentifier.getLength() )
    {
        OUString aStr( pMap->aIdentifier );
        size_t nHash = aStr.hashCode() & (HASHARRAYSIZE-1);

        m_aHashList[nHash].Insert(pMap);
        pMap++;
    }
}

// ---------------------------------------------------------------------

UINT32 UHashMap::getId( const OUString& rCompareString )
{
    size_t nHash = rCompareString.hashCode() & (HASHARRAYSIZE-1);

    UHashMapEntryList& rList = m_aHashList[nHash];

    UHashMapEntry * pMap = rList.First();

    while(pMap)
    {
        if( rCompareString == pMap->aIdentifier )
            return pMap->nId;

        pMap = rList.Next();
    }

    return UHASHMAP_NOTFOUND;
}

/***********************************************************************
* class SvxUnoPropertyMapProvider                                      *
***********************************************************************/

EXTERN_C
#if defined( PM2 ) && (!defined( CSET ) && !defined ( MTW ) && !defined( WTC ))
int _stdcall
#else
#ifdef WNT
int _cdecl
#else
int
#endif
#endif
Svx_CompareMap(const void* pSmaller, const void* pBigger )
{
    int nDiff = strcmp( ((const SfxItemPropertyMap*)pSmaller)->pName,
                        ((const SfxItemPropertyMap*)pBigger)->pName );
    return nDiff;
}

// ---------------------------------------------------------------------

SvxUnoPropertyMapProvider::SvxUnoPropertyMapProvider()
{
    for(UINT16 i=0;i<SVXMAP_END;aMapArr[i++]=NULL);
}

// ---------------------------------------------------------------------

void SvxUnoPropertyMapProvider::Sort(USHORT nId)
{
    SfxItemPropertyMap* pTemp = aMapArr[nId];
    UINT16 i = 0;
    while(pTemp[i].pName) { i++; }
    qsort(aMapArr[nId], i, sizeof(SfxItemPropertyMap), Svx_CompareMap);
}

// ---------------------------------------------------------------------

SfxItemPropertyMap* SvxUnoPropertyMapProvider::GetMap(UINT16 nPropertyId)
{
    DBG_ASSERT(nPropertyId < SVXMAP_END, "Id ?" )
    if(!aMapArr[nPropertyId]) {
        switch(nPropertyId) {
            case SVXMAP_SHAPE: aMapArr[SVXMAP_SHAPE]=ImplGetSvxShapePropertyMap(); break;
            case SVXMAP_CONNECTOR: aMapArr[SVXMAP_CONNECTOR]=ImplGetSvxConnectorPropertyMap(); break;
            case SVXMAP_DIMENSIONING: aMapArr[SVXMAP_DIMENSIONING]=ImplGetSvxDimensioningPropertyMap(); break;
            case SVXMAP_CIRCLE: aMapArr[SVXMAP_CIRCLE]=ImplGetSvxCirclePropertyMap(); break;
            case SVXMAP_POLYPOLYGON: aMapArr[SVXMAP_POLYPOLYGON]=ImplGetSvxPolyPolygonPropertyMap(); break;
            case SVXMAP_POLYPOLYGONBEZIER: aMapArr[SVXMAP_POLYPOLYGONBEZIER]=ImplGetSvxPolyPolygonBezierPropertyMap(); break;
            case SVXMAP_GRAPHICOBJECT: aMapArr[SVXMAP_GRAPHICOBJECT]=ImplGetSvxGraphicObjectPropertyMap(); break;
            case SVXMAP_3DSCENEOBJECT: aMapArr[SVXMAP_3DSCENEOBJECT]=ImplGetSvx3DSceneObjectPropertyMap(); break;
            case SVXMAP_3DCUBEOBJEKT: aMapArr[SVXMAP_3DCUBEOBJEKT]=ImplGetSvx3DCubeObjectPropertyMap(); break;
            case SVXMAP_3DSPHEREOBJECT: aMapArr[SVXMAP_3DSPHEREOBJECT]=ImplGetSvx3DSphereObjectPropertyMap(); break;
            case SVXMAP_3DLATHEOBJECT: aMapArr[SVXMAP_3DLATHEOBJECT]=ImplGetSvx3DLatheObjectPropertyMap(); break;
            case SVXMAP_3DEXTRUDEOBJECT: aMapArr[SVXMAP_3DEXTRUDEOBJECT]=ImplGetSvx3DExtrudeObjectPropertyMap(); break;
            case SVXMAP_3DPOLYGONOBJECT: aMapArr[SVXMAP_3DPOLYGONOBJECT]=ImplGetSvx3DPolygonObjectPropertyMap(); break;
            case SVXMAP_ALL: aMapArr[SVXMAP_ALL]=ImplGetSvxAllPropertyMap(); break;
            case SVXMAP_GROUP: aMapArr[SVXMAP_GROUP]=ImplGetSvxGroupPropertyMap(); break;
            default:
                DBG_ERROR( "Unknown property map for SvxUnoPropertyMapProvider!" );
        }
        Sort(nPropertyId);
    }
    return aMapArr[nPropertyId];
}

// #####################################################################

/** returns an empty UString(). most times sufficient */
::rtl::OUString SAL_CALL SvxServiceInfoHelper::getImplementationName() throw( ::com::sun::star::uno::RuntimeException )
{
    return ::rtl::OUString();
}

/** the base implementation iterates over the service names from <code>getSupportedServiceNames</code> */
sal_Bool SAL_CALL SvxServiceInfoHelper::supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    return supportsService( ServiceName, getSupportedServiceNames() );
}

sal_Bool SAL_CALL SvxServiceInfoHelper::supportsService( const ::rtl::OUString& ServiceName, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& SupportedServices ) throw()
{
    const ::rtl::OUString * pArray = SupportedServices.getConstArray();
    for( INT32 i = 0; i < SupportedServices.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return TRUE;
    return FALSE;
}

/** the base implementation has no supported services */
::com::sun::star::uno::Sequence< ::rtl::OUString > SvxServiceInfoHelper::getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq(0);
    return aSeq;
}

/** this method concatenates the given sequences and returns the result
 */
::com::sun::star::uno::Sequence< ::rtl::OUString > SvxServiceInfoHelper::concatSequences( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rSeq1, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rSeq2 ) throw()
{
    const sal_Int32 nLen1 = rSeq1.getLength();
    const sal_Int32 nLen2 = rSeq2.getLength();

    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSeq( nLen1 + nLen2 );

    ::rtl::OUString* pStrings = aSeq.getArray();

    sal_Int32 nIdx;
    const ::rtl::OUString* pStringSrc = rSeq1.getConstArray();
    for( nIdx = 0; nIdx < nLen1; nIdx++ )
        *pStrings++ = *pStringSrc++;

    pStringSrc = rSeq2.getConstArray();
    for( nIdx = 0; nIdx < nLen2; nIdx++ )
        *pStrings++ = *pStringSrc++;

    return aSeq;
}

/** this method adds a variable number of char pointer to a given Sequence
 */
void SvxServiceInfoHelper::addToSequence( ::com::sun::star::uno::Sequence< ::rtl::OUString >& rSeq, UINT16 nServices, /* char * */ ... ) throw()
{
    UINT32 nCount = rSeq.getLength();

    rSeq.realloc( nCount + nServices );
    OUString* pStrings = rSeq.getArray();

    va_list marker;
    va_start( marker, nServices );
    for( UINT16 i = 0 ; i < nServices; i++ )
        pStrings[nCount++] = OUString::createFromAscii(va_arg( marker, char*));
    va_end( marker );
}



