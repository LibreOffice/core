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



$(eval $(call gb_Library_Library,avmedia))

$(eval $(call gb_Library_add_package_headers,avmedia,avmedia_inc))

$(eval $(call gb_Library_set_componentfile,avmedia,avmedia/util/avmedia))

$(eval $(call gb_Library_set_include,avmedia,\
        $$(INCLUDE) \
	-I$(SRCDIR)/avmedia/inc \
	-I$(SRCDIR)/avmedia/source/inc \
))

$(eval $(call gb_Library_add_defs,avmedia,\
	-DAVMEDIA_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_api,avmedia,\
	offapi \
	udkapi \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_add_cxxflags,avmedia,\
    -x objective-c++ \
))
endif

$(eval $(call gb_Library_add_linked_libs,avmedia,\
	comphelper \
	cppuhelper \
	cppu \
	sal \
	sfx \
	stl \
	svl \
	svt \
	tl \
	utl \
	vcl \
	vos3 \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_noexception_objects,avmedia,\
	avmedia/source/framework/mediaitem \
	avmedia/source/framework/mediamisc \
	avmedia/source/framework/mediacontrol \
	avmedia/source/framework/mediaplayer \
	avmedia/source/viewer/mediaevent_impl \
))

$(eval $(call gb_Library_add_exception_objects,avmedia,\
	avmedia/source/framework/mediatoolbox \
	avmedia/source/framework/soundhandler \
	avmedia/source/viewer/mediawindow \
	avmedia/source/viewer/mediawindowbase_impl \
	avmedia/source/viewer/mediawindow_impl \
))

# vim: set noet sw=4 ts=4:
