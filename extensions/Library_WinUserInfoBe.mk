# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Library_Library,WinUserInfoBe))

$(eval $(call gb_Library_set_componentfile,WinUserInfoBe,extensions/source/config/WinUserInfo/WinUserInfoBe,services))

$(eval $(call gb_Library_use_sdk_api,WinUserInfoBe))

$(eval $(call gb_Library_add_exception_objects,WinUserInfoBe,\
	extensions/source/config/WinUserInfo/WinUserInfoBe \
))

$(eval $(call gb_Library_use_libraries,WinUserInfoBe,\
	comphelper \
	cppuhelper \
	cppu \
	sal \
	tl \
))

$(eval $(call gb_Library_use_system_win32_libs,WinUserInfoBe,\
	Ole32 \
	OleAut32 \
	Activeds \
	Adsiid \
	Secur32 \
))

# vim:set noet sw=4 ts=4:
