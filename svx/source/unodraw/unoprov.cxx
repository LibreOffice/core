/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/macros.h>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/media/ZoomLevel.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <boost/unordered_map.hpp>
#include <tools/fldunit.hxx>
#include <tools/shl.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <svx/dialmgr.hxx>
#include "svx/unoapi.hxx"
#include <editeng/unotext.hxx>
#include <svx/unoshprp.hxx>
#include <editeng/editeng.hxx>
#include "svx/globl3d.hxx"
#include <svx/dialogs.hrc>
#include <svx/svdpool.hxx>
#include <svx/svdobj.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans::PropertyAttribute;
using ::com::sun::star::drawing::TextVerticalAdjust;

SfxItemPropertyMapEntry* ImplGetSvxShapePropertyMap()
{
    static SfxItemPropertyMapEntry aShapePropertyMap_Impl[] =
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
        {0,0,0,0,0,0}

    };

    return aShapePropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvxTextShapePropertyMap()
{
    static SfxItemPropertyMapEntry aTextShapePropertyMap_Impl[] =
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
        {0,0,0,0,0,0}

    };

    return aTextShapePropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvxConnectorPropertyMap()
{
    static SfxItemPropertyMapEntry aConnectorPropertyMap_Impl[] =
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
        {0,0,0,0,0,0}

    };

    return aConnectorPropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvxDimensioningPropertyMap()
{
    static SfxItemPropertyMapEntry aDimensioningPropertyMap_Impl[] =
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
        {0,0,0,0,0,0}

    };

    return aDimensioningPropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvxCirclePropertyMap()
{
    static SfxItemPropertyMapEntry aCirclePropertyMap_Impl[] =
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
        {0,0,0,0,0,0}

    };

    return aCirclePropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvxPolyPolygonPropertyMap()
{
    static SfxItemPropertyMapEntry aPolyPolygonPropertyMap_Impl[] =
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
        {0,0,0,0,0,0}

    };

    return aPolyPolygonPropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvxPolyPolygonBezierPropertyMap()
{
    static SfxItemPropertyMapEntry aPolyPolygonBezierPropertyMap_Impl[] =
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
        {0,0,0,0,0,0}

    };

    return aPolyPolygonBezierPropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvxGraphicObjectPropertyMap()
{
    static SfxItemPropertyMapEntry aGraphicObjectPropertyMap_Impl[] =
    {
        SPECIAL_GRAPHOBJ_PROPERTIES

        // #i25616#
        FILL_PROPERTIES

        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES

        // #i118485# Full properties now, shear included
        MISC_OBJ_PROPERTIES

        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        // #FontWork#
        FONTWORK_PROPERTIES
        { MAP_CHAR_LEN("IsMirrored"),               OWN_ATTR_MIRRORED           , &::getCppuBooleanType(), 0, 0},
        { MAP_CHAR_LEN("UserDefinedAttributes"),        SDRATTR_XMLATTRIBUTES,      &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("GraphicStream"), OWN_ATTR_GRAPHIC_STREAM, &::com::sun::star::io::XInputStream::static_type(), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        {0,0,0,0,0,0}

    };

    return aGraphicObjectPropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvx3DSceneObjectPropertyMap()
{
    static SfxItemPropertyMapEntry a3DSceneObjectPropertyMap_Impl[] =
    {
        SPECIAL_3DSCENEOBJECT_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        {0,0,0,0,0,0}

    };

    return a3DSceneObjectPropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvx3DCubeObjectPropertyMap()
{
    static SfxItemPropertyMapEntry a3DCubeObjectPropertyMap_Impl[] =
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
        {0,0,0,0,0,0}

    };

    return a3DCubeObjectPropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvx3DSphereObjectPropertyMap()
{
    static SfxItemPropertyMapEntry a3DSphereObjectPropertyMap_Impl[] =
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
        {0,0,0,0,0,0}

    };
    return a3DSphereObjectPropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvx3DLatheObjectPropertyMap()
{
    static SfxItemPropertyMapEntry a3DLatheObjectPropertyMap_Impl[] =
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
        {0,0,0,0,0,0}

    };

    return a3DLatheObjectPropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvx3DExtrudeObjectPropertyMap()
{
    static SfxItemPropertyMapEntry a3DExtrudeObjectPropertyMap_Impl[] =
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
        {0,0,0,0,0,0}

    };

    return a3DExtrudeObjectPropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvx3DPolygonObjectPropertyMap()
{
    static SfxItemPropertyMapEntry a3DPolygonObjectPropertyMap_Impl[] =
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
        {0,0,0,0,0,0}

    };

    return a3DPolygonObjectPropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvxAllPropertyMap()
{
    static SfxItemPropertyMapEntry aAllPropertyMap_Impl[] =
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
        {0,0,0,0,0,0}

    };

    return aAllPropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvxGroupPropertyMap()
{
    static SfxItemPropertyMapEntry aGroupPropertyMap_Impl[] =
    {
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        {0,0,0,0,0,0}

    };

    return aGroupPropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvxOle2PropertyMap()
{
    static SfxItemPropertyMapEntry aOle2PropertyMap_Impl[] =
    {
        // #i118485# Adding properties for line, fill, text, shadow, fontwork, rotate, shear
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        FONTWORK_PROPERTIES

        { MAP_CHAR_LEN("ThumbnailGraphicURL"),      OWN_ATTR_THUMBNAIL          , &::getCppuType(( const OUString*)0), 0, 0 },
        { MAP_CHAR_LEN("Model"),                    OWN_ATTR_OLEMODEL           , &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("EmbeddedObject"),           OWN_ATTR_OLE_EMBEDDED_OBJECT    , &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject >*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("EmbeddedObjectNoNewClient"),OWN_ATTR_OLE_EMBEDDED_OBJECT_NONEWCLIENT, &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject >*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("OriginalSize"),             OWN_ATTR_OLESIZE            , &::getCppuType(( const ::com::sun::star::awt::Size*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("CLSID"),                    OWN_ATTR_CLSID              , &::getCppuType(( const OUString*)0), 0, 0 },
        { MAP_CHAR_LEN("IsInternal"),               OWN_ATTR_INTERNAL_OLE       , &::getBooleanCppuType() , ::com::sun::star::beans::PropertyAttribute::READONLY,     0},
        { MAP_CHAR_LEN("VisibleArea"),              OWN_ATTR_OLE_VISAREA        , &::getCppuType((const ::com::sun::star::awt::Rectangle*)0), 0, 0},
        { MAP_CHAR_LEN("Aspect"),                   OWN_ATTR_OLE_ASPECT         , &::getCppuType((const sal_Int64*)0), 0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OLE2_PERSISTNAME),  OWN_ATTR_PERSISTNAME        , &::getCppuType(( const OUString*)0), 0, 0 },
        { MAP_CHAR_LEN("LinkURL"),                  OWN_ATTR_OLE_LINKURL        , &::getCppuType(( const OUString*)0), 0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_GRAPHOBJ_GRAPHIC),   OWN_ATTR_VALUE_GRAPHIC     , &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic>*)0),     0,     0},
        {0,0,0,0,0,0}

    };

    return aOle2PropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvxPluginPropertyMap()
{
    static SfxItemPropertyMapEntry aPluginPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("PluginMimeType"),           OWN_ATTR_PLUGIN_MIMETYPE    , &::getCppuType((const OUString*)0),    0,  0},
        { MAP_CHAR_LEN("PluginURL"),                OWN_ATTR_PLUGIN_URL         , &::getCppuType((const OUString*)0),    0,  0},
        { MAP_CHAR_LEN("PluginCommands"),           OWN_ATTR_PLUGIN_COMMANDS    , SEQTYPE(::getCppuType((::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >*)0)), 0, 0},
        { MAP_CHAR_LEN("Transformation"),           OWN_ATTR_TRANSFORMATION     , &::getCppuType((const struct com::sun::star::drawing::HomogenMatrix3*)0), 0, 0 }, \
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_ZORDER),   OWN_ATTR_ZORDER             , &::getCppuType((const sal_Int32*)0),      0,      0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERID),  SDRATTR_LAYERID             , &::getCppuType((const sal_Int16*)0),      0,  0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERNAME),SDRATTR_LAYERNAME           , &::getCppuType((const OUString*)0),    0,  0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP), OWN_ATTR_LDBITMAP           , &::getCppuType(( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME),   OWN_ATTR_LDNAME             , &::getCppuType(( const OUString*)0),   ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_OLE2_METAFILE),     OWN_ATTR_METAFILE           , SEQTYPE(::getCppuType((::com::sun::star::uno::Sequence<sal_Int8>*)0)), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("ThumbnailGraphicURL"),      OWN_ATTR_THUMBNAIL          , &::getCppuType(( const OUString*)0), 0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_MOVEPROTECT),      SDRATTR_OBJMOVEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_SIZEPROTECT),      SDRATTR_OBJSIZEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OLE2_PERSISTNAME),  OWN_ATTR_PERSISTNAME        , &::getCppuType(( const OUString*)0), 0, 0 },
        { MAP_CHAR_LEN("LinkURL"),                  OWN_ATTR_OLE_LINKURL        , &::getCppuType(( const OUString*)0), 0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_BOUNDRECT), OWN_ATTR_BOUNDRECT,            &::getCppuType((const ::com::sun::star::awt::Rectangle*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("VisibleArea"),              OWN_ATTR_OLE_VISAREA        , &::getCppuType((const ::com::sun::star::awt::Rectangle*)0), 0, 0},
        // #i68101#
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_TITLE),        OWN_ATTR_MISC_OBJ_TITLE         , &::getCppuType((const OUString*)0),    0,  0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_DESCRIPTION),  OWN_ATTR_MISC_OBJ_DESCRIPTION   , &::getCppuType((const OUString*)0),    0,  0},
        {0,0,0,0,0,0}

    };

    return aPluginPropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvxFramePropertyMap()
{
    //TODO/LATER: new properties for ScrollingMode and DefaultBorder
    static SfxItemPropertyMapEntry aFramePropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("FrameURL"),                 OWN_ATTR_FRAME_URL          , &::getCppuType((const OUString*)0),    0,  0},
        { MAP_CHAR_LEN("FrameName"),                OWN_ATTR_FRAME_NAME         , &::getCppuType((const OUString*)0),    0,  0},
        { MAP_CHAR_LEN("FrameIsAutoScroll"),        OWN_ATTR_FRAME_ISAUTOSCROLL , &::getBooleanCppuType() , ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { MAP_CHAR_LEN("FrameIsBorder"),            OWN_ATTR_FRAME_ISBORDER     , &::getBooleanCppuType() , 0, 0},
        { MAP_CHAR_LEN("FrameMarginWidth"),         OWN_ATTR_FRAME_MARGIN_WIDTH , &::getCppuType((const sal_Int32*)0),      0,      0},
        { MAP_CHAR_LEN("FrameMarginHeight"),        OWN_ATTR_FRAME_MARGIN_HEIGHT, &::getCppuType((const sal_Int32*)0),      0,      0},
        { MAP_CHAR_LEN("Transformation"),           OWN_ATTR_TRANSFORMATION     , &::getCppuType((const struct com::sun::star::drawing::HomogenMatrix3*)0), 0, 0 }, \
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_ZORDER),   OWN_ATTR_ZORDER             , &::getCppuType((const sal_Int32*)0),      0,      0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERID),  SDRATTR_LAYERID             , &::getCppuType((const sal_Int16*)0),      0,  0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERNAME),SDRATTR_LAYERNAME           , &::getCppuType((const OUString*)0),    0,  0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP), OWN_ATTR_LDBITMAP           , &::getCppuType(( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME),   OWN_ATTR_LDNAME             , &::getCppuType(( const OUString*)0),   ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_OLE2_METAFILE),     OWN_ATTR_METAFILE           , SEQTYPE(::getCppuType((::com::sun::star::uno::Sequence<sal_Int8>*)0)), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("ThumbnailGraphicURL"),      OWN_ATTR_THUMBNAIL          , &::getCppuType(( const OUString*)0), 0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_MOVEPROTECT),      SDRATTR_OBJMOVEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_SIZEPROTECT),      SDRATTR_OBJSIZEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OLE2_PERSISTNAME),  OWN_ATTR_PERSISTNAME        , &::getCppuType(( const OUString*)0), 0, 0 },
        { MAP_CHAR_LEN("LinkURL"),                  OWN_ATTR_OLE_LINKURL        , &::getCppuType(( const OUString*)0), 0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_BOUNDRECT), OWN_ATTR_BOUNDRECT,            &::getCppuType((const ::com::sun::star::awt::Rectangle*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("VisibleArea"),              OWN_ATTR_OLE_VISAREA        , &::getCppuType((const ::com::sun::star::awt::Rectangle*)0), 0, 0},
        // #i68101#
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_TITLE),        OWN_ATTR_MISC_OBJ_TITLE         , &::getCppuType((const OUString*)0),    0,  0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_DESCRIPTION),  OWN_ATTR_MISC_OBJ_DESCRIPTION   , &::getCppuType((const OUString*)0),    0,  0},
        {0,0,0,0,0,0}

    };

    return aFramePropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvxAppletPropertyMap()
{
    static SfxItemPropertyMapEntry aAppletPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("AppletCodeBase"),           OWN_ATTR_APPLET_CODEBASE    , &::getCppuType(( const OUString*)0), 0, 0},
        { MAP_CHAR_LEN("AppletName"),               OWN_ATTR_APPLET_NAME        , &::getCppuType(( const OUString*)0), 0, 0},
        { MAP_CHAR_LEN("AppletCode"),               OWN_ATTR_APPLET_CODE        , &::getCppuType(( const OUString*)0), 0, 0},
        { MAP_CHAR_LEN("AppletCommands"),           OWN_ATTR_APPLET_COMMANDS    , SEQTYPE(::getCppuType((::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >*)0)), 0, 0},
        { MAP_CHAR_LEN("AppletDocBase"),            OWN_ATTR_APPLET_DOCBASE     , &::getCppuType(( const OUString*)0), 0, 0},
        { MAP_CHAR_LEN("AppletIsScript"),           OWN_ATTR_APPLET_ISSCRIPT    , &::getBooleanCppuType(), 0, 0 },
        { MAP_CHAR_LEN("Transformation"),           OWN_ATTR_TRANSFORMATION     , &::getCppuType((const struct com::sun::star::drawing::HomogenMatrix3*)0), 0, 0 }, \
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_ZORDER),   OWN_ATTR_ZORDER             , &::getCppuType((const sal_Int32*)0),      0,      0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERID),  SDRATTR_LAYERID             , &::getCppuType((const sal_Int16*)0),      0,  0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERNAME),SDRATTR_LAYERNAME           , &::getCppuType((const OUString*)0),    0,  0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP), OWN_ATTR_LDBITMAP           , &::getCppuType(( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME),   OWN_ATTR_LDNAME             , &::getCppuType(( const OUString*)0),   ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_OLE2_METAFILE),     OWN_ATTR_METAFILE           , SEQTYPE(::getCppuType((::com::sun::star::uno::Sequence<sal_Int8>*)0)), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("ThumbnailGraphicURL"),      OWN_ATTR_THUMBNAIL          , &::getCppuType(( const OUString*)0), 0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_MOVEPROTECT),      SDRATTR_OBJMOVEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_SIZEPROTECT),      SDRATTR_OBJSIZEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OLE2_PERSISTNAME),  OWN_ATTR_PERSISTNAME        , &::getCppuType(( const OUString*)0), 0, 0 },
        { MAP_CHAR_LEN("LinkURL"),                  OWN_ATTR_OLE_LINKURL        , &::getCppuType(( const OUString*)0), 0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_BOUNDRECT), OWN_ATTR_BOUNDRECT,            &::getCppuType((const ::com::sun::star::awt::Rectangle*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("VisibleArea"),              OWN_ATTR_OLE_VISAREA        , &::getCppuType((const ::com::sun::star::awt::Rectangle*)0), 0, 0},
        // #i68101#
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_TITLE),        OWN_ATTR_MISC_OBJ_TITLE         , &::getCppuType((const OUString*)0),    0,  0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_DESCRIPTION),  OWN_ATTR_MISC_OBJ_DESCRIPTION   , &::getCppuType((const OUString*)0),    0,  0},
        {0,0,0,0,0,0}

    };

    return aAppletPropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvxControlShapePropertyMap()
{
    static SfxItemPropertyMapEntry aControlPropertyMap_Impl[] =
    {
        // the following properties are mapped to the XControl Model of this shape
        { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTNAME),        0,  &::getCppuType((const OUString*)0),  0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTSTYLENAME),   0,  &::getCppuType((const OUString*)0),  0, 0 },
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
        { MAP_CHAR_LEN("TextVerticalAdjust"),               0,  &::getCppuType((const TextVerticalAdjust*)0), MAYBEVOID, 0 },
        { MAP_CHAR_LEN("ControlBackground"),                0,  &::getCppuType((const sal_Int32*)0),        0, 0 },
        { MAP_CHAR_LEN("ControlBorder"),                    0,  &::getCppuType((const sal_Int16*)0),        0, 0 },
        { MAP_CHAR_LEN("ControlBorderColor"),               0,  &::getCppuType((const sal_Int32*)0),        0, 0 },
        { MAP_CHAR_LEN("ControlSymbolColor"),               0,  &::getCppuType((const sal_Int16*)0),        0, 0 },
        { MAP_CHAR_LEN("ImageScaleMode"),                   0,  &::getCppuType((const sal_Int16*)0),        0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_MOVEPROTECT),      SDRATTR_OBJMOVEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_SIZEPROTECT),      SDRATTR_OBJSIZEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN("ControlTextEmphasis"),              0,  &::getCppuType((const sal_Int16*)0),        0, 0 },
        { MAP_CHAR_LEN("ControlWritingMode"),               0,  &::cppu::UnoType< sal_Int16 >::get(), 0, 0},
        // the following properties are handled by SvxShape
        { MAP_CHAR_LEN("Transformation"),           OWN_ATTR_TRANSFORMATION     , &::getCppuType((const struct com::sun::star::drawing::HomogenMatrix3*)0), 0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_ZORDER),   OWN_ATTR_ZORDER             , &::getCppuType((const sal_Int32*)0),      0,      0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERID),  SDRATTR_LAYERID             , &::getCppuType((const sal_Int16*)0),      0,  0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERNAME),SDRATTR_LAYERNAME           , &::getCppuType((const OUString*)0),    0,  0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP), OWN_ATTR_LDBITMAP           , &::getCppuType(( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME),   OWN_ATTR_LDNAME             , &::getCppuType(( const OUString*)0),   ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("UserDefinedAttributes"),        SDRATTR_XMLATTRIBUTES,      &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_BOUNDRECT), OWN_ATTR_BOUNDRECT,            &::getCppuType((const ::com::sun::star::awt::Rectangle*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        // #i68101#
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_TITLE),        OWN_ATTR_MISC_OBJ_TITLE         , &::getCppuType((const OUString*)0),    0,  0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_DESCRIPTION),  OWN_ATTR_MISC_OBJ_DESCRIPTION   , &::getCppuType((const OUString*)0),    0,  0},
        // #i112587#
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_PRINTABLE),    SDRATTR_OBJPRINTABLE            , &::getBooleanCppuType(),                      0,  0}, \
        { MAP_CHAR_LEN("Visible"),                      SDRATTR_OBJVISIBLE              , &::getBooleanCppuType(),                      0,  0}, \
        {0,0,0,0,0,0}
    };

    return aControlPropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvxPageShapePropertyMap()
{
    static SfxItemPropertyMapEntry aPageShapePropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("PageNumber"),               OWN_ATTR_PAGE_NUMBER        , &::getCppuType((const sal_Int32*)0),      0, 0},
        { MAP_CHAR_LEN("Transformation"),           OWN_ATTR_TRANSFORMATION     , &::getCppuType((const struct com::sun::star::drawing::HomogenMatrix3*)0), 0, 0 }, \
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_ZORDER),   OWN_ATTR_ZORDER             , &::getCppuType((const sal_Int32*)0),      0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERID),  SDRATTR_LAYERID             , &::getCppuType((const sal_Int16*)0),      0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERNAME),SDRATTR_LAYERNAME           , &::getCppuType((const OUString*)0), 0, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP), OWN_ATTR_LDBITMAP           , &::getCppuType(( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME),   OWN_ATTR_LDNAME             , &::getCppuType(( const OUString*)0),   ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_MOVEPROTECT),      SDRATTR_OBJMOVEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_SIZEPROTECT),      SDRATTR_OBJSIZEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_BOUNDRECT), OWN_ATTR_BOUNDRECT,            &::getCppuType((const ::com::sun::star::awt::Rectangle*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        // #i68101#
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_TITLE),        OWN_ATTR_MISC_OBJ_TITLE         , &::getCppuType((const OUString*)0),    0,  0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_DESCRIPTION),  OWN_ATTR_MISC_OBJ_DESCRIPTION   , &::getCppuType((const OUString*)0),    0,  0},
        {0,0,0,0,0,0}

    };

    return aPageShapePropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvxCaptionPropertyMap()
{
    static SfxItemPropertyMapEntry aCaptionPropertyMap_Impl[] =
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
        {0,0,0,0,0,0}

    };

    return aCaptionPropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvxCustomShapePropertyMap()
{
    static SfxItemPropertyMapEntry aCustomShapePropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("CustomShapeEngine"),            SDRATTR_CUSTOMSHAPE_ENGINE,         &::getCppuType((const OUString*)0),  0, 0 },
        { MAP_CHAR_LEN("CustomShapeData"),          SDRATTR_CUSTOMSHAPE_DATA,               &::getCppuType((const OUString*)0),  0, 0 },
        { MAP_CHAR_LEN("CustomShapeGeometry"),      SDRATTR_CUSTOMSHAPE_GEOMETRY,           SEQTYPE(::getCppuType((::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >*)0)),  0, 0 },
        { MAP_CHAR_LEN("CustomShapeGraphicURL"),        SDRATTR_CUSTOMSHAPE_REPLACEMENT_URL,    &::getCppuType((const OUString*)0),  0,  0},
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
        {0,0,0,0,0,0}

    };
    return aCustomShapePropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvxMediaShapePropertyMap()
{
    static SfxItemPropertyMapEntry aMediaShapePropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_ZORDER), OWN_ATTR_ZORDER, &::getCppuType((const sal_Int32*)0), 0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERID), SDRATTR_LAYERID, &::getCppuType((const sal_Int16*)0), 0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERNAME), SDRATTR_LAYERNAME, &::getCppuType((const OUString*)0), 0, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP), OWN_ATTR_LDBITMAP, &::getCppuType(( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME), OWN_ATTR_LDNAME, &::getCppuType(( const OUString*)0),  ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("Transformation"), OWN_ATTR_TRANSFORMATION, &::getCppuType((const struct com::sun::star::drawing::HomogenMatrix3*)0), 0, 0 },
        { MAP_CHAR_LEN("MediaURL"), OWN_ATTR_MEDIA_URL, &::getCppuType((const OUString*)0), 0, 0},
        { MAP_CHAR_LEN("PreferredSize"), OWN_ATTR_MEDIA_PREFERREDSIZE, &::getCppuType((const ::com::sun::star::awt::Size*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("Loop"), OWN_ATTR_MEDIA_LOOP, &::getCppuType((const sal_Bool*)0), 0, 0},
        { MAP_CHAR_LEN("Mute"), OWN_ATTR_MEDIA_MUTE, &::getCppuType((const sal_Bool*)0), 0, 0},
        { MAP_CHAR_LEN("VolumeDB"), OWN_ATTR_MEDIA_VOLUMEDB, &::getCppuType((const sal_Int16*)0), 0, 0},
        { MAP_CHAR_LEN("Zoom"), OWN_ATTR_MEDIA_ZOOM, &::getCppuType((const ::com::sun::star::media::ZoomLevel*)0), 0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_MOVEPROTECT), SDRATTR_OBJMOVEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_SIZEPROTECT), SDRATTR_OBJSIZEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_BOUNDRECT), OWN_ATTR_BOUNDRECT, &::getCppuType((const ::com::sun::star::awt::Rectangle*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        // #i68101#
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_TITLE),        OWN_ATTR_MISC_OBJ_TITLE         , &::getCppuType((const OUString*)0),    0,  0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_DESCRIPTION),  OWN_ATTR_MISC_OBJ_DESCRIPTION   , &::getCppuType((const OUString*)0),    0,  0},
        {MAP_CHAR_LEN("PrivateStream"), OWN_ATTR_MEDIA_STREAM, &::com::sun::star::io::XInputStream::static_type(), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN("PrivateTempFileURL"), OWN_ATTR_MEDIA_TEMPFILEURL, &::getCppuType((const OUString*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        {0,0,0,0,0,0}

    };

    return aMediaShapePropertyMap_Impl;
}

SfxItemPropertyMapEntry* ImplGetSvxTableShapePropertyMap()
{
    static SfxItemPropertyMapEntry aTableShapePropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_ZORDER),       OWN_ATTR_ZORDER, &::getCppuType((const sal_Int32*)0), 0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERID),      SDRATTR_LAYERID, &::getCppuType((const sal_Int16*)0), 0,    0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_LAYERNAME),    SDRATTR_LAYERNAME, &::getCppuType((const OUString*)0), 0, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYBITMAP),     OWN_ATTR_LDBITMAP, &::com::sun::star::awt::XBitmap::static_type(), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_LINKDISPLAYNAME),       OWN_ATTR_LDNAME, &::getCppuType(( const OUString*)0),    ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("Transformation"),               OWN_ATTR_TRANSFORMATION, &::getCppuType((const struct com::sun::star::drawing::HomogenMatrix3*)0), 0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_MOVEPROTECT),  SDRATTR_OBJMOVEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_SIZEPROTECT),  SDRATTR_OBJSIZEPROTECT, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_BOUNDRECT),    OWN_ATTR_BOUNDRECT, &::getCppuType((const ::com::sun::star::awt::Rectangle*)0), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_TITLE),        OWN_ATTR_MISC_OBJ_TITLE         , &::getCppuType((const OUString*)0),    0,  0},
        { MAP_CHAR_LEN(UNO_NAME_MISC_OBJ_DESCRIPTION),  OWN_ATTR_MISC_OBJ_DESCRIPTION   , &::getCppuType((const OUString*)0),    0,  0},
        { MAP_CHAR_LEN("Model"),                        OWN_ATTR_OLEMODEL               , &::com::sun::star::table::XTable::static_type(), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN("TableTemplate"),                OWN_ATTR_TABLETEMPLATE          , &::com::sun::star::container::XIndexAccess::static_type(), 0, 0},
        { MAP_CHAR_LEN("UseFirstRowStyle"),             OWN_ATTR_TABLETEMPLATE_FIRSTROW, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN("UseLastRowStyle"),              OWN_ATTR_TABLETEMPLATE_LASTROW, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN("UseFirstColumnStyle"),          OWN_ATTR_TABLETEMPLATE_FIRSTCOLUMN, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN("UseLastColumnStyle"),           OWN_ATTR_TABLETEMPLATE_LASTCOLUMN, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN("UseBandingRowStyle"),           OWN_ATTR_TABLETEMPLATE_BANDINGROWS, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN("UseBandingColumnStyle"),        OWN_ATTR_TABLETEMPLATE_BANDINGCOULUMNS, &::getBooleanCppuType(),0, 0},
        { MAP_CHAR_LEN("ReplacementGraphic"),           OWN_ATTR_BITMAP, &::com::sun::star::graphic::XGraphic::static_type(), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},

        {0,0,0,0,0,0}
    };

    return aTableShapePropertyMap_Impl;
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
        {0,0,0,0,0,0}

    };

    return aSvxDrawingDefaultsPropertyMap_Impl;
}

comphelper::PropertyMapEntry* ImplGetAdditionalWriterDrawingDefaultsPropertyMap()
{
    static comphelper::PropertyMapEntry aSvxAdditionalDefaultsPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("IsFollowingTextFlow"), SID_SW_FOLLOW_TEXT_FLOW, &::getBooleanCppuType(), 0, 0},
        {0,0,0,0,0,0}

    };

    return aSvxAdditionalDefaultsPropertyMap_Impl;
}

/***********************************************************************
* class UHashMap                                                       *
***********************************************************************/

typedef ::boost::unordered_map< OUString, sal_uInt32, OUStringHash > UHashMapImpl;

namespace {
  static const UHashMapImpl &GetUHashImpl()
  {
      static UHashMapImpl aImpl(63);
      static bool bInited = false;
      if (!bInited) {
          const struct { const char *name; sal_Int32 length; sal_uInt32 id; } aInit[] = {
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.RectangleShape"),      OBJ_RECT },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.EllipseShape"),            OBJ_CIRC },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.ControlShape"),            OBJ_UNO  },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.ConnectorShape"),      OBJ_EDGE },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.MeasureShape"),            OBJ_MEASURE },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.LineShape"),           OBJ_LINE },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.PolyPolygonShape"),        OBJ_POLY },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.PolyLineShape"),       OBJ_PLIN },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.OpenBezierShape"),     OBJ_PATHLINE },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.ClosedBezierShape"),   OBJ_PATHFILL },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.OpenFreeHandShape"),   OBJ_FREELINE },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.ClosedFreeHandShape"), OBJ_FREEFILL },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.PolyPolygonPathShape"),    OBJ_PATHPOLY },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.PolyLinePathShape"),   OBJ_PATHPLIN },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.GraphicObjectShape"),  OBJ_GRAF },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.GroupShape"),          OBJ_GRUP },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.TextShape"),           OBJ_TEXT },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.OLE2Shape"),           OBJ_OLE2 },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.PageShape"),           OBJ_PAGE },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.CaptionShape"),            OBJ_CAPTION },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.FrameShape"),          OBJ_FRAME },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.PluginShape"),         OBJ_OLE2_PLUGIN },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.AppletShape"),         OBJ_OLE2_APPLET },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.CustomShape"),         OBJ_CUSTOMSHAPE },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.MediaShape"),          OBJ_MEDIA },

              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.Shape3DSceneObject"),  E3D_POLYSCENE_ID  | E3D_INVENTOR_FLAG },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.Shape3DCubeObject"),   E3D_CUBEOBJ_ID    | E3D_INVENTOR_FLAG },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.Shape3DSphereObject"), E3D_SPHEREOBJ_ID  | E3D_INVENTOR_FLAG },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.Shape3DLatheObject"),  E3D_LATHEOBJ_ID   | E3D_INVENTOR_FLAG },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.Shape3DExtrudeObject"),    E3D_EXTRUDEOBJ_ID | E3D_INVENTOR_FLAG },
              { RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.Shape3DPolygonObject"),    E3D_POLYGONOBJ_ID | E3D_INVENTOR_FLAG },
          };
          for (sal_uInt32 i = 0; i < sizeof(aInit)/sizeof(aInit[0]); i++)
              aImpl[OUString( aInit[i].name, aInit[i].length, RTL_TEXTENCODING_ASCII_US ) ] = aInit[i].id;
          bInited = true;
        }
      return aImpl;
  }
}

//----------------------------------------------------------------------
OUString UHashMap::getNameFromId(sal_uInt32 nId)
{
    const UHashMapImpl &rMap = GetUHashImpl();

    for (UHashMapImpl::const_iterator it = rMap.begin(); it != rMap.end(); ++it)
    {
        if (it->second == nId)
            return it->first;
    }
    OSL_FAIL("[CL] unknown SdrObjekt identifier");
    return OUString();
}

uno::Sequence< OUString > UHashMap::getServiceNames()
{
    const UHashMapImpl &rMap = GetUHashImpl();

    uno::Sequence< OUString > aSeq( rMap.size() );
    OUString* pStrings = aSeq.getArray();

    int i = 0;
    for (UHashMapImpl::const_iterator it = rMap.begin(); it != rMap.end(); ++it)
        pStrings[i++] = it->first;

    return aSeq;
}

sal_uInt32 UHashMap::getId( const OUString& rCompareString )
{
    const UHashMapImpl &rMap = GetUHashImpl();
    UHashMapImpl::const_iterator it = rMap.find( rCompareString );
    if( it == rMap.end() )
        return UHASHMAP_NOTFOUND;
    else
        return it->second;
}

/***********************************************************************
* class SvxUnoPropertyMapProvider                                      *
***********************************************************************/

struct theSvxMapProvider :
    public rtl::Static<SvxUnoPropertyMapProvider, theSvxMapProvider>
{
};

SvxUnoPropertyMapProvider& getSvxMapProvider()
{
    return theSvxMapProvider::get();
}

// ---------------------------------------------------------------------

SvxUnoPropertyMapProvider::SvxUnoPropertyMapProvider()
{
    for(sal_uInt16 i=0;i<SVXMAP_END; i++)
    {
        aSetArr[i] = 0;
        aMapArr[i] = 0;
    }
}

SvxUnoPropertyMapProvider::~SvxUnoPropertyMapProvider()
{
    for(sal_uInt16 i=0;i<SVXMAP_END; i++)
        delete aSetArr[i];
}

// ---------------------------------------------------------------------

const SfxItemPropertyMapEntry* SvxUnoPropertyMapProvider::GetMap(sal_uInt16 nPropertyId)
{
    DBG_ASSERT(nPropertyId < SVXMAP_END, "Id ?" );
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
            case SVXMAP_MEDIA: aMapArr[SVXMAP_MEDIA]=ImplGetSvxMediaShapePropertyMap(); break;
            case SVXMAP_TABLE: aMapArr[SVXMAP_TABLE]=ImplGetSvxTableShapePropertyMap(); break;
            case SVXMAP_PAGE: aMapArr[SVXMAP_PAGE] = ImplGetSvxPageShapePropertyMap(); break;

            default:
                OSL_FAIL( "Unknown property map for SvxUnoPropertyMapProvider!" );
        }
//      Sort(nPropertyId);
    }
    return aMapArr[nPropertyId];
}
const SvxItemPropertySet* SvxUnoPropertyMapProvider::GetPropertySet(sal_uInt16 nPropertyId, SfxItemPool& rPool)
{
    if( !aSetArr[nPropertyId] )
        aSetArr[nPropertyId] = new SvxItemPropertySet( GetMap( nPropertyId ), rPool );
    return aSetArr[nPropertyId];
}

// #####################################################################

/** maps the API constant MeasureUnit to a vcl MapUnit enum.
    Returns false if conversion is not supported.

    @cl: for warnings01 I found out that this method never worked so I thin
         it is not used at all
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
*/

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
        return false;
    }

    return true;
}

bool SvxUnoConvertResourceString( int nSourceResIds, int nDestResIds, int nCount, OUString& rString ) throw()
{
    // first, calculate the search string length without an optional number behind the name
    sal_Int32 nLength = rString.getLength();
    while( nLength > 0 )
    {
        const sal_Unicode nChar = rString[ nLength - 1 ];
        if( (nChar < '0') || (nChar > '9') )
            break;

        nLength--;
    }

    // if we cut off a number, also cut of some spaces
    if( nLength != rString.getLength() )
    {
        while( nLength > 0 )
        {
            const sal_Unicode nChar = rString[ nLength - 1 ];
            if( nChar != ' ' )
                break;

            nLength--;
        }
    }

    const OUString aShortString( rString.copy( 0, nLength ) );

    int i;
    for( i = 0; i < nCount; i++ )
    {
        sal_uInt16 nResId = (sal_uInt16)(nSourceResIds + i);
        const ResId aRes( SVX_RES(nResId));
        const OUString aCompare( aRes.toString() );
        if( aShortString == aCompare )
        {
            sal_uInt16 nNewResId = (sal_uInt16)(nDestResIds + i);
            ResId aNewRes( SVX_RES( nNewResId ));
            rString = rString.replaceAt( 0, aShortString.getLength(), aNewRes.toString() );
            return true;
        }
        else if( rString == aCompare )
        {
            sal_uInt16 nNewResId = (sal_uInt16)(nDestResIds + i);
            ResId aNewRes( SVX_RES( nNewResId ));
            rString = aNewRes.toString();
            return true;
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////
// #i122649# Some comments on the below arrays:
// - They need to have the same order and count of items
// - They are used to translate between translated and non-translated color names
// - To make longer names be found which start with the same basic string,
//   these have to be in front of others
//
// It would be nice to:
// - evtl. organize these in a single array with 2-dimensional inner to elliminate
//   the possibility to define it wrong
// - change the compare to also work when a shorter name is in front of a longer one

static const sal_uInt16 SvxUnoColorNameDefResId[] =
{
    RID_SVXSTR_COLOR_BLUEGREY_DEF,
    RID_SVXSTR_COLOR_BLACK_DEF,
    RID_SVXSTR_COLOR_BLUE_CLASSIC_DEF,
    RID_SVXSTR_COLOR_BLUE_DEF,
    RID_SVXSTR_COLOR_GREEN_DEF,
    RID_SVXSTR_COLOR_CYAN_DEF,
    RID_SVXSTR_COLOR_RED_DEF,
    RID_SVXSTR_COLOR_MAGENTA_DEF,
    RID_SVXSTR_COLOR_GREY_DEF,
    RID_SVXSTR_COLOR_YELLOWGREEN_DEF,
    RID_SVXSTR_COLOR_YELLOW_DEF,
    RID_SVXSTR_COLOR_WHITE_DEF,
    RID_SVXSTR_COLOR_ORANGE_DEF,
    RID_SVXSTR_COLOR_VIOLET_DEF,
    RID_SVXSTR_COLOR_BORDEAUX_DEF,
    RID_SVXSTR_COLOR_PALE_YELLOW_DEF,
    RID_SVXSTR_COLOR_PALE_GREEN_DEF,
    RID_SVXSTR_COLOR_DARKVIOLET_DEF,
    RID_SVXSTR_COLOR_SALMON_DEF,
    RID_SVXSTR_COLOR_SEABLUE_DEF,
    RID_SVXSTR_COLOR_CHART_DEF,
    RID_SVXSTR_COLOR_PURPLE_DEF,
    RID_SVXSTR_COLOR_SKYBLUE_DEF,
    RID_SVXSTR_COLOR_PINK_DEF,
    RID_SVXSTR_COLOR_TURQUOISE_DEF,
    RID_SVXSTR_COLOR_LIBRE_GREEN_1_DEF,
    RID_SVXSTR_COLOR_LIBRE_GREEN_ACCENT_DEF,
    RID_SVXSTR_COLOR_LIBRE_BLUE_ACCENT_DEF,
    RID_SVXSTR_COLOR_LIBRE_ORANGE_ACCENT_DEF,
    RID_SVXSTR_COLOR_LIBRE_PURPLE_DEF,
    RID_SVXSTR_COLOR_LIBRE_PURPLE_ACCENT_DEF,
    RID_SVXSTR_COLOR_LIBRE_YELLOW_ACCENT_DEF,
    RID_SVXSTR_COLOR_TANGO_BUTTER_DEF,
    RID_SVXSTR_COLOR_TANGO_ORANGE_DEF,
    RID_SVXSTR_COLOR_TANGO_CHOCOLATE_DEF,
    RID_SVXSTR_COLOR_TANGO_CHAMELEON_DEF,
    RID_SVXSTR_COLOR_TANGO_SKY_BLUE_DEF,
    RID_SVXSTR_COLOR_TANGO_PLUM_DEF,
    RID_SVXSTR_COLOR_TANGO_SCARLET_RED_DEF,
    RID_SVXSTR_COLOR_TANGO_ALUMINIUM_DEF
};

static const sal_uInt16 SvxUnoColorNameResId[] =
{
    RID_SVXSTR_COLOR_BLUEGREY,
    RID_SVXSTR_COLOR_BLACK,
    RID_SVXSTR_COLOR_BLUE_CLASSIC,
    RID_SVXSTR_COLOR_BLUE,
    RID_SVXSTR_COLOR_GREEN,
    RID_SVXSTR_COLOR_CYAN,
    RID_SVXSTR_COLOR_RED,
    RID_SVXSTR_COLOR_MAGENTA,
    RID_SVXSTR_COLOR_GREY,
    RID_SVXSTR_COLOR_YELLOWGREEN,
    RID_SVXSTR_COLOR_YELLOW,
    RID_SVXSTR_COLOR_WHITE,
    RID_SVXSTR_COLOR_ORANGE,
    RID_SVXSTR_COLOR_VIOLET,
    RID_SVXSTR_COLOR_BORDEAUX,
    RID_SVXSTR_COLOR_PALE_YELLOW,
    RID_SVXSTR_COLOR_PALE_GREEN,
    RID_SVXSTR_COLOR_DARKVIOLET,
    RID_SVXSTR_COLOR_SALMON,
    RID_SVXSTR_COLOR_SEABLUE,
    RID_SVXSTR_COLOR_CHART,
    RID_SVXSTR_COLOR_PURPLE,
    RID_SVXSTR_COLOR_SKYBLUE,
    RID_SVXSTR_COLOR_PINK,
    RID_SVXSTR_COLOR_TURQUOISE,
    RID_SVXSTR_COLOR_LIBRE_GREEN_1,
    RID_SVXSTR_COLOR_LIBRE_GREEN_ACCENT,
    RID_SVXSTR_COLOR_LIBRE_BLUE_ACCENT,
    RID_SVXSTR_COLOR_LIBRE_ORANGE_ACCENT,
    RID_SVXSTR_COLOR_LIBRE_PURPLE,
    RID_SVXSTR_COLOR_LIBRE_PURPLE_ACCENT,
    RID_SVXSTR_COLOR_LIBRE_YELLOW_ACCENT,
    RID_SVXSTR_COLOR_TANGO_BUTTER,
    RID_SVXSTR_COLOR_TANGO_ORANGE,
    RID_SVXSTR_COLOR_TANGO_CHOCOLATE,
    RID_SVXSTR_COLOR_TANGO_CHAMELEON,
    RID_SVXSTR_COLOR_TANGO_SKY_BLUE,
    RID_SVXSTR_COLOR_TANGO_PLUM,
    RID_SVXSTR_COLOR_TANGO_SCARLET_RED,
    RID_SVXSTR_COLOR_TANGO_ALUMINIUM
};

//////////////////////////////////////////////////////////////////////////////

bool SvxUnoConvertResourceString( const sal_uInt16* pSourceResIds, const sal_uInt16* pDestResIds, int nCount, OUString& rString ) throw()
{
    //We replace e.g. "Gray 10%" with the translation of Gray, but we shouldn't
    //replace "Red Hat 1" with the translation of Red :-)
    sal_Int32 nLength = rString.getLength();
    while( nLength > 0 )
    {
        const sal_Unicode nChar = rString[nLength-1];
        if (nChar != '%' && (nChar < '0' || nChar > '9'))
            break;
        nLength--;
    }
    OUString sStr = rString.copy(0, nLength).trim();

    for(int i = 0; i < nCount; ++i )
    {
        OUString aStrDefName = SVX_RESSTR( pSourceResIds[i] );
        if( sStr == aStrDefName )
        {
            OUString aReplace = SVX_RESSTR( pDestResIds[i] );
            rString = rString.replaceAt( 0, aStrDefName.getLength(), aReplace );
            return true;
        }
    }

    return false;
}

/** if the given name is a predefined name for the current language it is replaced by
    the corresponding api name.
*/
OUString SvxUnogetApiNameForItem(const sal_Int16 nWhich, const OUString& rInternalName) throw()
{
    OUString aNew = rInternalName;

    if( nWhich == XATTR_LINECOLOR )
    {
        if( SvxUnoConvertResourceString( SvxUnoColorNameResId, SvxUnoColorNameDefResId, sizeof( SvxUnoColorNameResId ) / sizeof( sal_uInt16 ), aNew ) )
        {
            return aNew;
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
                return aNew;
            }
        }
    }

    // just use previous name, if nothing else was found.
    return rInternalName;
}

/** if the given name is a predefined api name it is replaced by the predefined name
    for the current language.
*/
OUString SvxUnogetInternalNameForItem(const sal_Int16 nWhich, const OUString& rApiName) throw()
{
    OUString aNew = rApiName;

    if( nWhich == XATTR_LINECOLOR )
    {
        if( SvxUnoConvertResourceString( SvxUnoColorNameDefResId, SvxUnoColorNameResId, sizeof( SvxUnoColorNameResId ) / sizeof( sal_uInt16 ), aNew ) )
        {
            return aNew;
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
                return aNew;
            }
        }
    }

    // just use previous name, if nothing else was found.
    return rApiName;
}

///////////////////////////////////////////////////////////////////////

comphelper::PropertySetInfo* SvxPropertySetInfoPool::getOrCreate( sal_Int32 nServiceId ) throw()
{
    SolarMutexGuard aGuard;

    if( nServiceId > SVXUNO_SERVICEID_LASTID )
    {
        OSL_FAIL( "unknown service id!" );
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
            mpInfos[SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS_WRITER]->remove( OUString( UNO_NAME_EDIT_PARA_IS_HANGING_PUNCTUATION ) );
            // OD 13.10.2003 #i18732# - add property map for writer item 'IsFollowingTextFlow'
            mpInfos[SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS_WRITER]->add( ImplGetAdditionalWriterDrawingDefaultsPropertyMap() );
            break;

        default:
            OSL_FAIL( "unknown service id!" );
        }
    }

    return mpInfos[ nServiceId ];
}

comphelper::PropertySetInfo* SvxPropertySetInfoPool::mpInfos[SVXUNO_SERVICEID_LASTID+1] = { NULL };

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
