# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,macab1))

$(eval $(call gb_Library_set_componentfile,macab1,connectivity/source/drivers/macab/macab1,services))

$(eval $(call gb_Library_use_external,macab1,boost_headers))

$(eval $(call gb_Library_use_sdk_api,macab1))

$(eval $(call gb_Library_use_libraries,macab1,\
	comphelper \
	cppu \
	cppuhelper \
	dbtools \
	sal \
	salhelper \
	tl \
))

$(eval $(call gb_Library_set_include,macab1,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
	-I$(WORKDIR)/YaccTarget/connectivity/source/parse \
))

$(eval $(call gb_Library_add_exception_objects,macab1,\
	connectivity/source/drivers/macab/MacabDriver \
))

# vim: set noet sw=4 ts=4:
