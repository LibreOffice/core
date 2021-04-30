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
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/media/ZoomLevel.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <tools/debug.hxx>
#include <tools/fldunit.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <comphelper/sequence.hxx>
#include <svx/dialmgr.hxx>
#include <svx/unoapi.hxx>
#include <svx/unoshprp.hxx>
#include <svx/svxids.hrc>
#include <svx/svdobjkind.hxx>
#include <svx/strings.hrc>
#include <strings.hxx>

#include "shapeimpl.hxx"
#include <unordered_map>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans::PropertyAttribute;
using ::com::sun::star::drawing::TextVerticalAdjust;

static SfxItemPropertyMapEntry const * ImplGetSvxShapePropertyMap()
{
    static SfxItemPropertyMapEntry const aShapePropertyMap_Impl[] =
    {
        EDGERADIUS_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        GLOW_PROPERTIES
        SOFTEDGE_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        // #FontWork#
        FONTWORK_PROPERTIES
        { u"UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aShapePropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvxTextShapePropertyMap()
{
    static SfxItemPropertyMapEntry const aTextShapePropertyMap_Impl[] =
    {
        EDGERADIUS_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES_NO_SHEAR
        LINKTARGET_PROPERTIES
        GLOW_PROPERTIES
        SOFTEDGE_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        // #FontWork#
        FONTWORK_PROPERTIES
        { u"UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aTextShapePropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvxConnectorPropertyMap()
{
    static SfxItemPropertyMapEntry const aConnectorPropertyMap_Impl[] =
    {
        SPECIAL_CONNECTOR_PROPERTIES
        EDGERADIUS_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        GLOW_PROPERTIES
        SOFTEDGE_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        // #FontWork#
        FONTWORK_PROPERTIES
        { u"UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aConnectorPropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvxDimensioningPropertyMap()
{
    static SfxItemPropertyMapEntry const aDimensioningPropertyMap_Impl[] =
    {
        SPECIAL_DIMENSIONING_PROPERTIES
        EDGERADIUS_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        GLOW_PROPERTIES
        SOFTEDGE_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        // #FontWork#
        FONTWORK_PROPERTIES
        { u"UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aDimensioningPropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvxCirclePropertyMap()
{
    static SfxItemPropertyMapEntry const aCirclePropertyMap_Impl[] =
    {
        SPECIAL_CIRCLE_PROPERTIES
        EDGERADIUS_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        GLOW_PROPERTIES
        SOFTEDGE_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        // #FontWork#
        FONTWORK_PROPERTIES
        { u"UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aCirclePropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvxPolyPolygonPropertyMap()
{
    static SfxItemPropertyMapEntry const aPolyPolygonPropertyMap_Impl[] =
    {
        { u"Geometry", OWN_ATTR_BASE_GEOMETRY, cppu::UnoType<css::drawing::PointSequenceSequence>::get(), 0, 0 },
        SPECIAL_POLYGON_PROPERTIES
        SPECIAL_POLYPOLYGON_PROPERTIES
        SPECIAL_POLYPOLYGONBEZIER_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        GLOW_PROPERTIES
        SOFTEDGE_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        // #FontWork#
        FONTWORK_PROPERTIES
        { u"UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aPolyPolygonPropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvxGraphicObjectPropertyMap()
{
    static SfxItemPropertyMapEntry const aGraphicObjectPropertyMap_Impl[] =
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
        GLOW_PROPERTIES
        SOFTEDGE_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        // #FontWork#
        FONTWORK_PROPERTIES
        { u"IsMirrored", OWN_ATTR_MIRRORED, cppu::UnoType<bool>::get(), 0, 0},
        { u"UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"GraphicStream", OWN_ATTR_GRAPHIC_STREAM, cppu::UnoType<css::io::XInputStream>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aGraphicObjectPropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvx3DSceneObjectPropertyMap()
{
    static SfxItemPropertyMapEntry const a3DSceneObjectPropertyMap_Impl[] =
    {
        SPECIAL_3DSCENEOBJECT_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return a3DSceneObjectPropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvx3DCubeObjectPropertyMap()
{
    static SfxItemPropertyMapEntry const a3DCubeObjectPropertyMap_Impl[] =
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
        { u"UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return a3DCubeObjectPropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvx3DSphereObjectPropertyMap()
{
    static SfxItemPropertyMapEntry const a3DSphereObjectPropertyMap_Impl[] =
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
        { u"UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };
    return a3DSphereObjectPropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvx3DLatheObjectPropertyMap()
{
    static SfxItemPropertyMapEntry const a3DLatheObjectPropertyMap_Impl[] =
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
        { u"UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return a3DLatheObjectPropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvx3DExtrudeObjectPropertyMap()
{
    static SfxItemPropertyMapEntry const a3DExtrudeObjectPropertyMap_Impl[] =
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
        { u"UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return a3DExtrudeObjectPropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvx3DPolygonObjectPropertyMap()
{
    static SfxItemPropertyMapEntry const a3DPolygonObjectPropertyMap_Impl[] =
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
        { u"UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return a3DPolygonObjectPropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvxAllPropertyMap()
{
    static SfxItemPropertyMapEntry const aAllPropertyMap_Impl[] =
    {
        GLOW_PROPERTIES
        SOFTEDGE_PROPERTIES
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
        { u"UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aAllPropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvxGroupPropertyMap()
{
    static SfxItemPropertyMapEntry const aGroupPropertyMap_Impl[] =
    {
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aGroupPropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvxOle2PropertyMap()
{
    static SfxItemPropertyMapEntry const aOle2PropertyMap_Impl[] =
    {
        // #i118485# Adding properties for line, fill, text, shadow, fontwork, rotate, shear
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        GLOW_PROPERTIES
        SOFTEDGE_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        FONTWORK_PROPERTIES

        { u"ThumbnailGraphic",         OWN_ATTR_THUMBNAIL          , cppu::UnoType<css::graphic::XGraphic>::get(), 0, 0 },
        { u"Model",                    OWN_ATTR_OLEMODEL           , cppu::UnoType<css::frame::XModel>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"EmbeddedObject",           OWN_ATTR_OLE_EMBEDDED_OBJECT, cppu::UnoType<css::embed::XEmbeddedObject>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"EmbeddedObjectNoNewClient",OWN_ATTR_OLE_EMBEDDED_OBJECT_NONEWCLIENT, cppu::UnoType<css::embed::XEmbeddedObject>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"OriginalSize",             OWN_ATTR_OLESIZE            , cppu::UnoType<css::awt::Size>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"CLSID",                    OWN_ATTR_CLSID              , cppu::UnoType<OUString>::get(), 0, 0 },
        { u"IsInternal",               OWN_ATTR_INTERNAL_OLE       , cppu::UnoType<bool>::get() , css::beans::PropertyAttribute::READONLY,     0},
        { u"VisibleArea",              OWN_ATTR_OLE_VISAREA        , cppu::UnoType<css::awt::Rectangle>::get(), 0, 0},
        { u"Aspect",                   OWN_ATTR_OLE_ASPECT         , cppu::UnoType<sal_Int64>::get(), 0, 0},
        { u"" UNO_NAME_OLE2_PERSISTNAME,  OWN_ATTR_PERSISTNAME        , cppu::UnoType<OUString>::get(), 0, 0 },
        { u"LinkURL",                  OWN_ATTR_OLE_LINKURL        , cppu::UnoType<OUString>::get(), 0, 0 },
        { u"" UNO_NAME_GRAPHOBJ_GRAPHIC,   OWN_ATTR_VALUE_GRAPHIC     , cppu::UnoType<css::graphic::XGraphic>::get(),     0,     0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aOle2PropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvxPluginPropertyMap()
{
    static SfxItemPropertyMapEntry const aPluginPropertyMap_Impl[] =
    {
        { u"PluginMimeType",           OWN_ATTR_PLUGIN_MIMETYPE    , cppu::UnoType<OUString>::get(),    0,  0},
        { u"PluginURL",                OWN_ATTR_PLUGIN_URL         , cppu::UnoType<OUString>::get(),    0,  0},
        { u"PluginCommands",           OWN_ATTR_PLUGIN_COMMANDS    , cppu::UnoType<css::uno::Sequence< css::beans::PropertyValue >>::get(), 0, 0},
        { u"Transformation",           OWN_ATTR_TRANSFORMATION     , cppu::UnoType<css::drawing::HomogenMatrix3>::get(), 0, 0 },
        { u"" UNO_NAME_MISC_OBJ_ZORDER,   OWN_ATTR_ZORDER             , cppu::UnoType<sal_Int32>::get(),      0,      0},
        { u"" UNO_NAME_MISC_OBJ_LAYERID,  SDRATTR_LAYERID             , cppu::UnoType<sal_Int16>::get(),      0,  0},
        { u"" UNO_NAME_MISC_OBJ_LAYERNAME,SDRATTR_LAYERNAME           , cppu::UnoType<OUString>::get(),    0,  0},
        { u"" UNO_NAME_LINKDISPLAYBITMAP, OWN_ATTR_LDBITMAP           , cppu::UnoType<css::awt::XBitmap>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_LINKDISPLAYNAME,   OWN_ATTR_LDNAME             , cppu::UnoType<OUString>::get(),   css::beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_OLE2_METAFILE,     OWN_ATTR_METAFILE           , cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"ThumbnailGraphic",         OWN_ATTR_THUMBNAIL          , cppu::UnoType<css::graphic::XGraphic>::get(), 0, 0 },
        { u"" UNO_NAME_MISC_OBJ_MOVEPROTECT,      SDRATTR_OBJMOVEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { u"" UNO_NAME_MISC_OBJ_SIZEPROTECT,      SDRATTR_OBJSIZEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { u"" UNO_NAME_OLE2_PERSISTNAME,  OWN_ATTR_PERSISTNAME        , cppu::UnoType<OUString>::get(), 0, 0 },
        { u"LinkURL",                  OWN_ATTR_OLE_LINKURL        , cppu::UnoType<OUString>::get(), 0, 0 },
        { u"" UNO_NAME_MISC_OBJ_BOUNDRECT, OWN_ATTR_BOUNDRECT,            cppu::UnoType<css::awt::Rectangle>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"VisibleArea",              OWN_ATTR_OLE_VISAREA        , cppu::UnoType<css::awt::Rectangle>::get(), 0, 0},
        { u"UINameSingular",               OWN_ATTR_UINAME_SINGULAR        , ::cppu::UnoType<OUString>::get(),    css::beans::PropertyAttribute::READONLY,   0},
        // #i68101#
        { u"" UNO_NAME_MISC_OBJ_TITLE,        OWN_ATTR_MISC_OBJ_TITLE         , cppu::UnoType<OUString>::get(),    0,  0},
        { u"" UNO_NAME_MISC_OBJ_DESCRIPTION,  OWN_ATTR_MISC_OBJ_DESCRIPTION   , cppu::UnoType<OUString>::get(),    0,  0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aPluginPropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvxFramePropertyMap()
{
    //TODO/LATER: new properties for ScrollingMode and DefaultBorder
    static SfxItemPropertyMapEntry const aFramePropertyMap_Impl[] =
    {
        { u"FrameURL",                 OWN_ATTR_FRAME_URL          , cppu::UnoType<OUString>::get(),    0,  0},
        { u"FrameName",                OWN_ATTR_FRAME_NAME         , cppu::UnoType<OUString>::get(),    0,  0},
        { u"FrameIsAutoScroll",        OWN_ATTR_FRAME_ISAUTOSCROLL , cppu::UnoType<bool>::get() , css::beans::PropertyAttribute::MAYBEVOID, 0},
        { u"FrameIsBorder",            OWN_ATTR_FRAME_ISBORDER     , cppu::UnoType<bool>::get() , 0, 0},
        { u"FrameMarginWidth",         OWN_ATTR_FRAME_MARGIN_WIDTH , cppu::UnoType<sal_Int32>::get(),      0,      0},
        { u"FrameMarginHeight",        OWN_ATTR_FRAME_MARGIN_HEIGHT, cppu::UnoType<sal_Int32>::get(),      0,      0},
        { u"Transformation",           OWN_ATTR_TRANSFORMATION     , cppu::UnoType<css::drawing::HomogenMatrix3>::get(), 0, 0 },
        { u"" UNO_NAME_MISC_OBJ_ZORDER,   OWN_ATTR_ZORDER             , cppu::UnoType<sal_Int32>::get(),      0,      0},
        { u"" UNO_NAME_MISC_OBJ_LAYERID,  SDRATTR_LAYERID             , cppu::UnoType<sal_Int16>::get(),      0,  0},
        { u"" UNO_NAME_MISC_OBJ_LAYERNAME,SDRATTR_LAYERNAME           , cppu::UnoType<OUString>::get(),    0,  0},
        { u"" UNO_NAME_LINKDISPLAYBITMAP, OWN_ATTR_LDBITMAP           , cppu::UnoType<css::awt::XBitmap>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_LINKDISPLAYNAME,   OWN_ATTR_LDNAME             , cppu::UnoType<OUString>::get(),   css::beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_OLE2_METAFILE,     OWN_ATTR_METAFILE           , cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"ThumbnailGraphic",         OWN_ATTR_THUMBNAIL          , cppu::UnoType<css::graphic::XGraphic>::get(), 0, 0 },
        { u"" UNO_NAME_MISC_OBJ_MOVEPROTECT,      SDRATTR_OBJMOVEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { u"" UNO_NAME_MISC_OBJ_SIZEPROTECT,      SDRATTR_OBJSIZEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { u"" UNO_NAME_OLE2_PERSISTNAME,  OWN_ATTR_PERSISTNAME        , cppu::UnoType<OUString>::get(), 0, 0 },
        { u"LinkURL",                  OWN_ATTR_OLE_LINKURL        , cppu::UnoType<OUString>::get(), 0, 0 },
        { u"" UNO_NAME_MISC_OBJ_BOUNDRECT, OWN_ATTR_BOUNDRECT,            cppu::UnoType<css::awt::Rectangle>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"VisibleArea",              OWN_ATTR_OLE_VISAREA        , cppu::UnoType<css::awt::Rectangle>::get(), 0, 0},
        { u"UINameSingular",               OWN_ATTR_UINAME_SINGULAR        , ::cppu::UnoType<OUString>::get(),    css::beans::PropertyAttribute::READONLY,   0},
        // #i68101#
        { u"" UNO_NAME_MISC_OBJ_TITLE,        OWN_ATTR_MISC_OBJ_TITLE         , cppu::UnoType<OUString>::get(),    0,  0},
        { u"" UNO_NAME_MISC_OBJ_DESCRIPTION,  OWN_ATTR_MISC_OBJ_DESCRIPTION   , cppu::UnoType<OUString>::get(),    0,  0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aFramePropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvxAppletPropertyMap()
{
    static SfxItemPropertyMapEntry const aAppletPropertyMap_Impl[] =
    {
        { u"AppletCodeBase",           OWN_ATTR_APPLET_CODEBASE    , cppu::UnoType<OUString>::get(), 0, 0},
        { u"AppletName",               OWN_ATTR_APPLET_NAME        , cppu::UnoType<OUString>::get(), 0, 0},
        { u"AppletCode",               OWN_ATTR_APPLET_CODE        , cppu::UnoType<OUString>::get(), 0, 0},
        { u"AppletCommands",           OWN_ATTR_APPLET_COMMANDS    , cppu::UnoType<css::uno::Sequence< css::beans::PropertyValue >>::get(), 0, 0},
        { u"AppletDocBase",            OWN_ATTR_APPLET_DOCBASE     , cppu::UnoType<OUString>::get(), 0, 0},
        { u"AppletIsScript",           OWN_ATTR_APPLET_ISSCRIPT    , cppu::UnoType<bool>::get(), 0, 0 },
        { u"Transformation",           OWN_ATTR_TRANSFORMATION     , cppu::UnoType<css::drawing::HomogenMatrix3>::get(), 0, 0 },
        { u"" UNO_NAME_MISC_OBJ_ZORDER,   OWN_ATTR_ZORDER             , cppu::UnoType<sal_Int32>::get(),      0,      0},
        { u"" UNO_NAME_MISC_OBJ_LAYERID,  SDRATTR_LAYERID             , cppu::UnoType<sal_Int16>::get(),      0,  0},
        { u"" UNO_NAME_MISC_OBJ_LAYERNAME,SDRATTR_LAYERNAME           , cppu::UnoType<OUString>::get(),    0,  0},
        { u"" UNO_NAME_LINKDISPLAYBITMAP, OWN_ATTR_LDBITMAP           , cppu::UnoType<css::awt::XBitmap>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_LINKDISPLAYNAME,   OWN_ATTR_LDNAME             , cppu::UnoType<OUString>::get(),   css::beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_OLE2_METAFILE,     OWN_ATTR_METAFILE           , cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"ThumbnailGraphic",         OWN_ATTR_THUMBNAIL          , cppu::UnoType<css::graphic::XGraphic>::get(), 0, 0 },
        { u"" UNO_NAME_MISC_OBJ_MOVEPROTECT,      SDRATTR_OBJMOVEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { u"" UNO_NAME_MISC_OBJ_SIZEPROTECT,      SDRATTR_OBJSIZEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { u"" UNO_NAME_OLE2_PERSISTNAME,  OWN_ATTR_PERSISTNAME        , cppu::UnoType<OUString>::get(), 0, 0 },
        { u"LinkURL",                  OWN_ATTR_OLE_LINKURL        , cppu::UnoType<OUString>::get(), 0, 0 },
        { u"" UNO_NAME_MISC_OBJ_BOUNDRECT, OWN_ATTR_BOUNDRECT,            cppu::UnoType<css::awt::Rectangle>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"VisibleArea",              OWN_ATTR_OLE_VISAREA        , cppu::UnoType<css::awt::Rectangle>::get(), 0, 0},
        { u"UINameSingular",               OWN_ATTR_UINAME_SINGULAR        , ::cppu::UnoType<OUString>::get(),    css::beans::PropertyAttribute::READONLY,   0},
        // #i68101#
        { u"" UNO_NAME_MISC_OBJ_TITLE,        OWN_ATTR_MISC_OBJ_TITLE         , cppu::UnoType<OUString>::get(),    0,  0},
        { u"" UNO_NAME_MISC_OBJ_DESCRIPTION,  OWN_ATTR_MISC_OBJ_DESCRIPTION   , cppu::UnoType<OUString>::get(),    0,  0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aAppletPropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvxControlShapePropertyMap()
{
    static SfxItemPropertyMapEntry const aControlPropertyMap_Impl[] =
    {
        // the following properties are mapped to the XControl Model of this shape
        { u"" UNO_NAME_EDIT_CHAR_FONTNAME,        0,  cppu::UnoType<OUString>::get(),  0, 0 },
        { u"" UNO_NAME_EDIT_CHAR_FONTSTYLENAME,   0,  cppu::UnoType<OUString>::get(),  0, 0 },
        { u"" UNO_NAME_EDIT_CHAR_FONTFAMILY,      0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { u"" UNO_NAME_EDIT_CHAR_FONTCHARSET,     0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { u"" UNO_NAME_EDIT_CHAR_HEIGHT,          0,  cppu::UnoType<float>::get(),            0, 0 },
        { u"" UNO_NAME_EDIT_CHAR_FONTPITCH,       0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { u"" UNO_NAME_EDIT_CHAR_POSTURE,         0,  cppu::UnoType<css::awt::FontSlant>::get(),0, 0 },
        { u"" UNO_NAME_EDIT_CHAR_WEIGHT,          0,  cppu::UnoType<float>::get(),            0, 0 },
        { u"" UNO_NAME_EDIT_CHAR_UNDERLINE,       0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { u"" UNO_NAME_EDIT_CHAR_STRIKEOUT,       0,  cppu::UnoType<sal_Int16>::get(),        0, 0},
        { u"" UNO_NAME_EDIT_CHAR_CASEMAP,         0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { u"" UNO_NAME_EDIT_CHAR_COLOR,           0,  cppu::UnoType<sal_Int32>::get(),        0, 0 },
        { u"CharBackColor",                    0,  cppu::UnoType<sal_Int32>::get(),        0, 0 },
        { u"CharBackTransparent",              0,  cppu::UnoType<bool>::get(),             0, 0 },
        { u"CharRelief",                       0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { u"CharUnderlineColor",               0,  cppu::UnoType<sal_Int32>::get(),        0, 0 },
        { u"CharKerning",                      0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { u"CharWordMode",                     0,  cppu::UnoType<bool>::get(),                    0, 0 },
        { u"" UNO_NAME_EDIT_PARA_ADJUST,          0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { u"TextVerticalAdjust",               0,  cppu::UnoType<TextVerticalAdjust>::get(), MAYBEVOID, 0 },
        { u"ControlBackground",                0,  cppu::UnoType<sal_Int32>::get(),        0, 0 },
        { u"ControlBorder",                    0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { u"ControlBorderColor",               0,  cppu::UnoType<sal_Int32>::get(),        0, 0 },
        { u"ControlSymbolColor",               0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { u"ImageScaleMode",                   0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { u"" UNO_NAME_MISC_OBJ_MOVEPROTECT,      SDRATTR_OBJMOVEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { u"" UNO_NAME_MISC_OBJ_SIZEPROTECT,      SDRATTR_OBJSIZEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { u"ControlTextEmphasis",              0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { u"ControlWritingMode",               0,  cppu::UnoType< sal_Int16 >::get(), 0, 0},
        // the following properties are handled by SvxShape
        { u"Transformation",           OWN_ATTR_TRANSFORMATION     , cppu::UnoType<css::drawing::HomogenMatrix3>::get(), 0, 0 },
        { u"" UNO_NAME_MISC_OBJ_ZORDER,   OWN_ATTR_ZORDER             , cppu::UnoType<sal_Int32>::get(),      0,      0},
        { u"" UNO_NAME_MISC_OBJ_LAYERID,  SDRATTR_LAYERID             , cppu::UnoType<sal_Int16>::get(),      0,  0},
        { u"" UNO_NAME_MISC_OBJ_LAYERNAME,SDRATTR_LAYERNAME           , cppu::UnoType<OUString>::get(),    0,  0},
        { u"" UNO_NAME_LINKDISPLAYBITMAP, OWN_ATTR_LDBITMAP           , cppu::UnoType<css::awt::XBitmap>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_LINKDISPLAYNAME,   OWN_ATTR_LDNAME             , cppu::UnoType<OUString>::get(),   css::beans::PropertyAttribute::READONLY, 0},
        { u"UserDefinedAttributes",        SDRATTR_XMLATTRIBUTES,      cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        {u"ParaUserDefinedAttributes",         EE_PARA_XMLATTRIBS,     cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"" UNO_NAME_MISC_OBJ_BOUNDRECT, OWN_ATTR_BOUNDRECT,            cppu::UnoType<css::awt::Rectangle>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"UINameSingular",               OWN_ATTR_UINAME_SINGULAR        , ::cppu::UnoType<OUString>::get(),    css::beans::PropertyAttribute::READONLY,   0},
        // #i68101#
        { u"" UNO_NAME_MISC_OBJ_TITLE,        OWN_ATTR_MISC_OBJ_TITLE         , cppu::UnoType<OUString>::get(),    0,  0},
        { u"" UNO_NAME_MISC_OBJ_DESCRIPTION,  OWN_ATTR_MISC_OBJ_DESCRIPTION   , cppu::UnoType<OUString>::get(),    0,  0},
        // #i112587#
        { u"" UNO_NAME_MISC_OBJ_PRINTABLE,    SDRATTR_OBJPRINTABLE            , cppu::UnoType<bool>::get(),                      0,  0},
        { u"Visible",                      SDRATTR_OBJVISIBLE              , cppu::UnoType<bool>::get(),                      0,  0},
        { u"" UNO_NAME_MISC_OBJ_INTEROPGRABBAG,   OWN_ATTR_INTEROPGRABBAG,    cppu::UnoType<css::uno::Sequence< css::beans::PropertyValue >>::get(),  0,  0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aControlPropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvxPageShapePropertyMap()
{
    static SfxItemPropertyMapEntry const aPageShapePropertyMap_Impl[] =
    {
        { u"PageNumber",               OWN_ATTR_PAGE_NUMBER        , cppu::UnoType<sal_Int32>::get(),      0, 0},
        { u"Transformation",           OWN_ATTR_TRANSFORMATION     , cppu::UnoType<css::drawing::HomogenMatrix3>::get(), 0, 0 },
        { u"" UNO_NAME_MISC_OBJ_ZORDER,   OWN_ATTR_ZORDER             , cppu::UnoType<sal_Int32>::get(),      0, 0},
        { u"" UNO_NAME_MISC_OBJ_LAYERID,  SDRATTR_LAYERID             , cppu::UnoType<sal_Int16>::get(),      0, 0},
        { u"" UNO_NAME_MISC_OBJ_LAYERNAME,SDRATTR_LAYERNAME           , cppu::UnoType<OUString>::get(), 0, 0},
        { u"" UNO_NAME_LINKDISPLAYBITMAP, OWN_ATTR_LDBITMAP           , cppu::UnoType<css::awt::XBitmap>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_LINKDISPLAYNAME,   OWN_ATTR_LDNAME             , cppu::UnoType<OUString>::get(),   css::beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_MISC_OBJ_MOVEPROTECT,      SDRATTR_OBJMOVEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { u"" UNO_NAME_MISC_OBJ_SIZEPROTECT,      SDRATTR_OBJSIZEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { u"" UNO_NAME_MISC_OBJ_BOUNDRECT, OWN_ATTR_BOUNDRECT,            cppu::UnoType<css::awt::Rectangle>::get(), css::beans::PropertyAttribute::READONLY, 0},
        // #i68101#
        { u"" UNO_NAME_MISC_OBJ_TITLE,        OWN_ATTR_MISC_OBJ_TITLE         , cppu::UnoType<OUString>::get(),    0,  0},
        { u"" UNO_NAME_MISC_OBJ_DESCRIPTION,  OWN_ATTR_MISC_OBJ_DESCRIPTION   , cppu::UnoType<OUString>::get(),    0,  0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aPageShapePropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvxCaptionPropertyMap()
{
    static SfxItemPropertyMapEntry const aCaptionPropertyMap_Impl[] =
    {
        { u"CaptionPoint",             OWN_ATTR_CAPTION_POINT,     cppu::UnoType<css::awt::Point>::get(),   0, 0 },
        { u"CaptionType",              SDRATTR_CAPTIONTYPE,        cppu::UnoType<sal_Int16>::get(), 0, 0},
        { u"CaptionIsFixedAngle",      SDRATTR_CAPTIONFIXEDANGLE,  cppu::UnoType<bool>::get(), 0, 0},
        { u"CaptionAngle",             SDRATTR_CAPTIONANGLE,       cppu::UnoType<sal_Int32>::get(),    0,  0},
        { u"CaptionGap",               SDRATTR_CAPTIONGAP,         cppu::UnoType<sal_Int32>::get(),    0,  0, PropertyMoreFlags::METRIC_ITEM},
        { u"CaptionEscapeDirection",   SDRATTR_CAPTIONESCDIR,      cppu::UnoType<sal_Int32>::get(),    0,  0},
        { u"CaptionIsEscapeRelative",  SDRATTR_CAPTIONESCISREL,    cppu::UnoType<bool>::get(), 0, 0},
        { u"CaptionEscapeRelative",    SDRATTR_CAPTIONESCREL,      cppu::UnoType<sal_Int32>::get(),    0,  0},
        { u"CaptionEscapeAbsolute",    SDRATTR_CAPTIONESCABS,      cppu::UnoType<sal_Int32>::get(),    0,  0, PropertyMoreFlags::METRIC_ITEM},
        { u"CaptionLineLength",        SDRATTR_CAPTIONLINELEN,     cppu::UnoType<sal_Int32>::get(),    0,  0, PropertyMoreFlags::METRIC_ITEM},
        { u"CaptionIsFitLineLength",   SDRATTR_CAPTIONFITLINELEN,  cppu::UnoType<bool>::get(), 0, 0},
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
        { u"UserDefinedAttributes",        SDRATTR_XMLATTRIBUTES,      cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        {u"ParaUserDefinedAttributes",         EE_PARA_XMLATTRIBS,     cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aCaptionPropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvxCustomShapePropertyMap()
{
    static SfxItemPropertyMapEntry const aCustomShapePropertyMap_Impl[] =
    {
        { u"CustomShapeEngine",            SDRATTR_CUSTOMSHAPE_ENGINE,         cppu::UnoType<OUString>::get(),  0, 0 },
        { u"CustomShapeData",          SDRATTR_CUSTOMSHAPE_DATA,               cppu::UnoType<OUString>::get(),  0, 0 },
        { u"CustomShapeGeometry",      SDRATTR_CUSTOMSHAPE_GEOMETRY,
        cppu::UnoType<css::uno::Sequence< css::beans::PropertyValue >>::get(),  0, 0 },
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        GLOW_PROPERTIES
        SOFTEDGE_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        {u"UserDefinedAttributes",     SDRATTR_XMLATTRIBUTES,      cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        {u"ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS,         cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };
    return aCustomShapePropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvxMediaShapePropertyMap()
{
    static SfxItemPropertyMapEntry const aMediaShapePropertyMap_Impl[] =
    {
        { u"" UNO_NAME_MISC_OBJ_ZORDER, OWN_ATTR_ZORDER, cppu::UnoType<sal_Int32>::get(), 0, 0},
        { u"" UNO_NAME_MISC_OBJ_LAYERID, SDRATTR_LAYERID, cppu::UnoType<sal_Int16>::get(), 0, 0},
        { u"" UNO_NAME_MISC_OBJ_LAYERNAME, SDRATTR_LAYERNAME, cppu::UnoType<OUString>::get(), 0, 0},
        { u"" UNO_NAME_LINKDISPLAYBITMAP, OWN_ATTR_LDBITMAP, cppu::UnoType<css::awt::XBitmap>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_LINKDISPLAYNAME, OWN_ATTR_LDNAME, cppu::UnoType<OUString>::get(),  css::beans::PropertyAttribute::READONLY, 0},
        { u"Transformation", OWN_ATTR_TRANSFORMATION, cppu::UnoType<css::drawing::HomogenMatrix3>::get(), 0, 0 },
        { u"MediaURL", OWN_ATTR_MEDIA_URL, cppu::UnoType<OUString>::get(), 0, 0},
        { u"PreferredSize", OWN_ATTR_MEDIA_PREFERREDSIZE, cppu::UnoType<css::awt::Size>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"Loop", OWN_ATTR_MEDIA_LOOP, cppu::UnoType<sal_Bool>::get(), 0, 0},
        { u"Mute", OWN_ATTR_MEDIA_MUTE, cppu::UnoType<sal_Bool>::get(), 0, 0},
        { u"VolumeDB", OWN_ATTR_MEDIA_VOLUMEDB, cppu::UnoType<sal_Int16>::get(), 0, 0},
        { u"Zoom", OWN_ATTR_MEDIA_ZOOM, cppu::UnoType<css::media::ZoomLevel>::get(), 0, 0},
        { u"" UNO_NAME_MISC_OBJ_MOVEPROTECT, SDRATTR_OBJMOVEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { u"" UNO_NAME_MISC_OBJ_SIZEPROTECT, SDRATTR_OBJSIZEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { u"" UNO_NAME_MISC_OBJ_BOUNDRECT, OWN_ATTR_BOUNDRECT, cppu::UnoType<css::awt::Rectangle>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"UINameSingular",               OWN_ATTR_UINAME_SINGULAR        , ::cppu::UnoType<OUString>::get(),    css::beans::PropertyAttribute::READONLY,   0},
        // #i68101#
        { u"" UNO_NAME_MISC_OBJ_TITLE,        OWN_ATTR_MISC_OBJ_TITLE         , cppu::UnoType<OUString>::get(),    0,  0},
        { u"" UNO_NAME_MISC_OBJ_DESCRIPTION,  OWN_ATTR_MISC_OBJ_DESCRIPTION   , cppu::UnoType<OUString>::get(),    0,  0},
        {u"PrivateStream", OWN_ATTR_MEDIA_STREAM, cppu::UnoType<css::io::XInputStream>::get(), 0, 0},
        {u"PrivateTempFileURL", OWN_ATTR_MEDIA_TEMPFILEURL, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"MediaMimeType", OWN_ATTR_MEDIA_MIMETYPE, cppu::UnoType<OUString>::get(), 0, 0},
        { u"FallbackGraphic", OWN_ATTR_FALLBACK_GRAPHIC, cppu::UnoType<css::graphic::XGraphic>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_GRAPHOBJ_GRAPHIC,   OWN_ATTR_VALUE_GRAPHIC     , cppu::UnoType<css::graphic::XGraphic>::get(), 0, 0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aMediaShapePropertyMap_Impl;
}

static SfxItemPropertyMapEntry const * ImplGetSvxTableShapePropertyMap()
{
    static SfxItemPropertyMapEntry const  aTableShapePropertyMap_Impl[] =
    {
        SHADOW_PROPERTIES
        { u"" UNO_NAME_MISC_OBJ_ZORDER,       OWN_ATTR_ZORDER, cppu::UnoType<sal_Int32>::get(), 0, 0},
        { u"" UNO_NAME_MISC_OBJ_LAYERID,      SDRATTR_LAYERID, cppu::UnoType<sal_Int16>::get(), 0,    0},
        { u"" UNO_NAME_MISC_OBJ_LAYERNAME,    SDRATTR_LAYERNAME, cppu::UnoType<OUString>::get(), 0, 0},
        { u"" UNO_NAME_LINKDISPLAYBITMAP,     OWN_ATTR_LDBITMAP, cppu::UnoType<css::awt::XBitmap>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_LINKDISPLAYNAME,       OWN_ATTR_LDNAME, cppu::UnoType<OUString>::get(),    css::beans::PropertyAttribute::READONLY, 0},
        { u"Transformation",               OWN_ATTR_TRANSFORMATION, cppu::UnoType<css::drawing::HomogenMatrix3>::get(), 0, 0 },
        { u"" UNO_NAME_MISC_OBJ_MOVEPROTECT,  SDRATTR_OBJMOVEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { u"" UNO_NAME_MISC_OBJ_SIZEPROTECT,  SDRATTR_OBJSIZEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { u"" UNO_NAME_MISC_OBJ_BOUNDRECT,    OWN_ATTR_BOUNDRECT, cppu::UnoType<css::awt::Rectangle>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"" UNO_NAME_MISC_OBJ_NAME,         SDRATTR_OBJECTNAME, cppu::UnoType<OUString>::get(),    0,      0},
        { u"UINameSingular",               OWN_ATTR_UINAME_SINGULAR        , ::cppu::UnoType<OUString>::get(),    css::beans::PropertyAttribute::READONLY,   0},
        { u"" UNO_NAME_MISC_OBJ_TITLE,        OWN_ATTR_MISC_OBJ_TITLE         , cppu::UnoType<OUString>::get(),    0,  0},
        { u"" UNO_NAME_MISC_OBJ_DESCRIPTION,  OWN_ATTR_MISC_OBJ_DESCRIPTION   , cppu::UnoType<OUString>::get(),    0,  0},
        { u"Model",                        OWN_ATTR_OLEMODEL               , cppu::UnoType<css::table::XTable>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"TableTemplate",                OWN_ATTR_TABLETEMPLATE          , cppu::UnoType<css::container::XIndexAccess>::get(), 0, 0},
        { u"UseFirstRowStyle",             OWN_ATTR_TABLETEMPLATE_FIRSTROW, cppu::UnoType<bool>::get(),0, 0},
        { u"UseLastRowStyle",              OWN_ATTR_TABLETEMPLATE_LASTROW, cppu::UnoType<bool>::get(),0, 0},
        { u"UseFirstColumnStyle",          OWN_ATTR_TABLETEMPLATE_FIRSTCOLUMN, cppu::UnoType<bool>::get(),0, 0},
        { u"UseLastColumnStyle",           OWN_ATTR_TABLETEMPLATE_LASTCOLUMN, cppu::UnoType<bool>::get(),0, 0},
        { u"UseBandingRowStyle",           OWN_ATTR_TABLETEMPLATE_BANDINGROWS, cppu::UnoType<bool>::get(),0, 0},
        { u"UseBandingColumnStyle",        OWN_ATTR_TABLETEMPLATE_BANDINGCOLUMNS, cppu::UnoType<bool>::get(),0, 0},
        { u"ReplacementGraphic",           OWN_ATTR_REPLACEMENT_GRAPHIC, cppu::UnoType<css::graphic::XGraphic>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aTableShapePropertyMap_Impl;
}

static comphelper::PropertyMapEntry const * ImplGetSvxDrawingDefaultsPropertyMap()
{
    static comphelper::PropertyMapEntry const aSvxDrawingDefaultsPropertyMap_Impl[] =
    {
        GLOW_PROPERTIES
        SOFTEDGE_PROPERTIES
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
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aSvxDrawingDefaultsPropertyMap_Impl;
}

static comphelper::PropertyMapEntry const * ImplGetAdditionalWriterDrawingDefaultsPropertyMap()
{
    static comphelper::PropertyMapEntry const aSvxAdditionalDefaultsPropertyMap_Impl[] =
    {
        { "IsFollowingTextFlow", SID_SW_FOLLOW_TEXT_FLOW, cppu::UnoType<bool>::get(), 0, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aSvxAdditionalDefaultsPropertyMap_Impl;
}

typedef std::unordered_map< OUString, sal_uInt32 > UHashMapImpl;

namespace {

const UHashMapImpl& GetUHashImpl()
{
    static UHashMapImpl const aImpl
    {
            { "com.sun.star.drawing.RectangleShape",       OBJ_RECT },
            { "com.sun.star.drawing.EllipseShape",         OBJ_CIRC },
            { "com.sun.star.drawing.ControlShape",         OBJ_UNO  },
            { "com.sun.star.drawing.ConnectorShape",       OBJ_EDGE },
            { "com.sun.star.drawing.MeasureShape",         OBJ_MEASURE },
            { "com.sun.star.drawing.LineShape",            OBJ_LINE },
            { "com.sun.star.drawing.PolyPolygonShape",     OBJ_POLY },
            { "com.sun.star.drawing.PolyLineShape",        OBJ_PLIN },
            { "com.sun.star.drawing.OpenBezierShape",      OBJ_PATHLINE },
            { "com.sun.star.drawing.ClosedBezierShape",    OBJ_PATHFILL },
            { "com.sun.star.drawing.OpenFreeHandShape",    OBJ_FREELINE },
            { "com.sun.star.drawing.ClosedFreeHandShape",  OBJ_FREEFILL },
            { "com.sun.star.drawing.PolyPolygonPathShape", OBJ_PATHPOLY },
            { "com.sun.star.drawing.PolyLinePathShape",    OBJ_PATHPLIN },
            { "com.sun.star.drawing.GraphicObjectShape",   OBJ_GRAF },
            { "com.sun.star.drawing.GroupShape",           OBJ_GRUP },
            { "com.sun.star.drawing.TextShape",            OBJ_TEXT },
            { "com.sun.star.drawing.OLE2Shape",            OBJ_OLE2 },
            { "com.sun.star.drawing.PageShape",            OBJ_PAGE },
            { "com.sun.star.drawing.CaptionShape",         OBJ_CAPTION },
            { "com.sun.star.drawing.FrameShape",           OBJ_FRAME },
            { "com.sun.star.drawing.PluginShape",          OBJ_OLE2_PLUGIN },
            { "com.sun.star.drawing.AppletShape",          OBJ_OLE2_APPLET },
            { "com.sun.star.drawing.CustomShape",          OBJ_CUSTOMSHAPE },
            { "com.sun.star.drawing.MediaShape",           OBJ_MEDIA },

            { "com.sun.star.drawing.Shape3DSceneObject",   E3D_SCENE_ID  | E3D_INVENTOR_FLAG },
            { "com.sun.star.drawing.Shape3DCubeObject",    E3D_CUBEOBJ_ID    | E3D_INVENTOR_FLAG },
            { "com.sun.star.drawing.Shape3DSphereObject",  E3D_SPHEREOBJ_ID  | E3D_INVENTOR_FLAG },
            { "com.sun.star.drawing.Shape3DLatheObject",   E3D_LATHEOBJ_ID   | E3D_INVENTOR_FLAG },
            { "com.sun.star.drawing.Shape3DExtrudeObject", E3D_EXTRUDEOBJ_ID | E3D_INVENTOR_FLAG },
            { "com.sun.star.drawing.Shape3DPolygonObject", E3D_POLYGONOBJ_ID | E3D_INVENTOR_FLAG },
    };

    return aImpl;
}

}


OUString UHashMap::getNameFromId(sal_uInt32 nId)
{
    const UHashMapImpl &rMap = GetUHashImpl();

    auto it = std::find_if(rMap.begin(), rMap.end(),
        [nId](const UHashMapImpl::value_type& rEntry) { return rEntry.second == nId; });
    if (it != rMap.end())
        return it->first;
    OSL_FAIL("[CL] unknown SdrObject identifier");
    return OUString();
}

uno::Sequence< OUString > UHashMap::getServiceNames()
{
    return comphelper::mapKeysToSequence( GetUHashImpl() );
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

namespace {

struct theSvxMapProvider :
    public rtl::Static<SvxUnoPropertyMapProvider, theSvxMapProvider>
{
};

}

SvxUnoPropertyMapProvider& getSvxMapProvider()
{
    return theSvxMapProvider::get();
}


SvxUnoPropertyMapProvider::SvxUnoPropertyMapProvider()
{
    for(sal_uInt16 i=0;i<SVXMAP_END; i++)
    {
        aSetArr[i] = nullptr;
        aMapArr[i] = nullptr;
    }
}

SvxUnoPropertyMapProvider::~SvxUnoPropertyMapProvider()
{
}


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
            case SVXMAP_GRAPHICOBJECT: aMapArr[SVXMAP_GRAPHICOBJECT]=ImplGetSvxGraphicObjectPropertyMap(); break;
            case SVXMAP_3DSCENEOBJECT: aMapArr[SVXMAP_3DSCENEOBJECT]=ImplGetSvx3DSceneObjectPropertyMap(); break;
            case SVXMAP_3DCUBEOBJECT: aMapArr[SVXMAP_3DCUBEOBJECT]=ImplGetSvx3DCubeObjectPropertyMap(); break;
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
        aSetArr[nPropertyId].reset(new SvxItemPropertySet( GetMap( nPropertyId ), rPool ));
    return aSetArr[nPropertyId].get();
}

/** maps the vcl MapUnit enum to an API constant MeasureUnit.
    Returns false if conversion is not supported.
*/
bool SvxMapUnitToMeasureUnit( const MapUnit eVcl, short& eApi ) noexcept
{
    switch( eVcl )
    {
    case MapUnit::Map100thMM:      eApi = util::MeasureUnit::MM_100TH;     break;
    case MapUnit::Map10thMM:       eApi = util::MeasureUnit::MM_10TH;      break;
    case MapUnit::MapMM:            eApi = util::MeasureUnit::MM;           break;
    case MapUnit::MapCM:            eApi = util::MeasureUnit::CM;           break;
    case MapUnit::Map1000thInch:   eApi = util::MeasureUnit::INCH_1000TH;  break;
    case MapUnit::Map100thInch:    eApi = util::MeasureUnit::INCH_100TH;   break;
    case MapUnit::Map10thInch:     eApi = util::MeasureUnit::INCH_10TH;    break;
    case MapUnit::MapInch:          eApi = util::MeasureUnit::INCH;         break;
    case MapUnit::MapPoint:         eApi = util::MeasureUnit::POINT;        break;
    case MapUnit::MapTwip:          eApi = util::MeasureUnit::TWIP;         break;
    case MapUnit::MapRelative:      eApi = util::MeasureUnit::PERCENT;      break;
    default:
        return false;
    }

    return true;
}

/** maps the API constant MeasureUnit to a vcl MapUnit enum.
    Returns false if conversion is not supported.
*/

bool SvxMeasureUnitToFieldUnit( const short eApi, FieldUnit& eVcl ) noexcept
{
    switch( eApi )
    {
    case util::MeasureUnit::MM:         eVcl = FieldUnit::MM;        break;
    case util::MeasureUnit::CM:         eVcl = FieldUnit::CM;        break;
    case util::MeasureUnit::M:          eVcl = FieldUnit::M;         break;
    case util::MeasureUnit::KM:         eVcl = FieldUnit::KM;        break;
    case util::MeasureUnit::TWIP:       eVcl = FieldUnit::TWIP;      break;
    case util::MeasureUnit::POINT:      eVcl = FieldUnit::POINT;     break;
    case util::MeasureUnit::PICA:       eVcl = FieldUnit::PICA;      break;
    case util::MeasureUnit::INCH:       eVcl = FieldUnit::INCH;      break;
    case util::MeasureUnit::FOOT:       eVcl = FieldUnit::FOOT;      break;
    case util::MeasureUnit::MILE:       eVcl = FieldUnit::MILE;      break;
    case util::MeasureUnit::PERCENT:    eVcl = FieldUnit::PERCENT;   break;
    case util::MeasureUnit::MM_100TH:   eVcl = FieldUnit::MM_100TH;  break;
    default:
        return false;
    }

    return true;
}

/** maps the vcl MapUnit enum to an API constant MeasureUnit.
    Returns false if conversion is not supported.
*/
bool SvxFieldUnitToMeasureUnit( const FieldUnit eVcl, short& eApi ) noexcept
{
    switch( eVcl )
    {
    case FieldUnit::MM:          eApi = util::MeasureUnit::MM;       break;
    case FieldUnit::CM:          eApi = util::MeasureUnit::CM;       break;
    case FieldUnit::M:           eApi = util::MeasureUnit::M;        break;
    case FieldUnit::KM:          eApi = util::MeasureUnit::KM;       break;
    case FieldUnit::TWIP:        eApi = util::MeasureUnit::TWIP;     break;
    case FieldUnit::POINT:       eApi = util::MeasureUnit::POINT;    break;
    case FieldUnit::PICA:        eApi = util::MeasureUnit::PICA;     break;
    case FieldUnit::INCH:        eApi = util::MeasureUnit::INCH;     break;
    case FieldUnit::FOOT:        eApi = util::MeasureUnit::FOOT;     break;
    case FieldUnit::MILE:        eApi = util::MeasureUnit::MILE;     break;
    case FieldUnit::PERCENT:     eApi = util::MeasureUnit::PERCENT;  break;
    case FieldUnit::MM_100TH:    eApi = util::MeasureUnit::MM_100TH; break;
    default:
        return false;
    }

    return true;
}

static const char* RID_SVXSTR_BMP_DEF[] =
{
    RID_SVXSTR_BMP0_DEF,
    RID_SVXSTR_BMP1_DEF,
    RID_SVXSTR_BMP2_DEF,
    RID_SVXSTR_BMP3_DEF,
    RID_SVXSTR_BMP4_DEF,
    RID_SVXSTR_BMP5_DEF,
    RID_SVXSTR_BMP6_DEF,
    RID_SVXSTR_BMP7_DEF,
    RID_SVXSTR_BMP8_DEF,
    RID_SVXSTR_BMP9_DEF,
    RID_SVXSTR_BMP10_DEF,
    RID_SVXSTR_BMP11_DEF,
    RID_SVXSTR_BMP12_DEF,
    RID_SVXSTR_BMP13_DEF,
    RID_SVXSTR_BMP14_DEF,
    RID_SVXSTR_BMP15_DEF,
    RID_SVXSTR_BMP16_DEF,
    RID_SVXSTR_BMP17_DEF,
    RID_SVXSTR_BMP18_DEF,
    RID_SVXSTR_BMP19_DEF,
    RID_SVXSTR_BMP20_DEF,
    RID_SVXSTR_BMP21_DEF,
    RID_SVXSTR_BMP22_DEF,
    RID_SVXSTR_BMP23_DEF,
    RID_SVXSTR_BMP24_DEF,
    RID_SVXSTR_BMP25_DEF,
    RID_SVXSTR_BMP26_DEF,
    RID_SVXSTR_BMP27_DEF,
    RID_SVXSTR_BMP28_DEF,
    RID_SVXSTR_BMP29_DEF,
    RID_SVXSTR_BMP30_DEF,
    RID_SVXSTR_BMP31_DEF,
    RID_SVXSTR_BMP32_DEF,
    RID_SVXSTR_BMP33_DEF,
    RID_SVXSTR_BMP34_DEF,
    RID_SVXSTR_BMP35_DEF,
    RID_SVXSTR_BMP36_DEF,
    RID_SVXSTR_BMP37_DEF,
    RID_SVXSTR_BMP38_DEF,
    RID_SVXSTR_BMP39_DEF,
    RID_SVXSTR_BMP40_DEF,
    RID_SVXSTR_BMP41_DEF,
    RID_SVXSTR_BMP42_DEF,
    RID_SVXSTR_BMP43_DEF,
    RID_SVXSTR_BMP44_DEF,
    RID_SVXSTR_BMP45_DEF,
    RID_SVXSTR_BMP46_DEF,
    RID_SVXSTR_BMP47_DEF,
    RID_SVXSTR_BMP48_DEF,
    RID_SVXSTR_BMP49_DEF,
    RID_SVXSTR_BMP50_DEF,
    RID_SVXSTR_BMP51_DEF,
    RID_SVXSTR_BMP52_DEF,
    RID_SVXSTR_BMP53_DEF,
    RID_SVXSTR_BMP54_DEF,
    RID_SVXSTR_BMP55_DEF,
    RID_SVXSTR_BMP56_DEF,
    RID_SVXSTR_BMP57_DEF,
    RID_SVXSTR_BMP58_DEF,
    RID_SVXSTR_BMP59_DEF,
    RID_SVXSTR_BMP60_DEF,
    RID_SVXSTR_BMP61_DEF,
    RID_SVXSTR_BMP62_DEF,
    RID_SVXSTR_BMP63_DEF,
    RID_SVXSTR_BMP64_DEF,
    RID_SVXSTR_BMP65_DEF,
    RID_SVXSTR_BMP66_DEF,
    RID_SVXSTR_BMP67_DEF,
    RID_SVXSTR_BMP68_DEF,
    RID_SVXSTR_BMP69_DEF,
    RID_SVXSTR_BMP70_DEF,
    RID_SVXSTR_BMP71_DEF,
    RID_SVXSTR_BMP72_DEF,
    RID_SVXSTR_BMP73_DEF,
    RID_SVXSTR_BMP74_DEF,
    RID_SVXSTR_BMP75_DEF,
    RID_SVXSTR_BMP76_DEF,
    RID_SVXSTR_BMP77_DEF,
    RID_SVXSTR_BMP78_DEF,
    RID_SVXSTR_BMP79_DEF,
    RID_SVXSTR_BMP80_DEF,
    RID_SVXSTR_BMP81_DEF,
    RID_SVXSTR_BMP82_DEF,
    RID_SVXSTR_BMP83_DEF,
    RID_SVXSTR_BMP84_DEF,
    RID_SVXSTR_BMP85_DEF,
    RID_SVXSTR_BMP86_DEF,
    RID_SVXSTR_BMP87_DEF,
    RID_SVXSTR_BMP88_DEF,
    RID_SVXSTR_BMP89_DEF,
    RID_SVXSTR_BMP90_DEF,
    RID_SVXSTR_BMP91_DEF,
    RID_SVXSTR_BMP92_DEF
};

static const char* RID_SVXSTR_BMP[] =
{
    RID_SVXSTR_BMP0,
    RID_SVXSTR_BMP1,
    RID_SVXSTR_BMP2,
    RID_SVXSTR_BMP3,
    RID_SVXSTR_BMP4,
    RID_SVXSTR_BMP5,
    RID_SVXSTR_BMP6,
    RID_SVXSTR_BMP7,
    RID_SVXSTR_BMP8,
    RID_SVXSTR_BMP9,
    RID_SVXSTR_BMP10,
    RID_SVXSTR_BMP11,
    RID_SVXSTR_BMP12,
    RID_SVXSTR_BMP13,
    RID_SVXSTR_BMP14,
    RID_SVXSTR_BMP15,
    RID_SVXSTR_BMP16,
    RID_SVXSTR_BMP17,
    RID_SVXSTR_BMP18,
    RID_SVXSTR_BMP19,
    RID_SVXSTR_BMP20,
    RID_SVXSTR_BMP21,
    RID_SVXSTR_BMP22,
    RID_SVXSTR_BMP23,
    RID_SVXSTR_BMP24,
    RID_SVXSTR_BMP25,
    RID_SVXSTR_BMP26,
    RID_SVXSTR_BMP27,
    RID_SVXSTR_BMP28,
    RID_SVXSTR_BMP29,
    RID_SVXSTR_BMP30,
    RID_SVXSTR_BMP31,
    RID_SVXSTR_BMP32,
    RID_SVXSTR_BMP33,
    RID_SVXSTR_BMP34,
    RID_SVXSTR_BMP35,
    RID_SVXSTR_BMP36,
    RID_SVXSTR_BMP37,
    RID_SVXSTR_BMP38,
    RID_SVXSTR_BMP39,
    RID_SVXSTR_BMP40,
    RID_SVXSTR_BMP41,
    RID_SVXSTR_BMP42,
    RID_SVXSTR_BMP43,
    RID_SVXSTR_BMP44,
    RID_SVXSTR_BMP45,
    RID_SVXSTR_BMP46,
    RID_SVXSTR_BMP47,
    RID_SVXSTR_BMP48,
    RID_SVXSTR_BMP49,
    RID_SVXSTR_BMP50,
    RID_SVXSTR_BMP51,
    RID_SVXSTR_BMP52,
    RID_SVXSTR_BMP53,
    RID_SVXSTR_BMP54,
    RID_SVXSTR_BMP55,
    RID_SVXSTR_BMP56,
    RID_SVXSTR_BMP57,
    RID_SVXSTR_BMP58,
    RID_SVXSTR_BMP59,
    RID_SVXSTR_BMP60,
    RID_SVXSTR_BMP61,
    RID_SVXSTR_BMP62,
    RID_SVXSTR_BMP63,
    RID_SVXSTR_BMP64,
    RID_SVXSTR_BMP65,
    RID_SVXSTR_BMP66,
    RID_SVXSTR_BMP67,
    RID_SVXSTR_BMP68,
    RID_SVXSTR_BMP69,
    RID_SVXSTR_BMP70,
    RID_SVXSTR_BMP71,
    RID_SVXSTR_BMP72,
    RID_SVXSTR_BMP73,
    RID_SVXSTR_BMP74,
    RID_SVXSTR_BMP75,
    RID_SVXSTR_BMP76,
    RID_SVXSTR_BMP77,
    RID_SVXSTR_BMP78,
    RID_SVXSTR_BMP79,
    RID_SVXSTR_BMP80,
    RID_SVXSTR_BMP81,
    RID_SVXSTR_BMP82,
    RID_SVXSTR_BMP83,
    RID_SVXSTR_BMP84,
    RID_SVXSTR_BMP85,
    RID_SVXSTR_BMP86,
    RID_SVXSTR_BMP87,
    RID_SVXSTR_BMP88,
    RID_SVXSTR_BMP89,
    RID_SVXSTR_BMP90,
    RID_SVXSTR_BMP91,
    RID_SVXSTR_BMP92
};

static const char* RID_SVXSTR_DASH_DEF[] =
{
    RID_SVXSTR_DASH0_DEF,
    RID_SVXSTR_DASH1_DEF,
    RID_SVXSTR_DASH2_DEF,
    RID_SVXSTR_DASH3_DEF,
    RID_SVXSTR_DASH4_DEF,
    RID_SVXSTR_DASH5_DEF,
    RID_SVXSTR_DASH6_DEF,
    RID_SVXSTR_DASH7_DEF,
    RID_SVXSTR_DASH8_DEF,
    RID_SVXSTR_DASH9_DEF,
    RID_SVXSTR_DASH10_DEF,
    RID_SVXSTR_DASH11_DEF,
    RID_SVXSTR_DASH12_DEF,
    RID_SVXSTR_DASH13_DEF,
    RID_SVXSTR_DASH14_DEF,
    RID_SVXSTR_DASH15_DEF,
    RID_SVXSTR_DASH16_DEF,
    RID_SVXSTR_DASH17_DEF,
    RID_SVXSTR_DASH18_DEF,
    RID_SVXSTR_DASH19_DEF,
    RID_SVXSTR_DASH20_DEF,
    RID_SVXSTR_DASH21_DEF
};

static const char* RID_SVXSTR_DASH[] =
{
    RID_SVXSTR_DASH0,
    RID_SVXSTR_DASH1,
    RID_SVXSTR_DASH2,
    RID_SVXSTR_DASH3,
    RID_SVXSTR_DASH4,
    RID_SVXSTR_DASH5,
    RID_SVXSTR_DASH6,
    RID_SVXSTR_DASH7,
    RID_SVXSTR_DASH8,
    RID_SVXSTR_DASH9,
    RID_SVXSTR_DASH10,
    RID_SVXSTR_DASH11,
    RID_SVXSTR_DASH12,
    RID_SVXSTR_DASH13,
    RID_SVXSTR_DASH14,
    RID_SVXSTR_DASH15,
    RID_SVXSTR_DASH16,
    RID_SVXSTR_DASH17,
    RID_SVXSTR_DASH18,
    RID_SVXSTR_DASH19,
    RID_SVXSTR_DASH20,
    RID_SVXSTR_DASH21
};

static const char* RID_SVXSTR_LEND_DEF[] =
{
    RID_SVXSTR_LEND0_DEF,
    RID_SVXSTR_LEND1_DEF,
    RID_SVXSTR_LEND2_DEF,
    RID_SVXSTR_LEND3_DEF,
    RID_SVXSTR_LEND4_DEF,
    RID_SVXSTR_LEND5_DEF,
    RID_SVXSTR_LEND6_DEF,
    RID_SVXSTR_LEND7_DEF,
    RID_SVXSTR_LEND8_DEF,
    RID_SVXSTR_LEND9_DEF,
    RID_SVXSTR_LEND10_DEF,
    RID_SVXSTR_LEND11_DEF,
    RID_SVXSTR_LEND12_DEF,
    RID_SVXSTR_LEND13_DEF,
    RID_SVXSTR_LEND14_DEF,
    RID_SVXSTR_LEND15_DEF,
    RID_SVXSTR_LEND16_DEF,
    RID_SVXSTR_LEND17_DEF,
    RID_SVXSTR_LEND18_DEF,
    RID_SVXSTR_LEND19_DEF,
    RID_SVXSTR_LEND20_DEF,
    RID_SVXSTR_LEND21_DEF,
    RID_SVXSTR_LEND22_DEF,
    RID_SVXSTR_LEND23_DEF,
    RID_SVXSTR_LEND24_DEF,
    RID_SVXSTR_LEND25_DEF,
    RID_SVXSTR_LEND26_DEF,
    RID_SVXSTR_LEND27_DEF,
    RID_SVXSTR_LEND28_DEF,
    RID_SVXSTR_LEND29_DEF,
    RID_SVXSTR_LEND30_DEF,
    RID_SVXSTR_LEND31_DEF
};

static const char* RID_SVXSTR_LEND[] =
{
    RID_SVXSTR_LEND0,
    RID_SVXSTR_LEND1,
    RID_SVXSTR_LEND2,
    RID_SVXSTR_LEND3,
    RID_SVXSTR_LEND4,
    RID_SVXSTR_LEND5,
    RID_SVXSTR_LEND6,
    RID_SVXSTR_LEND7,
    RID_SVXSTR_LEND8,
    RID_SVXSTR_LEND9,
    RID_SVXSTR_LEND10,
    RID_SVXSTR_LEND11,
    RID_SVXSTR_LEND12,
    RID_SVXSTR_LEND13,
    RID_SVXSTR_LEND14,
    RID_SVXSTR_LEND15,
    RID_SVXSTR_LEND16,
    RID_SVXSTR_LEND17,
    RID_SVXSTR_LEND18,
    RID_SVXSTR_LEND19,
    RID_SVXSTR_LEND20,
    RID_SVXSTR_LEND21,
    RID_SVXSTR_LEND22,
    RID_SVXSTR_LEND23,
    RID_SVXSTR_LEND24,
    RID_SVXSTR_LEND25,
    RID_SVXSTR_LEND26,
    RID_SVXSTR_LEND27,
    RID_SVXSTR_LEND28,
    RID_SVXSTR_LEND29,
    RID_SVXSTR_LEND30,
    RID_SVXSTR_LEND31
};

static const char* RID_SVXSTR_GRDT_DEF[] =
{
    RID_SVXSTR_GRDT0_DEF,
    RID_SVXSTR_GRDT1_DEF,
    RID_SVXSTR_GRDT2_DEF,
    RID_SVXSTR_GRDT3_DEF,
    RID_SVXSTR_GRDT4_DEF,
    RID_SVXSTR_GRDT5_DEF,
    RID_SVXSTR_GRDT6_DEF,
    RID_SVXSTR_GRDT7_DEF,
    RID_SVXSTR_GRDT8_DEF,
    RID_SVXSTR_GRDT9_DEF,
    RID_SVXSTR_GRDT10_DEF,
    RID_SVXSTR_GRDT11_DEF,
    RID_SVXSTR_GRDT12_DEF,
    RID_SVXSTR_GRDT13_DEF,
    RID_SVXSTR_GRDT14_DEF,
    RID_SVXSTR_GRDT15_DEF,
    RID_SVXSTR_GRDT16_DEF,
    RID_SVXSTR_GRDT17_DEF,
    RID_SVXSTR_GRDT18_DEF,
    RID_SVXSTR_GRDT19_DEF,
    RID_SVXSTR_GRDT20_DEF,
    RID_SVXSTR_GRDT21_DEF,
    RID_SVXSTR_GRDT22_DEF,
    RID_SVXSTR_GRDT23_DEF,
    RID_SVXSTR_GRDT24_DEF,
    RID_SVXSTR_GRDT25_DEF,
    RID_SVXSTR_GRDT26_DEF,
    RID_SVXSTR_GRDT27_DEF,
    RID_SVXSTR_GRDT28_DEF,
    RID_SVXSTR_GRDT29_DEF,
    RID_SVXSTR_GRDT30_DEF,
    RID_SVXSTR_GRDT31_DEF,
    RID_SVXSTR_GRDT32_DEF,
    RID_SVXSTR_GRDT33_DEF,
    RID_SVXSTR_GRDT34_DEF,
    RID_SVXSTR_GRDT35_DEF,
    RID_SVXSTR_GRDT36_DEF,
    RID_SVXSTR_GRDT37_DEF,
    RID_SVXSTR_GRDT38_DEF,
    RID_SVXSTR_GRDT39_DEF,
    RID_SVXSTR_GRDT40_DEF,
    RID_SVXSTR_GRDT41_DEF,
    RID_SVXSTR_GRDT42_DEF,
    RID_SVXSTR_GRDT43_DEF,
    RID_SVXSTR_GRDT44_DEF,
    RID_SVXSTR_GRDT45_DEF,
    RID_SVXSTR_GRDT46_DEF,
    RID_SVXSTR_GRDT47_DEF,
    RID_SVXSTR_GRDT48_DEF,
    RID_SVXSTR_GRDT49_DEF,
    RID_SVXSTR_GRDT50_DEF,
    RID_SVXSTR_GRDT51_DEF,
    RID_SVXSTR_GRDT52_DEF,
    RID_SVXSTR_GRDT53_DEF,
    RID_SVXSTR_GRDT54_DEF,
    RID_SVXSTR_GRDT55_DEF,
    RID_SVXSTR_GRDT56_DEF,
    RID_SVXSTR_GRDT57_DEF,
    RID_SVXSTR_GRDT58_DEF,
    RID_SVXSTR_GRDT59_DEF,
    RID_SVXSTR_GRDT60_DEF,
    RID_SVXSTR_GRDT61_DEF,
    RID_SVXSTR_GRDT62_DEF,
    RID_SVXSTR_GRDT63_DEF,
    RID_SVXSTR_GRDT64_DEF,
    RID_SVXSTR_GRDT65_DEF,
    RID_SVXSTR_GRDT66_DEF,
    RID_SVXSTR_GRDT67_DEF,
    RID_SVXSTR_GRDT68_DEF,
    RID_SVXSTR_GRDT69_DEF,
    RID_SVXSTR_GRDT70_DEF,
    RID_SVXSTR_GRDT71_DEF,
    RID_SVXSTR_GRDT72_DEF,
    RID_SVXSTR_GRDT73_DEF,
    RID_SVXSTR_GRDT74_DEF,
    RID_SVXSTR_GRDT75_DEF,
    RID_SVXSTR_GRDT76_DEF,
    RID_SVXSTR_GRDT77_DEF,
    RID_SVXSTR_GRDT78_DEF,
    RID_SVXSTR_GRDT79_DEF,
    RID_SVXSTR_GRDT80_DEF,
    RID_SVXSTR_GRDT81_DEF,
    RID_SVXSTR_GRDT82_DEF,
    RID_SVXSTR_GRDT83_DEF,
    RID_SVXSTR_GRDT84_DEF
};

static const char* RID_SVXSTR_GRDT[] =
{
    RID_SVXSTR_GRDT0,
    RID_SVXSTR_GRDT1,
    RID_SVXSTR_GRDT2,
    RID_SVXSTR_GRDT3,
    RID_SVXSTR_GRDT4,
    RID_SVXSTR_GRDT5,
    RID_SVXSTR_GRDT6,
    RID_SVXSTR_GRDT7,
    RID_SVXSTR_GRDT8,
    RID_SVXSTR_GRDT9,
    RID_SVXSTR_GRDT10,
    RID_SVXSTR_GRDT11,
    RID_SVXSTR_GRDT12,
    RID_SVXSTR_GRDT13,
    RID_SVXSTR_GRDT14,
    RID_SVXSTR_GRDT15,
    RID_SVXSTR_GRDT16,
    RID_SVXSTR_GRDT17,
    RID_SVXSTR_GRDT18,
    RID_SVXSTR_GRDT19,
    RID_SVXSTR_GRDT20,
    RID_SVXSTR_GRDT21,
    RID_SVXSTR_GRDT22,
    RID_SVXSTR_GRDT23,
    RID_SVXSTR_GRDT24,
    RID_SVXSTR_GRDT25,
    RID_SVXSTR_GRDT26,
    RID_SVXSTR_GRDT27,
    RID_SVXSTR_GRDT28,
    RID_SVXSTR_GRDT29,
    RID_SVXSTR_GRDT30,
    RID_SVXSTR_GRDT31,
    RID_SVXSTR_GRDT32,
    RID_SVXSTR_GRDT33,
    RID_SVXSTR_GRDT34,
    RID_SVXSTR_GRDT35,
    RID_SVXSTR_GRDT36,
    RID_SVXSTR_GRDT37,
    RID_SVXSTR_GRDT38,
    RID_SVXSTR_GRDT39,
    RID_SVXSTR_GRDT40,
    RID_SVXSTR_GRDT41,
    RID_SVXSTR_GRDT42,
    RID_SVXSTR_GRDT43,
    RID_SVXSTR_GRDT44,
    RID_SVXSTR_GRDT45,
    RID_SVXSTR_GRDT46,
    RID_SVXSTR_GRDT47,
    RID_SVXSTR_GRDT48,
    RID_SVXSTR_GRDT49,
    RID_SVXSTR_GRDT50,
    RID_SVXSTR_GRDT51,
    RID_SVXSTR_GRDT52,
    RID_SVXSTR_GRDT53,
    RID_SVXSTR_GRDT54,
    RID_SVXSTR_GRDT55,
    RID_SVXSTR_GRDT56,
    RID_SVXSTR_GRDT57,
    RID_SVXSTR_GRDT58,
    RID_SVXSTR_GRDT59,
    RID_SVXSTR_GRDT60,
    RID_SVXSTR_GRDT61,
    RID_SVXSTR_GRDT62,
    RID_SVXSTR_GRDT63,
    RID_SVXSTR_GRDT64,
    RID_SVXSTR_GRDT65,
    RID_SVXSTR_GRDT66,
    RID_SVXSTR_GRDT67,
    RID_SVXSTR_GRDT68,
    RID_SVXSTR_GRDT69,
    RID_SVXSTR_GRDT70,
    RID_SVXSTR_GRDT71,
    RID_SVXSTR_GRDT72,
    RID_SVXSTR_GRDT73,
    RID_SVXSTR_GRDT74,
    RID_SVXSTR_GRDT75,
    RID_SVXSTR_GRDT76,
    RID_SVXSTR_GRDT77,
    RID_SVXSTR_GRDT78,
    RID_SVXSTR_GRDT79,
    RID_SVXSTR_GRDT80,
    RID_SVXSTR_GRDT81,
    RID_SVXSTR_GRDT82,
    RID_SVXSTR_GRDT83,
    RID_SVXSTR_GRDT84
};

static const char* RID_SVXSTR_HATCHS_DEF[] =
{
    RID_SVXSTR_HATCH0_DEF,
    RID_SVXSTR_HATCH1_DEF,
    RID_SVXSTR_HATCH2_DEF,
    RID_SVXSTR_HATCH3_DEF,
    RID_SVXSTR_HATCH4_DEF,
    RID_SVXSTR_HATCH5_DEF,
    RID_SVXSTR_HATCH6_DEF,
    RID_SVXSTR_HATCH7_DEF,
    RID_SVXSTR_HATCH8_DEF,
    RID_SVXSTR_HATCH9_DEF,
    RID_SVXSTR_HATCH10_DEF,
    RID_SVXSTR_HATCH11_DEF,
    RID_SVXSTR_HATCH12_DEF,
    RID_SVXSTR_HATCH13_DEF,
    RID_SVXSTR_HATCH14_DEF,
    RID_SVXSTR_HATCH15_DEF
};

static const char* RID_SVXSTR_HATCHS[] =
{
    RID_SVXSTR_HATCH0,
    RID_SVXSTR_HATCH1,
    RID_SVXSTR_HATCH2,
    RID_SVXSTR_HATCH3,
    RID_SVXSTR_HATCH4,
    RID_SVXSTR_HATCH5,
    RID_SVXSTR_HATCH6,
    RID_SVXSTR_HATCH7,
    RID_SVXSTR_HATCH8,
    RID_SVXSTR_HATCH9,
    RID_SVXSTR_HATCH10,
    RID_SVXSTR_HATCH11,
    RID_SVXSTR_HATCH12,
    RID_SVXSTR_HATCH13,
    RID_SVXSTR_HATCH14,
    RID_SVXSTR_HATCH15
};

static const char* RID_SVXSTR_TRASNGR_DEF[] =
{
    RID_SVXSTR_TRASNGR0_DEF
};

static const char* RID_SVXSTR_TRASNGR[] =
{
    RID_SVXSTR_TRASNGR0
};

static bool SvxUnoGetResourceRanges( const sal_uInt16 nWhich, const char**& pApiResIds, const char**& pIntResIds, int& nCount ) noexcept
{
    switch( nWhich )
    {
    case XATTR_FILLBITMAP:
        pApiResIds = RID_SVXSTR_BMP_DEF;
        pIntResIds = RID_SVXSTR_BMP;
        nCount = SAL_N_ELEMENTS(RID_SVXSTR_BMP_DEF);
        break;
    case XATTR_LINEDASH:
        pApiResIds = RID_SVXSTR_DASH_DEF;
        pIntResIds = RID_SVXSTR_DASH;
        nCount = SAL_N_ELEMENTS(RID_SVXSTR_DASH_DEF);
        break;

    case XATTR_LINESTART:
    case XATTR_LINEEND:
        pApiResIds = RID_SVXSTR_LEND_DEF;
        pIntResIds = RID_SVXSTR_LEND;
        nCount = SAL_N_ELEMENTS(RID_SVXSTR_LEND_DEF);
        break;

    case XATTR_FILLGRADIENT:
        pApiResIds = RID_SVXSTR_GRDT_DEF;
        pIntResIds = RID_SVXSTR_GRDT;
        nCount = SAL_N_ELEMENTS(RID_SVXSTR_GRDT_DEF);
        break;

    case XATTR_FILLHATCH:
        pApiResIds = RID_SVXSTR_HATCHS_DEF;
        pIntResIds = RID_SVXSTR_HATCHS;
        nCount = SAL_N_ELEMENTS(RID_SVXSTR_HATCHS_DEF);
        break;

    case XATTR_FILLFLOATTRANSPARENCE:
        pApiResIds = RID_SVXSTR_TRASNGR_DEF;
        pIntResIds = RID_SVXSTR_TRASNGR;
        nCount = SAL_N_ELEMENTS(RID_SVXSTR_TRASNGR_DEF);
        break;

    default:
        return false;
    }
    return true;
}

/// @throws std::exception
static bool SvxUnoConvertResourceString(const char **pSourceResIds, const char** pDestResIds, int nCount, OUString& rString, bool bToApi)
{
    // first, calculate the search string length without an optional number after the name
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

    for (int i = 0; i < nCount; ++i)
    {
        if (bToApi)
        {
            const OUString & aCompare = SvxResId(pSourceResIds[i]);
            if( aShortString == aCompare )
            {
                rString = rString.replaceAt( 0, aShortString.getLength(), OUString::createFromAscii(pDestResIds[i]) );
                return true;
            }
            else if( rString == aCompare )
            {
                rString = OUString::createFromAscii(pDestResIds[i]);
                return true;
            }
        }
        else
        {
            auto pCompare = pSourceResIds[i];
            if( aShortString.equalsAscii(pCompare) )
            {
                rString = rString.replaceAt( 0, aShortString.getLength(), SvxResId(pDestResIds[i]) );
                return true;
            }
            else if( rString.equalsAscii(pCompare) )
            {
                rString = SvxResId(pDestResIds[i]);
                return true;
            }
        }
    }

    return false;
}


// #i122649# Some comments on the below arrays:
// - They need to have the same order and count of items
// - They are used to translate between translated and non-translated color names
// - To make longer names be found which start with the same basic string,
//   these have to be in front of others

// It would be nice to:
// - evtl. organize these in a single array with 2-dimensional inner to eliminate
//   the possibility to define it wrong
// - change the compare to also work when a shorter name is in front of a longer one

static const char* SvxUnoColorNameDefResId[] =
{
    RID_SVXSTR_COLOR_BLUEGREY_DEF,
    RID_SVXSTR_COLOR_BLACK_DEF,
    RID_SVXSTR_COLOR_BLUE_CLASSIC_DEF,
    RID_SVXSTR_COLOR_BLUE_DEF,
    RID_SVXSTR_COLOR_GREEN_DEF,
    RID_SVXSTR_COLOR_RED_DEF,
    RID_SVXSTR_COLOR_MAGENTA_DEF,
    RID_SVXSTR_COLOR_GREY_DEF,
    RID_SVXSTR_COLOR_YELLOWGREEN_DEF,
    RID_SVXSTR_COLOR_YELLOW_DEF,
    RID_SVXSTR_COLOR_WHITE_DEF,
    RID_SVXSTR_COLOR_ORANGE_DEF,
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
    RID_SVXSTR_COLOR_GOLD_DEF,
    RID_SVXSTR_COLOR_BRICK_DEF,
    RID_SVXSTR_COLOR_INDIGO_DEF,
    RID_SVXSTR_COLOR_TEAL_DEF,
    RID_SVXSTR_COLOR_LIME_DEF,
    RID_SVXSTR_COLOR_LIGHTGRAY_DEF,
    RID_SVXSTR_COLOR_LIGHTYELLOW_DEF,
    RID_SVXSTR_COLOR_LIGHTGOLD_DEF,
    RID_SVXSTR_COLOR_LIGHTORANGE_DEF,
    RID_SVXSTR_COLOR_LIGHTBRICK_DEF,
    RID_SVXSTR_COLOR_LIGHTRED_DEF,
    RID_SVXSTR_COLOR_LIGHTMAGENTA_DEF,
    RID_SVXSTR_COLOR_LIGHTPURPLE_DEF,
    RID_SVXSTR_COLOR_LIGHTINDIGO_DEF,
    RID_SVXSTR_COLOR_LIGHTBLUE_DEF,
    RID_SVXSTR_COLOR_LIGHTTEAL_DEF,
    RID_SVXSTR_COLOR_LIGHTGREEN_DEF,
    RID_SVXSTR_COLOR_LIGHTLIME_DEF,
    RID_SVXSTR_COLOR_DARKGRAY_DEF,
    RID_SVXSTR_COLOR_DARKYELLOW_DEF,
    RID_SVXSTR_COLOR_DARKGOLD_DEF,
    RID_SVXSTR_COLOR_DARKORANGE_DEF,
    RID_SVXSTR_COLOR_DARKBRICK_DEF,
    RID_SVXSTR_COLOR_DARKRED_DEF,
    RID_SVXSTR_COLOR_DARKMAGENTA_DEF,
    RID_SVXSTR_COLOR_DARKPURPLE_DEF,
    RID_SVXSTR_COLOR_DARKINDIGO_DEF,
    RID_SVXSTR_COLOR_DARKBLUE_DEF,
    RID_SVXSTR_COLOR_DARKTEAL_DEF,
    RID_SVXSTR_COLOR_DARKGREEN_DEF,
    RID_SVXSTR_COLOR_DARKLIME_DEF,
    RID_SVXSTR_COLOR_VIOLET_DEF,
    RID_SVXSTR_COLOR_VIOLET_OUG_DEF,
    RID_SVXSTR_COLOR_BLUE_OUG_DEF,
    RID_SVXSTR_COLOR_AZURE_OUG_DEF,
    RID_SVXSTR_COLOR_SPRINGGREEN_OUG_DEF,
    RID_SVXSTR_COLOR_GREEN_OUG_DEF,
    RID_SVXSTR_COLOR_CHARTREUSEGREEN_OUG_DEF,
    RID_SVXSTR_COLOR_ORANGE_OUG_DEF,
    RID_SVXSTR_COLOR_RED_OUG_DEF,
    RID_SVXSTR_COLOR_ROSE_OUG_DEF,
    RID_SVXSTR_COLOR_AZURE_DEF,
    RID_SVXSTR_COLOR_CYAN_DEF,
    RID_SVXSTR_COLOR_SPRINGGREEN_DEF,
    RID_SVXSTR_COLOR_CHARTREUSEGREEN_DEF,
    RID_SVXSTR_COLOR_ROSE_DEF,
    RID_SVXSTR_COLOR_MATERIAL_GRAY_A_DEF,
    RID_SVXSTR_COLOR_MATERIAL_YELLOW_A_DEF,
    RID_SVXSTR_COLOR_MATERIAL_AMBER_A_DEF,
    RID_SVXSTR_COLOR_MATERIAL_AMBER_DEF,
    RID_SVXSTR_COLOR_MATERIAL_ORANGE_A_DEF,
    RID_SVXSTR_COLOR_MATERIAL_DEEP_ORANGE_A_DEF,
    RID_SVXSTR_COLOR_MATERIAL_DEEP_ORANGE_DEF,
    RID_SVXSTR_COLOR_MATERIAL_RED_A_DEF,
    RID_SVXSTR_COLOR_MATERIAL_PINK_A_DEF,
    RID_SVXSTR_COLOR_MATERIAL_PURPLE_A_DEF,
    RID_SVXSTR_COLOR_MATERIAL_DEEP_PURPLE_A_DEF,
    RID_SVXSTR_COLOR_MATERIAL_DEEP_PURPLE_DEF,
    RID_SVXSTR_COLOR_MATERIAL_INDIGO_A_DEF,
    RID_SVXSTR_COLOR_MATERIAL_BLUE_A_DEF,
    RID_SVXSTR_COLOR_MATERIAL_LIGHT_BLUE_A_DEF,
    RID_SVXSTR_COLOR_MATERIAL_CYAN_A_DEF,
    RID_SVXSTR_COLOR_MATERIAL_TEAL_A_DEF,
    RID_SVXSTR_COLOR_MATERIAL_GREEN_A_DEF,
    RID_SVXSTR_COLOR_MATERIAL_LIGHT_GREEN_A_DEF,
    RID_SVXSTR_COLOR_MATERIAL_LIME_A_DEF,
    RID_SVXSTR_COLOR_MATERIAL_BROWN_A_DEF,
    RID_SVXSTR_COLOR_MATERIAL_BROWN_DEF,
    RID_SVXSTR_COLOR_MATERIAL_BLUE_GRAY_A_DEF,
    RID_SVXSTR_COLOR_MATERIAL_BLUE_GRAY_DEF,
    RID_SVXSTR_COLOR_LIBRE_GREEN_1_DEF,
    RID_SVXSTR_COLOR_LIBRE_GREEN_ACCENT_DEF,
    RID_SVXSTR_COLOR_LIBRE_BLUE_ACCENT_DEF,
    RID_SVXSTR_COLOR_LIBRE_ORANGE_ACCENT_DEF,
    RID_SVXSTR_COLOR_LIBRE_PURPLE_DEF,
    RID_SVXSTR_COLOR_LIBRE_PURPLE_ACCENT_DEF,
    RID_SVXSTR_COLOR_LIBRE_YELLOW_ACCENT_DEF
};

static const char* SvxUnoColorNameResId[] =
{
    RID_SVXSTR_COLOR_BLUEGREY,
    RID_SVXSTR_COLOR_BLACK,
    RID_SVXSTR_COLOR_BLUE_CLASSIC,
    RID_SVXSTR_COLOR_BLUE,
    RID_SVXSTR_COLOR_GREEN,
    RID_SVXSTR_COLOR_RED,
    RID_SVXSTR_COLOR_MAGENTA,
    RID_SVXSTR_COLOR_GREY,
    RID_SVXSTR_COLOR_YELLOWGREEN,
    RID_SVXSTR_COLOR_YELLOW,
    RID_SVXSTR_COLOR_WHITE,
    RID_SVXSTR_COLOR_ORANGE,
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
    RID_SVXSTR_COLOR_GOLD,
    RID_SVXSTR_COLOR_BRICK,
    RID_SVXSTR_COLOR_INDIGO,
    RID_SVXSTR_COLOR_TEAL,
    RID_SVXSTR_COLOR_LIME,
    RID_SVXSTR_COLOR_LIGHTGRAY,
    RID_SVXSTR_COLOR_LIGHTYELLOW,
    RID_SVXSTR_COLOR_LIGHTGOLD,
    RID_SVXSTR_COLOR_LIGHTORANGE,
    RID_SVXSTR_COLOR_LIGHTBRICK,
    RID_SVXSTR_COLOR_LIGHTRED,
    RID_SVXSTR_COLOR_LIGHTMAGENTA,
    RID_SVXSTR_COLOR_LIGHTPURPLE,
    RID_SVXSTR_COLOR_LIGHTINDIGO,
    RID_SVXSTR_COLOR_LIGHTBLUE,
    RID_SVXSTR_COLOR_LIGHTTEAL,
    RID_SVXSTR_COLOR_LIGHTGREEN,
    RID_SVXSTR_COLOR_LIGHTLIME,
    RID_SVXSTR_COLOR_DARKGRAY,
    RID_SVXSTR_COLOR_DARKYELLOW,
    RID_SVXSTR_COLOR_DARKGOLD,
    RID_SVXSTR_COLOR_DARKORANGE,
    RID_SVXSTR_COLOR_DARKBRICK,
    RID_SVXSTR_COLOR_DARKRED,
    RID_SVXSTR_COLOR_DARKMAGENTA,
    RID_SVXSTR_COLOR_DARKPURPLE,
    RID_SVXSTR_COLOR_DARKINDIGO,
    RID_SVXSTR_COLOR_DARKBLUE,
    RID_SVXSTR_COLOR_DARKTEAL,
    RID_SVXSTR_COLOR_DARKGREEN,
    RID_SVXSTR_COLOR_DARKLIME,
    RID_SVXSTR_COLOR_VIOLET,
    RID_SVXSTR_COLOR_VIOLET_OUG,
    RID_SVXSTR_COLOR_BLUE_OUG,
    RID_SVXSTR_COLOR_AZURE_OUG,
    RID_SVXSTR_COLOR_SPRINGGREEN_OUG,
    RID_SVXSTR_COLOR_GREEN_OUG,
    RID_SVXSTR_COLOR_CHARTREUSEGREEN_OUG,
    RID_SVXSTR_COLOR_ORANGE_OUG,
    RID_SVXSTR_COLOR_RED_OUG,
    RID_SVXSTR_COLOR_ROSE_OUG,
    RID_SVXSTR_COLOR_AZURE,
    RID_SVXSTR_COLOR_CYAN,
    RID_SVXSTR_COLOR_SPRINGGREEN,
    RID_SVXSTR_COLOR_CHARTREUSEGREEN,
    RID_SVXSTR_COLOR_ROSE,
    RID_SVXSTR_COLOR_MATERIAL_GRAY_A,
    RID_SVXSTR_COLOR_MATERIAL_YELLOW_A,
    RID_SVXSTR_COLOR_MATERIAL_AMBER_A,
    RID_SVXSTR_COLOR_MATERIAL_AMBER,
    RID_SVXSTR_COLOR_MATERIAL_ORANGE_A,
    RID_SVXSTR_COLOR_MATERIAL_DEEP_ORANGE_A,
    RID_SVXSTR_COLOR_MATERIAL_DEEP_ORANGE,
    RID_SVXSTR_COLOR_MATERIAL_RED_A,
    RID_SVXSTR_COLOR_MATERIAL_PINK_A,
    RID_SVXSTR_COLOR_MATERIAL_PURPLE_A,
    RID_SVXSTR_COLOR_MATERIAL_DEEP_PURPLE_A,
    RID_SVXSTR_COLOR_MATERIAL_DEEP_PURPLE,
    RID_SVXSTR_COLOR_MATERIAL_INDIGO_A,
    RID_SVXSTR_COLOR_MATERIAL_BLUE_A,
    RID_SVXSTR_COLOR_MATERIAL_LIGHT_BLUE_A,
    RID_SVXSTR_COLOR_MATERIAL_CYAN_A,
    RID_SVXSTR_COLOR_MATERIAL_TEAL_A,
    RID_SVXSTR_COLOR_MATERIAL_GREEN_A,
    RID_SVXSTR_COLOR_MATERIAL_LIGHT_GREEN_A,
    RID_SVXSTR_COLOR_MATERIAL_LIME_A,
    RID_SVXSTR_COLOR_MATERIAL_BROWN_A,
    RID_SVXSTR_COLOR_MATERIAL_BROWN,
    RID_SVXSTR_COLOR_MATERIAL_BLUE_GRAY_A,
    RID_SVXSTR_COLOR_MATERIAL_BLUE_GRAY,
    RID_SVXSTR_COLOR_LIBRE_GREEN_1,
    RID_SVXSTR_COLOR_LIBRE_GREEN_ACCENT,
    RID_SVXSTR_COLOR_LIBRE_BLUE_ACCENT,
    RID_SVXSTR_COLOR_LIBRE_ORANGE_ACCENT,
    RID_SVXSTR_COLOR_LIBRE_PURPLE,
    RID_SVXSTR_COLOR_LIBRE_PURPLE_ACCENT,
    RID_SVXSTR_COLOR_LIBRE_YELLOW_ACCENT
};

/// @throws std::exception
static bool SvxUnoConvertResourceStringBuiltIn(const char** pSourceResIds, const char** pDestResIds, int nCount, OUString& rString, bool bToApi)
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
        if (bToApi)
        {
            OUString aStrDefName = SvxResId(pSourceResIds[i]);
            if( sStr == aStrDefName )
            {
                OUString aReplace = OUString::createFromAscii(pDestResIds[i]);
                rString = rString.replaceAt( 0, aStrDefName.getLength(), aReplace );
                return true;
            }
        }
        else
        {
            if( sStr.equalsAscii(pSourceResIds[i]) )
            {
                OUString aReplace = SvxResId(pDestResIds[i]);
                rString = rString.replaceAt( 0, strlen(pSourceResIds[i]), aReplace );
                return true;
            }
        }
    }

    return false;
}

/** if the given name is a predefined name for the current language it is replaced by
    the corresponding api name.
*/
OUString SvxUnogetApiNameForItem(const sal_uInt16 nWhich, const OUString& rInternalName)
{
    OUString aNew = rInternalName;

    if( nWhich == sal_uInt16(XATTR_LINECOLOR) )
    {
        if (SvxUnoConvertResourceStringBuiltIn(SvxUnoColorNameResId, SvxUnoColorNameDefResId, SAL_N_ELEMENTS(SvxUnoColorNameResId), aNew, true))
        {
            return aNew;
        }
    }
    else
    {
        const char** pApiResIds;
        const char** pIntResIds;
        int nCount;

        if( SvxUnoGetResourceRanges(nWhich, pApiResIds, pIntResIds, nCount))
        {
            if (SvxUnoConvertResourceString(pIntResIds, pApiResIds, nCount, aNew, true))
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
OUString SvxUnogetInternalNameForItem(const sal_uInt16 nWhich, const OUString& rApiName)
{
    OUString aNew = rApiName;

    if( nWhich == sal_uInt16(XATTR_LINECOLOR) )
    {
        if (SvxUnoConvertResourceStringBuiltIn(SvxUnoColorNameDefResId, SvxUnoColorNameResId, SAL_N_ELEMENTS(SvxUnoColorNameResId), aNew, false))
        {
            return aNew;
        }
    }
    else
    {
        const char** pApiResIds;
        const char** pIntResIds;
        int nCount;

        if (SvxUnoGetResourceRanges(nWhich, pApiResIds, pIntResIds, nCount))
        {
            if (SvxUnoConvertResourceString(pApiResIds, pIntResIds, nCount, aNew, false))
            {
                return aNew;
            }
        }
    }

    // just use previous name, if nothing else was found.
    return rApiName;
}


rtl::Reference<comphelper::PropertySetInfo> const & SvxPropertySetInfoPool::getOrCreate( sal_Int32 nServiceId ) noexcept
{
    SolarMutexGuard aGuard;

    assert( nServiceId <= SVXUNO_SERVICEID_LASTID );

    if( !mxInfos[ nServiceId ].is() )
    {
        mxInfos[nServiceId] = new comphelper::PropertySetInfo();

        switch( nServiceId )
        {
        case SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS:
            mxInfos[SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS]->add( ImplGetSvxDrawingDefaultsPropertyMap() );
            break;
        case SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS_WRITER:
            mxInfos[SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS_WRITER]->add( ImplGetSvxDrawingDefaultsPropertyMap() );
            mxInfos[SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS_WRITER]->remove( UNO_NAME_EDIT_PARA_IS_HANGING_PUNCTUATION );
            // OD 13.10.2003 #i18732# - add property map for writer item 'IsFollowingTextFlow'
            mxInfos[SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS_WRITER]->add( ImplGetAdditionalWriterDrawingDefaultsPropertyMap() );
            break;

        default:
            OSL_FAIL( "unknown service id!" );
        }
    }

    return mxInfos[ nServiceId ];
}

rtl::Reference<comphelper::PropertySetInfo> SvxPropertySetInfoPool::mxInfos[SVXUNO_SERVICEID_LASTID+1] = { nullptr };

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
