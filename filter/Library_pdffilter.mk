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

$(eval $(call gb_Library_Library,pdffilter))

$(eval $(call gb_Library_set_componentfile,pdffilter,filter/source/pdf/pdffilter))

$(eval $(call gb_Library_use_external,pdffilter,boost_headers))

$(eval $(call gb_Library_use_sdk_api,pdffilter))

$(eval $(call gb_Library_set_include,pdffilter,\
	-I$(SRCDIR)/filter/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_custom_headers,pdffilter,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_libraries,pdffilter,\
	svt \
	svx \
	sfx \
	tk \
	vcl \
	utl \
	tl \
	svl \
	i18nlangtag \
	comphelper \
	basegfx \
	cppuhelper \
	cppu \
	sal \
	drawinglayercore \
	drawinglayer \
))

$(eval $(call gb_Library_add_exception_objects,pdffilter,\
	filter/source/pdf/impdialog \
	filter/source/pdf/pdfdecomposer \
	filter/source/pdf/pdfdialog \
	filter/source/pdf/pdfexport \
	filter/source/pdf/pdffilter \
	filter/source/pdf/pdfinteract \
))

# vim: set noet sw=4 ts=4:
