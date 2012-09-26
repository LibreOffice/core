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

$(eval $(call gb_Library_Library,dbtools))

$(eval $(call gb_Library_use_packages,dbtools,\
	connectivity_inc \
))

$(eval $(call gb_Library_add_defs,dbtools,\
	-DOOO_DLLIMPLEMENTATION_DBTOOLS \
	-DBOOST_SPIRIT_USE_OLD_NAMESPACE \
))

$(eval $(call gb_Library_set_componentfile,dbtools,connectivity/source/dbtools/dbtools))

$(eval $(call gb_Library_set_include,dbtools,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/source/inc \
))

$(eval $(call gb_Library_use_sdk_api,dbtools))

$(eval $(call gb_Library_use_libraries,dbtools,\
	cppu \
	cppuhelper \
	sal \
	salhelper \
	$(if $(filter IOS,$(OS)),,jvmaccess) \
	utl \
	tl \
	comphelper \
	i18nisolang1 \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,dbtools))

ifeq ($(filter-out NETBSD MACOSX,$(OS)),)
$(eval $(call gb_Library_use_libraries,dbtools,\
	ucbhelper \
))
endif

#connectivity/source/commontools/RowFunctionParser.cxx disable optimization?

$(eval $(call gb_Library_add_noexception_objects,dbtools,\
	connectivity/source/simpledbt/refbase \
))

$(eval $(call gb_Library_add_grammars,dbtools,\
	connectivity/source/parse/sqlbison \
))

$(call gb_YaccTarget_get_target,connectivity/source/parse/sqlbison) : T_YACCFLAGS := -d -l -pSQLyy -bsql

$(eval $(call gb_Library_add_scanners,dbtools,\
connectivity/source/parse/sqlflex \
))

$(call gb_LexTarget_get_scanner_target,connectivity/source/parse/sqlflex) : T_LEXFLAGS := -i -8 -PSQLyy -L



$(eval $(call gb_Library_add_exception_objects,dbtools,\
	connectivity/source/commontools/AutoRetrievingBase \
	connectivity/source/commontools/BlobHelper \
	connectivity/source/commontools/CommonTools \
	connectivity/source/commontools/ConnectionWrapper \
	connectivity/source/commontools/DateConversion \
	connectivity/source/commontools/DriversConfig \
	connectivity/source/commontools/FDatabaseMetaDataResultSet \
	connectivity/source/commontools/FDatabaseMetaDataResultSetMetaData \
	connectivity/source/commontools/FValue \
	connectivity/source/commontools/ParamterSubstitution \
	connectivity/source/commontools/RowFunctionParser \
	connectivity/source/commontools/TColumnsHelper \
	connectivity/source/commontools/TConnection \
	connectivity/source/commontools/TDatabaseMetaDataBase \
	connectivity/source/commontools/TIndex \
	connectivity/source/commontools/TIndexColumns \
	connectivity/source/commontools/TIndexes \
	connectivity/source/commontools/TKey \
	connectivity/source/commontools/TKeyColumns \
	connectivity/source/commontools/TKeys \
	connectivity/source/commontools/TPrivilegesResultSet \
	connectivity/source/commontools/TSkipDeletedSet \
	connectivity/source/commontools/TSortIndex \
	connectivity/source/commontools/TTableHelper \
	connectivity/source/commontools/conncleanup \
	connectivity/source/commontools/dbcharset \
	connectivity/source/commontools/dbconversion \
	connectivity/source/commontools/dbexception \
	connectivity/source/commontools/dbmetadata \
	connectivity/source/commontools/dbtools \
	connectivity/source/commontools/dbtools2 \
	connectivity/source/commontools/filtermanager \
	connectivity/source/commontools/formattedcolumnvalue \
	connectivity/source/commontools/parameters \
	connectivity/source/commontools/paramwrapper \
	connectivity/source/commontools/predicateinput \
	connectivity/source/commontools/propertyids \
	connectivity/source/commontools/sqlerror \
	connectivity/source/commontools/statementcomposer \
	connectivity/source/commontools/warningscontainer \
	connectivity/source/parse/PColumn \
	connectivity/source/parse/internalnode \
	connectivity/source/parse/sqliterator \
	connectivity/source/parse/sqlnode \
	connectivity/source/resource/sharedresources \
	connectivity/source/sdbcx/VCatalog \
	connectivity/source/sdbcx/VCollection \
	connectivity/source/sdbcx/VColumn \
	connectivity/source/sdbcx/VDescriptor \
	connectivity/source/sdbcx/VGroup \
	connectivity/source/sdbcx/VIndex \
	connectivity/source/sdbcx/VIndexColumn \
	connectivity/source/sdbcx/VKey \
	connectivity/source/sdbcx/VKeyColumn \
	connectivity/source/sdbcx/VTable \
	connectivity/source/sdbcx/VUser \
	connectivity/source/sdbcx/VView \
	connectivity/source/simpledbt/charset_s \
	connectivity/source/simpledbt/dbtfactory \
	connectivity/source/simpledbt/parsenode_s \
	connectivity/source/simpledbt/parser_s \
	connectivity/source/simpledbt/staticdbtools_s \
))

# vim: set noet sw=4 ts=4:
