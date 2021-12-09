# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ado))

$(eval $(call gb_Library_set_componentfile,ado,connectivity/source/drivers/ado/ado,services))

$(eval $(call gb_Library_set_include,ado,\
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
	$$(INCLUDE) \
	-I$(WORKDIR)/YaccTarget/connectivity/source/parse \
))

$(eval $(call gb_Library_use_external,ado,boost_headers))

$(eval $(call gb_Library_set_precompiled_header,ado,connectivity/inc/pch/precompiled_ado))

$(eval $(call gb_Library_use_sdk_api,ado))

ifeq ($(COM),GCC)
$(eval $(call gb_Library_add_cxxflags,ado,\
	-fpermissive \
))
endif

$(eval $(call gb_Library_use_system_win32_libs,ado,\
	ole32 \
	oleaut32 \
	uuid \
))

$(eval $(call gb_Library_use_libraries,ado,\
	cppu \
	cppuhelper \
	sal \
	salhelper \
	utl \
	dbtools \
	comphelper \
))

$(eval $(call gb_Library_add_exception_objects,ado,\
	connectivity/source/drivers/ado/ADatabaseMetaDataImpl \
	connectivity/source/drivers/ado/Aolevariant \
	connectivity/source/drivers/ado/ADatabaseMetaData \
	connectivity/source/drivers/ado/AColumn \
	connectivity/source/drivers/ado/AColumns \
	connectivity/source/drivers/ado/AIndex \
	connectivity/source/drivers/ado/AIndexes \
	connectivity/source/drivers/ado/AKey \
	connectivity/source/drivers/ado/AKeys \
	connectivity/source/drivers/ado/AUser \
	connectivity/source/drivers/ado/AUsers \
	connectivity/source/drivers/ado/AGroup \
	connectivity/source/drivers/ado/AGroups \
	connectivity/source/drivers/ado/ACatalog \
	connectivity/source/drivers/ado/AView \
	connectivity/source/drivers/ado/AViews \
	connectivity/source/drivers/ado/ATable \
	connectivity/source/drivers/ado/ATables \
	connectivity/source/drivers/ado/ACallableStatement \
	connectivity/source/drivers/ado/ADatabaseMetaDataResultSetMetaData \
	connectivity/source/drivers/ado/ADatabaseMetaDataResultSet \
	connectivity/source/drivers/ado/AResultSet \
	connectivity/source/drivers/ado/AConnection \
	connectivity/source/drivers/ado/AStatement \
	connectivity/source/drivers/ado/APreparedStatement \
	connectivity/source/drivers/ado/AResultSetMetaData \
	connectivity/source/drivers/ado/ADriver \
	connectivity/source/drivers/ado/Awrapado \
	connectivity/source/drivers/ado/adoimp \
))

# Runtime dependency for unit-tests
$(call gb_Library_get_target,ado) :| $(call gb_Library_get_target,affine_uno_uno)

# vim: set noet sw=4 ts=4:
