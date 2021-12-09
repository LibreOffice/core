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

$(eval $(call gb_Library_Library,log))

$(eval $(call gb_Library_set_componentfile,log,extensions/source/logging/log,services))

$(eval $(call gb_Library_use_external,log,boost_headers))

$(eval $(call gb_Library_use_sdk_api,log))

$(eval $(call gb_Library_add_exception_objects,log,\
	extensions/source/logging/consolehandler \
	extensions/source/logging/csvformatter \
	extensions/source/logging/filehandler \
	extensions/source/logging/logger \
	extensions/source/logging/loggerconfig \
	extensions/source/logging/loghandler \
	extensions/source/logging/logrecord \
	extensions/source/logging/plaintextformatter \
	extensions/source/logging/simpletextformatter \
))

$(eval $(call gb_Library_use_libraries,log,\
	comphelper \
	cppuhelper \
	cppu \
	sal \
	tl \
))

# vim:set noet sw=4 ts=4:
