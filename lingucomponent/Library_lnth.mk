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



$(eval $(call gb_Library_Library,lnth))

$(eval $(call gb_Library_add_precompiled_header,lnth,$(SRCDIR)/lingucomponent/inc/pch/precompiled_lnth))

$(eval $(call gb_Library_set_componentfile,lnth,lingucomponent/source/thesaurus/libnth/lnth))

$(eval $(call gb_Library_set_include,lnth,\
	$$(INCLUDE) \
	-I$(SRCDIR)/lingucomponent/inc \
	-I$(SRCDIR)/lingucomponent/inc/pch \
	-I$(SRCDIR)/lingucomponent/source/lingutil \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_api,lnth, \
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,lnth,\
	cppu \
	cppuhelper \
	i18nisolang1 \
	lng \
	sal \
	stl \
	svl \
	tl \
	utl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_linked_static_libs,lnth,\
	ulingu \
))

$(eval $(call gb_Library_use_externals,lnth,\
	mythes \
))

$(eval $(call gb_Library_add_exception_objects,lnth,\
	lingucomponent/source/thesaurus/libnth/nthesdta \
	lingucomponent/source/thesaurus/libnth/nthesimp \
	lingucomponent/source/thesaurus/libnth/ntreg \
))

# vim: set noet sw=4 ts=4:

