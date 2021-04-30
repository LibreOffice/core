# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,drawinglayer))

$(eval $(call gb_Library_set_include,drawinglayer,\
    $$(INCLUDE) \
    -I$(SRCDIR)/drawinglayer/inc \
))

$(eval $(call gb_Library_add_defs,drawinglayer,\
    -DDRAWINGLAYER_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_set_precompiled_header,drawinglayer,drawinglayer/inc/pch/precompiled_drawinglayer))

$(eval $(call gb_Library_set_componentfile,drawinglayer,drawinglayer/drawinglayer))

$(eval $(call gb_Library_use_sdk_api,drawinglayer))

$(eval $(call gb_Library_use_externals,drawinglayer,\
	boost_headers \
	libxml2 \
))

ifneq ($(ENABLE_WASM_STRIP_CANVAS),TRUE)
$(eval $(call gb_Library_use_libraries,drawinglayer,\
    canvastools \
    cppcanvas \
))
endif

$(eval $(call gb_Library_use_libraries,drawinglayer,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    i18nlangtag \
    sal \
    salhelper \
    svl \
    svt \
    tk \
    tl \
    vcl \
))

$(eval $(call gb_Library_add_exception_objects,drawinglayer,\
    drawinglayer/source/animation/animationtiming \
    drawinglayer/source/attribute/fillgraphicattribute \
    drawinglayer/source/attribute/fillgradientattribute \
    drawinglayer/source/attribute/fillhatchattribute \
    drawinglayer/source/attribute/fontattribute \
    drawinglayer/source/attribute/lineattribute \
    drawinglayer/source/attribute/linestartendattribute \
    drawinglayer/source/attribute/materialattribute3d \
    drawinglayer/source/attribute/sdrallattribute3d \
    drawinglayer/source/attribute/sdrfillattribute \
    drawinglayer/source/attribute/sdrfillgraphicattribute \
    drawinglayer/source/attribute/sdrglowattribute \
    drawinglayer/source/attribute/sdrlightattribute3d \
    drawinglayer/source/attribute/sdrlightingattribute3d \
    drawinglayer/source/attribute/sdrlineattribute \
    drawinglayer/source/attribute/sdrlinestartendattribute \
    drawinglayer/source/attribute/sdrobjectattribute3d \
    drawinglayer/source/attribute/sdrsceneattribute3d \
    drawinglayer/source/attribute/sdrshadowattribute \
    drawinglayer/source/attribute/strokeattribute \
    drawinglayer/source/geometry/viewinformation2d \
    drawinglayer/source/geometry/viewinformation3d \
    drawinglayer/source/primitive2d/animatedprimitive2d \
    drawinglayer/source/primitive2d/backgroundcolorprimitive2d \
    drawinglayer/source/primitive2d/baseprimitive2d \
    drawinglayer/source/primitive2d/bitmapprimitive2d \
    drawinglayer/source/primitive2d/borderlineprimitive2d \
    drawinglayer/source/primitive2d/BufferedDecompositionPrimitive2D \
    drawinglayer/source/primitive2d/controlprimitive2d \
    drawinglayer/source/primitive2d/cropprimitive2d \
    drawinglayer/source/primitive2d/discretebitmapprimitive2d \
    drawinglayer/source/primitive2d/discreteshadowprimitive2d \
    drawinglayer/source/primitive2d/embedded3dprimitive2d \
    drawinglayer/source/primitive2d/epsprimitive2d \
    drawinglayer/source/primitive2d/fillgraphicprimitive2d \
    drawinglayer/source/primitive2d/fillgradientprimitive2d \
    drawinglayer/source/primitive2d/fillhatchprimitive2d \
    drawinglayer/source/primitive2d/glowprimitive2d \
    drawinglayer/source/primitive2d/graphicprimitivehelper2d \
    drawinglayer/source/primitive2d/graphicprimitive2d \
    drawinglayer/source/primitive2d/gridprimitive2d \
    drawinglayer/source/primitive2d/groupprimitive2d \
    drawinglayer/source/primitive2d/helplineprimitive2d \
    drawinglayer/source/primitive2d/hiddengeometryprimitive2d \
    drawinglayer/source/primitive2d/invertprimitive2d \
    drawinglayer/source/primitive2d/markerarrayprimitive2d \
    drawinglayer/source/primitive2d/maskprimitive2d \
    drawinglayer/source/primitive2d/mediaprimitive2d \
    drawinglayer/source/primitive2d/metafileprimitive2d \
    drawinglayer/source/primitive2d/modifiedcolorprimitive2d \
    drawinglayer/source/primitive2d/objectinfoprimitive2d \
    drawinglayer/source/primitive2d/pagehierarchyprimitive2d \
    drawinglayer/source/primitive2d/pagepreviewprimitive2d \
    drawinglayer/source/primitive2d/patternfillprimitive2d \
    drawinglayer/source/primitive2d/pointarrayprimitive2d \
    drawinglayer/source/primitive2d/polygonprimitive2d \
    drawinglayer/source/primitive2d/PolyPolygonHairlinePrimitive2D \
    drawinglayer/source/primitive2d/PolyPolygonMarkerPrimitive2D \
    drawinglayer/source/primitive2d/PolyPolygonStrokePrimitive2D \
    drawinglayer/source/primitive2d/PolyPolygonColorPrimitive2D \
    drawinglayer/source/primitive2d/PolyPolygonGradientPrimitive2D \
    drawinglayer/source/primitive2d/PolyPolygonHatchPrimitive2D \
    drawinglayer/source/primitive2d/PolyPolygonGraphicPrimitive2D \
    drawinglayer/source/primitive2d/PolyPolygonSelectionPrimitive2D \
    drawinglayer/source/primitive2d/primitivetools2d \
    drawinglayer/source/primitive2d/Primitive2DContainer \
    drawinglayer/source/primitive2d/sceneprimitive2d \
    drawinglayer/source/primitive2d/sdrdecompositiontools2d \
    drawinglayer/source/primitive2d/shadowprimitive2d \
    drawinglayer/source/primitive2d/softedgeprimitive2d \
    drawinglayer/source/primitive2d/structuretagprimitive2d \
    drawinglayer/source/primitive2d/svggradientprimitive2d \
    drawinglayer/source/primitive2d/textbreakuphelper \
    drawinglayer/source/primitive2d/textdecoratedprimitive2d \
    drawinglayer/source/primitive2d/texteffectprimitive2d \
    drawinglayer/source/primitive2d/textenumsprimitive2d \
    drawinglayer/source/primitive2d/texthierarchyprimitive2d \
    drawinglayer/source/primitive2d/textlayoutdevice \
    drawinglayer/source/primitive2d/textlineprimitive2d \
    drawinglayer/source/primitive2d/textprimitive2d \
    drawinglayer/source/primitive2d/textstrikeoutprimitive2d \
    drawinglayer/source/primitive2d/Tools \
    drawinglayer/source/primitive2d/transformprimitive2d \
    drawinglayer/source/primitive2d/transparenceprimitive2d \
    drawinglayer/source/primitive2d/unifiedtransparenceprimitive2d \
    drawinglayer/source/primitive2d/wallpaperprimitive2d \
    drawinglayer/source/primitive2d/wrongspellprimitive2d \
    drawinglayer/source/primitive3d/baseprimitive3d \
    drawinglayer/source/primitive3d/groupprimitive3d \
    drawinglayer/source/primitive3d/hatchtextureprimitive3d \
    drawinglayer/source/primitive3d/hiddengeometryprimitive3d \
    drawinglayer/source/primitive3d/modifiedcolorprimitive3d \
    drawinglayer/source/primitive3d/polygonprimitive3d \
    drawinglayer/source/primitive3d/polygontubeprimitive3d \
    drawinglayer/source/primitive3d/polypolygonprimitive3d \
    drawinglayer/source/primitive3d/sdrcubeprimitive3d \
    drawinglayer/source/primitive3d/sdrdecompositiontools3d \
    drawinglayer/source/primitive3d/sdrextrudelathetools3d \
    drawinglayer/source/primitive3d/sdrextrudeprimitive3d \
    drawinglayer/source/primitive3d/sdrlatheprimitive3d \
    drawinglayer/source/primitive3d/sdrpolypolygonprimitive3d \
    drawinglayer/source/primitive3d/sdrprimitive3d \
    drawinglayer/source/primitive3d/sdrsphereprimitive3d \
    drawinglayer/source/primitive3d/shadowprimitive3d \
    drawinglayer/source/primitive3d/textureprimitive3d \
    drawinglayer/source/primitive3d/transformprimitive3d \
    drawinglayer/source/primitive3d/Tools \
    drawinglayer/source/processor2d/baseprocessor2d \
    drawinglayer/source/processor2d/processor2dtools \
    drawinglayer/source/processor2d/contourextractor2d \
    drawinglayer/source/processor2d/getdigitlanguage \
    drawinglayer/source/processor2d/helperwrongspellrenderer \
    drawinglayer/source/processor2d/hittestprocessor2d \
    drawinglayer/source/processor2d/linegeometryextractor2d \
    drawinglayer/source/processor2d/objectinfoextractor2d \
    drawinglayer/source/processor2d/processorfromoutputdevice \
    drawinglayer/source/processor2d/textaspolygonextractor2d \
    drawinglayer/source/processor2d/vclhelperbufferdevice \
    drawinglayer/source/processor2d/vclmetafileprocessor2d \
    drawinglayer/source/processor2d/vclpixelprocessor2d \
    drawinglayer/source/processor2d/vclprocessor2d \
    drawinglayer/source/processor3d/baseprocessor3d \
    drawinglayer/source/processor3d/cutfindprocessor3d \
    drawinglayer/source/processor3d/defaultprocessor3d \
    drawinglayer/source/processor3d/geometry2dextractor \
    drawinglayer/source/processor3d/shadow3dextractor \
    drawinglayer/source/processor3d/zbufferprocessor3d \
    drawinglayer/source/texture/texture3d \
    drawinglayer/source/tools/converters \
    drawinglayer/source/tools/emfplushelper \
    drawinglayer/source/tools/emfphelperdata \
    drawinglayer/source/tools/emfpbrush \
    drawinglayer/source/tools/emfppath \
    drawinglayer/source/tools/emfppen \
    drawinglayer/source/tools/emfpregion \
    drawinglayer/source/tools/emfpimage \
    drawinglayer/source/tools/emfpimageattributes \
    drawinglayer/source/tools/emfpfont \
    drawinglayer/source/tools/emfpstringformat \
    drawinglayer/source/tools/emfpcustomlinecap \
    drawinglayer/source/tools/wmfemfhelper \
    drawinglayer/source/tools/primitive2dxmldump \
    drawinglayer/source/drawinglayeruno/xprimitive2drenderer \
    drawinglayer/source/texture/texture \
    drawinglayer/source/dumper/XShapeDumper \
    drawinglayer/source/dumper/EnhancedShapeDumper \
))

# vim: set noet sw=4 ts=4:
