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
#include <tools/fldunit.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <comphelper/sequence.hxx>
#include <svx/dialmgr.hxx>
#include "svx/unoapi.hxx"
#include <editeng/unotext.hxx>
#include <svx/unoshprp.hxx>
#include <editeng/editeng.hxx>
#include "svx/globl3d.hxx"
#include <svx/dialogs.hrc>
#include <svx/svdpool.hxx>
#include <svx/svdobj.hxx>

#include <shapeimpl.hxx>
#include <unordered_map>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans::PropertyAttribute;
using ::com::sun::star::drawing::TextVerticalAdjust;

SfxItemPropertyMapEntry const * ImplGetSvxShapePropertyMap()
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
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        // #FontWork#
        FONTWORK_PROPERTIES
        CUSTOMSHAPE_PROPERTIES
        { "UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { "ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aShapePropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvxTextShapePropertyMap()
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
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        // #FontWork#
        FONTWORK_PROPERTIES
        CUSTOMSHAPE_PROPERTIES
        { "UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { "ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aTextShapePropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvxConnectorPropertyMap()
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
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        // #FontWork#
        FONTWORK_PROPERTIES
        { "UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { "ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aConnectorPropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvxDimensioningPropertyMap()
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
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        // #FontWork#
        FONTWORK_PROPERTIES
        { "UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { "ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aDimensioningPropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvxCirclePropertyMap()
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
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        // #FontWork#
        FONTWORK_PROPERTIES
        CUSTOMSHAPE_PROPERTIES
        { "UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { "ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aCirclePropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvxPolyPolygonPropertyMap()
{
    static SfxItemPropertyMapEntry const aPolyPolygonPropertyMap_Impl[] =
    {
        { "Geometry", OWN_ATTR_BASE_GEOMETRY, cppu::UnoType<css::drawing::PointSequenceSequence>::get(), 0, 0 },
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
        { "UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { "ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aPolyPolygonPropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvxPolyPolygonBezierPropertyMap()
{
    static SfxItemPropertyMapEntry const aPolyPolygonBezierPropertyMap_Impl[] =
    {
        { "Geometry", OWN_ATTR_BASE_GEOMETRY, cppu::UnoType<css::drawing::PolyPolygonBezierCoords>::get(), 0, 0 },
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
        { "UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { "ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aPolyPolygonBezierPropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvxGraphicObjectPropertyMap()
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
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        // #FontWork#
        FONTWORK_PROPERTIES
        { "IsMirrored", OWN_ATTR_MIRRORED, cppu::UnoType<bool>::get(), 0, 0},
        { "UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { "ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { "GraphicStream", OWN_ATTR_GRAPHIC_STREAM, cppu::UnoType<css::io::XInputStream>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aGraphicObjectPropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvx3DSceneObjectPropertyMap()
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
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return a3DSceneObjectPropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvx3DCubeObjectPropertyMap()
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
        { "UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { "ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return a3DCubeObjectPropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvx3DSphereObjectPropertyMap()
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
        { "UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { "ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return a3DSphereObjectPropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvx3DLatheObjectPropertyMap()
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
        { "UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { "ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return a3DLatheObjectPropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvx3DExtrudeObjectPropertyMap()
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
        { "UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { "ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return a3DExtrudeObjectPropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvx3DPolygonObjectPropertyMap()
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
        { "UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { "ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return a3DPolygonObjectPropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvxAllPropertyMap()
{
    static SfxItemPropertyMapEntry const aAllPropertyMap_Impl[] =
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
        { "UserDefinedAttributes", SDRATTR_XMLATTRIBUTES, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { "ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS, cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aAllPropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvxGroupPropertyMap()
{
    static SfxItemPropertyMapEntry const aGroupPropertyMap_Impl[] =
    {
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aGroupPropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvxOle2PropertyMap()
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
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        FONTWORK_PROPERTIES

        { "ThumbnailGraphicURL",      OWN_ATTR_THUMBNAIL          , cppu::UnoType<OUString>::get(), 0, 0 },
        { "Model",                    OWN_ATTR_OLEMODEL           , cppu::UnoType<css::frame::XModel>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { "EmbeddedObject",           OWN_ATTR_OLE_EMBEDDED_OBJECT, cppu::UnoType<css::embed::XEmbeddedObject>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { "EmbeddedObjectNoNewClient",OWN_ATTR_OLE_EMBEDDED_OBJECT_NONEWCLIENT, cppu::UnoType<css::embed::XEmbeddedObject>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { "OriginalSize",             OWN_ATTR_OLESIZE            , cppu::UnoType<css::awt::Size>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { "CLSID",                    OWN_ATTR_CLSID              , cppu::UnoType<OUString>::get(), 0, 0 },
        { "IsInternal",               OWN_ATTR_INTERNAL_OLE       , cppu::UnoType<bool>::get() , css::beans::PropertyAttribute::READONLY,     0},
        { "VisibleArea",              OWN_ATTR_OLE_VISAREA        , cppu::UnoType<css::awt::Rectangle>::get(), 0, 0},
        { "Aspect",                   OWN_ATTR_OLE_ASPECT         , cppu::UnoType<sal_Int64>::get(), 0, 0},
        { UNO_NAME_OLE2_PERSISTNAME,  OWN_ATTR_PERSISTNAME        , cppu::UnoType<OUString>::get(), 0, 0 },
        { "LinkURL",                  OWN_ATTR_OLE_LINKURL        , cppu::UnoType<OUString>::get(), 0, 0 },
        { UNO_NAME_GRAPHOBJ_GRAPHIC,   OWN_ATTR_VALUE_GRAPHIC     , cppu::UnoType<css::graphic::XGraphic>::get(),     0,     0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aOle2PropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvxPluginPropertyMap()
{
    static SfxItemPropertyMapEntry const aPluginPropertyMap_Impl[] =
    {
        { "PluginMimeType",           OWN_ATTR_PLUGIN_MIMETYPE    , cppu::UnoType<OUString>::get(),    0,  0},
        { "PluginURL",                OWN_ATTR_PLUGIN_URL         , cppu::UnoType<OUString>::get(),    0,  0},
        { "PluginCommands",           OWN_ATTR_PLUGIN_COMMANDS    , cppu::UnoType<css::uno::Sequence< css::beans::PropertyValue >>::get(), 0, 0},
        { "Transformation",           OWN_ATTR_TRANSFORMATION     , cppu::UnoType<css::drawing::HomogenMatrix3>::get(), 0, 0 },
        { UNO_NAME_MISC_OBJ_ZORDER,   OWN_ATTR_ZORDER             , cppu::UnoType<sal_Int32>::get(),      0,      0},
        { UNO_NAME_MISC_OBJ_LAYERID,  SDRATTR_LAYERID             , cppu::UnoType<sal_Int16>::get(),      0,  0},
        { UNO_NAME_MISC_OBJ_LAYERNAME,SDRATTR_LAYERNAME           , cppu::UnoType<OUString>::get(),    0,  0},
        { UNO_NAME_LINKDISPLAYBITMAP, OWN_ATTR_LDBITMAP           , cppu::UnoType<css::awt::XBitmap>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { UNO_NAME_LINKDISPLAYNAME,   OWN_ATTR_LDNAME             , cppu::UnoType<OUString>::get(),   css::beans::PropertyAttribute::READONLY, 0},
        { UNO_NAME_OLE2_METAFILE,     OWN_ATTR_METAFILE           , cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { "ThumbnailGraphicURL",      OWN_ATTR_THUMBNAIL          , cppu::UnoType<OUString>::get(), 0, 0 },
        { UNO_NAME_MISC_OBJ_MOVEPROTECT,      SDRATTR_OBJMOVEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { UNO_NAME_MISC_OBJ_SIZEPROTECT,      SDRATTR_OBJSIZEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { UNO_NAME_OLE2_PERSISTNAME,  OWN_ATTR_PERSISTNAME        , cppu::UnoType<OUString>::get(), 0, 0 },
        { "LinkURL",                  OWN_ATTR_OLE_LINKURL        , cppu::UnoType<OUString>::get(), 0, 0 },
        { UNO_NAME_MISC_OBJ_BOUNDRECT, OWN_ATTR_BOUNDRECT,            cppu::UnoType<css::awt::Rectangle>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { "VisibleArea",              OWN_ATTR_OLE_VISAREA        , cppu::UnoType<css::awt::Rectangle>::get(), 0, 0},
        // #i68101#
        { UNO_NAME_MISC_OBJ_TITLE,        OWN_ATTR_MISC_OBJ_TITLE         , cppu::UnoType<OUString>::get(),    0,  0},
        { UNO_NAME_MISC_OBJ_DESCRIPTION,  OWN_ATTR_MISC_OBJ_DESCRIPTION   , cppu::UnoType<OUString>::get(),    0,  0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aPluginPropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvxFramePropertyMap()
{
    //TODO/LATER: new properties for ScrollingMode and DefaultBorder
    static SfxItemPropertyMapEntry const aFramePropertyMap_Impl[] =
    {
        { "FrameURL",                 OWN_ATTR_FRAME_URL          , cppu::UnoType<OUString>::get(),    0,  0},
        { "FrameName",                OWN_ATTR_FRAME_NAME         , cppu::UnoType<OUString>::get(),    0,  0},
        { "FrameIsAutoScroll",        OWN_ATTR_FRAME_ISAUTOSCROLL , cppu::UnoType<bool>::get() , css::beans::PropertyAttribute::MAYBEVOID, 0},
        { "FrameIsBorder",            OWN_ATTR_FRAME_ISBORDER     , cppu::UnoType<bool>::get() , 0, 0},
        { "FrameMarginWidth",         OWN_ATTR_FRAME_MARGIN_WIDTH , cppu::UnoType<sal_Int32>::get(),      0,      0},
        { "FrameMarginHeight",        OWN_ATTR_FRAME_MARGIN_HEIGHT, cppu::UnoType<sal_Int32>::get(),      0,      0},
        { "Transformation",           OWN_ATTR_TRANSFORMATION     , cppu::UnoType<css::drawing::HomogenMatrix3>::get(), 0, 0 },
        { UNO_NAME_MISC_OBJ_ZORDER,   OWN_ATTR_ZORDER             , cppu::UnoType<sal_Int32>::get(),      0,      0},
        { UNO_NAME_MISC_OBJ_LAYERID,  SDRATTR_LAYERID             , cppu::UnoType<sal_Int16>::get(),      0,  0},
        { UNO_NAME_MISC_OBJ_LAYERNAME,SDRATTR_LAYERNAME           , cppu::UnoType<OUString>::get(),    0,  0},
        { UNO_NAME_LINKDISPLAYBITMAP, OWN_ATTR_LDBITMAP           , cppu::UnoType<css::awt::XBitmap>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { UNO_NAME_LINKDISPLAYNAME,   OWN_ATTR_LDNAME             , cppu::UnoType<OUString>::get(),   css::beans::PropertyAttribute::READONLY, 0},
        { UNO_NAME_OLE2_METAFILE,     OWN_ATTR_METAFILE           , cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { "ThumbnailGraphicURL",      OWN_ATTR_THUMBNAIL          , cppu::UnoType<OUString>::get(), 0, 0 },
        { UNO_NAME_MISC_OBJ_MOVEPROTECT,      SDRATTR_OBJMOVEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { UNO_NAME_MISC_OBJ_SIZEPROTECT,      SDRATTR_OBJSIZEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { UNO_NAME_OLE2_PERSISTNAME,  OWN_ATTR_PERSISTNAME        , cppu::UnoType<OUString>::get(), 0, 0 },
        { "LinkURL",                  OWN_ATTR_OLE_LINKURL        , cppu::UnoType<OUString>::get(), 0, 0 },
        { UNO_NAME_MISC_OBJ_BOUNDRECT, OWN_ATTR_BOUNDRECT,            cppu::UnoType<css::awt::Rectangle>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { "VisibleArea",              OWN_ATTR_OLE_VISAREA        , cppu::UnoType<css::awt::Rectangle>::get(), 0, 0},
        // #i68101#
        { UNO_NAME_MISC_OBJ_TITLE,        OWN_ATTR_MISC_OBJ_TITLE         , cppu::UnoType<OUString>::get(),    0,  0},
        { UNO_NAME_MISC_OBJ_DESCRIPTION,  OWN_ATTR_MISC_OBJ_DESCRIPTION   , cppu::UnoType<OUString>::get(),    0,  0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aFramePropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvxAppletPropertyMap()
{
    static SfxItemPropertyMapEntry const aAppletPropertyMap_Impl[] =
    {
        { "AppletCodeBase",           OWN_ATTR_APPLET_CODEBASE    , cppu::UnoType<OUString>::get(), 0, 0},
        { "AppletName",               OWN_ATTR_APPLET_NAME        , cppu::UnoType<OUString>::get(), 0, 0},
        { "AppletCode",               OWN_ATTR_APPLET_CODE        , cppu::UnoType<OUString>::get(), 0, 0},
        { "AppletCommands",           OWN_ATTR_APPLET_COMMANDS    , cppu::UnoType<css::uno::Sequence< css::beans::PropertyValue >>::get(), 0, 0},
        { "AppletDocBase",            OWN_ATTR_APPLET_DOCBASE     , cppu::UnoType<OUString>::get(), 0, 0},
        { "AppletIsScript",           OWN_ATTR_APPLET_ISSCRIPT    , cppu::UnoType<bool>::get(), 0, 0 },
        { "Transformation",           OWN_ATTR_TRANSFORMATION     , cppu::UnoType<css::drawing::HomogenMatrix3>::get(), 0, 0 },
        { UNO_NAME_MISC_OBJ_ZORDER,   OWN_ATTR_ZORDER             , cppu::UnoType<sal_Int32>::get(),      0,      0},
        { UNO_NAME_MISC_OBJ_LAYERID,  SDRATTR_LAYERID             , cppu::UnoType<sal_Int16>::get(),      0,  0},
        { UNO_NAME_MISC_OBJ_LAYERNAME,SDRATTR_LAYERNAME           , cppu::UnoType<OUString>::get(),    0,  0},
        { UNO_NAME_LINKDISPLAYBITMAP, OWN_ATTR_LDBITMAP           , cppu::UnoType<css::awt::XBitmap>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { UNO_NAME_LINKDISPLAYNAME,   OWN_ATTR_LDNAME             , cppu::UnoType<OUString>::get(),   css::beans::PropertyAttribute::READONLY, 0},
        { UNO_NAME_OLE2_METAFILE,     OWN_ATTR_METAFILE           , cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { "ThumbnailGraphicURL",      OWN_ATTR_THUMBNAIL          , cppu::UnoType<OUString>::get(), 0, 0 },
        { UNO_NAME_MISC_OBJ_MOVEPROTECT,      SDRATTR_OBJMOVEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { UNO_NAME_MISC_OBJ_SIZEPROTECT,      SDRATTR_OBJSIZEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { UNO_NAME_OLE2_PERSISTNAME,  OWN_ATTR_PERSISTNAME        , cppu::UnoType<OUString>::get(), 0, 0 },
        { "LinkURL",                  OWN_ATTR_OLE_LINKURL        , cppu::UnoType<OUString>::get(), 0, 0 },
        { UNO_NAME_MISC_OBJ_BOUNDRECT, OWN_ATTR_BOUNDRECT,            cppu::UnoType<css::awt::Rectangle>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { "VisibleArea",              OWN_ATTR_OLE_VISAREA        , cppu::UnoType<css::awt::Rectangle>::get(), 0, 0},
        // #i68101#
        { UNO_NAME_MISC_OBJ_TITLE,        OWN_ATTR_MISC_OBJ_TITLE         , cppu::UnoType<OUString>::get(),    0,  0},
        { UNO_NAME_MISC_OBJ_DESCRIPTION,  OWN_ATTR_MISC_OBJ_DESCRIPTION   , cppu::UnoType<OUString>::get(),    0,  0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aAppletPropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvxControlShapePropertyMap()
{
    static SfxItemPropertyMapEntry const aControlPropertyMap_Impl[] =
    {
        // the following properties are mapped to the XControl Model of this shape
        { UNO_NAME_EDIT_CHAR_FONTNAME,        0,  cppu::UnoType<OUString>::get(),  0, 0 },
        { UNO_NAME_EDIT_CHAR_FONTSTYLENAME,   0,  cppu::UnoType<OUString>::get(),  0, 0 },
        { UNO_NAME_EDIT_CHAR_FONTFAMILY,      0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { UNO_NAME_EDIT_CHAR_FONTCHARSET,     0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { UNO_NAME_EDIT_CHAR_HEIGHT,          0,  cppu::UnoType<float>::get(),            0, 0 },
        { UNO_NAME_EDIT_CHAR_FONTPITCH,       0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { UNO_NAME_EDIT_CHAR_POSTURE,         0,  cppu::UnoType<css::awt::FontSlant>::get(),0, 0 },
        { UNO_NAME_EDIT_CHAR_WEIGHT,          0,  cppu::UnoType<float>::get(),            0, 0 },
        { UNO_NAME_EDIT_CHAR_UNDERLINE,       0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { UNO_NAME_EDIT_CHAR_STRIKEOUT,       0,  cppu::UnoType<sal_Int16>::get(),        0, 0},
        { UNO_NAME_EDIT_CHAR_CASEMAP,         0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { UNO_NAME_EDIT_CHAR_COLOR,           0,  cppu::UnoType<sal_Int32>::get(),        0, 0 },
        { "CharBackColor",                    0,  cppu::UnoType<sal_Int32>::get(),        0, 0 },
        { "CharBackTransparent",              0,  cppu::UnoType<bool>::get(),             0, 0 },
        { "CharRelief",                       0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { "CharUnderlineColor",               0,  cppu::UnoType<sal_Int32>::get(),        0, 0 },
        { "CharKerning",                      0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { "CharWordMode",                     0,  cppu::UnoType<bool>::get(),                    0, 0 },
        { UNO_NAME_EDIT_PARA_ADJUST,          0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { "TextVerticalAdjust",               0,  cppu::UnoType<TextVerticalAdjust>::get(), MAYBEVOID, 0 },
        { "ControlBackground",                0,  cppu::UnoType<sal_Int32>::get(),        0, 0 },
        { "ControlBorder",                    0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { "ControlBorderColor",               0,  cppu::UnoType<sal_Int32>::get(),        0, 0 },
        { "ControlSymbolColor",               0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { "ImageScaleMode",                   0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { UNO_NAME_MISC_OBJ_MOVEPROTECT,      SDRATTR_OBJMOVEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { UNO_NAME_MISC_OBJ_SIZEPROTECT,      SDRATTR_OBJSIZEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { "ControlTextEmphasis",              0,  cppu::UnoType<sal_Int16>::get(),        0, 0 },
        { "ControlWritingMode",               0,  cppu::UnoType< sal_Int16 >::get(), 0, 0},
        // the following properties are handled by SvxShape
        { "Transformation",           OWN_ATTR_TRANSFORMATION     , cppu::UnoType<css::drawing::HomogenMatrix3>::get(), 0, 0 },
        { UNO_NAME_MISC_OBJ_ZORDER,   OWN_ATTR_ZORDER             , cppu::UnoType<sal_Int32>::get(),      0,      0},
        { UNO_NAME_MISC_OBJ_LAYERID,  SDRATTR_LAYERID             , cppu::UnoType<sal_Int16>::get(),      0,  0},
        { UNO_NAME_MISC_OBJ_LAYERNAME,SDRATTR_LAYERNAME           , cppu::UnoType<OUString>::get(),    0,  0},
        { UNO_NAME_LINKDISPLAYBITMAP, OWN_ATTR_LDBITMAP           , cppu::UnoType<css::awt::XBitmap>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { UNO_NAME_LINKDISPLAYNAME,   OWN_ATTR_LDNAME             , cppu::UnoType<OUString>::get(),   css::beans::PropertyAttribute::READONLY, 0},
        { "UserDefinedAttributes",        SDRATTR_XMLATTRIBUTES,      cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        {OUString("ParaUserDefinedAttributes"),         EE_PARA_XMLATTRIBS,     cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { UNO_NAME_MISC_OBJ_BOUNDRECT, OWN_ATTR_BOUNDRECT,            cppu::UnoType<css::awt::Rectangle>::get(), css::beans::PropertyAttribute::READONLY, 0},
        // #i68101#
        { UNO_NAME_MISC_OBJ_TITLE,        OWN_ATTR_MISC_OBJ_TITLE         , cppu::UnoType<OUString>::get(),    0,  0},
        { UNO_NAME_MISC_OBJ_DESCRIPTION,  OWN_ATTR_MISC_OBJ_DESCRIPTION   , cppu::UnoType<OUString>::get(),    0,  0},
        // #i112587#
        { UNO_NAME_MISC_OBJ_PRINTABLE,    SDRATTR_OBJPRINTABLE            , cppu::UnoType<bool>::get(),                      0,  0},
        { "Visible",                      SDRATTR_OBJVISIBLE              , cppu::UnoType<bool>::get(),                      0,  0},
        { UNO_NAME_MISC_OBJ_INTEROPGRABBAG,   OWN_ATTR_INTEROPGRABBAG,    cppu::UnoType<css::uno::Sequence< css::beans::PropertyValue >>::get(),  0,  0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aControlPropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvxPageShapePropertyMap()
{
    static SfxItemPropertyMapEntry const aPageShapePropertyMap_Impl[] =
    {
        { "PageNumber",               OWN_ATTR_PAGE_NUMBER        , cppu::UnoType<sal_Int32>::get(),      0, 0},
        { "Transformation",           OWN_ATTR_TRANSFORMATION     , cppu::UnoType<css::drawing::HomogenMatrix3>::get(), 0, 0 },
        { UNO_NAME_MISC_OBJ_ZORDER,   OWN_ATTR_ZORDER             , cppu::UnoType<sal_Int32>::get(),      0, 0},
        { UNO_NAME_MISC_OBJ_LAYERID,  SDRATTR_LAYERID             , cppu::UnoType<sal_Int16>::get(),      0, 0},
        { UNO_NAME_MISC_OBJ_LAYERNAME,SDRATTR_LAYERNAME           , cppu::UnoType<OUString>::get(), 0, 0},
        { UNO_NAME_LINKDISPLAYBITMAP, OWN_ATTR_LDBITMAP           , cppu::UnoType<css::awt::XBitmap>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { UNO_NAME_LINKDISPLAYNAME,   OWN_ATTR_LDNAME             , cppu::UnoType<OUString>::get(),   css::beans::PropertyAttribute::READONLY, 0},
        { UNO_NAME_MISC_OBJ_MOVEPROTECT,      SDRATTR_OBJMOVEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { UNO_NAME_MISC_OBJ_SIZEPROTECT,      SDRATTR_OBJSIZEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { UNO_NAME_MISC_OBJ_BOUNDRECT, OWN_ATTR_BOUNDRECT,            cppu::UnoType<css::awt::Rectangle>::get(), css::beans::PropertyAttribute::READONLY, 0},
        // #i68101#
        { UNO_NAME_MISC_OBJ_TITLE,        OWN_ATTR_MISC_OBJ_TITLE         , cppu::UnoType<OUString>::get(),    0,  0},
        { UNO_NAME_MISC_OBJ_DESCRIPTION,  OWN_ATTR_MISC_OBJ_DESCRIPTION   , cppu::UnoType<OUString>::get(),    0,  0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aPageShapePropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvxCaptionPropertyMap()
{
    static SfxItemPropertyMapEntry const aCaptionPropertyMap_Impl[] =
    {
        { "CaptionPoint",             OWN_ATTR_CAPTION_POINT,     cppu::UnoType<css::awt::Point>::get(),   0, 0 },
        { "CaptionType",              SDRATTR_CAPTIONTYPE,        cppu::UnoType<sal_Int16>::get(), 0, 0},
        { "CaptionIsFixedAngle",      SDRATTR_CAPTIONFIXEDANGLE,  cppu::UnoType<bool>::get(), 0, 0},
        { "CaptionAngle",             SDRATTR_CAPTIONANGLE,       cppu::UnoType<sal_Int32>::get(),    0,  0},
        { "CaptionGap",               SDRATTR_CAPTIONGAP,         cppu::UnoType<sal_Int32>::get(),    0,  SFX_METRIC_ITEM},
        { "CaptionEscapeDirection",   SDRATTR_CAPTIONESCDIR,      cppu::UnoType<sal_Int32>::get(),    0,  0},
        { "CaptionIsEscapeRelative",  SDRATTR_CAPTIONESCISREL,    cppu::UnoType<bool>::get(), 0, 0},
        { "CaptionEscapeRelative",    SDRATTR_CAPTIONESCREL,      cppu::UnoType<sal_Int32>::get(),    0,  0},
        { "CaptionEscapeAbsolute",    SDRATTR_CAPTIONESCABS,      cppu::UnoType<sal_Int32>::get(),    0,  SFX_METRIC_ITEM},
        { "CaptionLineLength",        SDRATTR_CAPTIONLINELEN,     cppu::UnoType<sal_Int32>::get(),    0,  SFX_METRIC_ITEM},
        { "CaptionIsFitLineLength",   SDRATTR_CAPTIONFITLINELEN,  cppu::UnoType<bool>::get(), 0, 0},
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
        { "UserDefinedAttributes",        SDRATTR_XMLATTRIBUTES,      cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { "ParaUserDefinedAttributes",         EE_PARA_XMLATTRIBS,     cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aCaptionPropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvxCustomShapePropertyMap()
{
    static SfxItemPropertyMapEntry const aCustomShapePropertyMap_Impl[] =
    {
        { "CustomShapeEngine",            SDRATTR_CUSTOMSHAPE_ENGINE,         cppu::UnoType<OUString>::get(),  0, 0 },
        { "CustomShapeData",          SDRATTR_CUSTOMSHAPE_DATA,               cppu::UnoType<OUString>::get(),  0, 0 },
        { "CustomShapeGeometry",      SDRATTR_CUSTOMSHAPE_GEOMETRY,
        cppu::UnoType<css::uno::Sequence< css::beans::PropertyValue >>::get(),  0, 0 },
        { "CustomShapeGraphicURL",        SDRATTR_CUSTOMSHAPE_REPLACEMENT_URL,    cppu::UnoType<OUString>::get(),  0,  0},
        CUSTOMSHAPE_PROPERTIES
        FILL_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        SHAPE_DESCRIPTOR_PROPERTIES
        MISC_OBJ_PROPERTIES
        LINKTARGET_PROPERTIES
        SHADOW_PROPERTIES
        TEXT_PROPERTIES
        { "UserDefinedAttributes",     SDRATTR_XMLATTRIBUTES,      cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { "ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS,         cppu::UnoType<css::container::XNameContainer>::get(),        0,     0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aCustomShapePropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvxMediaShapePropertyMap()
{
    static SfxItemPropertyMapEntry const aMediaShapePropertyMap_Impl[] =
    {
        { UNO_NAME_MISC_OBJ_ZORDER, OWN_ATTR_ZORDER, cppu::UnoType<sal_Int32>::get(), 0, 0},
        { UNO_NAME_MISC_OBJ_LAYERID, SDRATTR_LAYERID, cppu::UnoType<sal_Int16>::get(), 0, 0},
        { UNO_NAME_MISC_OBJ_LAYERNAME, SDRATTR_LAYERNAME, cppu::UnoType<OUString>::get(), 0, 0},
        { UNO_NAME_LINKDISPLAYBITMAP, OWN_ATTR_LDBITMAP, cppu::UnoType<css::awt::XBitmap>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { UNO_NAME_LINKDISPLAYNAME, OWN_ATTR_LDNAME, cppu::UnoType<OUString>::get(),  css::beans::PropertyAttribute::READONLY, 0},
        { "Transformation", OWN_ATTR_TRANSFORMATION, cppu::UnoType<css::drawing::HomogenMatrix3>::get(), 0, 0 },
        { "MediaURL", OWN_ATTR_MEDIA_URL, cppu::UnoType<OUString>::get(), 0, 0},
        { "PreferredSize", OWN_ATTR_MEDIA_PREFERREDSIZE, cppu::UnoType<css::awt::Size>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { "Loop", OWN_ATTR_MEDIA_LOOP, cppu::UnoType<sal_Bool>::get(), 0, 0},
        { "Mute", OWN_ATTR_MEDIA_MUTE, cppu::UnoType<sal_Bool>::get(), 0, 0},
        { "VolumeDB", OWN_ATTR_MEDIA_VOLUMEDB, cppu::UnoType<sal_Int16>::get(), 0, 0},
        { "Zoom", OWN_ATTR_MEDIA_ZOOM, cppu::UnoType<css::media::ZoomLevel>::get(), 0, 0},
        { UNO_NAME_MISC_OBJ_MOVEPROTECT, SDRATTR_OBJMOVEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { UNO_NAME_MISC_OBJ_SIZEPROTECT, SDRATTR_OBJSIZEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { UNO_NAME_MISC_OBJ_BOUNDRECT, OWN_ATTR_BOUNDRECT, cppu::UnoType<css::awt::Rectangle>::get(), css::beans::PropertyAttribute::READONLY, 0},
        // #i68101#
        { UNO_NAME_MISC_OBJ_TITLE,        OWN_ATTR_MISC_OBJ_TITLE         , cppu::UnoType<OUString>::get(),    0,  0},
        { UNO_NAME_MISC_OBJ_DESCRIPTION,  OWN_ATTR_MISC_OBJ_DESCRIPTION   , cppu::UnoType<OUString>::get(),    0,  0},
        { "PrivateStream", OWN_ATTR_MEDIA_STREAM, cppu::UnoType<css::io::XInputStream>::get(), 0, 0},
        { "PrivateTempFileURL", OWN_ATTR_MEDIA_TEMPFILEURL, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { "MediaMimeType", OWN_ATTR_MEDIA_MIMETYPE, cppu::UnoType<OUString>::get(), 0, 0},
        { "FallbackGraphic", OWN_ATTR_FALLBACK_GRAPHIC, cppu::UnoType<css::graphic::XGraphic>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aMediaShapePropertyMap_Impl;
}

SfxItemPropertyMapEntry const * ImplGetSvxTableShapePropertyMap()
{
    static SfxItemPropertyMapEntry const  aTableShapePropertyMap_Impl[] =
    {
        { UNO_NAME_MISC_OBJ_ZORDER,       OWN_ATTR_ZORDER, cppu::UnoType<sal_Int32>::get(), 0, 0},
        { UNO_NAME_MISC_OBJ_LAYERID,      SDRATTR_LAYERID, cppu::UnoType<sal_Int16>::get(), 0,    0},
        { UNO_NAME_MISC_OBJ_LAYERNAME,    SDRATTR_LAYERNAME, cppu::UnoType<OUString>::get(), 0, 0},
        { UNO_NAME_LINKDISPLAYBITMAP,     OWN_ATTR_LDBITMAP, cppu::UnoType<css::awt::XBitmap>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { UNO_NAME_LINKDISPLAYNAME,       OWN_ATTR_LDNAME, cppu::UnoType<OUString>::get(),    css::beans::PropertyAttribute::READONLY, 0},
        { "Transformation",               OWN_ATTR_TRANSFORMATION, cppu::UnoType<css::drawing::HomogenMatrix3>::get(), 0, 0 },
        { UNO_NAME_MISC_OBJ_MOVEPROTECT,  SDRATTR_OBJMOVEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { UNO_NAME_MISC_OBJ_SIZEPROTECT,  SDRATTR_OBJSIZEPROTECT, cppu::UnoType<bool>::get(),0, 0},
        { UNO_NAME_MISC_OBJ_BOUNDRECT,    OWN_ATTR_BOUNDRECT, cppu::UnoType<css::awt::Rectangle>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { UNO_NAME_MISC_OBJ_NAME,         SDRATTR_OBJECTNAME, cppu::UnoType<rtl::OUString>::get(),    0,      0},
        { UNO_NAME_MISC_OBJ_TITLE,        OWN_ATTR_MISC_OBJ_TITLE         , cppu::UnoType<OUString>::get(),    0,  0},
        { UNO_NAME_MISC_OBJ_DESCRIPTION,  OWN_ATTR_MISC_OBJ_DESCRIPTION   , cppu::UnoType<OUString>::get(),    0,  0},
        { "Model",                        OWN_ATTR_OLEMODEL               , cppu::UnoType<css::table::XTable>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { "TableTemplate",                OWN_ATTR_TABLETEMPLATE          , cppu::UnoType<css::container::XIndexAccess>::get(), 0, 0},
        { "UseFirstRowStyle",             OWN_ATTR_TABLETEMPLATE_FIRSTROW, cppu::UnoType<bool>::get(),0, 0},
        { "UseLastRowStyle",              OWN_ATTR_TABLETEMPLATE_LASTROW, cppu::UnoType<bool>::get(),0, 0},
        { "UseFirstColumnStyle",          OWN_ATTR_TABLETEMPLATE_FIRSTCOLUMN, cppu::UnoType<bool>::get(),0, 0},
        { "UseLastColumnStyle",           OWN_ATTR_TABLETEMPLATE_LASTCOLUMN, cppu::UnoType<bool>::get(),0, 0},
        { "UseBandingRowStyle",           OWN_ATTR_TABLETEMPLATE_BANDINGROWS, cppu::UnoType<bool>::get(),0, 0},
        { "UseBandingColumnStyle",        OWN_ATTR_TABLETEMPLATE_BANDINGCOULUMNS, cppu::UnoType<bool>::get(),0, 0},
        { "ReplacementGraphic",           OWN_ATTR_BITMAP, cppu::UnoType<css::graphic::XGraphic>::get(), css::beans::PropertyAttribute::READONLY, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aTableShapePropertyMap_Impl;
}

comphelper::PropertyMapEntry const * ImplGetSvxDrawingDefaultsPropertyMap()
{
    static comphelper::PropertyMapEntry const aSvxDrawingDefaultsPropertyMap_Impl[] =
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
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aSvxDrawingDefaultsPropertyMap_Impl;
}

comphelper::PropertyMapEntry const * ImplGetAdditionalWriterDrawingDefaultsPropertyMap()
{
    static comphelper::PropertyMapEntry const aSvxAdditionalDefaultsPropertyMap_Impl[] =
    {
        { "IsFollowingTextFlow", SID_SW_FOLLOW_TEXT_FLOW, cppu::UnoType<bool>::get(), 0, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aSvxAdditionalDefaultsPropertyMap_Impl;
}

typedef std::unordered_map< OUString, sal_uInt32, OUStringHash > UHashMapImpl;

namespace {

const UHashMapImpl& GetUHashImpl()
{
    static const UHashMapImpl aImpl {
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

            { "com.sun.star.drawing.Shape3DSceneObject",   E3D_POLYSCENE_ID  | E3D_INVENTOR_FLAG },
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

    for (UHashMapImpl::const_iterator it = rMap.begin(); it != rMap.end(); ++it)
    {
        if (it->second == nId)
            return it->first;
    }
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

struct theSvxMapProvider :
    public rtl::Static<SvxUnoPropertyMapProvider, theSvxMapProvider>
{
};

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
    for(sal_uInt16 i=0;i<SVXMAP_END; i++)
        delete aSetArr[i];
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
bool SvxMapUnitToMeasureUnit( const MapUnit eVcl, short& eApi ) throw()
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

bool SvxMeasureUnitToFieldUnit( const short eApi, FieldUnit& eVcl ) throw()
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
        return false;
    }

    return true;
}

/** maps the vcl MapUnit enum to a API constant MeasureUnit.
    Returns false if conversion is not supported.
*/
bool SvxFieldUnitToMeasureUnit( const FieldUnit eVcl, short& eApi ) throw()
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
        return false;
    }

    return true;
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

bool SvxUnoConvertResourceString( int nSourceResIds, int nDestResIds, int nCount, OUString& rString ) throw(std::exception)
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


// #i122649# Some comments on the below arrays:
// - They need to have the same order and count of items
// - They are used to translate between translated and non-translated color names
// - To make longer names be found which start with the same basic string,
//   these have to be in front of others

// It would be nice to:
// - evtl. organize these in a single array with 2-dimensional inner to eliminate
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


bool SvxUnoConvertResourceString( const sal_uInt16* pSourceResIds, const sal_uInt16* pDestResIds, int nCount, OUString& rString ) throw (std::exception)
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
OUString SvxUnogetApiNameForItem(const sal_Int16 nWhich, const OUString& rInternalName) throw(std::exception)
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
OUString SvxUnogetInternalNameForItem(const sal_Int16 nWhich, const OUString& rApiName) throw(std::exception)
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


comphelper::PropertySetInfo* SvxPropertySetInfoPool::getOrCreate( sal_Int32 nServiceId ) throw()
{
    SolarMutexGuard aGuard;

    if( nServiceId > SVXUNO_SERVICEID_LASTID )
    {
        OSL_FAIL( "unknown service id!" );
        return nullptr;
    }

    if( mpInfos[ nServiceId ] == nullptr )
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
            mpInfos[SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS_WRITER]->remove( UNO_NAME_EDIT_PARA_IS_HANGING_PUNCTUATION );
            // OD 13.10.2003 #i18732# - add property map for writer item 'IsFollowingTextFlow'
            mpInfos[SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS_WRITER]->add( ImplGetAdditionalWriterDrawingDefaultsPropertyMap() );
            break;

        default:
            OSL_FAIL( "unknown service id!" );
        }
    }

    return mpInfos[ nServiceId ];
}

comphelper::PropertySetInfo* SvxPropertySetInfoPool::mpInfos[SVXUNO_SERVICEID_LASTID+1] = { nullptr };

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
