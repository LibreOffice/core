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



$(eval $(call gb_Library_Library,MacOSXSpell))

$(eval $(call gb_Library_add_precompiled_header,MacOSXSpell,$(SRCDIR)/lingucomponent/inc/pch/precompiled_mac))

$(eval $(call gb_Library_set_componentfile,MacOSXSpell,lingucomponent/source/spellcheck/macosxspell/MacOSXSpell))

$(eval $(call gb_Library_set_include,MacOSXSpell,\
	$$(INCLUDE) \
	-I$(SRCDIR)/lingucomponent/inc \
	-I$(SRCDIR)/lingucomponent/inc/pch \
	-I$(SRCDIR)/lingucomponent/source/lingutil \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_api,MacOSXSpell, \
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,MacOSXSpell,\
	cppu \
	cppuhelper \
	lng \
	sal \
	stl \
	svl \
	tl \
	ucbhelper \
	utl \
	vos3 \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_libs,MacOSXSpell,\
	-framework Cocoa \
	-framework Carbon \
	-framework CoreFoundation \
))

$(eval $(call gb_Library_add_cxxflags,MacOSXSpell,\
    -x -objective-c++ -stdlib=libc++ \
))

# vim: set noet sw=4 ts=4:

