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

$(eval $(call gb_StaticLibrary_StaticLibrary,sldshw_s))

$(eval $(call gb_StaticLibrary_add_precompiled_header,sldshw_s,$(SRCDIR)/slideshow/inc/pch/precompiled_slideshow))

$(eval $(call gb_StaticLibrary_set_include,sldshw_s,\
        $$(INCLUDE) \
	-I$(SRCDIR)/slideshow/source/inc \
	-I$(SRCDIR)/slideshow/inc/pch \
))

$(eval $(call gb_StaticLibrary_add_defs,sldshw_s,\
	-DBOOST_SPIRIT_USE_OLD_NAMESPACE \
))

# FIXME: no way to set CFLAGSWARNCXX any more! Comment was:
# SunStudio12: anachronism warning in boost code (smilfunctionparser.cxx)
# reevaluate with newer boost or compiler version
#ifeq ($(OS),SOLARIS)
#ifeq ($(CCNUMVER),00050009)
#$(eval $(call gb_StaticLibrary_add_cxxflags,sldshw_s,\
#	badargtype2w
#))
#endif
#endif

$(eval $(call gb_StaticLibrary_add_api,sldshw_s,\
	offapi \
	udkapi \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,sldshw_s,\
	slideshow/source/engine/transitions/barwipepolypolygon \
	slideshow/source/engine/transitions/boxwipe \
	slideshow/source/engine/transitions/clippingfunctor \
	slideshow/source/engine/transitions/combtransition \
	slideshow/source/engine/transitions/fourboxwipe \
	slideshow/source/engine/transitions/barndoorwipe \
	slideshow/source/engine/transitions/iriswipe \
	slideshow/source/engine/transitions/veewipe \
	slideshow/source/engine/transitions/ellipsewipe \
	slideshow/source/engine/transitions/checkerboardwipe \
	slideshow/source/engine/transitions/randomwipe \
	slideshow/source/engine/transitions/waterfallwipe \
	slideshow/source/engine/transitions/clockwipe \
	slideshow/source/engine/transitions/fanwipe \
	slideshow/source/engine/transitions/pinwheelwipe \
	slideshow/source/engine/transitions/snakewipe \
	slideshow/source/engine/transitions/spiralwipe \
	slideshow/source/engine/transitions/sweepwipe \
	slideshow/source/engine/transitions/figurewipe \
	slideshow/source/engine/transitions/doublediamondwipe \
	slideshow/source/engine/transitions/zigzagwipe \
	slideshow/source/engine/transitions/parametricpolypolygonfactory \
	slideshow/source/engine/transitions/shapetransitionfactory \
	slideshow/source/engine/transitions/slidetransitionfactory \
	slideshow/source/engine/transitions/transitionfactorytab \
	slideshow/source/engine/transitions/transitiontools \
	slideshow/source/engine/transitions/slidechangebase \
	slideshow/source/engine/activities/activitybase \
	slideshow/source/engine/activities/activitiesfactory \
	slideshow/source/engine/activities/continuousactivitybase \
	slideshow/source/engine/activities/continuouskeytimeactivitybase \
	slideshow/source/engine/activities/discreteactivitybase \
	slideshow/source/engine/activities/simplecontinuousactivitybase \
	slideshow/source/engine/animationnodes/animationaudionode \
	slideshow/source/engine/animationnodes/animationcommandnode \
	slideshow/source/engine/animationnodes/animationbasenode \
	slideshow/source/engine/animationnodes/animationcolornode \
	slideshow/source/engine/animationnodes/animationnodefactory \
	slideshow/source/engine/animationnodes/animationpathmotionnode \
	slideshow/source/engine/animationnodes/animationsetnode \
	slideshow/source/engine/animationnodes/animationtransformnode \
	slideshow/source/engine/animationnodes/animationtransitionfilternode \
	slideshow/source/engine/animationnodes/basecontainernode \
	slideshow/source/engine/animationnodes/basenode \
	slideshow/source/engine/animationnodes/nodetools \
	slideshow/source/engine/animationnodes/paralleltimecontainer \
	slideshow/source/engine/animationnodes/propertyanimationnode \
	slideshow/source/engine/animationnodes/sequentialtimecontainer \
	slideshow/source/engine/animationnodes/generateevent \
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
	slideshow/source/engine/shapes/viewappletshape \
	slideshow/source/engine/shapes/viewbackgroundshape \
	slideshow/source/engine/shapes/viewmediashape \
	slideshow/source/engine/shapes/viewshape \
	slideshow/source/engine/slide/layer \
	slideshow/source/engine/slide/layermanager \
	slideshow/source/engine/slide/shapemanagerimpl \
	slideshow/source/engine/slide/slideanimations \
	slideshow/source/engine/slide/slideimpl \
	slideshow/source/engine/slide/userpaintoverlay \
	slideshow/source/engine/activitiesqueue \
	slideshow/source/engine/animatedsprite \
	slideshow/source/engine/animationfactory \
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
	slideshow/source/engine/shapesubset \
	slideshow/source/engine/slidebitmap \
	slideshow/source/engine/slideshowcontext \
	slideshow/source/engine/slideshowimpl \
	slideshow/source/engine/slideview \
	slideshow/source/engine/soundplayer \
	slideshow/source/engine/tools \
	slideshow/source/engine/unoviewcontainer \
	slideshow/source/engine/usereventqueue \
	slideshow/source/engine/waitsymbol \
	slideshow/source/engine/wakeupevent \
	slideshow/source/engine/debug \
))

ifeq ($(DEBUG),TRUE)
$(eval $(call gb_StaticLibrary_add_exception_objects,sldshw_s,\
	slideshow/source/engine/sp_debug \
))
endif

# Disable optimization for SunCC Sparc (funny loops
# when parsing e.g. "x+width/2")
# Do not disable optimization for SunCC++ 5.5 Solaris x86,
# this compiler has an ICE on smilfunctionparser.cxx *without*
# optimization
ifeq ($(OS)$(CPU),SOLARISS)
$(eval $(call gb_StaticLibrary_add_cxxobjects,sldshw_s,\
	slideshow/source/engine/smilfunctionparser \
	, $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
else
# same issue for MACOSX
ifeq ($(OS),MACOSX)
$(eval $(call gb_StaticLibrary_add_cxxobjects,sldshw_s,\
	slideshow/source/engine/smilfunctionparser \
	, $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
else
$(eval $(call gb_StaticLibrary_add_exception_objects,sldshw_s,\
	slideshow/source/engine/smilfunctionparser \
))
endif
endif



# vim: set noet sw=4 ts=4:
