/*************************************************************************
 *
 *  $RCSfile: unoprov.cxx,v $
 *
 *  $Revision: 1.54 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 13:22:52 $
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

#ifndef _COM_SUN_STAR_UTIL_MEASUREUNIT_HPP_
#include <com/sun/star/util/MeasureUnit.hpp>
#endif

#ifndef _VCL_FLDUNIT_HXX
#include <vcl/fldunit.hxx>
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include <comphelper/propertysetinfo.hxx>

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

#include "unotext.hxx"
#include "unoshprp.hxx"
#include "svdobj.hxx"
#include "globl3d.hxx"
#include "dialogs.hrc"

using namespace ::rtl;
using namespace ::com::sun::star;

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
        // #FontWork#
        FONTWORK_PROPERTIES
        CUSTOMSHAPE_PROPERTIES
        { MAP_CHAR_LEN("UserDefinedAttributes"),        SDRATTR_XMLATTRIBUTES,      &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {0,0,0,0,0}
    };

    return aShapePropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxTextShapePropertyMap()
{
    static SfxItemPropertyMap aTextShapePropertyMap_Impl[] =
    {
        EDGERADIUS_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES_NO_SHEAR
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        // #FontWork#
        FONTWORK_PROPERTIES
        CUSTOMSHAPE_PROPERTIES
        { MAP_CHAR_LEN("UserDefinedAttributes"),        SDRATTR_XMLATTRIBUTES,      &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {0,0,0,0,0}
    };

    return aTextShapePropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxConnectorPropertyMap()
{
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
        // #FontWork#
        FONTWORK_PROPERTIES
        { MAP_CHAR_LEN("UserDefinedAttributes"),        SDRATTR_XMLATTRIBUTES,      &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {0,0,0,0,0}
    };

    return aConnectorPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxDimensioningPropertyMap()
{
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
        // #FontWork#
        FONTWORK_PROPERTIES
        { MAP_CHAR_LEN("UserDefinedAttributes"),        SDRATTR_XMLATTRIBUTES,      &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {0,0,0,0,0}
    };

    return aDimensioningPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxCirclePropertyMap()
{
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
        // #FontWork#
        FONTWORK_PROPERTIES
        CUSTOMSHAPE_PROPERTIES
        { MAP_CHAR_LEN("UserDefinedAttributes"),        SDRATTR_XMLATTRIBUTES,      &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {0,0,0,0,0}
    };

    return aCirclePropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxPolyPolygonPropertyMap()
{
    static SfxItemPropertyMap aPolyPolygonPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("Geometry"), OWN_ATTR_BASE_GEOMETRY, SEQTYPE(::getCppuType((const ::com::sun::star::drawing::PointSequenceSequence*)0)), 0, 0 },
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
        // #FontWork#
        FONTWORK_PROPERTIES
        CUSTOMSHAPE_PROPERTIES
        { MAP_CHAR_LEN("UserDefinedAttributes"),        SDRATTR_XMLATTRIBUTES,      &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {0,0,0,0,0}
    };

    return aPolyPolygonPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxPolyPolygonBezierPropertyMap()
{
    static SfxItemPropertyMap aPolyPolygonBezierPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("Geometry"), OWN_ATTR_BASE_GEOMETRY, &::getCppuType((const ::com::sun::star::drawing::PolyPolygonBezierCoords*)0), 0, 0 },
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
        // #FontWork#
        FONTWORK_PROPERTIES
        CUSTOMSHAPE_PROPERTIES
        { MAP_CHAR_LEN("UserDefinedAttributes"),        SDRATTR_XMLATTRIBUTES,      &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {0,0,0,0,0}
    };

    return aPolyPolygonBezierPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxGraphicObjectPropertyMap()
{
    static SfxItemPropertyMap aGraphicObjectPropertyMap_Impl[] =
    {
        SPECIAL_GRAPHOBJ_PROPERTIES

        // #i25616#
        FILL_PROPERTIES

        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES_NO_SHEAR
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        // #FontWork#
        FONTWORK_PROPERTIES
        { MAP_CHAR_LEN("IsMirrored"),               OWN_ATTR_MIRRORED           , &::getCppuBooleanType(), 0, 0},
        { MAP_CHAR_LEN("UserDefinedAttributes"),        SDRATTR_XMLATTRIBUTES,      &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {0,0,0,0,0}
    };

    return aGraphicObjectPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvx3DSceneObjectPropertyMap()
{
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
        { MAP_CHAR_LEN("UserDefinedAttributes"),        SDRATTR_XMLATTRIBUTES,      &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {0,0,0,0,0}
    };

    return a3DCubeObjectPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvx3DSphereObjectPropertyMap()
{
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
        { MAP_CHAR_LEN("UserDefinedAttributes"),        SDRATTR_XMLATTRIBUTES,      &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {0,0,0,0,0}
    };
    return a3DSphereObjectPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvx3DLatheObjectPropertyMap()
{
    static SfxItemPropertyMap a3DLatheObjectPropertyMap_Impl[] =
    {
        SPECIAL_3DLATHEOBJECT_PROPERTIES

        // #107245# New 3D properties which are possible for lathe and extrude 3d objects
        SPECIAL_3DLATHEANDEXTRUDEOBJ_PROPERTIES

        SPECIAL_3DBACKSCALE_PROPERTIES
        MISC_3D_OBJ_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        { MAP_CHAR_LEN("UserDefinedAttributes"),        SDRATTR_XMLATTRIBUTES,      &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {0,0,0,0,0}
    };

    return a3DLatheObjectPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvx3DExtrudeObjectPropertyMap()
{
    static SfxItemPropertyMap a3DExtrudeObjectPropertyMap_Impl[] =
    {
        SPECIAL_3DEXTRUDEOBJECT_PROPERTIES

        // #107245# New 3D properties which are possible for lathe and extrude 3d objects
        SPECIAL_3DLATHEANDEXTRUDEOBJ_PROPERTIES

        SPECIAL_3DBACKSCALE_PROPERTIES
        MISC_3D_OBJ_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        { MAP_CHAR_LEN("UserDefinedAttributes"),        SDRATTR_XMLATTRIBUTES,      &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {0,0,0,0,0}
    };

    return a3DExtrudeObjectPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvx3DPolygonObjectPropertyMap()
{
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
        { MAP_CHAR_LEN("UserDefinedAttributes"),        SDRATTR_XMLATTRIBUTES,      &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {0,0,0,0,0}
    };

    return a3DPolygonObjectPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxAllPropertyMap()
{
    static SfxItemPropertyMap aAllPropertyMap_Impl[] =
    {
        SHADOW_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        FILL_PROPERTIES
        EDGERADIUS_PROPERTIES
        TEXT_PROPERTIES
        // #FontWork#
        FONTWORK_PROPERTIES
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

        // #107245# New 3D properties which are possible for lathe and extrude 3d objects
        SPECIAL_3DLATHEANDEXTRUDEOBJ_PROPERTIES

        SPECIAL_3DBACKSCALE_PROPERTIES
        SPECIAL_3DPOLYGONOBJECT_PROPERTIES
        CUSTOMSHAPE_PROPERTIES
        { MAP_CHAR_LEN("UserDefinedAttributes"),        SDRATTR_XMLATTRIBUTES,      &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {0,0,0,0,0}
    };

    return aAllPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxGroupPropertyMap()
{
    static SfxItemPropertyMap aGroupPropertyMap_Impl[] =
    {
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        {0,0,0,0,0}
    };

    return aGroupPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxOle2PropertyMap()
{
    static SfxItemPropertyMap aOle2PropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("Transformation"),           OWN_ATTR_TRANSFORMATION     , &::getCppuType((const struct com::sun::star::drawing::HomogenMatrix3*)0), 0, 0 }, \
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_ZORDER),   OWN_ATTR_ZORDER             , &::getCppuType((const sal_Int32*)0),      0,      0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERID),  SDRATTR_LAYERID             , &::getCppuType((const sal_Int16*)0),      0,  0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERNAME),SDRATTR_LAYERNAME           , &::getCppuType((const ::rtl::OUString*)0),    0,  0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP), OWN_ATTR_LDBITMAP           , &::getCppuType(( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME),   OWN_ATTR_LDNAME             , &::getCppuType(( const ::rtl::OUString*)0),   ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_OLE2_METAFILE),     OWN_ATTR_METAFILE           , SEQTYPE(::getCppuType((::com::sun::star::uno::Sequence<sal_Int8>*)0)), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("ThumbnailGraphicURL"),      OWN_ATTR_THUMBNAIL          , &::getCppuType(( const ::rtl::OUString*)0), 0, 0 },
        { MAP_CHAR_LEN("Model"),                    OWN_ATTR_OLEMODEL           , &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("OriginalSize"),             OWN_ATTR_OLESIZE            , &::getCppuType(( const ::com::sun::star::awt::Size*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("CLSID"),                    OWN_ATTR_CLSID              , &::getCppuType(( const ::rtl::OUString*)0), 0, 0 },
        { MAP_CHAR_LEN("IsInternal"),               OWN_ATTR_INTERNAL_OLE       , &::getBooleanCppuType() , ::com::sun::star::beans::PropertyAttribute::READONLY,     0},
        { MAP_CHAR_LEN("VisibleArea"),              OWN_ATTR_OLE_VISAREA        , &::getCppuType((const ::com::sun::star::awt::Rectangle*)0), 0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_MOVEPROTECT), SDRATTR_OBJMOVEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_SIZEPROTECT), SDRATTR_OBJSIZEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OLE2_PERSISTNAME),  OWN_ATTR_PERSISTNAME        , &::getCppuType(( const ::rtl::OUString*)0), 0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_BOUNDRECT), OWN_ATTR_BOUNDRECT,            &::getCppuType((const ::com::sun::star::awt::Rectangle*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        {0,0,0,0,0}
    };

    return aOle2PropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxPluginPropertyMap()
{
    static SfxItemPropertyMap aPluginPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("PluginMimeType"),           OWN_ATTR_PLUGIN_MIMETYPE    , &::getCppuType((const ::rtl::OUString*)0),    0,  0},
        { MAP_CHAR_LEN("PluginURL"),                OWN_ATTR_PLUGIN_URL         , &::getCppuType((const ::rtl::OUString*)0),    0,  0},
        { MAP_CHAR_LEN("PluginCommands"),           OWN_ATTR_PLUGIN_COMMANDS    , SEQTYPE(::getCppuType((::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >*)0)), 0, 0},
        { MAP_CHAR_LEN("Transformation"),           OWN_ATTR_TRANSFORMATION     , &::getCppuType((const struct com::sun::star::drawing::HomogenMatrix3*)0), 0, 0 }, \
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_ZORDER),   OWN_ATTR_ZORDER             , &::getCppuType((const sal_Int32*)0),      0,      0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERID),  SDRATTR_LAYERID             , &::getCppuType((const sal_Int16*)0),      0,  0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERNAME),SDRATTR_LAYERNAME           , &::getCppuType((const ::rtl::OUString*)0),    0,  0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP), OWN_ATTR_LDBITMAP           , &::getCppuType(( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME),   OWN_ATTR_LDNAME             , &::getCppuType(( const ::rtl::OUString*)0),   ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_OLE2_METAFILE),     OWN_ATTR_METAFILE           , SEQTYPE(::getCppuType((::com::sun::star::uno::Sequence<sal_Int8>*)0)), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("ThumbnailGraphicURL"),      OWN_ATTR_THUMBNAIL          , &::getCppuType(( const ::rtl::OUString*)0), 0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_MOVEPROTECT),      SDRATTR_OBJMOVEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_SIZEPROTECT),      SDRATTR_OBJSIZEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OLE2_PERSISTNAME),  OWN_ATTR_PERSISTNAME        , &::getCppuType(( const ::rtl::OUString*)0), 0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_BOUNDRECT), OWN_ATTR_BOUNDRECT,            &::getCppuType((const ::com::sun::star::awt::Rectangle*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        {0,0,0,0,0}
    };

    return aPluginPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxFramePropertyMap()
{
    static SfxItemPropertyMap aFramePropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("FrameURL"),                 OWN_ATTR_FRAME_URL          , &::getCppuType((const ::rtl::OUString*)0),    0,  0},
        { MAP_CHAR_LEN("FrameName"),                OWN_ATTR_FRAME_NAME         , &::getCppuType((const ::rtl::OUString*)0),    0,  0},
        { MAP_CHAR_LEN("FrameIsAutoScroll"),        OWN_ATTR_FRAME_ISAUTOSCROLL , &::getBooleanCppuType() , ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { MAP_CHAR_LEN("FrameIsBorder"),            OWN_ATTR_FRAME_ISBORDER     , &::getBooleanCppuType() , 0, 0},
        { MAP_CHAR_LEN("FrameMarginWidth"),         OWN_ATTR_FRAME_MARGIN_WIDTH , &::getCppuType((const sal_Int32*)0),      0,      0},
        { MAP_CHAR_LEN("FrameMarginHeight"),        OWN_ATTR_FRAME_MARGIN_HEIGHT, &::getCppuType((const sal_Int32*)0),      0,      0},
        { MAP_CHAR_LEN("Transformation"),           OWN_ATTR_TRANSFORMATION     , &::getCppuType((const struct com::sun::star::drawing::HomogenMatrix3*)0), 0, 0 }, \
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_ZORDER),   OWN_ATTR_ZORDER             , &::getCppuType((const sal_Int32*)0),      0,      0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERID),  SDRATTR_LAYERID             , &::getCppuType((const sal_Int16*)0),      0,  0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERNAME),SDRATTR_LAYERNAME           , &::getCppuType((const ::rtl::OUString*)0),    0,  0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP), OWN_ATTR_LDBITMAP           , &::getCppuType(( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME),   OWN_ATTR_LDNAME             , &::getCppuType(( const ::rtl::OUString*)0),   ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_OLE2_METAFILE),     OWN_ATTR_METAFILE           , SEQTYPE(::getCppuType((::com::sun::star::uno::Sequence<sal_Int8>*)0)), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("ThumbnailGraphicURL"),      OWN_ATTR_THUMBNAIL          , &::getCppuType(( const ::rtl::OUString*)0), 0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_MOVEPROTECT),      SDRATTR_OBJMOVEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_SIZEPROTECT),      SDRATTR_OBJSIZEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OLE2_PERSISTNAME),  OWN_ATTR_PERSISTNAME        , &::getCppuType(( const ::rtl::OUString*)0), 0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_BOUNDRECT), OWN_ATTR_BOUNDRECT,            &::getCppuType((const ::com::sun::star::awt::Rectangle*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        {0,0,0,0,0}
    };

    return aFramePropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxAppletPropertyMap()
{
    static SfxItemPropertyMap aAppletPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("AppletCodeBase"),           OWN_ATTR_APPLET_CODEBASE    , &::getCppuType(( const ::rtl::OUString*)0), 0, 0},
        { MAP_CHAR_LEN("AppletName"),               OWN_ATTR_APPLET_NAME        , &::getCppuType(( const ::rtl::OUString*)0), 0, 0},
        { MAP_CHAR_LEN("AppletCode"),               OWN_ATTR_APPLET_CODE        , &::getCppuType(( const ::rtl::OUString*)0), 0, 0},
        { MAP_CHAR_LEN("AppletCommands"),           OWN_ATTR_APPLET_COMMANDS    , SEQTYPE(::getCppuType((::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >*)0)), 0, 0},
        { MAP_CHAR_LEN("AppletIsScript"),           OWN_ATTR_APPLET_ISSCRIPT    , &::getBooleanCppuType(), 0, 0 },
        { MAP_CHAR_LEN("Transformation"),           OWN_ATTR_TRANSFORMATION     , &::getCppuType((const struct com::sun::star::drawing::HomogenMatrix3*)0), 0, 0 }, \
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_ZORDER),   OWN_ATTR_ZORDER             , &::getCppuType((const sal_Int32*)0),      0,      0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERID),  SDRATTR_LAYERID             , &::getCppuType((const sal_Int16*)0),      0,  0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERNAME),SDRATTR_LAYERNAME           , &::getCppuType((const ::rtl::OUString*)0),    0,  0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP), OWN_ATTR_LDBITMAP           , &::getCppuType(( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME),   OWN_ATTR_LDNAME             , &::getCppuType(( const ::rtl::OUString*)0),   ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_OLE2_METAFILE),     OWN_ATTR_METAFILE           , SEQTYPE(::getCppuType((::com::sun::star::uno::Sequence<sal_Int8>*)0)), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("ThumbnailGraphicURL"),      OWN_ATTR_THUMBNAIL          , &::getCppuType(( const ::rtl::OUString*)0), 0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_MOVEPROTECT),      SDRATTR_OBJMOVEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_SIZEPROTECT),      SDRATTR_OBJSIZEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OLE2_PERSISTNAME),  OWN_ATTR_PERSISTNAME        , &::getCppuType(( const ::rtl::OUString*)0), 0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_BOUNDRECT), OWN_ATTR_BOUNDRECT,            &::getCppuType((const ::com::sun::star::awt::Rectangle*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        {0,0,0,0,0}
    };

    return aAppletPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxControlShapePropertyMap()
{
    static SfxItemPropertyMap aControlPropertyMap_Impl[] =
    {
        // the following properties are mapped to the XControl Model of this shape
        { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTNAME),        0,  &::getCppuType((const ::rtl::OUString*)0),  0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTSTYLENAME),   0,  &::getCppuType((const ::rtl::OUString*)0),  0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTFAMILY),      0,  &::getCppuType((const sal_Int16*)0),        0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTCHARSET),     0,  &::getCppuType((const sal_Int16*)0),        0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_HEIGHT),          0,  &::getCppuType((const float*)0),            0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTPITCH),       0,  &::getCppuType((const sal_Int16*)0),        0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_POSTURE),         0,  &::getCppuType((const ::com::sun::star::awt::FontSlant*)0),0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_WEIGHT),          0,  &::getCppuType((const float*)0),            0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_UNDERLINE),       0,  &::getCppuType((const sal_Int16*)0),        0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_STRIKEOUT),       0,  &::getCppuType((const sal_Int16*)0),        0, 0},
        { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_COLOR),           0,  &::getCppuType((const sal_Int32*)0),        0, 0 },
        { MAP_CHAR_LEN("CharRelief"),                       0,  &::getCppuType((const sal_Int16*)0),        0, 0 },
        { MAP_CHAR_LEN("CharUnderlineColor"),               0,  &::getCppuType((const sal_Int32*)0),        0, 0 },
        { MAP_CHAR_LEN("CharKerning"),                      0,  &::getCppuType((const sal_Int16*)0),        0, 0 },
        { MAP_CHAR_LEN("CharWordMode"),                     0,  &::getBooleanCppuType(),                    0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_ADJUST),          0,  &::getCppuType((const sal_Int16*)0),        0, 0 },
        { MAP_CHAR_LEN("ControlBackground"),                0,  &::getCppuType((const sal_Int32*)0),        0, 0 },
        { MAP_CHAR_LEN("ControlBorder"),                    0,  &::getCppuType((const sal_Int16*)0),        0, 0 },
        { MAP_CHAR_LEN("ControlBorderColor"),               0,  &::getCppuType((const sal_Int32*)0),        0, 0 },
        { MAP_CHAR_LEN("ControlSymbolColor"),               0,  &::getCppuType((const sal_Int16*)0),        0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_MOVEPROTECT),      SDRATTR_OBJMOVEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_SIZEPROTECT),      SDRATTR_OBJSIZEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN("ControlTextEmphasis"),              0,  &::getCppuType((const sal_Int16*)0),        0, 0 },
        // the following properties are handled by SvxShape
        { MAP_CHAR_LEN("Transformation"),           OWN_ATTR_TRANSFORMATION     , &::getCppuType((const struct com::sun::star::drawing::HomogenMatrix3*)0), 0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_ZORDER),   OWN_ATTR_ZORDER             , &::getCppuType((const sal_Int32*)0),      0,      0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERID),  SDRATTR_LAYERID             , &::getCppuType((const sal_Int16*)0),      0,  0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERNAME),SDRATTR_LAYERNAME           , &::getCppuType((const ::rtl::OUString*)0),    0,  0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP), OWN_ATTR_LDBITMAP           , &::getCppuType(( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME),   OWN_ATTR_LDNAME             , &::getCppuType(( const ::rtl::OUString*)0),   ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("UserDefinedAttributes"),        SDRATTR_XMLATTRIBUTES,      &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_BOUNDRECT), OWN_ATTR_BOUNDRECT,            &::getCppuType((const ::com::sun::star::awt::Rectangle*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        {0,0,0,0,0}
    };

    return aControlPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxPageShapePropertyMap()
{
    static SfxItemPropertyMap aPageShapePropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("PageNumber"),               OWN_ATTR_PAGE_NUMBER        , &::getCppuType((const sal_Int32*)0),      0, 0},
        { MAP_CHAR_LEN("Transformation"),           OWN_ATTR_TRANSFORMATION     , &::getCppuType((const struct com::sun::star::drawing::HomogenMatrix3*)0), 0, 0 }, \
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_ZORDER),   OWN_ATTR_ZORDER             , &::getCppuType((const sal_Int32*)0),      0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERID),  SDRATTR_LAYERID             , &::getCppuType((const sal_Int16*)0),      0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERNAME),SDRATTR_LAYERNAME           , &::getCppuType((const ::rtl::OUString*)0), 0, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP), OWN_ATTR_LDBITMAP           , &::getCppuType(( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME),   OWN_ATTR_LDNAME             , &::getCppuType(( const ::rtl::OUString*)0),   ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_MOVEPROTECT),      SDRATTR_OBJMOVEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_SIZEPROTECT),      SDRATTR_OBJSIZEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_BOUNDRECT), OWN_ATTR_BOUNDRECT,            &::getCppuType((const ::com::sun::star::awt::Rectangle*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        {0,0,0,0,0}
    };

    return aPageShapePropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxCaptionPropertyMap()
{
    static SfxItemPropertyMap aCaptionPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("CaptionPoint"),             OWN_ATTR_CAPTION_POINT,     &::getCppuType((const com::sun::star::awt::Point*)0),   0, 0 },
        { MAP_CHAR_LEN("CaptionType"),              SDRATTR_CAPTIONTYPE,        &::getCppuType((const sal_Int16*)0), 0, 0},
        { MAP_CHAR_LEN("CaptionIsFixedAngle"),      SDRATTR_CAPTIONFIXEDANGLE,  &::getBooleanCppuType(), 0, 0},
        { MAP_CHAR_LEN("CaptionAngle"),             SDRATTR_CAPTIONANGLE,       &::getCppuType((const sal_Int32*)0),    0,  0},
        { MAP_CHAR_LEN("CaptionGap"),               SDRATTR_CAPTIONGAP,         &::getCppuType((const sal_Int32*)0),    0,  SFX_METRIC_ITEM},
        { MAP_CHAR_LEN("CaptionEscapeDirection"),   SDRATTR_CAPTIONESCDIR,      &::getCppuType((const sal_Int32*)0),    0,  0},
        { MAP_CHAR_LEN("CaptionIsEscapeRelative"),  SDRATTR_CAPTIONESCISREL,    &::getBooleanCppuType(), 0, 0},
        { MAP_CHAR_LEN("CaptionEscapeRelative"),    SDRATTR_CAPTIONESCREL,      &::getCppuType((const sal_Int32*)0),    0,  0},
        { MAP_CHAR_LEN("CaptionEscapeAbsolute"),    SDRATTR_CAPTIONESCABS,      &::getCppuType((const sal_Int32*)0),    0,  SFX_METRIC_ITEM},
        { MAP_CHAR_LEN("CaptionLineLength"),        SDRATTR_CAPTIONLINELEN,     &::getCppuType((const sal_Int32*)0),    0,  SFX_METRIC_ITEM},
        { MAP_CHAR_LEN("CaptionIsFitLineLength"),   SDRATTR_CAPTIONFITLINELEN,  &::getBooleanCppuType(), 0, 0},
        EDGERADIUS_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        // #FontWork#
        FONTWORK_PROPERTIES
        { MAP_CHAR_LEN("UserDefinedAttributes"),        SDRATTR_XMLATTRIBUTES,      &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {0,0,0,0,0}
    };

    return aCaptionPropertyMap_Impl;
}

SfxItemPropertyMap* ImplGetSvxCustomShapePropertyMap()
{
    static SfxItemPropertyMap aCustomShapePropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("CustomShapeEngine"),            SDRATTR_CUSTOMSHAPE_ENGINE,         &::getCppuType((const ::rtl::OUString*)0),  0, 0 },
        { MAP_CHAR_LEN("CustomShapeData"),          SDRATTR_CUSTOMSHAPE_DATA,               &::getCppuType((const ::rtl::OUString*)0),  0, 0 },
        { MAP_CHAR_LEN("CustomShapeGeometry"),      SDRATTR_CUSTOMSHAPE_GEOMETRY,           SEQTYPE(::getCppuType((::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >*)0)),  0, 0 },
        { MAP_CHAR_LEN("CustomShapeGraphicURL"),        SDRATTR_CUSTOMSHAPE_REPLACEMENT_URL,    &::getCppuType((const ::rtl::OUString*)0),  0,  0},
        CUSTOMSHAPE_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        {MAP_CHAR_LEN("UserDefinedAttributes"),     SDRATTR_XMLATTRIBUTES,      &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"), EE_PARA_XMLATTRIBS,         &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {0,0,0,0,0}
    };
    return aCustomShapePropertyMap_Impl;
}

comphelper::PropertyMapEntry* ImplGetSvxDrawingDefaultsPropertyMap()
{
    static comphelper::PropertyMapEntry aSvxDrawingDefaultsPropertyMap_Impl[] =
    {
        SHADOW_PROPERTIES
        LINE_PROPERTIES_DEFAULTS
        FILL_PROPERTIES_BMP
        FILL_PROPERTIES_DEFAULTS
        EDGERADIUS_PROPERTIES
        TEXT_PROPERTIES_DEFAULTS
        CONNECTOR_PROPERTIES
        SPECIAL_DIMENSIONING_PROPERTIES_DEFAULTS
        MISC_3D_OBJ_PROPERTIES
        SPECIAL_3DBACKSCALE_PROPERTIES
        {0,0,0,0,0}
    };

    return aSvxDrawingDefaultsPropertyMap_Impl;
}

// OD 13.10.2003 #i18732#
comphelper::PropertyMapEntry* ImplGetAdditionalWriterDrawingDefaultsPropertyMap()
{
    static comphelper::PropertyMapEntry aSvxAdditionalDefaultsPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("IsFollowingTextFlow"), SID_SW_FOLLOW_TEXT_FLOW, &::getBooleanCppuType(), 0, 0},
        {0,0,0,0,0}
    };

    return aSvxAdditionalDefaultsPropertyMap_Impl;
}

// ---------------------------------------------------------------------

SvxUnoPropertyMapProvider aSvxMapProvider;

UHashMapEntry pSdrShapeIdentifierMap[] =
{
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.RectangleShape"),       OBJ_RECT ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.EllipseShape"),         OBJ_CIRC ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.ControlShape"),         OBJ_UNO  ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.ConnectorShape"),       OBJ_EDGE ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.MeasureShape"),         OBJ_MEASURE ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.LineShape"),            OBJ_LINE ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.PolyPolygonShape"),     OBJ_POLY ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.PolyLineShape"),        OBJ_PLIN ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.OpenBezierShape"),      OBJ_PATHLINE ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.ClosedBezierShape"),    OBJ_PATHFILL ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.OpenFreeHandShape"),    OBJ_FREELINE ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.ClosedFreeHandShape"),  OBJ_FREEFILL ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.PolyPolygonPathShape"), OBJ_PATHPOLY ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.PolyLinePathShape"),    OBJ_PATHPLIN ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.GraphicObjectShape"),   OBJ_GRAF ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.GroupShape"),           OBJ_GRUP ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.TextShape"),            OBJ_TEXT ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.OLE2Shape"),            OBJ_OLE2 ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.PageShape"),            OBJ_PAGE ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.CaptionShape"),         OBJ_CAPTION ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.FrameShape"),           OBJ_FRAME ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.PluginShape"),          OBJ_OLE2_PLUGIN ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.AppletShape"),          OBJ_OLE2_APPLET ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.CustomShape"),          OBJ_CUSTOMSHAPE ),

    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.Shape3DSceneObject"),   E3D_POLYSCENE_ID  | E3D_INVENTOR_FLAG ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.Shape3DCubeObject"),    E3D_CUBEOBJ_ID    | E3D_INVENTOR_FLAG ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.Shape3DSphereObject"),  E3D_SPHEREOBJ_ID  | E3D_INVENTOR_FLAG ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.Shape3DLatheObject"),   E3D_LATHEOBJ_ID   | E3D_INVENTOR_FLAG ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.Shape3DExtrudeObject"), E3D_EXTRUDEOBJ_ID | E3D_INVENTOR_FLAG ),
    UHashMapEntry (RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.Shape3DPolygonObject"), E3D_POLYGONOBJ_ID | E3D_INVENTOR_FLAG ),
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
            case SVXMAP_CAPTION: aMapArr[SVXMAP_CAPTION]=ImplGetSvxCaptionPropertyMap(); break;
            case SVXMAP_OLE2: aMapArr[SVXMAP_OLE2]=ImplGetSvxOle2PropertyMap(); break;
            case SVXMAP_PLUGIN: aMapArr[SVXMAP_PLUGIN]=ImplGetSvxPluginPropertyMap(); break;
            case SVXMAP_FRAME: aMapArr[SVXMAP_FRAME]=ImplGetSvxFramePropertyMap(); break;
            case SVXMAP_APPLET: aMapArr[SVXMAP_APPLET]=ImplGetSvxAppletPropertyMap(); break;
            case SVXMAP_CONTROL: aMapArr[SVXMAP_CONTROL]=ImplGetSvxControlShapePropertyMap(); break;
            case SVXMAP_TEXT: aMapArr[SVXMAP_TEXT]=ImplGetSvxTextShapePropertyMap(); break;
            case SVXMAP_CUSTOMSHAPE: aMapArr[SVXMAP_CUSTOMSHAPE]=ImplGetSvxCustomShapePropertyMap(); break;

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



/** maps the API constant MeasureUnit to a vcl MapUnit enum.
    Returns false if conversion is not supported.
*/
sal_Bool SvxMeasureUnitToMapUnit( const short eApi, int& eVcl ) throw()
{
    switch( eVcl )
    {
    case util::MeasureUnit::MM_100TH:       eVcl = MAP_100TH_MM;    break;
    case util::MeasureUnit::MM_10TH:        eVcl = MAP_10TH_MM;     break;
    case util::MeasureUnit::MM:             eVcl = MAP_MM;          break;
    case util::MeasureUnit::CM:             eVcl = MAP_CM;          break;
    case util::MeasureUnit::INCH_1000TH:    eVcl = MAP_1000TH_INCH; break;
    case util::MeasureUnit::INCH_100TH:     eVcl = MAP_100TH_INCH;  break;
    case util::MeasureUnit::INCH_10TH:      eVcl = MAP_10TH_INCH;   break;
    case util::MeasureUnit::INCH:           eVcl = MAP_INCH;        break;
    case util::MeasureUnit::POINT:          eVcl = MAP_POINT;       break;
    case util::MeasureUnit::TWIP:           eVcl = MAP_TWIP;        break;
    case util::MeasureUnit::PERCENT:        eVcl = MAP_RELATIVE;    break;
    default:
        return false;
    }

    return true;
}

/** maps the vcl MapUnit enum to a API constant MeasureUnit.
    Returns false if conversion is not supported.
*/
sal_Bool SvxMapUnitToMeasureUnit( const short eVcl, short& eApi ) throw()
{
    switch( eVcl )
    {
    case MAP_100TH_MM:      eApi = util::MeasureUnit::MM_100TH;     break;
    case MAP_10TH_MM:       eApi = util::MeasureUnit::MM_10TH;      break;
    case MAP_MM:            eApi = util::MeasureUnit::MM;           break;
    case MAP_CM:            eApi = util::MeasureUnit::CM;           break;
    case MAP_1000TH_INCH:   eApi = util::MeasureUnit::INCH_1000TH;  break;
    case MAP_100TH_INCH:    eApi = util::MeasureUnit::INCH_100TH;   break;
    case MAP_10TH_INCH:     eApi = util::MeasureUnit::INCH_10TH;    break;
    case MAP_INCH:          eApi = util::MeasureUnit::INCH;         break;
    case MAP_POINT:         eApi = util::MeasureUnit::POINT;        break;
    case MAP_TWIP:          eApi = util::MeasureUnit::TWIP;         break;
    case MAP_RELATIVE:      eApi = util::MeasureUnit::PERCENT;      break;
    default:
        return false;
    }

    return true;
}

/** maps the API constant MeasureUnit to a vcl MapUnit enum.
    Returns false if conversion is not supported.
*/

sal_Bool SvxMeasureUnitToFieldUnit( const short eApi, short& eVcl ) throw()
{
    switch( eApi )
    {
    case util::MeasureUnit::MM:         eVcl = FUNIT_MM;        break;
    case util::MeasureUnit::CM:         eVcl = FUNIT_CM;        break;
    case util::MeasureUnit::M:          eVcl = FUNIT_M;         break;
    case util::MeasureUnit::KM:         eVcl = FUNIT_KM;        break;
    case util::MeasureUnit::TWIP:       eVcl = FUNIT_TWIP;      break;
    case util::MeasureUnit::POINT:      eVcl = FUNIT_POINT;     break;
    case util::MeasureUnit::PICA:       eVcl = FUNIT_PICA;      break;
    case util::MeasureUnit::INCH:       eVcl = FUNIT_INCH;      break;
    case util::MeasureUnit::FOOT:       eVcl = FUNIT_FOOT;      break;
    case util::MeasureUnit::MILE:       eVcl = FUNIT_MILE;      break;
    case util::MeasureUnit::PERCENT:    eVcl = FUNIT_PERCENT;   break;
    case util::MeasureUnit::MM_100TH:   eVcl = FUNIT_100TH_MM;  break;
    default:
        return sal_False;
    }

    return sal_True;
}

/** maps the vcl MapUnit enum to a API constant MeasureUnit.
    Returns false if conversion is not supported.
*/
sal_Bool SvxFieldUnitToMeasureUnit( const short eVcl, short& eApi ) throw()
{
    switch( eVcl )
    {
    case FUNIT_MM:          eApi = util::MeasureUnit::MM;       break;
    case FUNIT_CM:          eApi = util::MeasureUnit::CM;       break;
    case FUNIT_M:           eApi = util::MeasureUnit::M;        break;
    case FUNIT_KM:          eApi = util::MeasureUnit::KM;       break;
    case FUNIT_TWIP:        eApi = util::MeasureUnit::TWIP;     break;
    case FUNIT_POINT:       eApi = util::MeasureUnit::POINT;    break;
    case FUNIT_PICA:        eApi = util::MeasureUnit::PICA;     break;
    case FUNIT_INCH:        eApi = util::MeasureUnit::INCH;     break;
    case FUNIT_FOOT:        eApi = util::MeasureUnit::FOOT;     break;
    case FUNIT_MILE:        eApi = util::MeasureUnit::MILE;     break;
    case FUNIT_PERCENT:     eApi = util::MeasureUnit::PERCENT;  break;
    case FUNIT_100TH_MM:    eApi = util::MeasureUnit::MM_100TH; break;
    default:
        return sal_False;
    }

    return sal_True;
}

bool SvxUnoGetResourceRanges( const short nWhich, int& nApiResIds, int& nIntResIds, int& nCount ) throw()
{
    switch( nWhich )
    {
    case XATTR_FILLBITMAP:
        nApiResIds = RID_SVXSTR_BMP_DEF_START;
        nIntResIds = RID_SVXSTR_BMP_START;
        nCount = RID_SVXSTR_BMP_DEF_END - RID_SVXSTR_BMP_DEF_START + 1;
        break;

    case XATTR_LINEDASH:
        nApiResIds = RID_SVXSTR_DASH_DEF_START;
        nIntResIds = RID_SVXSTR_DASH_START;
        nCount = RID_SVXSTR_DASH_DEF_END - RID_SVXSTR_DASH_DEF_START + 1;
        break;

    case XATTR_LINESTART:
    case XATTR_LINEEND:
        nApiResIds = RID_SVXSTR_LEND_DEF_START;
        nIntResIds = RID_SVXSTR_LEND_START;
        nCount = RID_SVXSTR_LEND_DEF_END - RID_SVXSTR_LEND_DEF_START + 1;
        break;

    case XATTR_FILLGRADIENT:
        nApiResIds = RID_SVXSTR_GRDT_DEF_START;
        nIntResIds = RID_SVXSTR_GRDT_START;
        nCount = RID_SVXSTR_GRDT_DEF_END - RID_SVXSTR_GRDT_DEF_START + 1;
        break;

    case XATTR_FILLHATCH:
        nApiResIds = RID_SVXSTR_HATCH_DEF_START;
        nIntResIds = RID_SVXSTR_HATCH_START;
        nCount = RID_SVXSTR_HATCH_DEF_END - RID_SVXSTR_HATCH_DEF_START + 1;
        break;

    case XATTR_FILLFLOATTRANSPARENCE:
        nApiResIds = RID_SVXSTR_TRASNGR_DEF_START;
        nIntResIds = RID_SVXSTR_TRASNGR_START;
        nCount = RID_SVXSTR_TRASNGR_DEF_END - RID_SVXSTR_TRASNGR_DEF_START + 1;
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

bool SvxUnoConvertResourceString( int nSourceResIds, int nDestResIds, int nCount, String& rString ) throw()
{
    // first, calculate the search string length without an optional number behind the name
    int nLength = rString.Len();
    while( nLength > 0 )
    {
        const sal_Unicode nChar = rString.GetChar( nLength - 1 );
        if( (nChar < '0') || (nChar > '9') )
            break;

        nLength--;
    }

    // if we cut off a number, also cut of some spaces
    if( nLength != rString.Len() )
    {
        while( nLength > 0 )
        {
            const sal_Unicode nChar = rString.GetChar( nLength - 1 );
            if( nChar != ' ' )
                break;

            nLength--;
        }
    }

    const String aShortString( rString.Copy( 0, nLength ) );

    int i;
    for( i = 0; i < nCount; i++ )
    {
        USHORT nResId = (USHORT)(nSourceResIds + i);
        const ResId aRes( SVX_RES(nResId));
        const String aCompare( aRes );
        if( aShortString == aCompare )
        {
            USHORT nNewResId = (USHORT)(nDestResIds + i);
            ResId aNewRes( SVX_RES( nNewResId ));
            rString.Replace( 0, aShortString.Len(), String( aNewRes ) );
            return TRUE;
        }
        else if( rString == aCompare )
        {
            USHORT nNewResId = (USHORT)(nDestResIds + i);
            ResId aNewRes( SVX_RES( nNewResId ));
            rString = String( aNewRes );
            return TRUE;
        }
    }

    return FALSE;
}

static USHORT __READONLY_DATA SvxUnoColorNameDefResId[] =
{
    RID_SVXSTR_BLUEGREY_DEF,
    RID_SVXSTR_BLACK_DEF,
    RID_SVXSTR_BLUE_DEF,
    RID_SVXSTR_GREEN_DEF,
    RID_SVXSTR_CYAN_DEF,
    RID_SVXSTR_RED_DEF,
    RID_SVXSTR_MAGENTA_DEF,
    RID_SVXSTR_BROWN_DEF,
    RID_SVXSTR_GREY_DEF,
    RID_SVXSTR_LIGHTGREY_DEF,
    RID_SVXSTR_LIGHTBLUE_DEF,
    RID_SVXSTR_LIGHTGREEN_DEF,
    RID_SVXSTR_LIGHTCYAN_DEF,
    RID_SVXSTR_LIGHTRED_DEF,
    RID_SVXSTR_LIGHTMAGENTA_DEF,
    RID_SVXSTR_YELLOW_DEF,
    RID_SVXSTR_WHITE_DEF,
    RID_SVXSTR_ORANGE_DEF,
    RID_SVXSTR_VIOLET_DEF,
    RID_SVXSTR_BORDEAUX_DEF,
    RID_SVXSTR_PALE_YELLOW_DEF,
    RID_SVXSTR_PALE_GREEN_DEF,
    RID_SVXSTR_DKVIOLET_DEF,
    RID_SVXSTR_SALMON_DEF,
    RID_SVXSTR_SEABLUE_DEF,
    RID_SVXSTR_COLOR_SUN_DEF
};

static USHORT __READONLY_DATA SvxUnoColorNameResId[] =
{
    RID_SVXSTR_BLUEGREY,
    RID_SVXSTR_BLACK,
    RID_SVXSTR_BLUE,
    RID_SVXSTR_GREEN,
    RID_SVXSTR_CYAN,
    RID_SVXSTR_RED,
    RID_SVXSTR_MAGENTA,
    RID_SVXSTR_BROWN,
    RID_SVXSTR_GREY,
    RID_SVXSTR_LIGHTGREY,
    RID_SVXSTR_LIGHTBLUE,
    RID_SVXSTR_LIGHTGREEN,
    RID_SVXSTR_LIGHTCYAN,
    RID_SVXSTR_LIGHTRED,
    RID_SVXSTR_LIGHTMAGENTA,
    RID_SVXSTR_YELLOW,
    RID_SVXSTR_WHITE,
    RID_SVXSTR_ORANGE,
    RID_SVXSTR_VIOLET,
    RID_SVXSTR_BORDEAUX,
    RID_SVXSTR_PALE_YELLOW,
    RID_SVXSTR_PALE_GREEN,
    RID_SVXSTR_DKVIOLET,
    RID_SVXSTR_SALMON,
    RID_SVXSTR_SEABLUE,
    RID_SVXSTR_COLOR_SUN
};

bool SvxUnoConvertResourceString( USHORT* pSourceResIds, USHORT* pDestResIds, int nCount, String& rString ) throw()
{
    int i = 0;

    for( i = 0; i < nCount; i++ )
    {
        String aStrDefName = SVX_RESSTR( pSourceResIds[i] );
        if( rString.Search( aStrDefName ) == 0 )
        {
            String aReplace = SVX_RESSTR( pDestResIds[i] );
            rString.Replace( 0, aStrDefName.Len(), aReplace );
            return true;
        }
    }

    return false;
}

/** if the given name is a predefined name for the current language it is replaced by
    the corresponding api name.
*/
void SvxUnogetApiNameForItem( const sal_Int16 nWhich, const String& rInternalName, rtl::OUString& rApiName ) throw()
{
#ifndef SVX_LIGHT
    String aNew = rInternalName;

    if( nWhich == XATTR_LINECOLOR )
    {
        if( SvxUnoConvertResourceString( (USHORT*)SvxUnoColorNameResId, (USHORT*)SvxUnoColorNameDefResId, sizeof( SvxUnoColorNameResId ) / sizeof( USHORT ), aNew ) )
        {
            rApiName = aNew;
            return;
        }
    }
    else
    {
        int nApiResIds;
        int nIntResIds;
        int nCount;

        if( SvxUnoGetResourceRanges( nWhich, nApiResIds, nIntResIds, nCount ) )
        {
            if(SvxUnoConvertResourceString( nIntResIds, nApiResIds, nCount, aNew ) )
            {
                rApiName = aNew;
                return;
            }
        }
    }
#endif

    // just use previous name, if nothing else was found.
    rApiName = rInternalName;
}

/** if the given name is a predefined api name it is replaced by the predefined name
    for the current language.
*/
void SvxUnogetInternalNameForItem( const sal_Int16 nWhich, const rtl::OUString& rApiName, String& rInternalName ) throw()
{
#ifndef SVX_LIGHT
    String aNew = rApiName;

    if( nWhich == XATTR_LINECOLOR )
    {
        if( SvxUnoConvertResourceString( (USHORT*)SvxUnoColorNameDefResId, (USHORT*)SvxUnoColorNameResId, sizeof( SvxUnoColorNameResId ) / sizeof( USHORT ), aNew ) )
        {
            rInternalName = aNew;
            return;
        }
    }
    else
    {
        int nApiResIds;
        int nIntResIds;
        int nCount;

        if( SvxUnoGetResourceRanges( nWhich, nApiResIds, nIntResIds, nCount ) )
        {
            if(SvxUnoConvertResourceString( nApiResIds, nIntResIds, nCount, aNew ) )
            {
                rInternalName = aNew;
                return;
            }
        }
    }
#endif // !SVX_LIGHT

    // just use previous name, if nothing else was found.
    rInternalName = rApiName;
}

///////////////////////////////////////////////////////////////////////

comphelper::PropertySetInfo* SvxPropertySetInfoPool::getOrCreate( sal_Int32 nServiceId ) throw()
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    if( nServiceId > SVXUNO_SERVICEID_LASTID )
    {
        DBG_ERROR( "unknown service id!" );
        return NULL;
    }

    if( mpInfos[ nServiceId ] == NULL )
    {
        mpInfos[nServiceId] = new comphelper::PropertySetInfo();
        mpInfos[nServiceId]->acquire();

        switch( nServiceId )
        {
        case SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS:
            mpInfos[SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS]->add( ImplGetSvxDrawingDefaultsPropertyMap() );
            break;
        case SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS_WRITER:
            mpInfos[SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS_WRITER]->add( ImplGetSvxDrawingDefaultsPropertyMap() );
            mpInfos[SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS_WRITER]->remove( OUString( RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_EDIT_PARA_IS_HANGING_PUNCTUATION ) ) );
            // OD 13.10.2003 #i18732# - add property map for writer item 'IsFollowingTextFlow'
            mpInfos[SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS_WRITER]->add( ImplGetAdditionalWriterDrawingDefaultsPropertyMap() );
            break;

        default:
            DBG_ERROR( "unknown service id!" );
        }
    }

    return mpInfos[ nServiceId ];
}

comphelper::PropertySetInfo* SvxPropertySetInfoPool::mpInfos[SVXUNO_SERVICEID_LASTID+1] = { NULL };

