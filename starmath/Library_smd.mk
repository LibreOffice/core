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



$(eval $(call gb_Library_Library,smd))

$(eval $(call gb_Library_add_precompiled_header,sm,$(SRCDIR)/starmath/inc/pch/precompiled_starmath))

$(eval $(call gb_Library_set_componentfile,smd,starmath/util/smd))

$(eval $(call gb_Library_add_api,smd,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,smd,\
	$$(INCLUDE) \
	-I$(SRCDIR)/starmath/inc \
	-I$(SRCDIR)/starmath/inc/pch \
))

$(eval $(call gb_Library_add_linked_libs,smd,\
	cppu \
	cppuhelper \
	sal \
	sfx \
	sm \
	sot \
	svl \
	svt \
	tl \
	ucbhelper \
	vcl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,smd,\
	starmath/source/detreg \
	starmath/source/smdetect \
))

# vim: set noet sw=4 ts=4:
