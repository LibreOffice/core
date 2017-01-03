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



$(eval $(call gb_Library_Library,ucpfile1))

$(eval $(call gb_Library_add_precompiled_header,ucpfile1,$(SRCDIR)/ucb/inc/pch/precompiled_file))

$(eval $(call gb_Library_set_componentfile,ucpfile1,ucb/source/ucp/file/ucpfile1))

$(eval $(call gb_Library_set_include,ucpfile1,\
        $$(INCLUDE) \
	-I$(SRCDIR)/ucb/inc/pch \
	-I$(SRCDIR)/ucb/source/inc \
))

$(eval $(call gb_Library_add_api,ucpfile1,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,ucpfile1,\
	cppuhelper \
	cppu \
	sal \
	stl \
	ucbhelper \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,ucpfile1,\
	ucb/source/ucp/file/prov \
	ucb/source/ucp/file/bc \
	ucb/source/ucp/file/shell \
	ucb/source/ucp/file/filtask \
	ucb/source/ucp/file/filrow \
	ucb/source/ucp/file/filrset \
	ucb/source/ucp/file/filid \
	ucb/source/ucp/file/filnot \
	ucb/source/ucp/file/filprp \
	ucb/source/ucp/file/filinpstr \
	ucb/source/ucp/file/filstr \
	ucb/source/ucp/file/filcmd \
	ucb/source/ucp/file/filglob \
	ucb/source/ucp/file/filinsreq \
	ucb/source/ucp/file/filrec \
))

# vim: set noet sw=4 ts=4:
