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



$(eval $(call gb_Library_Library,ucphier1))

$(eval $(call gb_Library_add_precompiled_header,ucphier1,$(SRCDIR)/ucb/inc/pch/precompiled_hier))

$(eval $(call gb_Library_set_componentfile,ucphier1,ucb/source/ucp/hierarchy/ucphier1))

$(eval $(call gb_Library_set_include,ucphier1,\
        $$(INCLUDE) \
	-I$(SRCDIR)/ucb/inc/pch \
	-I$(SRCDIR)/ucb/source/inc \
))

$(eval $(call gb_Library_add_api,ucphier1,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,ucphier1,\
	cppuhelper \
	cppu \
	sal \
	salhelper \
	stl \
	ucbhelper \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,ucphier1,\
	ucb/source/ucp/hierarchy/hierarchyservices \
	ucb/source/ucp/hierarchy/hierarchydata \
	ucb/source/ucp/hierarchy/hierarchyprovider \
	ucb/source/ucp/hierarchy/hierarchycontent \
	ucb/source/ucp/hierarchy/hierarchycontentcaps \
	ucb/source/ucp/hierarchy/hierarchydatasupplier \
	ucb/source/ucp/hierarchy/dynamicresultset \
	ucb/source/ucp/hierarchy/hierarchydatasource \
	ucb/source/ucp/hierarchy/hierarchyuri \
))

# vim: set noet sw=4 ts=4:
