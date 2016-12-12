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



$(eval $(call gb_StaticLibrary_StaticLibrary,udm))

ifeq ($(OS),WNT)
$(eval $(call gb_StaticLibrary_add_cflags,udm,-GR))
else ifeq ($(OS),LINUX)
$(eval $(call gb_StaticLibrary_add_cflags,udm,-frtti))
else ifeq ($(OS),FREEBSD)
$(eval $(call gb_StaticLibrary_add_cflags,udm,-frtti))
else ifeq ($(OS),NETBSD)
$(eval $(call gb_StaticLibrary_add_cflags,udm,-frtti))
endif

$(eval $(call gb_StaticLibrary_set_include,udm,\
        $$(INCLUDE) \
	-I$(SRCDIR)/udm/source/inc \
	-I$(SRCDIR)/udm/inc \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,udm,\
	udm/source/html/htmlitem \
	udm/source/xml/xmlitem \
))

# vim: set noet sw=4 ts=4:
