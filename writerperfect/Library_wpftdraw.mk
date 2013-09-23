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

$(eval $(call gb_Library_Library,wpftdraw))

$(eval $(call gb_Library_set_componentfile,wpftdraw,writerperfect/util/wpftdraw))

$(eval $(call gb_Library_set_include,wpftdraw,\
	$$(INCLUDE) \
    -I$(SRCDIR)/writerperfect/source \
))

$(eval $(call gb_Library_use_sdk_api,wpftdraw))

$(eval $(call gb_Library_use_libraries,wpftdraw,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sot \
	tl \
	utl \
	xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_static_libraries,wpftdraw,\
	writerperfect \
))

$(eval $(call gb_Library_use_externals,wpftdraw,\
	cdr \
	mspub \
	odfgen \
	visio \
	wpg \
	wpd \
	zlib \
	lcms2 \
	libxml2 \
	icui18n \
	icuuc \
	icudata \
))

$(eval $(call gb_Library_add_exception_objects,wpftdraw,\
	writerperfect/source/draw/CDRImportFilter \
	writerperfect/source/draw/CMXImportFilter \
	writerperfect/source/draw/MSPUBImportFilter \
	writerperfect/source/draw/VisioImportFilter \
	writerperfect/source/draw/WPGImportFilter \
	writerperfect/source/draw/wpftdraw_genericfilter \
))

# vim: set noet sw=4 ts=4:
