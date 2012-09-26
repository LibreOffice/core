#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Library_Library,svgfilter))

$(eval $(call gb_Library_use_custom_headers,svgfilter,filter/source/svg))

$(eval $(call gb_Library_set_componentfile,svgfilter,filter/source/svg/svgfilter))

$(eval $(call gb_Library_add_defs,svgfilter,\
	-DUSE_MODERN_SPIRIT \
	-DFILTER_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_set_include,svgfilter,\
    $$(INCLUDE) \
    -I$(SRCDIR)/filter/inc \
))

$(eval $(call gb_Library_use_sdk_api,svgfilter))

$(eval $(call gb_Library_use_libraries,svgfilter,\
	svxcore \
	editeng \
	xo \
	svt \
	vcl \
	svl \
	utl \
	tl \
	sax \
	comphelper \
	basegfx \
	cppuhelper \
	cppu \
	sal \
	$(gb_UWINAPI) \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_use_externals,svgfilter,libxml2))

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
