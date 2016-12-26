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

$(eval $(call gb_Library_Library,OGLTrans))

$(eval $(call gb_Library_set_componentfile,OGLTrans,slideshow/source/engine/OGLTrans/ogltrans))

$(eval $(call gb_Library_set_include,OGLTrans,\
        $$(INCLUDE) \
	-I$(SRCDIR)/slideshow/source/inc \
	-I$(SRCDIR)/slideshow/inc/pch \
))

$(eval $(call gb_Library_add_api,OGLTrans,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,OGLTrans,\
	canvastools \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	vcl \
	$(gb_STDLIBS) \
))

ifeq ($(GUI),UNX)
ifneq ($(GUIBASE),aqua)
$(eval $(call gb_Library_add_libs,OGLTrans,\
	-lGL \
	-lGLU \
	-lX11 \
))
endif
else
$(eval $(call gb_Library_add_linked_libs,OGLTrans,\
	gdi32 \
	glu32 \
	opengl32 \
))
endif

$(eval $(call gb_Library_add_exception_objects,OGLTrans,\
	slideshow/source/engine/OGLTrans/OGLTrans_TransitionImpl \
	slideshow/source/engine/OGLTrans/OGLTrans_Shaders \
	slideshow/source/engine/OGLTrans/OGLTrans_TransitionerImpl \
))


# vim: set noet sw=4 ts=4:
