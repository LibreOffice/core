# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,drawinglayercore))

$(eval $(call gb_Library_set_include,drawinglayercore,\
    $$(INCLUDE) \
    -I$(SRCDIR)/drawinglayer/inc \
))

$(eval $(call gb_Library_add_defs,drawinglayercore,\
    -DDRAWINGLAYERCORE_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_set_precompiled_header,drawinglayercore,drawinglayer/inc/pch/precompiled_drawinglayercore))

$(eval $(call gb_Library_use_sdk_api,drawinglayercore))

$(eval $(call gb_Library_use_externals,drawinglayercore,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_Library_use_libraries,drawinglayercore,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    i18nlangtag \
    sal \
    salhelper \
    svl \
    tl \
))

$(eval $(call gb_Library_add_exception_objects,drawinglayercore,\
    drawinglayer/source/attribute/lineattribute \
    drawinglayer/source/attribute/strokeattribute \
    drawinglayer/source/attribute/linestartendattribute \
    drawinglayer/source/attribute/fillgradientattribute \
    drawinglayer/source/attribute/fillhatchattribute \
    drawinglayer/source/attribute/fontattribute \
    drawinglayer/source/geometry/viewinformation2d \
    drawinglayer/source/texture/texture \
    drawinglayer/source/primitive2d/baseprimitive2d \
    drawinglayer/source/primitive2d/Primitive2DContainer \
    drawinglayer/source/primitive2d/Tools \
    drawinglayer/source/primitive2d/polygonprimitive2d \
    drawinglayer/source/primitive2d/PolyPolygonColorPrimitive2D \
    drawinglayer/source/primitive2d/PolyPolygonHairlinePrimitive2D \
    drawinglayer/source/primitive2d/PolyPolygonMarkerPrimitive2D \
    drawinglayer/source/primitive2d/PolyPolygonStrokePrimitive2D \
    drawinglayer/source/primitive2d/PolyPolygonGradientPrimitive2D \
    drawinglayer/source/primitive2d/PolyPolygonHatchPrimitive2D \
    drawinglayer/source/primitive2d/PolyPolygonSelectionPrimitive2D \
    drawinglayer/source/primitive2d/fillgradientprimitive2d \
    drawinglayer/source/primitive2d/maskprimitive2d \
    drawinglayer/source/primitive2d/groupprimitive2d \
    drawinglayer/source/primitive2d/fillhatchprimitive2d \
    drawinglayer/source/primitive2d/primitivetools2d \
    drawinglayer/source/primitive2d/pointarrayprimitive2d \
    drawinglayer/source/primitive2d/modifiedcolorprimitive2d \
    drawinglayer/source/primitive2d/bitmapprimitive2d \
    drawinglayer/source/primitive2d/unifiedtransparenceprimitive2d \
    drawinglayer/source/primitive2d/transparenceprimitive2d \
    drawinglayer/source/primitive2d/pagepreviewprimitive2d \
    drawinglayer/source/primitive2d/transformprimitive2d \
    drawinglayer/source/primitive2d/svggradientprimitive2d \
    drawinglayer/source/primitive2d/objectinfoprimitive2d \
    drawinglayer/source/primitive2d/textenumsprimitive2d \
))

#    drawinglayer/source/primitive2d/epsprimitive2d \
#    drawinglayer/source/primitive2d/markerarrayprimitive2d \
#    drawinglayer/source/primitive2d/fillgraphicprimitive2d \
#    drawinglayer/source/primitive2d/textdecoratedprimitive2d \
#    drawinglayer/source/primitive2d/textprimitive2d \


# vim: set noet sw=4 ts=4:
