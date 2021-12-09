# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,flat))

$(eval $(call gb_Library_set_componentfile,flat,connectivity/source/drivers/flat/flat,services))

$(eval $(call gb_Library_use_external,flat,boost_headers))

$(eval $(call gb_Library_use_sdk_api,flat))

$(eval $(call gb_Library_set_precompiled_header,flat,connectivity/inc/pch/precompiled_flat))

$(eval $(call gb_Library_set_include,flat,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
	-I$(WORKDIR)/YaccTarget/connectivity/source/parse \
))

$(eval $(call gb_Library_use_libraries,flat,\
	cppu \
	cppuhelper \
	tl \
	svl \
	utl \
	i18nlangtag \
	sal \
	salhelper \
	dbtools \
	file \
	comphelper \
))

$(eval $(call gb_Library_add_exception_objects,flat,\
	connectivity/source/drivers/flat/EResultSet \
	connectivity/source/drivers/flat/EStatement \
	connectivity/source/drivers/flat/EPreparedStatement \
	connectivity/source/drivers/flat/ETable \
	connectivity/source/drivers/flat/EDatabaseMetaData \
	connectivity/source/drivers/flat/ECatalog \
	connectivity/source/drivers/flat/EColumns \
	connectivity/source/drivers/flat/ETables \
	connectivity/source/drivers/flat/EConnection \
	connectivity/source/drivers/flat/EDriver \
))

# vim: set noet sw=4 ts=4:
