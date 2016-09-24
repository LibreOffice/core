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

$(eval $(call gb_Library_Library,wpftimpress))

$(eval $(call gb_Library_set_include,wpftimpress,\
       -I$(SRCDIR)/writerperfect/inc \
       $$(INCLUDE) \
))

$(eval $(call gb_Library_set_componentfile,wpftimpress,writerperfect/source/impress/wpftimpress))

$(eval $(call gb_Library_use_sdk_api,wpftimpress))

$(eval $(call gb_Library_use_libraries,wpftimpress,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sot \
	svx \
	tl \
	ucbhelper \
	utl \
	vcl \
	writerperfect \
	xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_externals,wpftimpress,\
	boost_headers \
	etonyek \
	mwaw \
	odfgen \
	revenge \
	zlib \
	libxml2 \
))

$(eval $(call gb_Library_add_exception_objects,wpftimpress,\
	writerperfect/source/impress/KeynoteImportFilter \
	writerperfect/source/impress/MWAWPresentationImportFilter \
))

# vim: set noet sw=4 ts=4:
