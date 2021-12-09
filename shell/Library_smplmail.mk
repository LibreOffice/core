# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,smplmail))

$(eval $(call gb_Library_use_sdk_api,smplmail))

$(eval $(call gb_Library_use_system_win32_libs,smplmail,\
	advapi32 \
))

$(eval $(call gb_Library_use_libraries,smplmail,\
	cppu \
	cppuhelper \
	i18nlangtag \
	sal \
	tl \
	utl \
))

$(eval $(call gb_Library_set_componentfile,smplmail,shell/source/win32/simplemail/smplmail,services))

$(eval $(call gb_Library_add_exception_objects,smplmail,\
    shell/source/win32/simplemail/smplmailclient \
    shell/source/win32/simplemail/smplmailmsg \
    shell/source/win32/simplemail/smplmailsuppl \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
