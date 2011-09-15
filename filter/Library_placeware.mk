#***************************************************************
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#***************************************************************

$(eval $(call gb_Library_Library,placeware))

$(eval $(call gb_Library_set_componentfile,placeware,filter/source/placeware/placeware))

$(eval $(call gb_Library_add_api,placeware,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,placeware,\
	-I$(SRCDIR)/filter/inc/pch \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_linked_libs,placeware,\
	cppuhelper \
	cppu \
	sal \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,placeware,\
	filter/source/placeware/Base64Codec \
	filter/source/placeware/exporter \
	filter/source/placeware/filter \
	filter/source/placeware/tempfile \
	filter/source/placeware/uno \
	filter/source/placeware/zip \
))

# vim: set noet sw=4 ts=4:
