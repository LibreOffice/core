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

#ifndef INCLUDED_SVX_UNOSHPRP_HXX
#define INCLUDED_SVX_UNOSHPRP_HXX

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/drawing/RectanglePoint.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineCap.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/drawing/TextAnimationDirection.hpp>
#include <com/sun/star/drawing/TextAnimationKind.hpp>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/MeasureKind.hpp>
#include <com/sun/star/drawing/MeasureTextHorzPos.hpp>
#include <com/sun/star/drawing/MeasureTextVertPos.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/drawing/PolygonKind.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/ColorMode.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/NormalsKind.hpp>
#include <com/sun/star/drawing/TextureKind.hpp>
#include <com/sun/star/drawing/TextureMode.hpp>
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/CameraGeometry.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <svx/unoprov.hxx>
#include <editeng/unoprnms.hxx>
#include <svx/unomid.hxx>
#include <editeng/unotext.hxx>
#include <svl/itemprop.hxx>

#include <svx/svxids.hrc>

#define OWN_ATTR_VALUE_START_TEXT               (OWN_ATTR_VALUE_START+0)    // the next 10 entries are reserved for text
#define OWN_ATTR_VALUE_POLYGONKIND              (OWN_ATTR_VALUE_START+10)
#define OWN_ATTR_VALUE_POLYPOLYGON              (OWN_ATTR_VALUE_START+11)
#define OWN_ATTR_VALUE_POLYPOLYGONBEZIER        (OWN_ATTR_VALUE_START+12)
#define OWN_ATTR_VALUE_FILLBITMAP               (OWN_ATTR_VALUE_START+13)
#define OWN_ATTR_3D_VALUE_TRANSFORM_MATRIX      (OWN_ATTR_VALUE_START+14)
#define OWN_ATTR_3D_VALUE_POSITION              (OWN_ATTR_VALUE_START+15)
#define OWN_ATTR_3D_VALUE_SIZE                  (OWN_ATTR_VALUE_START+16)
#define OWN_ATTR_3D_VALUE_POS_IS_CENTER         (OWN_ATTR_VALUE_START+17)
#define OWN_ATTR_3D_VALUE_POLYPOLYGON3D         (OWN_ATTR_VALUE_START+18)
#define OWN_ATTR_3D_VALUE_LINEONLY              (OWN_ATTR_VALUE_START+19)
#define OWN_ATTR_BITMAP                         (OWN_ATTR_VALUE_START+20)
#define OWN_ATTR_EDGE_START_OBJ                 (OWN_ATTR_VALUE_START+21)
#define OWN_ATTR_EDGE_START_POS                 (OWN_ATTR_VALUE_START+22)
#define OWN_ATTR_EDGE_END_OBJ                   (OWN_ATTR_VALUE_START+23)
#define OWN_ATTR_EDGE_END_POS                   (OWN_ATTR_VALUE_START+24)
#define OWN_ATTR_FRAMERECT                      (OWN_ATTR_VALUE_START+25)
#define OWN_ATTR_VALUE_POLYGON                  (OWN_ATTR_VALUE_START+26)
#define OWN_ATTR_METAFILE                       (OWN_ATTR_VALUE_START+27)
#define OWN_ATTR_ISFONTWORK                     (OWN_ATTR_VALUE_START+28)
#define OWN_ATTR_BOUNDRECT                      (OWN_ATTR_VALUE_START+29)
#define OWN_ATTR_LDNAME                         (OWN_ATTR_VALUE_START+30)
#define OWN_ATTR_LDBITMAP                       (OWN_ATTR_VALUE_START+31)
#define OWN_ATTR_OLESIZE                        (OWN_ATTR_VALUE_START+32)
#define OWN_ATTR_GRAFURL                        (OWN_ATTR_VALUE_START+33)
#define OWN_ATTR_OLEMODEL                       (OWN_ATTR_VALUE_START+34)
#define OWN_ATTR_MIRRORED                       (OWN_ATTR_VALUE_START+35)
#define OWN_ATTR_CLSID                          (OWN_ATTR_VALUE_START+36)
#define OWN_ATTR_GLUEID_TAIL                    (OWN_ATTR_VALUE_START+37)
#define OWN_ATTR_GLUEID_HEAD                    (OWN_ATTR_VALUE_START+38)
#define OWN_ATTR_ZORDER                         (OWN_ATTR_VALUE_START+39)
#define OWN_ATTR_MEASURE_START_POS              (OWN_ATTR_VALUE_START+40)
#define OWN_ATTR_MEASURE_END_POS                (OWN_ATTR_VALUE_START+41)
#define OWN_ATTR_3D_VALUE_CAMERA_GEOMETRY       (OWN_ATTR_VALUE_START+42)
#define OWN_ATTR_WRITINGMODE                    (OWN_ATTR_VALUE_START+43)
#define OWN_ATTR_GRAFSTREAMURL                  (OWN_ATTR_VALUE_START+44)
#define OWN_ATTR_FILLBMP_MODE                   (OWN_ATTR_VALUE_START+45)
#define OWN_ATTR_TRANSFORMATION                 (OWN_ATTR_VALUE_START+46)
#define OWN_ATTR_BASE_GEOMETRY                  (OWN_ATTR_VALUE_START+47)

/// reuse attr slots for GraphicObject which will never be used together with graphic object
#define OWN_ATTR_REPLACEMENTGRAFURL             (OWN_ATTR_VALUE_START+14)

#define OWN_ATTR_APPLET_DOCBASE                 (OWN_ATTR_VALUE_START+48)
#define OWN_ATTR_APPLET_CODEBASE                (OWN_ATTR_VALUE_START+49)
#define OWN_ATTR_APPLET_NAME                    (OWN_ATTR_VALUE_START+50)
#define OWN_ATTR_APPLET_CODE                    (OWN_ATTR_VALUE_START+51)
#define OWN_ATTR_APPLET_COMMANDS                (OWN_ATTR_VALUE_START+52)
#define OWN_ATTR_APPLET_ISSCRIPT                (OWN_ATTR_VALUE_START+53)
#define OWN_ATTR_PLUGIN_MIMETYPE                (OWN_ATTR_VALUE_START+54)
#define OWN_ATTR_PLUGIN_URL                     (OWN_ATTR_VALUE_START+55)
#define OWN_ATTR_PLUGIN_COMMANDS                (OWN_ATTR_VALUE_START+56)
#define OWN_ATTR_FRAME_URL                      (OWN_ATTR_VALUE_START+57)
#define OWN_ATTR_FRAME_NAME                     (OWN_ATTR_VALUE_START+58)
#define OWN_ATTR_FRAME_ISAUTOSCROLL             (OWN_ATTR_VALUE_START+59)
#define OWN_ATTR_FRAME_ISBORDER                 (OWN_ATTR_VALUE_START+60)
#define OWN_ATTR_FRAME_MARGIN_WIDTH             (OWN_ATTR_VALUE_START+61)
#define OWN_ATTR_FRAME_MARGIN_HEIGHT            (OWN_ATTR_VALUE_START+62)

// reuse own attr from ole shapes for tables
#define OWN_ATTR_TABLETEMPLATE                  (OWN_ATTR_VALUE_START+48)
#define OWN_ATTR_TABLETEMPLATE_FIRSTROW         (OWN_ATTR_VALUE_START+49)
#define OWN_ATTR_TABLETEMPLATE_LASTROW          (OWN_ATTR_VALUE_START+50)
#define OWN_ATTR_TABLETEMPLATE_FIRSTCOLUMN      (OWN_ATTR_VALUE_START+51)
#define OWN_ATTR_TABLETEMPLATE_LASTCOLUMN       (OWN_ATTR_VALUE_START+52)
#define OWN_ATTR_TABLETEMPLATE_BANDINGROWS      (OWN_ATTR_VALUE_START+53)
#define OWN_ATTR_TABLETEMPLATE_BANDINGCOULUMNS  (OWN_ATTR_VALUE_START+54)
#define OWN_ATTR_TABLEBORDER                    (OWN_ATTR_VALUE_START+55)

#define OWN_ATTR_OLE_VISAREA                    (OWN_ATTR_VALUE_START+63)
#define OWN_ATTR_CAPTION_POINT                  (OWN_ATTR_VALUE_START+64)
#define OWN_ATTR_PAGE_NUMBER                    (OWN_ATTR_VALUE_START+65)
#define OWN_ATTR_THUMBNAIL                      (OWN_ATTR_VALUE_START+66)
#define OWN_ATTR_PERSISTNAME                    (OWN_ATTR_VALUE_START+67)
#define OWN_ATTR_OLE_EMBEDDED_OBJECT_NONEWCLIENT    (OWN_ATTR_VALUE_START+68)
#define OWN_ATTR_MEDIA_URL                      (OWN_ATTR_VALUE_START+69)
#define OWN_ATTR_MEDIA_PREFERREDSIZE            (OWN_ATTR_VALUE_START+70)
#define OWN_ATTR_MEDIA_LOOP                     (OWN_ATTR_VALUE_START+71)
#define OWN_ATTR_MEDIA_MUTE                     (OWN_ATTR_VALUE_START+72)
#define OWN_ATTR_MEDIA_VOLUMEDB                 (OWN_ATTR_VALUE_START+73)
#define OWN_ATTR_MEDIA_ZOOM                     (OWN_ATTR_VALUE_START+74)
#define OWN_ATTR_UINAME_SINGULAR                (OWN_ATTR_VALUE_START+75)
#define OWN_ATTR_UINAME_PLURAL                  (OWN_ATTR_VALUE_START+76)
#define OWN_ATTR_VALUE_GRAPHIC                  (OWN_ATTR_VALUE_START+77)
#define OWN_ATTR_INTERNAL_OLE                   (OWN_ATTR_VALUE_START+78)
#define OWN_ATTR_OLE_EMBEDDED_OBJECT            (OWN_ATTR_VALUE_START+79)
#define OWN_ATTR_OLE_ASPECT                     (OWN_ATTR_VALUE_START+80)

// #i68101#
#define OWN_ATTR_MISC_OBJ_TITLE                 (OWN_ATTR_VALUE_START+81)
#define OWN_ATTR_MISC_OBJ_DESCRIPTION           (OWN_ATTR_VALUE_START+82)

#define OWN_ATTR_GRAPHIC_STREAM                 (OWN_ATTR_VALUE_START+83)
#define OWN_ATTR_3D_VALUE_NORMALSPOLYGON3D      (OWN_ATTR_VALUE_START+84)
#define OWN_ATTR_3D_VALUE_TEXTUREPOLYGON3D      (OWN_ATTR_VALUE_START+85)

#define OWN_ATTR_OLE_LINKURL                    (OWN_ATTR_VALUE_START+86)

#define OWN_ATTR_STYLE                          (OWN_ATTR_VALUE_START+87)

#define OWN_ATTR_EDGE_POLYPOLYGONBEZIER         (OWN_ATTR_VALUE_START+88)

#define OWN_ATTR_MEDIA_STREAM                   (OWN_ATTR_VALUE_START+89)
#define OWN_ATTR_MEDIA_TEMPFILEURL              (OWN_ATTR_VALUE_START+90)
#define OWN_ATTR_INTEROPGRABBAG                 (OWN_ATTR_VALUE_START+91)
#define OWN_ATTR_MEDIA_MIMETYPE                 (OWN_ATTR_VALUE_START+92)
#define OWN_ATTR_FALLBACK_GRAPHIC               (OWN_ATTR_VALUE_START+93)
// ATTENTION: maximum is OWN_ATTR_VALUE_START+93 svx, see include/svl/solar.hrc

// #FontWork#
#define FONTWORK_PROPERTIES \
    { OUString("FontWorkStyle"),            XATTR_FORMTXTSTYLE,         /*ENUM*/::cppu::UnoType<sal_Int32>::get(),    0,  0}, \
    { OUString("FontWorkAdjust"),           XATTR_FORMTXTADJUST,        /*ENUM*/::cppu::UnoType<sal_Int32>::get(),    0,  0}, \
    { OUString("FontWorkDistance"),         XATTR_FORMTXTDISTANCE,      ::cppu::UnoType<sal_Int32>::get(),            0,  SFX_METRIC_ITEM}, \
    { OUString("FontWorkStart"),            XATTR_FORMTXTSTART,         ::cppu::UnoType<sal_Int32>::get(),            0,  SFX_METRIC_ITEM}, \
    { OUString("FontWorkMirror"),           XATTR_FORMTXTMIRROR,        cppu::UnoType<bool>::get(),                        0,  0}, \
    { OUString("FontWorkOutline"),          XATTR_FORMTXTOUTLINE,       cppu::UnoType<bool>::get(),                        0,  0}, \
    { OUString("FontWorkShadow"),           XATTR_FORMTXTSHADOW,        /*ENUM*/::cppu::UnoType<sal_Int32>::get(),    0,  0}, \
    { OUString("FontWorkShadowColor"),      XATTR_FORMTXTSHDWCOLOR,     ::cppu::UnoType<sal_Int32>::get(),            0,  0}, \
    { OUString("FontWorkShadowOffsetX"),    XATTR_FORMTXTSHDWXVAL,      ::cppu::UnoType<sal_Int32>::get(),            0,  SFX_METRIC_ITEM}, \
    { OUString("FontWorkShadowOffsetY"),    XATTR_FORMTXTSHDWYVAL,      ::cppu::UnoType<sal_Int32>::get(),            0,  SFX_METRIC_ITEM}, \
    { OUString("FontWorkHideForm"),         XATTR_FORMTXTHIDEFORM,      cppu::UnoType<bool>::get(),                        0,  0}, \
    { OUString("FontWorkShadowTransparence"),XATTR_FORMTXTSHDWTRANSP,   ::cppu::UnoType<sal_Int16>::get(),            0,  0},

#define SHADOW_PROPERTIES \
    { OUString(UNO_NAME_SHADOW),            SDRATTR_SHADOW,             cppu::UnoType<bool>::get(),    0,      0}, \
    { OUString(UNO_NAME_SHADOWCOLOR),       SDRATTR_SHADOWCOLOR,        ::cppu::UnoType<sal_Int32>::get(),    0,      0}, \
    { OUString(UNO_NAME_SHADOWTRANSPARENCE),SDRATTR_SHADOWTRANSPARENCE, ::cppu::UnoType<sal_Int16>::get(),    0,      0}, \
    { OUString(UNO_NAME_SHADOWXDIST),       SDRATTR_SHADOWXDIST,        ::cppu::UnoType<sal_Int32>::get(),    0,      SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_SHADOWYDIST),       SDRATTR_SHADOWYDIST,        ::cppu::UnoType<sal_Int32>::get(),    0,      SFX_METRIC_ITEM},

#define LINE_PROPERTIES_DEFAULTS\
    { OUString(UNO_NAME_LINECAP),           XATTR_LINECAP,          ::cppu::UnoType<com::sun::star::drawing::LineCap>::get(),     0,     0}, \
    { OUString(UNO_NAME_LINECOLOR),         XATTR_LINECOLOR,        ::cppu::UnoType<sal_Int32>::get() ,           0,     0}, \
    { OUString(UNO_NAME_LINEENDCENTER),     XATTR_LINEENDCENTER,    cppu::UnoType<bool>::get() ,           0,     0}, \
    { OUString(UNO_NAME_LINEENDWIDTH),      XATTR_LINEENDWIDTH,     ::cppu::UnoType<sal_Int32>::get() ,           0,     SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_LINEJOINT),         XATTR_LINEJOINT,        ::cppu::UnoType<com::sun::star::drawing::LineJoint>::get(),     0,     0}, \
    { OUString(UNO_NAME_LINESTARTCENTER),   XATTR_LINESTARTCENTER,  cppu::UnoType<bool>::get() ,           0,     0}, \
    { OUString(UNO_NAME_LINESTARTWIDTH),    XATTR_LINESTARTWIDTH,   ::cppu::UnoType<sal_Int32>::get() ,           0,     SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_LINESTYLE),         XATTR_LINESTYLE,        cppu::UnoType<css::drawing::LineStyle>::get() ,        0,     0},  \
    { OUString(UNO_NAME_LINETRANSPARENCE),  XATTR_LINETRANSPARENCE, ::cppu::UnoType<sal_Int16>::get() ,           0,     0}, \
    { OUString(UNO_NAME_LINEWIDTH),         XATTR_LINEWIDTH,        ::cppu::UnoType<sal_Int32>::get() ,           0,     SFX_METRIC_ITEM},

#define LINE_PROPERTIES \
    { OUString(UNO_NAME_LINEDASH),          XATTR_LINEDASH,         ::cppu::UnoType<com::sun::star::drawing::LineDash>::get() ,         0,     MID_LINEDASH},   \
    { OUString("LineDashName"),             XATTR_LINEDASH,         ::cppu::UnoType<OUString>::get() ,         0,     MID_NAME},   \
    LINE_PROPERTIES_DEFAULTS

#define LINE_PROPERTIES_START_END \
    { OUString(UNO_NAME_LINEEND),           XATTR_LINEEND,          ::cppu::UnoType<com::sun::star::drawing::PolyPolygonBezierCoords>::get(),   ::com::sun::star::beans::PropertyAttribute::MAYBEVOID,     0}, \
    { OUString("LineEndName"),              XATTR_LINEEND,          ::cppu::UnoType<OUString>::get(), 0, MID_NAME }, \
    { OUString(UNO_NAME_LINESTART),         XATTR_LINESTART,        ::cppu::UnoType<com::sun::star::drawing::PolyPolygonBezierCoords>::get(),   ::com::sun::star::beans::PropertyAttribute::MAYBEVOID,     0},  \
    { OUString("LineStartName"),            XATTR_LINESTART,        ::cppu::UnoType<OUString>::get(), 0, MID_NAME },

#define FILL_PROPERTIES_BMP \
    { OUString(UNO_NAME_FILLBMP_LOGICAL_SIZE),      XATTR_FILLBMP_SIZELOG,      cppu::UnoType<bool>::get() ,           0,     0}, \
    { OUString(UNO_NAME_FILLBMP_OFFSET_X),          XATTR_FILLBMP_TILEOFFSETX,  ::cppu::UnoType<sal_Int32>::get() ,           0,     0}, \
    { OUString(UNO_NAME_FILLBMP_OFFSET_Y),          XATTR_FILLBMP_TILEOFFSETY,  ::cppu::UnoType<sal_Int32>::get() ,           0,     0}, \
    { OUString(UNO_NAME_FILLBMP_POSITION_OFFSET_X), XATTR_FILLBMP_POSOFFSETX,   ::cppu::UnoType<sal_Int32>::get() ,           0,     0}, \
    { OUString(UNO_NAME_FILLBMP_POSITION_OFFSET_Y), XATTR_FILLBMP_POSOFFSETY,   ::cppu::UnoType<sal_Int32>::get() ,           0,     0}, \
    { OUString(UNO_NAME_FILLBMP_RECTANGLE_POINT),   XATTR_FILLBMP_POS,          ::cppu::UnoType<com::sun::star::drawing::RectanglePoint>::get() , 0,     0}, \
    { OUString(UNO_NAME_FILLBMP_SIZE_X),            XATTR_FILLBMP_SIZEX,        ::cppu::UnoType<sal_Int32>::get() ,           0,     SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_FILLBMP_SIZE_Y),            XATTR_FILLBMP_SIZEY,        ::cppu::UnoType<sal_Int32>::get() ,           0,     SFX_METRIC_ITEM},    \
    { OUString(UNO_NAME_FILLBMP_STRETCH),           XATTR_FILLBMP_STRETCH,      cppu::UnoType<bool>::get() ,           0,     0}, \
    { OUString(UNO_NAME_FILLBMP_TILE),              XATTR_FILLBMP_TILE,         cppu::UnoType<bool>::get() ,           0,     0},\
    { OUString(UNO_NAME_FILLBMP_MODE),              OWN_ATTR_FILLBMP_MODE,      ::cppu::UnoType<com::sun::star::drawing::BitmapMode>::get(),            0,     0},

#define FILL_PROPERTIES_DEFAULTS \
    { OUString(UNO_NAME_FILLCOLOR),         XATTR_FILLCOLOR, ::cppu::UnoType<sal_Int32>::get(),          0,     0}, \

#define FILL_PROPERTIES \
    FILL_PROPERTIES_BMP \
    FILL_PROPERTIES_DEFAULTS \
    { OUString(UNO_NAME_FILLBACKGROUND),    XATTR_FILLBACKGROUND    , cppu::UnoType<bool>::get(),              0,   0}, \
    { OUString(UNO_NAME_FILLBITMAP),        XATTR_FILLBITMAP        , cppu::UnoType<css::awt::XBitmap>::get()  ,       0,     MID_BITMAP}, \
    { OUString(UNO_NAME_FILLBITMAPNAME),           XATTR_FILLBITMAP        , ::cppu::UnoType<OUString>::get(), 0, MID_NAME }, \
    { OUString(UNO_NAME_FILLBITMAPURL),            XATTR_FILLBITMAP        , ::cppu::UnoType<OUString>::get(), 0, MID_GRAFURL }, \
    { OUString(UNO_NAME_FILLGRADIENTSTEPCOUNT),    XATTR_GRADIENTSTEPCOUNT , ::cppu::UnoType<sal_Int16>::get(), 0, 0}, \
    { OUString(UNO_NAME_FILLGRADIENT),      XATTR_FILLGRADIENT      , ::cppu::UnoType<com::sun::star::awt::Gradient>::get(),        0,     MID_FILLGRADIENT}, \
    { OUString(UNO_NAME_FILLGRADIENTNAME),         XATTR_FILLGRADIENT      , ::cppu::UnoType<OUString>::get(), 0, MID_NAME }, \
    { OUString(UNO_NAME_FILLHATCH),         XATTR_FILLHATCH         , ::cppu::UnoType<com::sun::star::drawing::Hatch>::get(),           0,     MID_FILLHATCH}, \
    { OUString(UNO_NAME_FILLHATCHNAME),            XATTR_FILLHATCH         , ::cppu::UnoType<OUString>::get(), 0, MID_NAME }, \
    { OUString(UNO_NAME_FILLSTYLE),         XATTR_FILLSTYLE         , ::cppu::UnoType<com::sun::star::drawing::FillStyle>::get() ,      0,     0}, \
    { OUString(UNO_NAME_FILL_TRANSPARENCE), XATTR_FILLTRANSPARENCE, ::cppu::UnoType<sal_Int16>::get() ,           0,     0}, \
    { OUString(UNO_NAME_FILLTRANSPARENCEGRADIENT), XATTR_FILLFLOATTRANSPARENCE, ::cppu::UnoType<com::sun::star::awt::Gradient>::get(),         0,     MID_FILLGRADIENT}, \
    { OUString(UNO_NAME_FILLTRANSPARENCEGRADIENTNAME), XATTR_FILLFLOATTRANSPARENCE, ::cppu::UnoType<OUString>::get(), 0, MID_NAME }, \
    { OUString(UNO_NAME_FILLCOLOR_2),       XATTR_SECONDARYFILLCOLOR, ::cppu::UnoType<sal_Int32>::get(),          0,     0}, \
    { OUString(UNO_NAME_GRAPHIC_GRAPHICCROP), SDRATTR_GRAFCROP      , ::cppu::UnoType<com::sun::star::text::GraphicCrop>::get(), 0, 0 },

#define EDGERADIUS_PROPERTIES \
    { OUString(UNO_NAME_EDGERADIUS),        SDRATTR_ECKENRADIUS     , ::cppu::UnoType<sal_Int32>::get()  ,            0,     SFX_METRIC_ITEM},

#define TEXT_PROPERTIES_DEFAULTS\
    { OUString(UNO_NAME_TEXT_WRITINGMODE),      SDRATTR_TEXTDIRECTION,          ::cppu::UnoType<com::sun::star::text::WritingMode>::get(),      0,      0},\
    { OUString(UNO_NAME_TEXT_ANIAMOUNT),        SDRATTR_TEXT_ANIAMOUNT,         ::cppu::UnoType<sal_Int16>::get(),        0,      0}, \
    { OUString(UNO_NAME_TEXT_ANICOUNT),         SDRATTR_TEXT_ANICOUNT,          ::cppu::UnoType<sal_Int16>::get(),        0,      0}, \
    { OUString(UNO_NAME_TEXT_ANIDELAY),         SDRATTR_TEXT_ANIDELAY,          ::cppu::UnoType<sal_Int16>::get(),        0,      0}, \
    { OUString(UNO_NAME_TEXT_ANIDIRECTION),     SDRATTR_TEXT_ANIDIRECTION,      cppu::UnoType<css::drawing::TextAnimationDirection>::get(),    0,      0}, \
    { OUString(UNO_NAME_TEXT_ANIKIND),          SDRATTR_TEXT_ANIKIND,           cppu::UnoType<css::drawing::TextAnimationKind>::get(), 0,      0}, \
    { OUString(UNO_NAME_TEXT_ANISTARTINSIDE),   SDRATTR_TEXT_ANISTARTINSIDE,    cppu::UnoType<bool>::get(),        0,      0}, \
    { OUString(UNO_NAME_TEXT_ANISTOPINSIDE),    SDRATTR_TEXT_ANISTOPINSIDE,     cppu::UnoType<bool>::get(),        0,      0}, \
    { OUString(UNO_NAME_TEXT_AUTOGROWHEIGHT),   SDRATTR_TEXT_AUTOGROWHEIGHT,    cppu::UnoType<bool>::get(),        0,      0}, \
    { OUString(UNO_NAME_TEXT_AUTOGROWWIDTH),    SDRATTR_TEXT_AUTOGROWWIDTH,     cppu::UnoType<bool>::get(),        0,      0}, \
    { OUString(UNO_NAME_TEXT_CONTOURFRAME),     SDRATTR_TEXT_CONTOURFRAME,      cppu::UnoType<bool>::get(),        0,      0}, \
    { OUString(UNO_NAME_TEXT_FITTOSIZE),        SDRATTR_TEXT_FITTOSIZE,         cppu::UnoType<css::drawing::TextFitToSizeType>::get(), 0,      0}, \
    { OUString(UNO_NAME_TEXT_HORZADJUST),       SDRATTR_TEXT_HORZADJUST,        cppu::UnoType<css::drawing::TextHorizontalAdjust>::get(),  0,      0}, \
    { OUString(UNO_NAME_TEXT_LEFTDIST),         SDRATTR_TEXT_LEFTDIST,          ::cppu::UnoType<sal_Int32>::get(),        0,      SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_TEXT_LOWERDIST),        SDRATTR_TEXT_LOWERDIST,         ::cppu::UnoType<sal_Int32>::get(),        0,      SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_TEXT_MAXFRAMEHEIGHT),   SDRATTR_TEXT_MAXFRAMEHEIGHT,    ::cppu::UnoType<sal_Int32>::get(),        0,      SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_TEXT_MAXFRAMEWIDTH),    SDRATTR_TEXT_MAXFRAMEWIDTH,     ::cppu::UnoType<sal_Int32>::get(),        0,      SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_TEXT_MINFRAMEHEIGHT),   SDRATTR_TEXT_MINFRAMEHEIGHT,    ::cppu::UnoType<sal_Int32>::get(),        0,      SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_TEXT_MINFRAMEWIDTH),    SDRATTR_TEXT_MINFRAMEWIDTH,     ::cppu::UnoType<sal_Int32>::get(),        0,      SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_TEXT_RIGHTDIST),        SDRATTR_TEXT_RIGHTDIST,         ::cppu::UnoType<sal_Int32>::get(),        0,      SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_TEXT_UPPERDIST),        SDRATTR_TEXT_UPPERDIST,         ::cppu::UnoType<sal_Int32>::get(),        0,      SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_TEXT_FONTINDEPENDENTLINESPACING),SDRATTR_TEXT_USEFIXEDCELLHEIGHT,cppu::UnoType<bool>::get(),                   0,      0},               \
    { OUString(UNO_NAME_TEXT_VERTADJUST),       SDRATTR_TEXT_VERTADJUST,        cppu::UnoType<css::drawing::TextVerticalAdjust>::get(),    0,      0},\
    { OUString(UNO_NAME_TEXT_WORDWRAP),         SDRATTR_TEXT_WORDWRAP,          cppu::UnoType<bool>::get(),        0,      0}, \
    { OUString(UNO_NAME_TEXT_CHAINNEXTNAME),    SDRATTR_TEXT_CHAINNEXTNAME,     ::cppu::UnoType<OUString>::get(),        0,      0}, \
    SVX_UNOEDIT_CHAR_PROPERTIES, \
    SVX_UNOEDIT_PARA_PROPERTIES,

#define TEXT_PROPERTIES \
    SVX_UNOEDIT_NUMBERING_PROPERTIE, \
    TEXT_PROPERTIES_DEFAULTS

//  { "HasLevels",                    OWN_ATTR_HASLEVELS,             cppu::UnoType<bool>::get(), ::com::sun::star::beans::PropertyAttribute::READONLY,      0},

#define MISC_OBJ_PROPERTIES_NO_SHEAR \
    { OUString("Transformation"),                   OWN_ATTR_TRANSFORMATION,    cppu::UnoType<css::drawing::HomogenMatrix3>::get(), 0, 0 }, \
    { OUString(UNO_NAME_MISC_OBJ_ZORDER),           OWN_ATTR_ZORDER,            ::cppu::UnoType<sal_Int32>::get(),        0,      0}, \
    { OUString(UNO_NAME_MISC_OBJ_FRAMERECT),        OWN_ATTR_FRAMERECT,         cppu::UnoType<css::awt::Rectangle>::get(), 0,  0 }, \
    { OUString(UNO_NAME_MISC_OBJ_ROTATEANGLE),      SDRATTR_ROTATEANGLE,        ::cppu::UnoType<sal_Int32>::get(),        0,  0}, \
    { OUString(UNO_NAME_BITMAP),                    OWN_ATTR_BITMAP,            cppu::UnoType<css::awt::XBitmap>::get(),   ::com::sun::star::beans::PropertyAttribute::READONLY, 0}, \
    { OUString(UNO_NAME_OLE2_METAFILE),             OWN_ATTR_METAFILE,          cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(), ::com::sun::star::beans::PropertyAttribute::READONLY, 0}, \
    { OUString("IsFontwork"),                       OWN_ATTR_ISFONTWORK,        cppu::UnoType<bool>::get(), ::com::sun::star::beans::PropertyAttribute::READONLY, 0}, \
    { OUString(UNO_NAME_MISC_OBJ_BOUNDRECT),        OWN_ATTR_BOUNDRECT,         cppu::UnoType<css::awt::Rectangle>::get(), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},

#define MISC_OBJ_PROPERTIES \
    { OUString(UNO_NAME_MISC_OBJ_INTEROPGRABBAG),   OWN_ATTR_INTEROPGRABBAG,    cppu::UnoType<css::uno::Sequence< ::com::sun::star::beans::PropertyValue >>::get(),  0,  0}, \
    MISC_OBJ_PROPERTIES_NO_SHEAR \
    { OUString(UNO_NAME_MISC_OBJ_SHEARANGLE),       SDRATTR_SHEARANGLE,         ::cppu::UnoType<sal_Int32>::get(),        0,  0},


#define SHAPE_DESCRIPTOR_PROPERTIES \
    { OUString(UNO_NAME_MISC_OBJ_LAYERID),      SDRATTR_LAYERID                 , ::cppu::UnoType<sal_Int16>::get(),          0,  0}, \
    { OUString(UNO_NAME_MISC_OBJ_LAYERNAME),    SDRATTR_LAYERNAME               , ::cppu::UnoType<OUString>::get(),    0,  0}, \
    { OUString(UNO_NAME_MISC_OBJ_MOVEPROTECT),  SDRATTR_OBJMOVEPROTECT          , cppu::UnoType<bool>::get(),                      0,  0}, \
    { OUString(UNO_NAME_MISC_OBJ_NAME),         SDRATTR_OBJECTNAME              , ::cppu::UnoType<OUString>::get(),    0,  0}, \
    { OUString(UNO_NAME_MISC_OBJ_PRINTABLE),    SDRATTR_OBJPRINTABLE            , cppu::UnoType<bool>::get(),                      0,  0}, \
    { OUString("Visible"),                      SDRATTR_OBJVISIBLE              , cppu::UnoType<bool>::get(),                      0,  0}, \
    { OUString(UNO_NAME_MISC_OBJ_SIZEPROTECT),  SDRATTR_OBJSIZEPROTECT          , cppu::UnoType<bool>::get(),                      0,  0},\
    { OUString("UINameSingular"),               OWN_ATTR_UINAME_SINGULAR        , ::cppu::UnoType<OUString>::get(),    ::com::sun::star::beans::PropertyAttribute::READONLY,   0}, \
    { OUString("UINamePlural"),                 OWN_ATTR_UINAME_PLURAL          , ::cppu::UnoType<OUString>::get(),    ::com::sun::star::beans::PropertyAttribute::READONLY,   0}, \
    /* #i68101# */ \
    { OUString(UNO_NAME_MISC_OBJ_TITLE),        OWN_ATTR_MISC_OBJ_TITLE         , ::cppu::UnoType<OUString>::get(),    0,  0}, \
    { OUString(UNO_NAME_MISC_OBJ_DESCRIPTION),  OWN_ATTR_MISC_OBJ_DESCRIPTION   , ::cppu::UnoType<OUString>::get(),    0,  0},

#define LINKTARGET_PROPERTIES \
    { OUString(UNO_NAME_LINKDISPLAYNAME),   OWN_ATTR_LDNAME             , ::cppu::UnoType<OUString>::get(),    ::com::sun::star::beans::PropertyAttribute::READONLY, 0}, \
    { OUString(UNO_NAME_LINKDISPLAYBITMAP), OWN_ATTR_LDBITMAP               , cppu::UnoType<css::awt::XBitmap>::get(), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},


#define CONNECTOR_PROPERTIES \
    { OUString(UNO_NAME_EDGEKIND),          SDRATTR_EDGEKIND,           ::cppu::UnoType<com::sun::star::drawing::ConnectorType>::get(),     0,     0}, \
    { OUString(UNO_NAME_EDGENODE1HORZDIST), SDRATTR_EDGENODE1HORZDIST,  ::cppu::UnoType<sal_Int32>::get(),       0,     0}, \
    { OUString(UNO_NAME_EDGENODE1VERTDIST), SDRATTR_EDGENODE1VERTDIST,  ::cppu::UnoType<sal_Int32>::get(),       0,     0}, \
    { OUString(UNO_NAME_EDGENODE2HORZDIST), SDRATTR_EDGENODE2HORZDIST,  ::cppu::UnoType<sal_Int32>::get(),       0,     0}, \
    { OUString(UNO_NAME_EDGENODE2VERTDIST), SDRATTR_EDGENODE2VERTDIST,  ::cppu::UnoType<sal_Int32>::get(),       0,     0},

#define SPECIAL_CONNECTOR_PROPERTIES \
    CONNECTOR_PROPERTIES \
    { OUString(UNO_NAME_EDGELINE1DELTA),    SDRATTR_EDGELINE1DELTA,     ::cppu::UnoType<sal_Int32>::get(),       0,     SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_EDGELINE2DELTA),    SDRATTR_EDGELINE2DELTA,     ::cppu::UnoType<sal_Int32>::get(),       0,     SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_EDGELINE3DELTA),    SDRATTR_EDGELINE3DELTA,     ::cppu::UnoType<sal_Int32>::get(),       0,     SFX_METRIC_ITEM}, \
    { OUString("StartShape"),               OWN_ATTR_EDGE_START_OBJ,    cppu::UnoType<css::drawing::XShape>::get(),        ::com::sun::star::beans::PropertyAttribute::MAYBEVOID,   0}, \
    { OUString("StartGluePointIndex"),      OWN_ATTR_GLUEID_HEAD,       ::cppu::UnoType<sal_Int32>::get(),       0,     0}, \
    { OUString("StartPosition"),            OWN_ATTR_EDGE_START_POS,    ::cppu::UnoType<com::sun::star::awt::Point>::get(),     0,   0}, \
    { OUString("EndShape"),                 OWN_ATTR_EDGE_END_OBJ,      cppu::UnoType<css::drawing::XShape>::get(),     ::com::sun::star::beans::PropertyAttribute::MAYBEVOID,   0}, \
    { OUString("EndPosition"),              OWN_ATTR_EDGE_END_POS,      ::cppu::UnoType<com::sun::star::awt::Point>::get(),     0,   0},\
    { OUString("EndGluePointIndex"),        OWN_ATTR_GLUEID_TAIL,       ::cppu::UnoType<sal_Int32>::get(),       0,     0}, \
\
    { OUString("EdgeStartConnection"),      OWN_ATTR_EDGE_START_OBJ,    cppu::UnoType<css::drawing::XShape>::get(),        ::com::sun::star::beans::PropertyAttribute::MAYBEVOID,   0}, \
    { OUString("EdgeStartPoint"),           OWN_ATTR_EDGE_START_POS,    ::cppu::UnoType<com::sun::star::awt::Point>::get(),     ::com::sun::star::beans::PropertyAttribute::READONLY,    0}, \
    { OUString("EdgeEndConnection"),        OWN_ATTR_EDGE_END_OBJ,      cppu::UnoType<css::drawing::XShape>::get(),     ::com::sun::star::beans::PropertyAttribute::MAYBEVOID,   0}, \
    { OUString("EdgeEndPoint"),             OWN_ATTR_EDGE_END_POS,      ::cppu::UnoType<com::sun::star::awt::Point>::get(),     ::com::sun::star::beans::PropertyAttribute::READONLY,    0}, \
\
    { OUString(UNO_NAME_POLYPOLYGONBEZIER), OWN_ATTR_EDGE_POLYPOLYGONBEZIER,    ::cppu::UnoType<com::sun::star::drawing::PolyPolygonBezierCoords>::get(),       0,  0},

#define SPECIAL_DIMENSIONING_PROPERTIES_DEFAULTS \
    { OUString(UNO_NAME_MEASUREBELOWREFEDGE),       SDRATTR_MEASUREBELOWREFEDGE,        cppu::UnoType<bool>::get(),        0,  0}, \
    { OUString(UNO_NAME_MEASUREFORMATSTRING),       SDRATTR_MEASUREFORMATSTRING,        ::cppu::UnoType<OUString>::get(),      0,  0}, \
    { OUString(UNO_NAME_MEASUREHELPLINE1LEN),       SDRATTR_MEASUREHELPLINE1LEN,        ::cppu::UnoType<sal_Int32>::get(),        0,  SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_MEASUREHELPLINE2LEN),       SDRATTR_MEASUREHELPLINE2LEN,        ::cppu::UnoType<sal_Int32>::get(),        0,  SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_MEASUREHELPLINEDIST),       SDRATTR_MEASUREHELPLINEDIST,        ::cppu::UnoType<sal_Int32>::get(),        0,  SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_MEASUREHELPLINEOVERHANG),   SDRATTR_MEASUREHELPLINEOVERHANG,    ::cppu::UnoType<sal_Int32>::get(),        0,  SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_MEASUREKIND),               SDRATTR_MEASUREKIND,                ::cppu::UnoType<com::sun::star::drawing::MeasureKind>::get(),   0,  0}, \
    { OUString(UNO_NAME_MEASURELINEDIST),           SDRATTR_MEASURELINEDIST,            ::cppu::UnoType<sal_Int32>::get(),        0,  SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_MEASUREOVERHANG),           SDRATTR_MEASUREOVERHANG,            ::cppu::UnoType<sal_Int32>::get(),        0,  SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_MEASUREUNIT),               SDRATTR_MEASUREUNIT,                ::cppu::UnoType<sal_Int32>::get(),    0,  0},\
    { OUString(UNO_NAME_MEASURESHOWUNIT),           SDRATTR_MEASURESHOWUNIT,            cppu::UnoType<bool>::get(),        0,  0}, \
    { OUString(UNO_NAME_MEASURETEXTAUTOANGLE),      SDRATTR_MEASURETEXTAUTOANGLE,       cppu::UnoType<bool>::get(),        0,  0}, \
    { OUString(UNO_NAME_MEASURETEXTAUTOANGLEVIEW),  SDRATTR_MEASURETEXTAUTOANGLEVIEW,   ::cppu::UnoType<sal_Int32>::get(),        0,  0}, \
    { OUString(UNO_NAME_MEASURETEXTFIXEDANGLE),     SDRATTR_MEASURETEXTFIXEDANGLE,      ::cppu::UnoType<sal_Int32>::get(),        0,  0}, \
    { OUString(UNO_NAME_MEASURETEXTHPOS),           SDRATTR_MEASURETEXTHPOS,            ::cppu::UnoType<com::sun::star::drawing::MeasureTextHorzPos>::get(),    0,  0}, \
    { OUString(UNO_NAME_MEASURETEXTISFIXEDANGLE),   SDRATTR_MEASURETEXTISFIXEDANGLE,    cppu::UnoType<bool>::get(),        0,  0}, \
    { OUString(UNO_NAME_MEASURETEXTROTA90),         SDRATTR_MEASURETEXTROTA90,          cppu::UnoType<bool>::get(),        0,  0}, \
    { OUString(UNO_NAME_MEASURETEXTUPSIDEDOWN),     SDRATTR_MEASURETEXTUPSIDEDOWN,      cppu::UnoType<bool>::get(),        0,  0}, \
    { OUString(UNO_NAME_MEASURETEXTVPOS),           SDRATTR_MEASURETEXTVPOS,            ::cppu::UnoType<com::sun::star::drawing::MeasureTextVertPos>::get(),    0,  0}, \
    { OUString("MeasureDecimalPlaces"),             SDRATTR_MEASUREDECIMALPLACES,       ::cppu::UnoType<sal_Int16>::get(),        0,  0},


#define SPECIAL_DIMENSIONING_PROPERTIES \
    SPECIAL_DIMENSIONING_PROPERTIES_DEFAULTS \
    { OUString("StartPosition"),                    OWN_ATTR_MEASURE_START_POS,         ::cppu::UnoType<com::sun::star::awt::Point>::get(),     0,   0},\
    { OUString("EndPosition"),                      OWN_ATTR_MEASURE_END_POS,           ::cppu::UnoType<com::sun::star::awt::Point>::get(),     0,   0},

#define SPECIAL_CIRCLE_PROPERTIES \
    { OUString(UNO_NAME_CIRCENDANGLE),  SDRATTR_CIRCENDANGLE,   ::cppu::UnoType<sal_Int32>::get(),        0,  0}, \
    { OUString(UNO_NAME_CIRCKIND),      SDRATTR_CIRCKIND,       ::cppu::UnoType<com::sun::star::drawing::CircleKind>::get(),    0,  0}, \
    { OUString(UNO_NAME_CIRCSTARTANGLE),SDRATTR_CIRCSTARTANGLE, ::cppu::UnoType<sal_Int32>::get(),        0,  0},

#define SPECIAL_POLYGON_PROPERTIES \
    { OUString(UNO_NAME_POLYGONKIND),   OWN_ATTR_VALUE_POLYGONKIND, ::cppu::UnoType<com::sun::star::drawing::PolygonKind>::get(), ::com::sun::star::beans::PropertyAttribute::READONLY, 0},

#define SPECIAL_POLYPOLYGON_PROPERTIES \
    { OUString(UNO_NAME_POLYPOLYGON),   OWN_ATTR_VALUE_POLYPOLYGON, ::cppu::UnoType<com::sun::star::drawing::PointSequenceSequence>::get(), 0,  0}, \
    { OUString(UNO_NAME_POLYGON),       OWN_ATTR_VALUE_POLYGON,     ::cppu::UnoType<com::sun::star::drawing::PointSequence>::get(),         0,  0},

#define SPECIAL_POLYPOLYGONBEZIER_PROPERTIES \
    { OUString(UNO_NAME_POLYPOLYGONBEZIER), OWN_ATTR_VALUE_POLYPOLYGONBEZIER,   ::cppu::UnoType<com::sun::star::drawing::PolyPolygonBezierCoords>::get(),       0,  0},

#define SPECIAL_GRAPHOBJ_PROPERTIES_DEFAULTS \
    { OUString(UNO_NAME_GRAPHIC_LUMINANCE),     SDRATTR_GRAFLUMINANCE       , ::cppu::UnoType<sal_Int16>::get(),          0,   0}, \
    { OUString(UNO_NAME_GRAPHIC_CONTRAST),      SDRATTR_GRAFCONTRAST        , ::cppu::UnoType<sal_Int16>::get(),          0,   0}, \
    { OUString(UNO_NAME_GRAPHIC_RED),           SDRATTR_GRAFRED             , ::cppu::UnoType<sal_Int16>::get(),          0,   0}, \
    { OUString(UNO_NAME_GRAPHIC_GREEN),         SDRATTR_GRAFGREEN           , ::cppu::UnoType<sal_Int16>::get(),          0,   0}, \
    { OUString(UNO_NAME_GRAPHIC_BLUE),          SDRATTR_GRAFBLUE            , ::cppu::UnoType<sal_Int16>::get(),          0,   0}, \
    { OUString(UNO_NAME_GRAPHIC_GAMMA),         SDRATTR_GRAFGAMMA           , cppu::UnoType<double>::get(),     0,   0}, \
    { OUString(UNO_NAME_GRAPHIC_TRANSPARENCY),  SDRATTR_GRAFTRANSPARENCE    , ::cppu::UnoType<sal_Int16>::get(),          0,   0}, \
    { OUString(UNO_NAME_GRAPHIC_COLOR_MODE),    SDRATTR_GRAFMODE            , ::cppu::UnoType<com::sun::star::drawing::ColorMode>::get(),       0,   0},

#define SPECIAL_GRAPHOBJ_PROPERTIES \
    SPECIAL_GRAPHOBJ_PROPERTIES_DEFAULTS \
    { OUString(UNO_NAME_GRAPHIC_GRAPHICCROP),           SDRATTR_GRAFCROP            , ::cppu::UnoType<com::sun::star::text::GraphicCrop>::get(),        0,  0 }, \
    { OUString(UNO_NAME_GRAPHOBJ_GRAFURL),              OWN_ATTR_GRAFURL            , ::cppu::UnoType<OUString>::get(), 0, 0 }, \
    { OUString(UNO_NAME_GRAPHOBJ_REPLACEMENTGRAFURL),   OWN_ATTR_REPLACEMENTGRAFURL , ::cppu::UnoType<OUString>::get(), 0, 0 }, \
    { OUString(UNO_NAME_GRAPHOBJ_GRAFSTREAMURL),        OWN_ATTR_GRAFSTREAMURL      , ::cppu::UnoType<OUString>::get(), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 }, \
    { OUString(UNO_NAME_GRAPHOBJ_FILLBITMAP),           OWN_ATTR_VALUE_FILLBITMAP   , cppu::UnoType<css::awt::XBitmap>::get()  ,    0,     0},    \
    { OUString(UNO_NAME_GRAPHOBJ_GRAPHIC),              OWN_ATTR_VALUE_GRAPHIC      , cppu::UnoType<css::graphic::XGraphic>::get()  ,   0,     0},


#define SPECIAL_3DSCENEOBJECT_PROPERTIES_DEFAULTS \
    { OUString(UNO_NAME_3D_SCENE_AMBIENTCOLOR),     SDRATTR_3DSCENE_AMBIENTCOLOR   , ::cppu::UnoType<sal_Int32>::get(),   0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_DISTANCE),         SDRATTR_3DSCENE_DISTANCE       , ::cppu::UnoType<sal_Int32>::get(),   0,  SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_3D_SCENE_FOCAL_LENGTH),     SDRATTR_3DSCENE_FOCAL_LENGTH   , ::cppu::UnoType<sal_Int32>::get(),   0,  SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTCOLOR_1),     SDRATTR_3DSCENE_LIGHTCOLOR_1       , ::cppu::UnoType<sal_Int32>::get(),   0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTCOLOR_2),     SDRATTR_3DSCENE_LIGHTCOLOR_2       , ::cppu::UnoType<sal_Int32>::get(),   0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTCOLOR_3),     SDRATTR_3DSCENE_LIGHTCOLOR_3       , ::cppu::UnoType<sal_Int32>::get(),   0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTCOLOR_4),     SDRATTR_3DSCENE_LIGHTCOLOR_4       , ::cppu::UnoType<sal_Int32>::get(),   0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTCOLOR_5),     SDRATTR_3DSCENE_LIGHTCOLOR_5       , ::cppu::UnoType<sal_Int32>::get(),   0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTCOLOR_6),     SDRATTR_3DSCENE_LIGHTCOLOR_6       , ::cppu::UnoType<sal_Int32>::get(),   0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTCOLOR_7),     SDRATTR_3DSCENE_LIGHTCOLOR_7       , ::cppu::UnoType<sal_Int32>::get(),   0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTCOLOR_8),     SDRATTR_3DSCENE_LIGHTCOLOR_8       , ::cppu::UnoType<sal_Int32>::get(),   0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTDIRECTION_1), SDRATTR_3DSCENE_LIGHTDIRECTION_1   , ::cppu::UnoType<com::sun::star::drawing::Direction3D>::get(),  0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTDIRECTION_2), SDRATTR_3DSCENE_LIGHTDIRECTION_2   , ::cppu::UnoType<com::sun::star::drawing::Direction3D>::get(),  0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTDIRECTION_3), SDRATTR_3DSCENE_LIGHTDIRECTION_3   , ::cppu::UnoType<com::sun::star::drawing::Direction3D>::get(),  0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTDIRECTION_4), SDRATTR_3DSCENE_LIGHTDIRECTION_4   , ::cppu::UnoType<com::sun::star::drawing::Direction3D>::get(),  0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTDIRECTION_5), SDRATTR_3DSCENE_LIGHTDIRECTION_5   , ::cppu::UnoType<com::sun::star::drawing::Direction3D>::get(),  0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTDIRECTION_6), SDRATTR_3DSCENE_LIGHTDIRECTION_6   , ::cppu::UnoType<com::sun::star::drawing::Direction3D>::get(),  0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTDIRECTION_7), SDRATTR_3DSCENE_LIGHTDIRECTION_7   , ::cppu::UnoType<com::sun::star::drawing::Direction3D>::get(),  0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTDIRECTION_8), SDRATTR_3DSCENE_LIGHTDIRECTION_8   , ::cppu::UnoType<com::sun::star::drawing::Direction3D>::get(),  0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTON_1),        SDRATTR_3DSCENE_LIGHTON_1          , cppu::UnoType<bool>::get(),   0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTON_2),        SDRATTR_3DSCENE_LIGHTON_2          , cppu::UnoType<bool>::get(),   0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTON_3),        SDRATTR_3DSCENE_LIGHTON_3          , cppu::UnoType<bool>::get(),   0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTON_4),        SDRATTR_3DSCENE_LIGHTON_4          , cppu::UnoType<bool>::get(),   0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTON_5),        SDRATTR_3DSCENE_LIGHTON_5          , cppu::UnoType<bool>::get(),   0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTON_6),        SDRATTR_3DSCENE_LIGHTON_6          , cppu::UnoType<bool>::get(),   0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTON_7),        SDRATTR_3DSCENE_LIGHTON_7          , cppu::UnoType<bool>::get(),   0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_LIGHTON_8),        SDRATTR_3DSCENE_LIGHTON_8          , cppu::UnoType<bool>::get(),   0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_PERSPECTIVE),      SDRATTR_3DSCENE_PERSPECTIVE    , cppu::UnoType<css::drawing::ProjectionMode>::get(),   0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_SHADOW_SLANT),     SDRATTR_3DSCENE_SHADOW_SLANT   , ::cppu::UnoType<sal_Int16>::get(),   0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_SHADE_MODE),       SDRATTR_3DSCENE_SHADE_MODE     , ::cppu::UnoType<com::sun::star::drawing::ShadeMode>::get(),    0,  0}, \
    { OUString(UNO_NAME_3D_SCENE_TWO_SIDED_LIGHTING),SDRATTR_3DSCENE_TWO_SIDED_LIGHTING, cppu::UnoType<bool>::get(),   0,  0},

#define SPECIAL_3DSCENEOBJECT_PROPERTIES \
    SPECIAL_3DSCENEOBJECT_PROPERTIES_DEFAULTS \
    { OUString(UNO_NAME_3D_TRANSFORM_MATRIX),       OWN_ATTR_3D_VALUE_TRANSFORM_MATRIX  , ::cppu::UnoType<com::sun::star::drawing::HomogenMatrix>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_CAMERA_GEOMETRY),        OWN_ATTR_3D_VALUE_CAMERA_GEOMETRY   , ::cppu::UnoType<com::sun::star::drawing::CameraGeometry>::get(), 0, 0},

// #i28528#
// Added extra Item (Bool) for chart2 to be able to show reduced line geometry (SDRATTR_3DOBJ_REDUCED_LINE_GEOMETRY)
#define MISC_3D_OBJ_PROPERTIES \
    { OUString(UNO_NAME_3D_DOUBLE_SIDED)             ,SDRATTR_3DOBJ_DOUBLE_SIDED            , cppu::UnoType<bool>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_MAT_COLOR)                ,SDRATTR_3DOBJ_MAT_COLOR               , ::cppu::UnoType<sal_Int32>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_MAT_EMISSION)             ,SDRATTR_3DOBJ_MAT_EMISSION            , ::cppu::UnoType<sal_Int32>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_MAT_SPECULAR)             ,SDRATTR_3DOBJ_MAT_SPECULAR            , ::cppu::UnoType<sal_Int32>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_MAT_SPECULAR_INTENSITY)   ,SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY  , ::cppu::UnoType<sal_Int16>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_NORMALS_INVERT)           ,SDRATTR_3DOBJ_NORMALS_INVERT          , cppu::UnoType<bool>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_NORMALS_KIND)             ,SDRATTR_3DOBJ_NORMALS_KIND            , ::cppu::UnoType<com::sun::star::drawing::NormalsKind>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_SHADOW_3D)                ,SDRATTR_3DOBJ_SHADOW_3D               , cppu::UnoType<bool>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_TEXTURE_FILTER)           ,SDRATTR_3DOBJ_TEXTURE_FILTER          , cppu::UnoType<bool>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_TEXTURE_KIND)             ,SDRATTR_3DOBJ_TEXTURE_KIND            , ::cppu::UnoType<com::sun::star::drawing::TextureKind>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_TEXTURE_MODE)             ,SDRATTR_3DOBJ_TEXTURE_MODE            , ::cppu::UnoType<com::sun::star::drawing::TextureMode>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_TEXTURE_PROJ_X)           ,SDRATTR_3DOBJ_TEXTURE_PROJ_X          , ::cppu::UnoType<com::sun::star::drawing::TextureProjectionMode>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_TEXTURE_PROJ_Y)           ,SDRATTR_3DOBJ_TEXTURE_PROJ_Y          , ::cppu::UnoType<com::sun::star::drawing::TextureProjectionMode>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_REDUCED_LINE_GEOMETRY)    ,SDRATTR_3DOBJ_REDUCED_LINE_GEOMETRY   , cppu::UnoType<bool>::get(), 0, 0},

#define SPECIAL_3DCUBEOBJECT_PROPERTIES \
    { OUString(UNO_NAME_3D_TRANSFORM_MATRIX) ,OWN_ATTR_3D_VALUE_TRANSFORM_MATRIX    , ::cppu::UnoType<com::sun::star::drawing::HomogenMatrix>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_POS)              ,OWN_ATTR_3D_VALUE_POSITION            , cppu::UnoType<css::drawing::Position3D>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_SIZE)             ,OWN_ATTR_3D_VALUE_SIZE                , ::cppu::UnoType<com::sun::star::drawing::Direction3D>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_POS_IS_CENTER)    ,OWN_ATTR_3D_VALUE_POS_IS_CENTER       , cppu::UnoType<bool>::get(), 0, 0},

#define SPECIAL_3DSPHEREOBJECT_PROPERTIES \
    { OUString(UNO_NAME_3D_TRANSFORM_MATRIX) ,OWN_ATTR_3D_VALUE_TRANSFORM_MATRIX    , ::cppu::UnoType<com::sun::star::drawing::HomogenMatrix>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_POS)              ,OWN_ATTR_3D_VALUE_POSITION            , cppu::UnoType<css::drawing::Position3D>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_SIZE)             ,OWN_ATTR_3D_VALUE_SIZE                , ::cppu::UnoType<com::sun::star::drawing::Direction3D>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_HORZ_SEGS)        ,SDRATTR_3DOBJ_HORZ_SEGS, ::cppu::UnoType<sal_Int32>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_VERT_SEGS)        ,SDRATTR_3DOBJ_VERT_SEGS, ::cppu::UnoType<sal_Int32>::get(), 0, 0},

// #107245# New 3D properties which are possible for lathe and extrude 3d objects
#define SPECIAL_3DLATHEANDEXTRUDEOBJ_PROPERTIES \
    { OUString(UNO_NAME_3D_SMOOTH_NORMALS)   , SDRATTR_3DOBJ_SMOOTH_NORMALS,    cppu::UnoType<bool>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_SMOOTH_LIDS)      , SDRATTR_3DOBJ_SMOOTH_LIDS,       cppu::UnoType<bool>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_CHARACTER_MODE)   , SDRATTR_3DOBJ_CHARACTER_MODE,    cppu::UnoType<bool>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_CLOSE_FRONT)      , SDRATTR_3DOBJ_CLOSE_FRONT,       cppu::UnoType<bool>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_CLOSE_BACK)       , SDRATTR_3DOBJ_CLOSE_BACK,        cppu::UnoType<bool>::get(), 0, 0},

#define SPECIAL_3DLATHEOBJECT_PROPERTIES \
    { OUString(UNO_NAME_3D_TRANSFORM_MATRIX) ,OWN_ATTR_3D_VALUE_TRANSFORM_MATRIX,::cppu::UnoType<com::sun::star::drawing::HomogenMatrix>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_POLYPOLYGON3D)    ,OWN_ATTR_3D_VALUE_POLYPOLYGON3D   ,::cppu::UnoType<com::sun::star::drawing::PolyPolygonShape3D>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_LATHE_END_ANGLE)  ,SDRATTR_3DOBJ_END_ANGLE           ,::cppu::UnoType<sal_Int16>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_HORZ_SEGS)        ,SDRATTR_3DOBJ_HORZ_SEGS           ,::cppu::UnoType<sal_Int32>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_PERCENT_DIAGONAL) ,SDRATTR_3DOBJ_PERCENT_DIAGONAL    ,::cppu::UnoType<sal_Int16>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_VERT_SEGS)        ,SDRATTR_3DOBJ_VERT_SEGS           ,::cppu::UnoType<sal_Int32>::get(), 0, 0},

#define SPECIAL_3DEXTRUDEOBJECT_PROPERTIES \
    { OUString(UNO_NAME_3D_TRANSFORM_MATRIX) ,OWN_ATTR_3D_VALUE_TRANSFORM_MATRIX,::cppu::UnoType<com::sun::star::drawing::HomogenMatrix>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_POLYPOLYGON3D)    ,OWN_ATTR_3D_VALUE_POLYPOLYGON3D   ,::cppu::UnoType<com::sun::star::drawing::PolyPolygonShape3D>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_EXTRUDE_DEPTH)    ,SDRATTR_3DOBJ_DEPTH               ,::cppu::UnoType<sal_Int32>::get(), 0, SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_3D_PERCENT_DIAGONAL) ,SDRATTR_3DOBJ_PERCENT_DIAGONAL    ,::cppu::UnoType<sal_Int16>::get(), 0, 0},

#define SPECIAL_3DPOLYGONOBJECT_PROPERTIES \
    { OUString(UNO_NAME_3D_TRANSFORM_MATRIX) ,OWN_ATTR_3D_VALUE_TRANSFORM_MATRIX, ::cppu::UnoType<com::sun::star::drawing::HomogenMatrix>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_POLYPOLYGON3D)    ,OWN_ATTR_3D_VALUE_POLYPOLYGON3D   , ::cppu::UnoType<com::sun::star::drawing::PolyPolygonShape3D>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_NORMALSPOLYGON3D) ,OWN_ATTR_3D_VALUE_NORMALSPOLYGON3D, ::cppu::UnoType<com::sun::star::drawing::PolyPolygonShape3D>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_TEXTUREPOLYGON3D) ,OWN_ATTR_3D_VALUE_TEXTUREPOLYGON3D, ::cppu::UnoType<com::sun::star::drawing::PolyPolygonShape3D>::get(), 0, 0}, \
    { OUString(UNO_NAME_3D_LINEONLY)         ,OWN_ATTR_3D_VALUE_LINEONLY        , cppu::UnoType<bool>::get(), 0, 0},

#define SPECIAL_3DBACKSCALE_PROPERTIES \
    { OUString(UNO_NAME_3D_BACKSCALE)        ,SDRATTR_3DOBJ_BACKSCALE           , ::cppu::UnoType<sal_Int16>::get(), 0, 0}, \

#define CUSTOMSHAPE_PROPERTIES \
    { OUString(UNO_NAME_CUSTOMSHAPE_ADJUSTMENT),SDRATTR_CUSTOMSHAPE_ADJUSTMENT,         cppu::UnoType<css::uno::Sequence<sal_Int32>>::get(), 0, 0}, \

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
