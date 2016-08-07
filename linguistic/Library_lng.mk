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



$(eval $(call gb_Library_Library,lng))

$(eval $(call gb_Library_add_package_headers,lng,linguistic_inc))

$(eval $(call gb_Library_add_precompiled_header,lng,$(SRCDIR)/linguistic/inc/pch/precompiled_linguistic))

$(eval $(call gb_Library_set_componentfile,lng,linguistic/source/lng))

$(eval $(call gb_Library_add_api,lng,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,lng,\
	$$(INCLUDE) \
	-I$(SRCDIR)/linguistic/inc \
	-I$(SRCDIR)/linguistic/inc/pch \
))

$(eval $(call gb_Library_add_defs,lng,\
	-DLNG_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,lng,\
	cppu \
	cppuhelper \
	comphelper \
	vos3 \
	tl \
	i18nisolang1 \
	svl \
	stl \
	sal \
	xo \
	ucbhelper \
	utl \
    $(gb_STDLIBS) \
))

$(call gb_Library_use_external,lng,icuuc)

$(eval $(call gb_Library_add_exception_objects,lng,\
	linguistic/source/convdic \
	linguistic/source/convdiclist \
	linguistic/source/convdicxml \
	linguistic/source/dicimp \
	linguistic/source/dlistimp \
	linguistic/source/gciterator \
	linguistic/source/hhconvdic \
	linguistic/source/hyphdsp \
	linguistic/source/hyphdta \
	linguistic/source/iprcache \
	linguistic/source/lngopt \
	linguistic/source/lngprophelp \
	linguistic/source/lngreg \
	linguistic/source/lngsvcmgr \
	linguistic/source/misc \
	linguistic/source/misc2 \
	linguistic/source/spelldsp \
	linguistic/source/spelldta \
	linguistic/source/thesdsp \
	linguistic/source/thesdta \
))

# vim: set noet sw=4 ts=4:
