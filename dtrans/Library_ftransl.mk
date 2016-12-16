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



$(eval $(call gb_Library_Library,ftransl))

$(eval $(call gb_Library_add_precompiled_header,ftransl,$(SRCDIR)/dtrans/inc/pch/precompiled_dtrans))

$(eval $(call gb_Library_set_componentfile,ftransl,dtrans/util/ftransl))

$(eval $(call gb_Library_set_include,ftransl,\
        $$(INCLUDE) \
	-I$(SRCDIR)/dtrans/inc/pch \
))

$(eval $(call gb_Library_add_api,ftransl,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,ftransl,\
	advapi32 \
	cppuhelper \
	cppu \
	gdi32 \
	ole32 \
	sal \
	stl \
	uwinapi \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,ftransl,\
	dtrans/source/win32/ftransl/ftransl \
	dtrans/source/win32/ftransl/ftranslentry \
	dtrans/source/win32/misc/ImplHelper \
))

# vim: set noet sw=4 ts=4:
