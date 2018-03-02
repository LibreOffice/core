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



$(eval $(call gb_Library_Library,avmediagst))

$(eval $(call gb_Library_add_package_headers,avmediagst,avmedia_inc))

$(eval $(call gb_Library_set_componentfile,avmediagst,avmedia/source/gstreamer/avmediagst))

$(eval $(call gb_Library_set_include,avmediagst,\
        $$(INCLUDE) \
	-I$(SRCDIR)/avmedia/inc \
	-I$(SRCDIR)/avmedia/source/inc \
	$(filter -I%,$(GSTREAMER_CFLAGS)) \
))

$(eval $(call gb_Library_add_cflags,avmediagst,\
	$(filter-out -I%,$(GSTREAMER_CFLAGS)) \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_add_cxxflags,avmediagst,\
    -x objective-c++ \
))
endif

$(eval $(call gb_Library_add_api,avmediagst,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,avmediagst,\
	comphelper \
	cppuhelper \
	cppu \
	sal \
	stl \
	tl \
	vcl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_libs,avmediagst,\
	$(GSTREAMER_LIBS) \
))

$(eval $(call gb_Library_add_exception_objects,avmediagst,\
	avmedia/source/gstreamer/gstuno \
	avmedia/source/gstreamer/gstplayer \
	avmedia/source/gstreamer/gstframegrabber \
))

$(eval $(call gb_Library_add_noexception_objects,avmediagst,\
	avmedia/source/gstreamer/gstmanager \
	avmedia/source/gstreamer/gstwindow \
))

# vim: set noet sw=4 ts=4:
