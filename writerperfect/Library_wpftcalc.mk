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

$(eval $(call gb_Library_Library,wpftcalc))

$(eval $(call gb_Library_set_include,wpftcalc,\
	-I$(SRCDIR)/writerperfect/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_set_componentfile,wpftcalc,writerperfect/source/calc/wpftcalc))

$(eval $(call gb_Library_use_sdk_api,wpftcalc))

$(eval $(call gb_Library_use_libraries,wpftcalc,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sot \
	svx \
	tl \
	utl \
	vcl \
	writerperfect \
	xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_externals,wpftcalc,\
	boost_headers \
	odfgen \
	revenge \
	mwaw \
	wps \
	zlib \
	lcms2 \
	libxml2 \
	icui18n \
	icuuc \
))

$(eval $(call gb_Library_add_exception_objects,wpftcalc,\
	writerperfect/source/calc/MSWorksCalcImportFilter \
	writerperfect/source/calc/MWAWCalcImportFilter \
	writerperfect/source/calc/wpftcalc_genericfilter \
))

# vim: set noet sw=4 ts=4:
