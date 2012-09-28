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

$(eval $(call gb_Library_Library,wpftwriter))

$(eval $(call gb_Library_set_componentfile,wpftwriter,writerperfect/util/wpftwriter))

$(eval $(call gb_Library_set_include,wpftwriter,\
	$$(INCLUDE) \
    -I$(SRCDIR)/writerperfect/source \
))

$(eval $(call gb_Library_use_sdk_api,wpftwriter))

$(eval $(call gb_Library_use_libraries,wpftwriter,\
	comphelper \
	cppu \
	cppuhelper \
	vcl \
	sal \
	sfx \
	sot \
	tl \
	ucbhelper \
	utl \
	xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_static_libraries,wpftwriter,\
	writerperfect \
))

$(eval $(call gb_Library_use_externals,wpftwriter,\
	wpd \
	wpg \
	wps \
))

$(eval $(call gb_Library_add_exception_objects,wpftwriter,\
	writerperfect/source/writer/MSWorksImportFilter \
	writerperfect/source/writer/WordPerfectImportFilter \
	writerperfect/source/writer/wpftwriter_genericfilter \
))

# vim: set noet sw=4 ts=4:
