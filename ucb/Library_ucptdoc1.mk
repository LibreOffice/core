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



$(eval $(call gb_Library_Library,ucptdoc1))

$(eval $(call gb_Library_add_precompiled_header,ucptdoc1,$(SRCDIR)/ucb/inc/pch/precompiled_tdoc))

$(eval $(call gb_Library_set_componentfile,ucptdoc1,ucb/source/ucp/tdoc/ucptdoc1))

$(eval $(call gb_Library_set_include,ucptdoc1,\
        $$(INCLUDE) \
	-I$(SRCDIR)/ucb/inc/pch \
	-I$(SRCDIR)/ucb/source/inc \
))

$(eval $(call gb_Library_add_api,ucptdoc1,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,ucptdoc1,\
	comphelper \
	cppuhelper \
	cppu \
	sal \
	salhelper \
	stl \
	ucbhelper \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,ucptdoc1,\
	ucb/source/ucp/tdoc/tdoc_provider \
	ucb/source/ucp/tdoc/tdoc_services \
	ucb/source/ucp/tdoc/tdoc_uri \
	ucb/source/ucp/tdoc/tdoc_content \
	ucb/source/ucp/tdoc/tdoc_contentcaps \
	ucb/source/ucp/tdoc/tdoc_storage \
	ucb/source/ucp/tdoc/tdoc_docmgr \
	ucb/source/ucp/tdoc/tdoc_datasupplier \
	ucb/source/ucp/tdoc/tdoc_resultset \
	ucb/source/ucp/tdoc/tdoc_documentcontentfactory \
	ucb/source/ucp/tdoc/tdoc_passwordrequest \
	ucb/source/ucp/tdoc/tdoc_stgelems \
))

# vim: set noet sw=4 ts=4:
