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



$(eval $(call gb_Library_Library,salhelper))

$(eval $(call gb_Library_add_package_headers,salhelper,salhelper_inc))

$(eval $(call gb_Library_set_include,salhelper,\
	$$(INCLUDE) \
	-I$(SRCDIR)/salhelper/inc \
))

#$(eval $(call gb_Library_add_api,salhelper, \
#        udkapi \
#	offapi \
#))

$(eval $(call gb_Library_add_defs,salhelper,\
	-DSALHELPER_DLLIMPLEMENTATION \
))

ifeq ($(COMNAME),msci)
$(eval $(call gb_Library_set_versionmap,salhelper,$(SRCDIR)/salhelper/source/msci.map))
else ifeq ($(COMNAME),mscx)
$(eval $(call gb_Library_set_versionmap,salhelper,$(SRCDIR)/salhelper/source/mscx.map))
else ifeq ($(GUI),OS2)
$(eval $(call gb_Library_set_versionmap,salhelper,$(SRCDIR)/salhelper/source/gcc3os2.map))
else ifeq ($(COMNAME),sunpro5)
$(eval $(call gb_Library_set_versionmap,salhelper,$(SRCDIR)/salhelper/source/sols.map))
else ifeq ($(COMNAME),gcc3)
$(eval $(call gb_Library_set_versionmap,salhelper,$(SRCDIR)/salhelper/source/gcc3.map))
endif

$(eval $(call gb_Library_add_linked_libs,salhelper,\
	sal \
	stl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,salhelper,\
	salhelper/source/condition \
	salhelper/source/dynload \
	salhelper/source/simplereferenceobject \
))

# vim: set noet sw=4 ts=4:

