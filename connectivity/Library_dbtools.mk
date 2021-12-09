# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,dbtools))

$(eval $(call gb_Library_add_defs,dbtools,\
	-DOOO_DLLIMPLEMENTATION_DBTOOLS \
))

$(eval $(call gb_Library_set_componentfile,dbtools,connectivity/source/dbtools/dbtools,services))

$(eval $(call gb_Library_set_include,dbtools,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
))

#$(eval $(call gb_Library_set_precompiled_header,dbtools,connectivity/inc/pch/precompiled_dbtools))

$(eval $(call gb_Library_use_external,dbtools,boost_headers))

$(eval $(call gb_Library_use_sdk_api,dbtools))

$(eval $(call gb_Library_use_libraries,dbtools,\
	cppu \
	cppuhelper \
	sal \
	salhelper \
	$(if $(ENABLE_JAVA), \
		jvmaccess) \
	utl \
	tl \
	comphelper \
	i18nlangtag \
	svt \
	vcl \
))

ifeq ($(filter-out NETBSD MACOSX,$(OS)),)
$(eval $(call gb_Library_use_libraries,dbtools,\
	ucbhelper \
))
endif

$(eval $(call gb_Library_add_exception_objects,dbtools,\
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
	connectivity/source/commontools/ParameterSubstitution \
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
))

# vim: set noet sw=4 ts=4:
