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



$(eval $(call gb_Library_Library,mcnttype))

$(eval $(call gb_Library_add_precompiled_header,mcnttype,$(SRCDIR)/dtrans/inc/pch/precompiled_dtrans))

$(eval $(call gb_Library_set_componentfile,mcnttype,dtrans/util/mcnttype))

$(eval $(call gb_Library_set_include,mcnttype,\
        $$(INCLUDE) \
	-I$(SRCDIR)/dtrans/inc/pch \
))

$(eval $(call gb_Library_add_api,mcnttype,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,mcnttype,\
	cppuhelper \
	cppu \
	sal \
	stl \
	$(gb_STDLIBS) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_linked_libs,mcnttype,\
	uwinapi \
))
endif

$(eval $(call gb_Library_add_exception_objects,mcnttype,\
	dtrans/source/cnttype/mctfentry \
	dtrans/source/cnttype/mcnttfactory \
	dtrans/source/cnttype/mcnttype \
))

# vim: set noet sw=4 ts=4:
