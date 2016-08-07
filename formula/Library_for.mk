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



$(eval $(call gb_Library_Library,for))

$(eval $(call gb_Library_add_package_headers,for,formula_inc))

$(eval $(call gb_Library_add_precompiled_header,for,$(SRCDIR)/formula/inc/pch/precompiled_formula))

$(eval $(call gb_Library_set_componentfile,for,formula/util/for))

$(eval $(call gb_Library_set_include,for,\
	$$(INCLUDE) \
	-I$(SRCDIR)/formula/inc \
	-I$(SRCDIR)/formula/inc/pch \
	-I$(SRCDIR)/formula/source/core/inc \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_api,for, \
        udkapi \
	offapi \
))

$(eval $(call gb_Library_add_defs,for,\
	-DFORMULA_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,for,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	stl \
	svl \
	svt \
	tl \
	utl \
	vcl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,for,\
	formula/source/core/api/FormulaCompiler \
	formula/source/core/api/FormulaOpCodeMapperObj \
	formula/source/core/api/services \
	formula/source/core/api/token \
	formula/source/core/resource/core_resource \
))

# vim: set noet sw=4 ts=4:

