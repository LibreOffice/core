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



$(eval $(call gb_Library_Library,avmediawin))

$(eval $(call gb_Library_add_package_headers,avmediawin,avmedia_inc))

$(eval $(call gb_Library_set_componentfile,avmediawin,avmedia/source/win/avmediawin))

$(eval $(call gb_Library_set_include,avmediawin,\
        $$(INCLUDE) \
	-I$(SRCDIR)/avmedia/inc \
	-I$(SRCDIR)/avmedia/source/inc \
))

$(eval $(call gb_Library_add_api,avmediawin,\
	offapi \
	udkapi \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_add_cxxflags,avmediawin,\
    -x objective-c++ \
))
endif

$(eval $(call gb_Library_add_linked_libs,avmediawin,\
	comphelper \
	cppuhelper \
	cppu \
	sal \
	stl \
	tl \
	utl \
	vcl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_linked_libs,avmediawin,\
	gdi32 \
	ole32 \
	oleaut32 \
	user32 \
))

ifeq ($(COM),GCC)
$(eval $(call gb_Library_add_linked_libs,avmediawin,\
	$(PSDK_HOME)/lib/strmiids \
))
else
$(eval $(call gb_Library_add_linked_libs,avmediawin,\
	strmiids \
))
endif

$(eval $(call gb_Library_add_noexception_objects,avmediawin,\
	avmedia/source/win/manager \
	avmedia/source/win/player \
	avmedia/source/win/window \
))

$(eval $(call gb_Library_add_exception_objects,avmediawin,\
	avmedia/source/win/framegrabber \
	avmedia/source/win/winuno \
))

# vim: set noet sw=4 ts=4:
