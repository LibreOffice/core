# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,dbpool2))

$(eval $(call gb_Library_set_componentfile,dbpool2,connectivity/source/cpool/dbpool2,services))

$(eval $(call gb_Library_set_include,dbpool2,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
))

$(eval $(call gb_Library_set_precompiled_header,dbpool2,connectivity/inc/pch/precompiled_dbpool2))

$(eval $(call gb_Library_use_sdk_api,dbpool2))

$(eval $(call gb_Library_use_libraries,dbpool2,\
	comphelper \
	cppu \
	cppuhelper \
	dbtools \
	sal \
	salhelper \
	tl \
))

$(eval $(call gb_Library_add_exception_objects,dbpool2,\
	connectivity/source/cpool/ZConnectionWrapper \
	connectivity/source/cpool/ZDriverWrapper \
	connectivity/source/cpool/ZPooledConnection \
	connectivity/source/cpool/ZConnectionPool \
	connectivity/source/cpool/ZPoolCollection \
))

# vim: set noet sw=4 ts=4:
