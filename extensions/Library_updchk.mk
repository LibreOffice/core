# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,updchk))

$(eval $(call gb_Library_set_componentfile,updchk,extensions/source/update/check/updchk.uno))

$(eval $(call gb_Library_set_include,updchk,\
	$$(INCLUDE) \
	-I$(SRCDIR)/extensions/inc \
))

$(eval $(call gb_Library_use_sdk_api,updchk))

$(eval $(call gb_Library_use_libraries,updchk,\
	cppuhelper \
	cppu \
	sal \
	salhelper \
	tl \
	utl \
	vcl \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,updchk,\
	ole32 \
	shell32 \
	wininet \
))

$(eval $(call gb_Library_add_exception_objects,updchk,\
	extensions/source/update/check/onlinecheck \
))
endif # OS WNT

$(eval $(call gb_Library_use_externals,updchk,\
	boost_headers \
    curl \
))

$(eval $(call gb_Library_add_exception_objects,updchk,\
	extensions/source/update/check/download \
	extensions/source/update/check/updatecheck \
	extensions/source/update/check/updatecheckconfig \
	extensions/source/update/check/updatecheckjob \
	extensions/source/update/check/updatehdl \
	extensions/source/update/check/updateprotocol \
))

# vim:set noet sw=4 ts=4:
