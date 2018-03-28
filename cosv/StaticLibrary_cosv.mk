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



$(eval $(call gb_StaticLibrary_StaticLibrary,cosv))

$(eval $(call gb_StaticLibrary_add_package_headers,cosv,cosv_inc))

#$(eval $(call gb_StaticLibrary_add_api,cosv,\
#	udkapi \
#	offapi \
#))

$(eval $(call gb_StaticLibrary_set_include,cosv,\
	-I$(SRCDIR)/cosv/inc \
	-I$(SRCDIR)/cosv/source/inc \
	$$(INCLUDE) \
))

#$(eval $(call gb_StaticLibrary_add_defs,cosv,\
#	-DBASEGFX_STATICLIBRARY \
#))

# copied sources are generated cxx sources
$(eval $(call gb_StaticLibrary_add_exception_objects,cosv,\
	cosv/source/service/comdline \
	cosv/source/service/comfunc \
	cosv/source/service/csv_ostream \
	cosv/source/service/datetime \
	cosv/source/service/std_outp \
	cosv/source/storage/dirchain \
	cosv/source/storage/file \
	cosv/source/storage/mbstream \
	cosv/source/storage/persist \
	cosv/source/storage/ploc \
	cosv/source/storage/ploc_dir \
	cosv/source/storage/plocroot \
	cosv/source/strings/str_types \
	cosv/source/strings/streamstr \
	cosv/source/strings/string \
	cosv/source/comphelp/badcast \
))

# vim: set noet sw=4 ts=4:
