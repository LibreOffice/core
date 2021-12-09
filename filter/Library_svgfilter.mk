# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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

$(eval $(call gb_Library_set_componentfile,svgfilter,filter/source/svg/svgfilter,services))

$(eval $(call gb_Library_add_defs,svgfilter,\
	-DBOOST_ALL_NO_LIB \
	-DUSE_MODERN_SPIRIT \
	-DFILTER_DLLIMPLEMENTATION \
))

ifeq ($(COM),MSC)
$(eval $(call gb_Library_add_defs,svgfilter,\
	-D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING \
	-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING \
))
endif

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
	i18nlangtag \
	sax \
	salhelper \
	comphelper \
	drawinglayer \
	basegfx \
	cppuhelper \
	cppu \
	sal \
))

$(eval $(call gb_Library_use_externals,svgfilter,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_Library_add_exception_objects,svgfilter,\
	filter/source/svg/svgfilter \
	filter/source/svg/svgexport \
	filter/source/svg/svgfontexport \
	filter/source/svg/svgwriter \
))

# vim: set noet sw=4 ts=4:
