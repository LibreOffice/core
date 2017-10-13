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



$(eval $(call gb_Library_Library,avmediaQuickTime))

$(eval $(call gb_Library_add_package_headers,avmediaQuickTime,avmedia_inc))

$(eval $(call gb_Library_set_componentfile,avmediaQuickTime,avmedia/source/quicktime/avmediaQuickTime))

$(eval $(call gb_Library_set_include,avmediaQuickTime,\
        $$(INCLUDE) \
	-I$(SRCDIR)/avmedia/inc \
	-I$(SRCDIR)/avmedia/source/inc \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_add_cxxflags,avmediaQuickTime,\
    -x objective-c++ \
))
endif

$(eval $(call gb_Library_add_api,avmediaQuickTime,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,avmediaQuickTime,\
	comphelper \
	cppuhelper \
	cppu \
	sal \
	tl \
	vcl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_libs,avmediaQuickTime,\
	-framework Cocoa \
	-framework QTKit \
	-framework QuickTime \
))

$(eval $(call gb_Library_add_noexception_objects,avmediaQuickTime,\
	avmedia/source/quicktime/qt_manager \
	avmedia/source/quicktime/qt_player \
	avmedia/source/quicktime/qt_window \
))

$(eval $(call gb_Library_add_exception_objects,avmediaQuickTime,\
	avmedia/source/quicktime/qt_framegrabber \
	avmedia/source/quicktime/qt_uno \
))

# vim: set noet sw=4 ts=4:
