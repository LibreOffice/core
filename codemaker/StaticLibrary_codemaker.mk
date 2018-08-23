###############################################################
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
###############################################################



$(eval $(call gb_StaticLibrary_StaticLibrary,codemaker))

$(eval $(call gb_StaticLibrary_add_package_headers,codemaker,codemaker_inc))

$(eval $(call gb_Library_add_precompiled_header,codemaker,$(SRCDIR)/codemaker/inc/pch/precompiled_codemaker))

#$(eval $(call gb_StaticLibrary_add_api,codemaker,\
#	udkapi \
#	offapi \
#))

$(eval $(call gb_StaticLibrary_set_include,codemaker,\
	-I$(SRCDIR)/codemaker/inc \
        -I$(SRCDIR)/codemaker/inc/pch \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,codemaker,\
	codemaker/source/codemaker/codemaker \
	codemaker/source/codemaker/dependencies \
	codemaker/source/codemaker/exceptiontree \
	codemaker/source/codemaker/global \
	codemaker/source/codemaker/options \
	codemaker/source/codemaker/typemanager \
	codemaker/source/codemaker/unotype \
))

# vim: set noet sw=4 ts=4:
