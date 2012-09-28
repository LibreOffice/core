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

$(eval $(call gb_Library_Library,file))

$(eval $(call gb_Library_add_defs,file,\
	-DOOO_DLLIMPLEMENTATION_FILE \
))

$(eval $(call gb_Library_set_include,file,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
))

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
