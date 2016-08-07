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



$(eval $(call gb_Executable_Executable,svidl))

$(eval $(call gb_Executable_add_api,svidl,\
	udkapi \
	offapi \
))

$(eval $(call gb_Executable_set_include,svidl,\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/ \
	-I$(SRCDIR)/idl/inc/pch \
	-I$(SRCDIR)/idl/inc \
))

$(eval $(call gb_Executable_add_defs,svidl,\
	-DIDL_COMPILER \
))

$(eval $(call gb_Executable_add_linked_libs,svidl,\
	tl \
	sal \
	stl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Executable_add_exception_objects,svidl,\
	idl/source/cmptools/char \
	idl/source/cmptools/hash \
	idl/source/cmptools/lex \
	idl/source/objects/basobj \
	idl/source/objects/bastype \
	idl/source/objects/module \
	idl/source/objects/object \
	idl/source/objects/slot \
	idl/source/objects/types \
	idl/source/prj/command \
	idl/source/prj/database \
	idl/source/prj/globals \
	idl/source/prj/svidl \
))

# vim: set noet sw=4 ts=4:
