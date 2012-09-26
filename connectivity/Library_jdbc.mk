# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 Peter Foley <pefoley2@verizon.net> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,jdbc))

$(eval $(call gb_Library_set_componentfile,jdbc,connectivity/source/drivers/jdbc/jdbc))

$(eval $(call gb_Library_use_sdk_api,jdbc))

$(eval $(call gb_Library_set_include,jdbc,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
))

$(eval $(call gb_Library_use_libraries,jdbc,\
	cppu \
	cppuhelper \
	sal \
	salhelper \
	jvmaccess \
	dbtools \
	utl \
	jvmfwk \
	comphelper \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,jdbc))

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
	connectivity/source/drivers/jdbc/jservices \
	connectivity/source/drivers/jdbc/JBigDecimal \
	connectivity/source/drivers/jdbc/tools \
	connectivity/source/drivers/jdbc/ContextClassLoader \
))

# vim: set noet sw=4 ts=4:
