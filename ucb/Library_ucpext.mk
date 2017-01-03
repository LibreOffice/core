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



$(eval $(call gb_Library_Library,ucpext))

$(eval $(call gb_Library_add_precompiled_header,ucpext,$(SRCDIR)/ucb/inc/pch/precompiled_ext))

$(eval $(call gb_Library_set_componentfile,ucpext,ucb/source/ucp/ext/ucpext))

$(eval $(call gb_Library_set_include,ucpext,\
        $$(INCLUDE) \
	-I$(SRCDIR)/ucb/inc/pch \
	-I$(SRCDIR)/ucb/source/inc \
))

$(eval $(call gb_Library_add_api,ucpext,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,ucpext,\
	comphelper \
	cppuhelper \
	cppu \
	sal \
	salhelper \
	stl \
	ucbhelper \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,ucpext,\
	ucb/source/ucp/ext/ucpext_provider \
	ucb/source/ucp/ext/ucpext_content \
	ucb/source/ucp/ext/ucpext_services \
	ucb/source/ucp/ext/ucpext_resultset \
	ucb/source/ucp/ext/ucpext_datasupplier \
))

# vim: set noet sw=4 ts=4:
