# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# Major Contributor(s):
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,slideshow))

$(eval $(call gb_Library_use_packages,slideshow,\
	animations_inc \
))

$(eval $(call gb_Library_set_include,slideshow,\
    $$(INCLUDE) \
    -I$(SRCDIR)/slideshow/source/inc \
))

$(eval $(call gb_Library_add_defs,slideshow,\
    -DBOOST_SPIRIT_USE_OLD_NAMESPACE \
    $(if $(filter TRUE,$(VERBOSE)),-DVERBOSE) \
))

ifneq ($(strip $(debug)$(DEBUG)),)
$(eval $(call gb_Library_add_defs,slideshow,\
    -DBOOST_SP_ENABLE_DEBUG_HOOKS \
))
endif

$(eval $(call gb_Library_use_sdk_api,slideshow))

$(eval $(call gb_Library_use_libraries,slideshow,\
    avmedia \
    basegfx \
    canvastools \
    comphelper \
    cppcanvas \
    cppu \
    cppuhelper \
    sal \
    svt \
    tl \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,slideshow))

$(eval $(call gb_Library_set_componentfile,slideshow,slideshow/util/slideshow))

$(eval $(call gb_Library_add_exception_objects,slideshow,\
    slideshow/source/engine/activities/activitiesfactory \
    slideshow/source/engine/activities/activitybase \
    slideshow/source/engine/activities/continuousactivitybase \
    slideshow/source/engine/activities/continuouskeytimeactivitybase \
    slideshow/source/engine/activities/discreteactivitybase \
    slideshow/source/engine/activitiesqueue \
    slideshow/source/engine/activities/simplecontinuousactivitybase \
    slideshow/source/engine/animatedsprite \
    slideshow/source/engine/animationfactory \
    slideshow/source/engine/animationnodes/animationaudionode \
    slideshow/source/engine/animationnodes/animationbasenode \
    slideshow/source/engine/animationnodes/animationcolornode \
    slideshow/source/engine/animationnodes/animationcommandnode \
    slideshow/source/engine/animationnodes/animationnodefactory \
    slideshow/source/engine/animationnodes/animationpathmotionnode \
    slideshow/source/engine/animationnodes/animationsetnode \
    slideshow/source/engine/animationnodes/animationtransformnode \
    slideshow/source/engine/animationnodes/animationtransitionfilternode \
    slideshow/source/engine/animationnodes/basecontainernode \
    slideshow/source/engine/animationnodes/basenode \
    slideshow/source/engine/animationnodes/generateevent \
    slideshow/source/engine/animationnodes/nodetools \
    slideshow/source/engine/animationnodes/paralleltimecontainer \
    slideshow/source/engine/animationnodes/propertyanimationnode \
    slideshow/source/engine/animationnodes/sequentialtimecontainer \
    slideshow/source/engine/attributemap \
    slideshow/source/engine/color \
    slideshow/source/engine/delayevent \
    slideshow/source/engine/effectrewinder \
    slideshow/source/engine/eventmultiplexer \
    slideshow/source/engine/eventqueue \
    slideshow/source/engine/expressionnodefactory \
    slideshow/source/engine/rehearsetimingsactivity \
    slideshow/source/engine/screenupdater \
    slideshow/source/engine/shapeattributelayer \
    slideshow/source/engine/shapes/appletshape \
    slideshow/source/engine/shapes/backgroundshape \
    slideshow/source/engine/shapes/drawinglayeranimation \
    slideshow/source/engine/shapes/drawshape \
    slideshow/source/engine/shapes/drawshapesubsetting \
    slideshow/source/engine/shapes/externalshapebase \
    slideshow/source/engine/shapes/gdimtftools \
    slideshow/source/engine/shapes/intrinsicanimationactivity \
    slideshow/source/engine/shapes/mediashape \
    slideshow/source/engine/shapes/shapeimporter \
    slideshow/source/engine/shapesubset \
    slideshow/source/engine/shapes/viewappletshape \
    slideshow/source/engine/shapes/viewbackgroundshape \
    slideshow/source/engine/shapes/viewmediashape \
    slideshow/source/engine/shapes/viewshape \
    slideshow/source/engine/slidebitmap \
    slideshow/source/engine/slide/layer \
    slideshow/source/engine/slide/layermanager \
    slideshow/source/engine/slide/shapemanagerimpl \
    slideshow/source/engine/slideshowcontext \
    slideshow/source/engine/slideshowimpl \
    slideshow/source/engine/slide/slideanimations \
    slideshow/source/engine/slide/slideimpl \
    slideshow/source/engine/slide/userpaintoverlay \
    slideshow/source/engine/slideview \
    slideshow/source/engine/soundplayer \
    slideshow/source/engine/tools \
    slideshow/source/engine/transitions/barndoorwipe \
    slideshow/source/engine/transitions/barwipepolypolygon \
    slideshow/source/engine/transitions/boxwipe \
    slideshow/source/engine/transitions/checkerboardwipe \
    slideshow/source/engine/transitions/clippingfunctor \
    slideshow/source/engine/transitions/clockwipe \
    slideshow/source/engine/transitions/combtransition \
    slideshow/source/engine/transitions/doublediamondwipe \
    slideshow/source/engine/transitions/ellipsewipe \
    slideshow/source/engine/transitions/fanwipe \
    slideshow/source/engine/transitions/figurewipe \
    slideshow/source/engine/transitions/fourboxwipe \
    slideshow/source/engine/transitions/iriswipe \
    slideshow/source/engine/transitions/parametricpolypolygonfactory \
    slideshow/source/engine/transitions/pinwheelwipe \
    slideshow/source/engine/transitions/randomwipe \
    slideshow/source/engine/transitions/shapetransitionfactory \
    slideshow/source/engine/transitions/slidechangebase \
    slideshow/source/engine/transitions/slidetransitionfactory \
    slideshow/source/engine/transitions/snakewipe \
    slideshow/source/engine/transitions/spiralwipe \
    slideshow/source/engine/transitions/sweepwipe \
    slideshow/source/engine/transitions/transitionfactorytab \
    slideshow/source/engine/transitions/transitiontools \
    slideshow/source/engine/transitions/veewipe \
    slideshow/source/engine/transitions/waterfallwipe \
    slideshow/source/engine/transitions/zigzagwipe \
    slideshow/source/engine/unoviewcontainer \
    slideshow/source/engine/usereventqueue \
    slideshow/source/engine/waitsymbol \
    slideshow/source/engine/wakeupevent \
))

ifeq ($(strip $(OS)),MACOSX)
$(eval $(call gb_Library_add_cxxobjects,slideshow,\
    slideshow/source/engine/smilfunctionparser \
    , $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
else
$(eval $(call gb_Library_add_exception_objects,slideshow,\
    slideshow/source/engine/smilfunctionparser \
))
endif

ifneq ($(strip $(debug)$(DEBUG)),)
$(eval $(call gb_Library_add_exception_objects,slideshow,\
    slideshow/source/engine/sp_debug \
))
endif

# vim: set noet sw=4 ts=4:
