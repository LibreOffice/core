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



$(eval $(call gb_Library_Library,java_uno_accessbridge))

$(eval $(call gb_Library_add_precompiled_header,java_uno_accessbridge,$(SRCDIR)/accessibility/inc/pch/precompiled_accessibility_accessbridge))

$(eval $(call gb_Library_set_include,java_uno_accessbridge,\
	$$(INCLUDE) \
	-I$(SRCDIR)/accessibility/inc \
	-I$(SRCDIR)/accessibility/inc/pch \
	-I$(SRCDIR)/accessibility/source/inc \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_api,java_uno_accessbridge, \
	offapi \
        udkapi \
))


#$(eval $(call gb_Library_add_defs,java_uno_accessbridge,\
#	-DFORMULA_DLLIMPLEMENTATION \
#))

$(eval $(call gb_Library_add_linked_libs,java_uno_accessbridge,\
	cppu \
	jvmaccess \
	sal \
	stl \
	vcl \
	tl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,java_uno_accessbridge,\
	accessibility/bridge/source/java/WindowsAccessBridgeAdapter \
))

# vim: set noet sw=4 ts=4:

