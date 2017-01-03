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



$(eval $(call gb_Library_Library,ucppkg1))

$(eval $(call gb_Library_add_precompiled_header,ucppkg1,$(SRCDIR)/ucb/inc/pch/precompiled_pkg))

$(eval $(call gb_Library_set_componentfile,ucppkg1,ucb/source/ucp/package/ucppkg1))

$(eval $(call gb_Library_set_include,ucppkg1,\
        $$(INCLUDE) \
	-I$(SRCDIR)/ucb/inc/pch \
	-I$(SRCDIR)/ucb/source/inc \
))

$(eval $(call gb_Library_add_api,ucppkg1,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,ucppkg1,\
	comphelper \
	cppuhelper \
	cppu \
	sal \
	salhelper \
	stl \
	ucbhelper \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,ucppkg1,\
	ucb/source/ucp/package/pkgservices \
	ucb/source/ucp/package/pkguri \
	ucb/source/ucp/package/pkgprovider \
	ucb/source/ucp/package/pkgcontent \
	ucb/source/ucp/package/pkgcontentcaps \
	ucb/source/ucp/package/pkgresultset \
	ucb/source/ucp/package/pkgdatasupplier \
))

# vim: set noet sw=4 ts=4:
