# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,drawinglayer))

$(eval $(call gb_Library_add_package_headers,drawinglayer,drawinglayer_inc))

$(eval $(call gb_Library_add_precompiled_header,drawinglayer,$(SRCDIR)/drawinglayer/inc/pch/precompiled_drawinglayer))

$(eval $(call gb_Library_set_include,drawinglayer,\
    $$(INCLUDE) \
    -I$(realpath $(SRCDIR)/drawinglayer/inc/pch) \
    -I$(OUTDIR)/inc \
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
    svl \
    svt \
    tk \
    tl \
    vcl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,drawinglayer,\
    drawinglayer/source/animation/animationtiming \
    drawinglayer/source/attribute/fillbitmapattribute \
    drawinglayer/source/attribute/fillgradientattribute \
    drawinglayer/source/attribute/fillhatchattribute \
    drawinglayer/source/attribute/fontattribute \
    drawinglayer/source/attribute/lineattribute \
    drawinglayer/source/attribute/linestartendattribute \
    drawinglayer/source/attribute/materialattribute3d \
    drawinglayer/source/attribute/sdrallattribute3d \
    drawinglayer/source/attribute/sdrfillattribute \
    drawinglayer/source/attribute/sdrfillbitmapattribute \
    drawinglayer/source/attribute/sdrlightattribute3d \
    drawinglayer/source/attribute/sdrlightingattribute3d \
    drawinglayer/source/attribute/sdrlineattribute \
    drawinglayer/source/attribute/sdrlinestartendattribute \
    drawinglayer/source/attribute/sdrobjectattribute3d \
    drawinglayer/source/attribute/sdrsceneattribute3d \
    drawinglayer/source/attribute/sdrshadowattribute \
    drawinglayer/source/attribute/strokeattribute \
    drawinglayer/source/primitive2d/animatedprimitive2d \
    drawinglayer/source/primitive2d/backgroundcolorprimitive2d \
    drawinglayer/source/primitive2d/baseprimitive2d \
    drawinglayer/source/primitive2d/bitmapprimitive2d \
    drawinglayer/source/primitive2d/borderlineprimitive2d \
    drawinglayer/source/primitive2d/chartprimitive2d \
    drawinglayer/source/primitive2d/clippedborderlineprimitive2d \
    drawinglayer/source/primitive2d/controlprimitive2d \
    drawinglayer/source/primitive2d/discretebitmapprimitive2d \
    drawinglayer/source/primitive2d/discreteshadowprimitive2d \
    drawinglayer/source/primitive2d/embedded3dprimitive2d \
    drawinglayer/source/primitive2d/epsprimitive2d \
    drawinglayer/source/primitive2d/fillbitmapprimitive2d \
    drawinglayer/source/primitive2d/fillgradientprimitive2d \
    drawinglayer/source/primitive2d/fillhatchprimitive2d \
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
    drawinglayer/source/primitive2d/pagepreviewprimitive2d \
    drawinglayer/source/primitive2d/pointarrayprimitive2d \
    drawinglayer/source/primitive2d/polygonprimitive2d \
    drawinglayer/source/primitive2d/polypolygonprimitive2d \
    drawinglayer/source/primitive2d/primitivetools2d \
    drawinglayer/source/primitive2d/sceneprimitive2d \
    drawinglayer/source/primitive2d/sdrdecompositiontools2d \
    drawinglayer/source/primitive2d/shadowprimitive2d \
    drawinglayer/source/primitive2d/structuretagprimitive2d \
    drawinglayer/source/primitive2d/textdecoratedprimitive2d \
    drawinglayer/source/primitive2d/texteffectprimitive2d \
    drawinglayer/source/primitive2d/textenumsprimitive2d \
    drawinglayer/source/primitive2d/texthierarchyprimitive2d \
    drawinglayer/source/primitive2d/textlayoutdevice \
    drawinglayer/source/primitive2d/textlineprimitive2d \
    drawinglayer/source/primitive2d/textprimitive2d \
    drawinglayer/source/primitive2d/textstrikeoutprimitive2d \
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
    drawinglayer/source/processor2d/baseprocessor2d \
    drawinglayer/source/processor2d/canvasprocessor \
    drawinglayer/source/processor2d/contourextractor2d \
    drawinglayer/source/processor2d/helperchartrenderer \
    drawinglayer/source/processor2d/helperwrongspellrenderer \
    drawinglayer/source/processor2d/hittestprocessor2d \
    drawinglayer/source/processor2d/linegeometryextractor2d \
    drawinglayer/source/processor2d/textaspolygonextractor2d \
    drawinglayer/source/processor2d/vclhelperbitmaprender \
    drawinglayer/source/processor2d/vclhelperbitmaptransform \
    drawinglayer/source/processor2d/vclhelperbufferdevice \
    drawinglayer/source/processor2d/vclhelpergradient \
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
    drawinglayer/source/texture/texture \
))

$(eval $(call gb_Library_add_noexception_objects,drawinglayer,\
    drawinglayer/source/geometry/viewinformation2d \
    drawinglayer/source/geometry/viewinformation3d \
))

# vim: set noet ts=4 sw=4:
