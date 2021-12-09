# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,dbase))

$(eval $(call gb_Library_set_componentfile,dbase,connectivity/source/drivers/dbase/dbase,services))

$(eval $(call gb_Library_use_external,dbase,boost_headers))

$(eval $(call gb_Library_use_sdk_api,dbase))

$(eval $(call gb_Library_set_include,dbase,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
	-I$(WORKDIR)/YaccTarget/connectivity/source/parse \
))

$(eval $(call gb_Library_set_precompiled_header,dbase,connectivity/inc/pch/precompiled_dbase))

$(eval $(call gb_Library_use_libraries,dbase,\
	cppu \
	cppuhelper \
	svl \
	tl \
	ucbhelper \
	sal \
	salhelper \
	dbtools \
	file \
	utl \
	comphelper \
	svt \
))

$(eval $(call gb_Library_add_exception_objects,dbase,\
	connectivity/source/drivers/dbase/DResultSet \
	connectivity/source/drivers/dbase/DStatement \
	connectivity/source/drivers/dbase/DPreparedStatement \
	connectivity/source/drivers/dbase/dindexnode \
	connectivity/source/drivers/dbase/DIndexIter \
	connectivity/source/drivers/dbase/DDatabaseMetaData \
	connectivity/source/drivers/dbase/DCatalog \
	connectivity/source/drivers/dbase/DColumns \
	connectivity/source/drivers/dbase/DIndexColumns \
	connectivity/source/drivers/dbase/DIndex \
	connectivity/source/drivers/dbase/DIndexes \
	connectivity/source/drivers/dbase/DTables \
	connectivity/source/drivers/dbase/DConnection \
	connectivity/source/drivers/dbase/DDriver \
	connectivity/source/drivers/dbase/DTable \
))

# vim: set noet sw=4 ts=4:
