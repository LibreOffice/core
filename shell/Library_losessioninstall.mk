# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,losessioninstall))

$(eval $(call gb_Library_set_componentfile,losessioninstall,shell/source/sessioninstall/losessioninstall,services))

$(eval $(call gb_Library_use_api,losessioninstall,\
    offapi \
    udkapi \
))

$(eval $(call gb_Library_use_libraries,losessioninstall,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
))

$(eval $(call gb_Library_use_externals,losessioninstall,\
	boost_headers \
    dbus \
    gio \
))

$(eval $(call gb_Library_add_exception_objects,losessioninstall,\
	shell/source/sessioninstall/SyncDbusSessionHelper \
))

# vim:set noet sw=4 ts=4:
