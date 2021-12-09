# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,cmdmail))

$(eval $(call gb_Library_use_sdk_api,cmdmail))

$(eval $(call gb_Library_use_libraries,cmdmail,\
	cppu \
	cppuhelper \
	sal \
	tl \
))

$(eval $(call gb_Library_set_componentfile,cmdmail,shell/source/cmdmail/cmdmail,services))

$(eval $(call gb_Library_add_exception_objects,cmdmail,\
    shell/source/cmdmail/cmdmailmsg \
    shell/source/cmdmail/cmdmailsuppl \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
