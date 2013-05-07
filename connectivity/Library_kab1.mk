# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,kab1))

$(eval $(call gb_Library_use_externals,kab1, \
    boost_headers \
    kde \
))

$(eval $(call gb_Library_set_componentfile,kab1,connectivity/source/drivers/kab/kab1))

$(eval $(call gb_Library_use_sdk_api,kab1))

$(eval $(call gb_Library_set_include,kab1,\
	-I$(SRCDIR)/connectivity/source/inc \
	$$(INCLUDE) \
	-I$(WORKDIR)/YaccTarget/connectivity/source/parse \
))

$(eval $(call gb_Library_use_libraries,kab1,\
	comphelper \
	cppu \
	cppuhelper \
	dbtools \
	sal \
	salhelper \
))

$(eval $(call gb_Library_add_exception_objects,kab1,\
	connectivity/source/drivers/kab/KDriver \
	connectivity/source/drivers/kab/KServices \
))

# vim: set noet sw=4 ts=4:
