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



$(eval $(call gb_Library_Library,jvmaccess))

ifeq ($(COMNAME),msci)
$(eval $(call gb_Library_set_versionmap,jvmaccess,$(SRCDIR)/jvmaccess/util/msvc_win32_intel.map))
else ifeq ($(COMNAME),sunpro5)
$(eval $(call gb_Library_set_versionmap,jvmaccess,$(SRCDIR)/jvmaccess/util/cc5_solaris_sparc.map))
else ifeq ($(GUI)$(COM),WNTGCC)
$(eval $(call gb_Library_set_versionmap,jvmaccess,$(SRCDIR)/jvmaccess/util/mingw.map))
else ifeq ($(COMNAME),gcc3)
$(eval $(call gb_Library_set_versionmap,jvmaccess,$(SRCDIR)/jvmaccess/util/gcc3.map))
endif

$(eval $(call gb_Library_set_include,jvmaccess,\
        $$(INCLUDE) \
	-I$(SRCDIR)/jvmaccess/inc \
))

$(eval $(call gb_Library_add_api,jvmaccess,\
	udkapi \
))

$(eval $(call gb_Library_add_defs,jvmaccess,\
	-DJVMACCESS_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,jvmaccess,\
	cppu \
	sal \
	salhelper \
	stl \
	$(gb_STDLIBS) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_linked_libs,jvmaccess,\
	advapi32 \
))
endif


$(eval $(call gb_Library_add_exception_objects,jvmaccess,\
	jvmaccess/source/classpath \
	jvmaccess/source/unovirtualmachine \
	jvmaccess/source/virtualmachine \
))

# vim: set noet sw=4 ts=4:
