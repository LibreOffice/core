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

$(eval $(call gb_Library_Library,lng))

$(eval $(call gb_Library_set_componentfile,lng,linguistic/source/lng,services))

$(eval $(call gb_Library_set_include,lng,\
	$$(INCLUDE) \
	-I$(SRCDIR)/linguistic/inc \
))

$(eval $(call gb_Library_use_sdk_api,lng))

$(eval $(call gb_Library_add_defs,lng,\
	-DLNG_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_libraries,lng,\
	comphelper \
	cppu \
	cppuhelper \
	i18nlangtag \
	sal \
	sax \
	svl \
	tl \
	ucbhelper \
	utl \
	vcl \
	xo \
))

$(eval $(call gb_Library_use_externals,lng,\
	boost_headers \
	icuuc \
	icu_headers \
))

$(eval $(call gb_Library_add_exception_objects,lng,\
	linguistic/source/convdic \
	linguistic/source/convdiclist \
	linguistic/source/convdicxml \
	linguistic/source/dicimp \
	linguistic/source/dlistimp \
	linguistic/source/gciterator \
	linguistic/source/hhconvdic \
	linguistic/source/hyphdsp \
	linguistic/source/hyphdta \
	linguistic/source/iprcache \
	linguistic/source/lngopt \
	linguistic/source/lngprophelp \
	linguistic/source/lngsvcmgr \
	linguistic/source/misc \
	linguistic/source/misc2 \
	linguistic/source/spelldsp \
	linguistic/source/spelldta \
	linguistic/source/thesdsp \
))

# vim: set noet sw=4 ts=4:
