# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,jdbc))

$(eval $(call gb_Library_set_componentfile,jdbc,connectivity/source/drivers/jdbc/jdbc,services))

$(eval $(call gb_Library_use_external,jdbc,boost_headers))

$(eval $(call gb_Library_use_sdk_api,jdbc))

$(eval $(call gb_Library_set_include,jdbc,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
	-I$(WORKDIR)/YaccTarget/connectivity/source/parse \
))

$(eval $(call gb_Library_use_common_precompiled_header,jdbc))

$(eval $(call gb_Library_use_libraries,jdbc,\
	cppu \
	cppuhelper \
	sal \
	salhelper \
	jvmaccess \
	dbtools \
	tl \
	utl \
	jvmfwk \
	comphelper \
))

$(eval $(call gb_Library_add_exception_objects,jdbc,\
	connectivity/source/drivers/jdbc/Array \
	connectivity/source/drivers/jdbc/Blob \
	connectivity/source/drivers/jdbc/Boolean \
	connectivity/source/drivers/jdbc/CallableStatement \
	connectivity/source/drivers/jdbc/Class \
	connectivity/source/drivers/jdbc/Clob \
	connectivity/source/drivers/jdbc/ConnectionLog \
	connectivity/source/drivers/jdbc/DatabaseMetaData \
	connectivity/source/drivers/jdbc/Date \
	connectivity/source/drivers/jdbc/DriverPropertyInfo \
	connectivity/source/drivers/jdbc/Exception \
	connectivity/source/drivers/jdbc/InputStream \
	connectivity/source/drivers/jdbc/JConnection \
	connectivity/source/drivers/jdbc/JDriver \
	connectivity/source/drivers/jdbc/JStatement \
	connectivity/source/drivers/jdbc/Object \
	connectivity/source/drivers/jdbc/PreparedStatement \
	connectivity/source/drivers/jdbc/Reader \
	connectivity/source/drivers/jdbc/Ref \
	connectivity/source/drivers/jdbc/ResultSet \
	connectivity/source/drivers/jdbc/ResultSetMetaData \
	connectivity/source/drivers/jdbc/SQLException \
	connectivity/source/drivers/jdbc/SQLWarning \
	connectivity/source/drivers/jdbc/String \
	connectivity/source/drivers/jdbc/Throwable \
	connectivity/source/drivers/jdbc/Timestamp \
	connectivity/source/drivers/jdbc/JBigDecimal \
	connectivity/source/drivers/jdbc/tools \
	connectivity/source/drivers/jdbc/ContextClassLoader \
))

# Runtime dependency for unit-tests
$(call gb_Library_get_target,jdbc) :| $(call gb_Library_get_target,affine_uno_uno)

# vim: set noet sw=4 ts=4:
