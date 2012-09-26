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

$(eval $(call gb_Library_Library,macabdrv1))

$(eval $(call gb_Library_use_sdk_api,macabdrv1))

$(eval $(call gb_Library_add_libs,macabdrv1,\
	-framework Carbon \
	-framework AddressBook \
))

$(eval $(call gb_Library_use_libraries,macabdrv1,\
	comphelper \
	cppu \
	cppuhelper \
	dbtools \
	sal \
	salhelper \
))

$(eval $(call gb_Library_add_standard_system_libs,macabdrv1))

$(eval $(call gb_Library_set_include,macabdrv1,\
        $$(INCLUDE) \
        -I$(SRCDIR)/connectivity/inc \
        -I$(SRCDIR)/connectivity/source/inc \
))

$(eval $(call gb_Library_add_exception_objects,macabdrv1,\
	connectivity/source/drivers/macab/MacabColumns \
	connectivity/source/drivers/macab/MacabTable \
	connectivity/source/drivers/macab/MacabTables \
	connectivity/source/drivers/macab/MacabCatalog \
	connectivity/source/drivers/macab/MacabResultSet \
	connectivity/source/drivers/macab/MacabStatement \
	connectivity/source/drivers/macab/MacabPreparedStatement \
	connectivity/source/drivers/macab/MacabDatabaseMetaData \
	connectivity/source/drivers/macab/MacabConnection \
	connectivity/source/drivers/macab/MacabResultSetMetaData \
	connectivity/source/drivers/macab/macabcondition \
	connectivity/source/drivers/macab/macaborder \
	connectivity/source/drivers/macab/MacabRecord \
	connectivity/source/drivers/macab/MacabRecords \
	connectivity/source/drivers/macab/MacabHeader \
	connectivity/source/drivers/macab/MacabGroup \
	connectivity/source/drivers/macab/MacabAddressBook \
))

# vim: set noet sw=4 ts=4:
