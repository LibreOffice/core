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



$(eval $(call gb_Library_Library,sax))

$(eval $(call gb_Library_add_api,sax,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_add_package_headers,sax,\
	sax_inc \
))

$(eval $(call gb_Library_set_include,sax,\
	$$(INCLUDE) \
	-I$(SRCDIR)/sax/inc \
))

$(eval $(call gb_Library_add_defs,sax,\
	-DSAX_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,sax,\
	sal \
	cppu \
	cppuhelper \
	comphelper \
	stl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,sax,\
	sax/source/tools/converter \
	sax/source/tools/fastattribs \
	sax/source/tools/fastserializer \
	sax/source/tools/fshelper \
))

# vim: set noet sw=4 ts=4:
