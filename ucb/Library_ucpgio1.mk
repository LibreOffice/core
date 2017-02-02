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



$(eval $(call gb_Library_Library,ucpgio1))

$(eval $(call gb_Library_set_componentfile,ucpgio1,ucb/source/ucp/gio/ucpgio))

$(eval $(call gb_Library_set_include,ucpgio1,\
        $$(INCLUDE) \
	-I$(SRCDIR)/ucb/inc/pch \
	-I$(SRCDIR)/ucb/source/inc \
	$(filter -I%,$(GIO_CFLAGS)) \
))

$(eval $(call gb_Library_add_cflags,ucpgio1,\
	$(filter-out -I%,$(GIO_CFLAGS)) \
))

$(eval $(call gb_Library_add_api,ucpgio1,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,ucpgio1,\
	cppuhelper \
	cppu \
	sal \
	salhelper \
	stl \
	ucbhelper \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_libs,ucpgio1,\
	$(GIO_LIBS) \
))

$(eval $(call gb_Library_add_exception_objects,ucpgio1,\
	ucb/source/ucp/gio/gio_provider \
	ucb/source/ucp/gio/gio_content \
	ucb/source/ucp/gio/gio_resultset \
	ucb/source/ucp/gio/gio_datasupplier \
	ucb/source/ucp/gio/gio_seekable \
	ucb/source/ucp/gio/gio_inputstream \
	ucb/source/ucp/gio/gio_outputstream \
	ucb/source/ucp/gio/gio_mount \
))

# vim: set noet sw=4 ts=4:
