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



$(eval $(call gb_Library_Library,ucpexpand1))

$(eval $(call gb_Library_add_precompiled_header,ucpexpand1,$(SRCDIR)/ucb/inc/pch/precompiled_expand))

$(eval $(call gb_Library_set_componentfile,ucpexpand1,ucb/source/ucp/expand/ucpexpand1))

$(eval $(call gb_Library_set_include,ucpexpand1,\
        $$(INCLUDE) \
	-I$(SRCDIR)/ucb/inc/pch \
	-I$(SRCDIR)/ucb/source/inc \
))

$(eval $(call gb_Library_add_api,ucpexpand1,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,ucpexpand1,\
	cppuhelper \
	cppu \
	sal \
	stl \
	ucbhelper \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,ucpexpand1,\
	ucb/source/ucp/expand/ucpexpand \
))

# vim: set noet sw=4 ts=4:
