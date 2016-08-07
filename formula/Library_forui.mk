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



$(eval $(call gb_Library_Library,forui))

$(eval $(call gb_Library_add_package_headers,forui,formula_inc))

$(eval $(call gb_Library_set_include,forui,\
	$$(INCLUDE) \
	-I$(SRCDIR)/formula/inc \
	-I$(SRCDIR)/formula/inc/pch \
	-I$(SRCDIR)/formula/source/ui/inc \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_api,forui, \
        udkapi \
        offapi \
))

$(eval $(call gb_Library_add_defs,forui,\
	-DFORMULA_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,forui,\
	cppu \
	cppuhelper \
	for \
	sal \
	sfx \
	stl \
	svl \
	svt \
	tl \
	utl \
	vcl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,forui,\
	formula/source/ui/dlg/formula \
	formula/source/ui/dlg/FormulaHelper \
	formula/source/ui/dlg/parawin \
	formula/source/ui/dlg/funcutl \
	formula/source/ui/dlg/funcpage \
	formula/source/ui/dlg/structpg \
	formula/source/ui/resource/ModuleHelper \
))

# vim: set noet sw=4 ts=4:

