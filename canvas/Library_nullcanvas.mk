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



$(eval $(call gb_Library_Library,nullcanvas))

$(eval $(call gb_Library_add_api,nullcanvas,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,nullcanvas,\
	$$(INCLUDE) \
	-I$(SRCDIR)/canvas/inc \
	-I$(SRCDIR)/canvas/inc/pch \
))

ifneq ($(strip $(VERBOSE)$(verbose)),)
$(eval $(call gb_Library_add_defs,nullcanvas,\
	-DVERBOSE \
))
endif

$(eval $(call gb_Library_add_linked_libs,nullcanvas,\
	cppu \
	sal \
	comphelper \
	cppuhelper \
	stl \
	basegfx \
	canvastools \
	vcl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,nullcanvas,\
	canvas/source/null/null_canvasbitmap \
	canvas/source/null/null_canvascustomsprite \
	canvas/source/null/null_canvasfont \
	canvas/source/null/null_canvashelper \
	canvas/source/null/null_devicehelper \
	canvas/source/null/null_spritecanvas \
	canvas/source/null/null_spritecanvashelper \
	canvas/source/null/null_spritehelper \
	canvas/source/null/null_textlayout \
))

# vim: set noet sw=4 ts=4:
