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

$(eval $(call gb_Library_Library,flash))

$(eval $(call gb_Library_set_componentfile,flash,filter/source/flash/flash))

$(eval $(call gb_Library_add_api,flash,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,flash,\
	-I$(SRCDIR)/filter/inc/pch \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_linked_libs,flash,\
	svt \
	vcl \
	utl \
	tl \
	comphelper \
	basegfx \
	cppuhelper \
	cppu \
	sal \
	$(gb_STDLIBS) \
))

$(call gb_Library_use_externals,flash,zlib)

$(eval $(call gb_Library_add_exception_objects,flash,\
	filter/source/flash/impswfdialog \
	filter/source/flash/swfdialog \
	filter/source/flash/swfexporter \
	filter/source/flash/swffilter \
	filter/source/flash/swfuno \
	filter/source/flash/swfwriter \
	filter/source/flash/swfwriter1 \
	filter/source/flash/swfwriter2 \
))

# vim: set noet sw=4 ts=4:
