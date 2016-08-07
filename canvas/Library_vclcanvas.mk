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



$(eval $(call gb_Library_Library,vclcanvas))

$(eval $(call gb_Library_set_componentfile,vclcanvas,canvas/source/vcl/vclcanvas))

$(eval $(call gb_Library_add_api,vclcanvas,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,vclcanvas,\
	$$(INCLUDE) \
	-I$(SRCDIR)/canvas/inc \
	-I$(SRCDIR)/canvas/inc/pch \
))

ifneq ($(strip $(VERBOSE)$(verbose)),)
$(eval $(call gb_Library_add_defs,vclcanvas,\
	-DVERBOSE \
))
endif

$(eval $(call gb_Library_add_linked_libs,vclcanvas,\
	sal \
	stl \
	cppu \
	basegfx \
	cppuhelper \
	comphelper \
	vcl \
	ootk \
	tl \
	svt \
	i18nisolang1 \
	canvastools \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,vclcanvas,\
	canvas/source/vcl/backbuffer \
	canvas/source/vcl/bitmapbackbuffer \
	canvas/source/vcl/cachedbitmap \
	canvas/source/vcl/canvas \
	canvas/source/vcl/canvasbitmap \
	canvas/source/vcl/canvasbitmaphelper \
	canvas/source/vcl/canvascustomsprite \
	canvas/source/vcl/canvasfont \
	canvas/source/vcl/canvashelper \
	canvas/source/vcl/devicehelper \
	canvas/source/vcl/impltools \
	canvas/source/vcl/services \
	canvas/source/vcl/spritecanvas \
	canvas/source/vcl/spritecanvashelper \
	canvas/source/vcl/spritedevicehelper \
	canvas/source/vcl/spritehelper \
	canvas/source/vcl/textlayout \
	canvas/source/vcl/windowoutdevholder \
))

# Solaris Sparc with Sun Compiler: noopt
ifneq ($(strip($OS)),SOLARIS)
$(eval $(call gb_Library_add_exception_objects,vclcanvas,\
	canvas/source/vcl/canvashelper_texturefill \
))
else
$(eval $(call gb_Library_add_cxxobjects,vclcanvas,\
	canvas/source/vcl/canvashelper_texturefill \
    , $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
endif

# vim: set noet sw=4 ts=4:
