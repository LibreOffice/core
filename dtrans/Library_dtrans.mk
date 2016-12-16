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



$(eval $(call gb_Library_Library,dtrans))

$(eval $(call gb_Library_add_precompiled_header,dtrans,$(SRCDIR)/dtrans/inc/pch/precompiled_dtrans))

$(eval $(call gb_Library_set_componentfile,dtrans,dtrans/source/generic/dtrans))

$(eval $(call gb_Library_set_include,dtrans,\
        $$(INCLUDE) \
	-I$(SRCDIR)/dtrans/inc/pch \
))

$(eval $(call gb_Library_add_api,dtrans,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,dtrans,\
	cppuhelper \
	cppu \
	sal \
	stl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,dtrans,\
	dtrans/source/generic/generic_clipboard \
	dtrans/source/generic/clipboardmanager \
	dtrans/source/generic/dtrans \
))

# vim: set noet sw=4 ts=4:
