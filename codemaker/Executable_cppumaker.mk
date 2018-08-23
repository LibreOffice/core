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



$(eval $(call gb_Executable_Executable,cppumaker))

$(eval $(call gb_Executable_set_targettype_gui,cppumaker,NO))

$(eval $(call gb_StaticLibrary_add_package_headers,cppumaker,codemaker_inc))

$(eval $(call gb_Library_add_precompiled_header,cppumaker,$(SRCDIR)/commoncpp/inc/pch/precompiled_cppumaker))

$(eval $(call gb_Executable_set_include,cppumaker,\
	$$(INCLUDE) \
	-I$(SRCDIR)/codemaker/inc/ \
	-I$(SRCDIR)/codemaker/inc/pch \
))

#$(eval $(call gb_Executable_add_defs,cppumaker,\
#	-D_TOOLS_STRINGLIST \
#))

$(eval $(call gb_Executable_add_linked_libs,cppumaker,\
	reg \
	sal \
	salhelper \
	stl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Executable_add_linked_static_libs,cppumaker,\
	codemaker \
	commoncpp \
))

#$(eval $(call gb_Executable_add_private_apis,cppumaker,\
#	$(SRCDIR)/codemaker/test/cppumaker/types.idl \
#))

$(eval $(call gb_Executable_add_exception_objects,cppumaker,\
	codemaker/source/cppumaker/cppumaker \
	codemaker/source/cppumaker/cppuoptions \
	codemaker/source/cppumaker/cpputype \
	codemaker/source/cppumaker/dumputils \
	codemaker/source/cppumaker/includes \
))

$(eval $(call gb_LinkTarget_set_cxx_optimization, \
	cppumaker/source/cppumaker/cpputype, $(gb_COMPILERNOOPTFLAGS) \
))

# vim: set noet sw=4 ts=4:
