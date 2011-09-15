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

$(eval $(call gb_Library_Library,svgfilter))

$(eval $(call gb_Library_add_package_headers,svgfilter,filter_generated))

$(eval $(call gb_Library_set_componentfile,svgfilter,filter/source/svg/svgfilter))

$(eval $(call gb_Library_add_defs,svgfilter,\
	-DUSE_MODERN_SPIRIT \
))

$(eval $(call gb_Library_add_api,svgfilter,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,svgfilter,\
	-I$(SRCDIR)/filter/inc/pch \
	$$(INCLUDE) \
	-I$(WORKDIR)/CustomTarget/filter/source/svg \
))

$(eval $(call gb_Library_add_linked_libs,svgfilter,\
	svxcore \
	editeng \
	xo \
	svt \
	vcl \
	svl \
	utl \
	tl \
	comphelper \
	basegfx \
	cppuhelper \
	cppu \
	sal \
	$(gb_STDLIBS) \
))

$(call gb_Library_use_externals,svgfilter,libxml2)

$(eval $(call gb_Library_add_exception_objects,svgfilter,\
	filter/source/svg/b2dellipse \
	filter/source/svg/impsvgdialog \
	filter/source/svg/parserfragments \
	filter/source/svg/svgdialog \
	filter/source/svg/svgexport \
	filter/source/svg/svgfilter \
	filter/source/svg/svgfontexport \
	filter/source/svg/svgimport \
	filter/source/svg/svgreader \
	filter/source/svg/svgwriter \
	filter/source/svg/tokenmap \
	filter/source/svg/units \
))

# vim: set noet sw=4 ts=4:
