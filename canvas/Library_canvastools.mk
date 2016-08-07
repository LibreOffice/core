###############################################################
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
###############################################################



$(eval $(call gb_Library_Library,canvastools))

$(eval $(call gb_Library_add_package_headers,canvastools,canvas_inc))

$(eval $(call gb_Library_add_precompiled_header,canvastools,$(SRCDIR)/canvas/inc/pch/precompiled_canvas))

$(eval $(call gb_Library_add_api,canvastools,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,canvastools,\
	$$(INCLUDE) \
	-I$(SRCDIR)/canvas/inc \
	-I$(SRCDIR)/canvas/inc/pch \
))

$(eval $(call gb_Library_add_defs,canvastools,\
	-DCANVASTOOLS_DLLIMPLEMENTATION \
))

ifneq ($(strip $(VERBOSE)$(verbose)),)
$(eval $(call gb_Library_add_defs,canvastools,\
	-DVERBOSE \
))
endif

ifneq ($(strip $(PROFILER)$(profiler)),)
$(eval $(call gb_Library_add_defs,canvastools,\
	-DPROFILER \
))
endif

$(eval $(call gb_Library_add_linked_libs,canvastools,\
	sal \
	stl \
	cppu \
	basegfx \
	cppuhelper \
	comphelper \
	vcl \
	ootk \
	tl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,canvastools,\
	canvas/source/tools/cachedprimitivebase \
	canvas/source/tools/canvascustomspritehelper \
	canvas/source/tools/canvastools \
	canvas/source/tools/elapsedtime \
	canvas/source/tools/page \
	canvas/source/tools/pagemanager \
	canvas/source/tools/parametricpolypolygon \
	canvas/source/tools/prioritybooster \
	canvas/source/tools/propertysethelper \
	canvas/source/tools/spriteredrawmanager \
	canvas/source/tools/surface \
	canvas/source/tools/surfaceproxy \
	canvas/source/tools/surfaceproxymanager \
	canvas/source/tools/verifyinput \
))

ifeq ($(strip $(ENABLE_AGG)),YES)
$(eval $(call gb_Library_add_exception_objects,canvastools,\
	canvas/source/tools/bitmap \
	canvas/source/tools/image \
))
$(eval $(call gb_Library_add_linked_libs,canvastools,\
	agg \
))
ifneq ($(strip $(AGG_VERSION)),)
$(eval $(call gb_Library_add_defs,canvastools,\
	-DAGG_VERSION=$(AGG_VERSION) \
))
endif
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_linked_libs,canvastools,\
	winmm \
))
endif

# vim: set noet sw=4 ts=4:
