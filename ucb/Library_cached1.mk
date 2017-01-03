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



$(eval $(call gb_Library_Library,cached1))

$(eval $(call gb_Library_add_precompiled_header,cached1,$(SRCDIR)/ucb/inc/pch/precompiled_cacher))

$(eval $(call gb_Library_set_componentfile,cached1,ucb/source/cacher/cached1))

$(eval $(call gb_Library_set_include,cached1,\
        $$(INCLUDE) \
	-I$(SRCDIR)/ucb/inc/pch \
	-I$(SRCDIR)/ucb/source/inc \
))

$(eval $(call gb_Library_add_api,cached1,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,cached1,\
	cppuhelper \
	cppu \
	sal \
	stl \
	$(gb_STDLIBS) \
))


$(eval $(call gb_Library_add_exception_objects,cached1,\
	ucb/source/cacher/contentresultsetwrapper \
	ucb/source/cacher/cachedcontentresultsetstub \
	ucb/source/cacher/cachedcontentresultset \
	ucb/source/cacher/dynamicresultsetwrapper \
	ucb/source/cacher/cacheddynamicresultsetstub \
	ucb/source/cacher/cacheddynamicresultset \
	ucb/source/cacher/cacheserv \
))

# vim: set noet sw=4 ts=4:
