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



$(eval $(call gb_Library_Library,ucpgvfs1))

$(eval $(call gb_Library_set_componentfile,ucpgvfs1,ucb/source/ucp/gvfs/ucpgvfs))

$(eval $(call gb_Library_set_include,ucpgvfs1,\
        $$(INCLUDE) \
	-I$(SRCDIR)/ucb/inc/pch \
	-I$(SRCDIR)/ucb/source/inc \
	$(filter -I%,$(GNOMEVFS_CFLAGS)) \
))

$(eval $(call gb_Library_add_cflags,ucpgvfs1,\
	$(filter-out -I%,$(GNOMEVFS_CFLAGS)) \
))

$(eval $(call gb_Library_add_api,ucpgvfs1,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,ucpgvfs1,\
	cppuhelper \
	cppu \
	sal \
	salhelper \
	stl \
	ucbhelper \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_libs,ucpgvfs1,\
	$(GNOMEVFS_LIBS) \
))

$(eval $(call gb_Library_add_exception_objects,ucpgvfs1,\
	ucb/source/ucp/gvfs/gvfs_content \
	ucb/source/ucp/gvfs/gvfs_directory \
	ucb/source/ucp/gvfs/gvfs_stream \
	ucb/source/ucp/gvfs/gvfs_provider \
))

# vim: set noet sw=4 ts=4:
