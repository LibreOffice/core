# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,slideshow))

$(eval $(call gb_Library_set_include,slideshow,\
    $$(INCLUDE) \
    -I$(SRCDIR)/slideshow/source/inc \
))

ifneq ($(strip $(debug)$(DEBUG)),)
$(eval $(call gb_Library_add_defs,slideshow,\
    -DBOOST_SP_ENABLE_DEBUG_HOOKS \
))
endif

$(eval $(call gb_Library_set_precompiled_header,slideshow,slideshow/inc/pch/precompiled_slideshow))

$(eval $(call gb_Library_use_externals,slideshow,\
	boost_headers \
	box2d \
))
ifeq ($(DISABLE_GUI),)
$(eval $(call gb_Library_use_externals,slideshow,\
     epoxy \
 ))
endif

$(eval $(call gb_Library_use_sdk_api,slideshow))

$(eval $(call gb_Library_use_libraries,slideshow,\
    $(call gb_Helper_optional,AVMEDIA,avmedia) \
    basegfx \
    canvastools \
    comphelper \
    cppcanvas \
    cppu \
    cppuhelper \
    sal \
    salhelper \
    svl \
    svt \
    svxcore \
    tl \
    utl \
    vcl \
))

$(eval $(call gb_Library_set_componentfile,slideshow,slideshow/util/slideshow,services))

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
    slideshow/source/engine/animationnodes/animationphysicsnode \
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
    slideshow/source/engine/box2dtools \
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
    slideshow/source/engine/slide/targetpropertiescreator \
    slideshow/source/engine/slide/userpaintoverlay \
    slideshow/source/engine/slideview \
    slideshow/source/engine/smilfunctionparser \
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
    slideshow/source/engine/pointersymbol \
    slideshow/source/engine/wakeupevent \
))

# vim: set noet sw=4 ts=4:
