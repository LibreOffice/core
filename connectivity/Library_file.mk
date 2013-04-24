# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,file))

$(eval $(call gb_Library_use_packages,file,\
	connectivity_generated \
))

$(eval $(call gb_Library_add_defs,file,\
	-DOOO_DLLIMPLEMENTATION_FILE \
))

$(eval $(call gb_Library_set_precompiled_header,file,$(SRCDIR)/connectivity/inc/pch/precompiled_file))

$(eval $(call gb_Library_set_include,file,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
))

$(eval $(call gb_Library_use_external,file,boost_headers))

$(eval $(call gb_Library_use_sdk_api,file))

$(eval $(call gb_Library_use_libraries,file,\
	cppu \
	cppuhelper \
	svl \
	tl \
	ucbhelper \
	sal \
	salhelper \
	dbtools \
	utl \
	comphelper \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,file,\
	connectivity/source/drivers/file/FCatalog \
	connectivity/source/drivers/file/FColumns \
	connectivity/source/drivers/file/FConnection \
	connectivity/source/drivers/file/FDatabaseMetaData \
	connectivity/source/drivers/file/FDateFunctions \
	connectivity/source/drivers/file/FDriver \
	connectivity/source/drivers/file/FNoException \
	connectivity/source/drivers/file/FNumericFunctions \
	connectivity/source/drivers/file/FPreparedStatement \
	connectivity/source/drivers/file/FResultSet \
	connectivity/source/drivers/file/FResultSetMetaData \
	connectivity/source/drivers/file/FStatement \
	connectivity/source/drivers/file/FStringFunctions \
	connectivity/source/drivers/file/FTable \
	connectivity/source/drivers/file/FTables \
	connectivity/source/drivers/file/fanalyzer \
	connectivity/source/drivers/file/fcode \
	connectivity/source/drivers/file/fcomp \
	connectivity/source/drivers/file/quotedstring \
))

# vim: set noet sw=4 ts=4:
