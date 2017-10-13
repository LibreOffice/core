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



$(eval $(call gb_Library_Library,avmediaMacAVF))

$(eval $(call gb_Library_add_package_headers,avmediaMacAVF,avmedia_inc))

$(eval $(call gb_Library_set_componentfile,avmediaMacAVF,avmedia/source/macavf/avmediaMacAVF))

$(eval $(call gb_Library_set_include,avmediaMacAVF,\
        $$(INCLUDE) \
	-I$(SRCDIR)/avmedia/inc \
	-I$(SRCDIR)/avmedia/source/inc \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_add_cxxflags,avmediaMacAVF,\
    -x objective-c++ \
))
endif

$(eval $(call gb_Library_add_api,avmediaMacAVF,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,avmediaMacAVF,\
	comphelper \
	cppuhelper \
	cppu \
	sal \
	tl \
	vcl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_libs,avmediaMacAVF,\
	-framework Cocoa \
	-framework AVFoundation \
	-framework CoreMedia \
))

$(eval $(call gb_Library_add_noexception_objects,avmediaMacAVF,\
	avmedia/source/macavf/macavf_manager \
	avmedia/source/macavf/macavf_player \
	avmedia/source/macavf/macavf_window \
))

$(eval $(call gb_Library_add_exception_objects,avmediaMacAVF,\
	avmedia/source/macavf/macavf_framegrabber \
	avmedia/source/macavf/macavf_uno \
))

# vim: set noet sw=4 ts=4:
