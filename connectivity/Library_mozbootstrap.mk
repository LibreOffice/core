# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,mozbootstrap))

$(eval $(call gb_Library_set_componentfile,mozbootstrap,connectivity/source/drivers/mozab/bootstrap/mozbootstrap,services))

$(eval $(call gb_Library_set_include,mozbootstrap,\
	-I$(SRCDIR)/connectivity/source/drivers/mozab \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,mozbootstrap))

$(eval $(call gb_Library_use_libraries,mozbootstrap,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
))

$(eval $(call gb_Library_add_exception_objects,mozbootstrap,\
	connectivity/source/drivers/mozab/bootstrap/MMozillaBootstrap \
	connectivity/source/drivers/mozab/bootstrap/MNSFolders \
	connectivity/source/drivers/mozab/bootstrap/MNSINIParser \
	connectivity/source/drivers/mozab/bootstrap/MNSProfileDiscover \
))

# vim: set noet sw=4 ts=4:
