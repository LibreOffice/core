# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,calc))

$(eval $(call gb_Library_set_componentfile,calc,connectivity/source/drivers/calc/calc,services))

$(eval $(call gb_Library_use_external,calc,boost_headers))

$(eval $(call gb_Library_use_sdk_api,calc))

$(eval $(call gb_Library_set_include,calc,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
	-I$(WORKDIR)/YaccTarget/connectivity/source/parse \
))

$(eval $(call gb_Library_set_precompiled_header,calc,connectivity/inc/pch/precompiled_calc))

$(eval $(call gb_Library_use_libraries,calc,\
	cppu \
	cppuhelper \
	svl \
	tl \
	utl \
	sal \
	salhelper \
	dbtools \
	file \
	comphelper \
))

$(eval $(call gb_Library_add_exception_objects,calc,\
	connectivity/source/drivers/calc/CDatabaseMetaData \
	connectivity/source/drivers/calc/CCatalog \
	connectivity/source/drivers/calc/CTable \
	connectivity/source/drivers/calc/CTables \
	connectivity/source/drivers/calc/CConnection \
	connectivity/source/drivers/calc/CDriver \
))

# vim: set noet sw=4 ts=4:
