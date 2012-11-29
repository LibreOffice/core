#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



$(eval $(call gb_Library_Library,drawinglayer))

$(eval $(call gb_Library_set_componentfile,drawinglayer,drawinglayer/drawinglayer))

$(eval $(call gb_Library_add_package_headers,drawinglayer,drawinglayer_inc))

$(eval $(call gb_Library_add_precompiled_header,drawinglayer,$(SRCDIR)/drawinglayer/inc/pch/precompiled_drawinglayer))

$(eval $(call gb_Library_set_include,drawinglayer,\
	$$(INCLUDE) \
	-I$(SRCDIR)/drawinglayer/inc \
	-I$(SRCDIR)/drawinglayer/inc/pch \
	-I$(SRCDIR)/drawinglayer/source/processor2d \
	-I$(SRCDIR)/solenv/inc \
	-I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_set_defs,drawinglayer,\
	$$(DEFS) \
	-DDRAWINGLAYER_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,drawinglayer,\
	avmedia \
	basegfx \
	canvastools \
	comphelper \
	cppcanvas \
	cppu \
	cppuhelper \
	i18nisolang1 \
	sal \
	stl \
	svl \
	svt \
	tk \
	tl \
	vcl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,drawinglayer,\
	drawinglayer/source/animation/animationtiming \
	drawinglayer/source/attribute/fillgradientattribute \
	drawinglayer/source/attribute/fillhatchattribute \
	drawinglayer/source/attribute/fillgraphicattribute \
	drawinglayer/source/attribute/fontattribute \
	drawinglayer/source/attribute/materialattribute3d \
	drawinglayer/source/attribute/sdrallattribute3d \
	drawinglayer/source/attribute/sdrlineattribute \
	drawinglayer/source/attribute/sdrlinestartendattribute \
	drawinglayer/source/attribute/sdrshadowattribute \
	drawinglayer/source/attribute/sdrfillattribute \
	drawinglayer/source/attribute/sdrobjectattribute3d \
	drawinglayer/source/attribute/sdrlightattribute3d \
	drawinglayer/source/attribute/sdrlightingattribute3d \
	drawinglayer/source/attribute/sdrsceneattribute3d \
	drawinglayer/source/attribute/sdrfillgraphicattribute \
	drawinglayer/source/attribute/lineattribute \
	drawinglayer/source/attribute/linestartendattribute \
	drawinglayer/source/attribute/strokeattribute \
	drawinglayer/source/geometry/viewinformation2d \
	drawinglayer/source/geometry/viewinformation3d \
	drawinglayer/source/primitive2d/transparenceprimitive2d \
	drawinglayer/source/primitive2d/animatedprimitive2d \
	drawinglayer/source/primitive2d/baseprimitive2d \
	drawinglayer/source/primitive2d/backgroundcolorprimitive2d \
	drawinglayer/source/primitive2d/bitmapprimitive2d \
	drawinglayer/source/primitive2d/borderlineprimitive2d \
	drawinglayer/source/primitive2d/controlprimitive2d \
	drawinglayer/source/primitive2d/cropprimitive2d \
	drawinglayer/source/primitive2d/discretebitmapprimitive2d \
	drawinglayer/source/primitive2d/discreteshadowprimitive2d \
	drawinglayer/source/primitive2d/embedded3dprimitive2d \
	drawinglayer/source/primitive2d/epsprimitive2d \
	drawinglayer/source/primitive2d/fillgraphicprimitive2d \
	drawinglayer/source/primitive2d/fillgradientprimitive2d \
	drawinglayer/source/primitive2d/fillhatchprimitive2d \
	drawinglayer/source/primitive2d/graphicprimitivehelper2d \
	drawinglayer/source/primitive2d/graphicprimitive2d \
	drawinglayer/source/primitive2d/gridprimitive2d \
	drawinglayer/source/primitive2d/groupprimitive2d \
	drawinglayer/source/primitive2d/helplineprimitive2d \
	drawinglayer/source/primitive2d/hiddengeometryprimitive2d \
	drawinglayer/source/primitive2d/invertprimitive2d \
	drawinglayer/source/primitive2d/markerarrayprimitive2d \
	drawinglayer/source/primitive2d/pointarrayprimitive2d \
	drawinglayer/source/primitive2d/maskprimitive2d \
	drawinglayer/source/primitive2d/mediaprimitive2d \
	drawinglayer/source/primitive2d/metafileprimitive2d \
	drawinglayer/source/primitive2d/modifiedcolorprimitive2d \
	drawinglayer/source/primitive2d/objectinfoprimitive2d \
	drawinglayer/source/primitive2d/pagepreviewprimitive2d \
	drawinglayer/source/primitive2d/patternfillprimitive2d \
	drawinglayer/source/primitive2d/polypolygonprimitive2d \
	drawinglayer/source/primitive2d/polygonprimitive2d \
	drawinglayer/source/primitive2d/primitivetools2d \
	drawinglayer/source/primitive2d/sceneprimitive2d \
	drawinglayer/source/primitive2d/sdrdecompositiontools2d \
	drawinglayer/source/primitive2d/shadowprimitive2d \
	drawinglayer/source/primitive2d/structuretagprimitive2d \
	drawinglayer/source/primitive2d/svggradientprimitive2d \
	drawinglayer/source/primitive2d/textbreakuphelper \
	drawinglayer/source/primitive2d/texteffectprimitive2d \
	drawinglayer/source/primitive2d/textenumsprimitive2d \
	drawinglayer/source/primitive2d/textlayoutdevice \
	drawinglayer/source/primitive2d/textlineprimitive2d \
	drawinglayer/source/primitive2d/textprimitive2d \
	drawinglayer/source/primitive2d/textstrikeoutprimitive2d \
	drawinglayer/source/primitive2d/textdecoratedprimitive2d \
	drawinglayer/source/primitive2d/texthierarchyprimitive2d \
	drawinglayer/source/primitive2d/transformprimitive2d \
	drawinglayer/source/primitive2d/unifiedtransparenceprimitive2d \
	drawinglayer/source/primitive2d/wallpaperprimitive2d \
	drawinglayer/source/primitive2d/wrongspellprimitive2d \
	drawinglayer/source/primitive3d/baseprimitive3d \
	drawinglayer/source/primitive3d/groupprimitive3d \
	drawinglayer/source/primitive3d/hatchtextureprimitive3d \
	drawinglayer/source/primitive3d/hiddengeometryprimitive3d \
	drawinglayer/source/primitive3d/modifiedcolorprimitive3d \
	drawinglayer/source/primitive3d/polypolygonprimitive3d \
	drawinglayer/source/primitive3d/polygonprimitive3d \
	drawinglayer/source/primitive3d/polygontubeprimitive3d \
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
	drawinglayer/source/processor2d/baseprocessor2d \
	drawinglayer/source/processor2d/vclhelperbitmaptransform \
	drawinglayer/source/processor2d/vclhelperbitmaprender \
	drawinglayer/source/processor2d/vclhelperbufferdevice \
	drawinglayer/source/processor2d/vclprocessor2d \
	drawinglayer/source/processor2d/helperwrongspellrenderer \
	drawinglayer/source/processor2d/vclpixelprocessor2d \
	drawinglayer/source/processor2d/vclmetafileprocessor2d \
	drawinglayer/source/processor2d/contourextractor2d \
	drawinglayer/source/processor2d/linegeometryextractor2d \
	drawinglayer/source/processor2d/hittestprocessor2d \
	drawinglayer/source/processor2d/textaspolygonextractor2d \
	drawinglayer/source/processor2d/objectinfoextractor2d \
	drawinglayer/source/processor3d/baseprocessor3d \
	drawinglayer/source/processor3d/cutfindprocessor3d \
	drawinglayer/source/processor3d/defaultprocessor3d \
	drawinglayer/source/processor3d/shadow3dextractor \
	drawinglayer/source/processor3d/geometry2dextractor \
	drawinglayer/source/processor3d/zbufferprocessor3d \
	drawinglayer/source/texture/texture \
	drawinglayer/source/texture/texture3d \
	drawinglayer/source/tools/converters \
	drawinglayer/source/drawinglayeruno/drawinglayeruno \
	drawinglayer/source/drawinglayeruno/xprimitive2drenderer \
))

# vim: set noet sw=4 ts=4:
