# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,dba))

$(eval $(call gb_Library_set_include,dba,\
    $$(INCLUDE) \
	-I$(SRCDIR)/dbaccess/source/inc \
	-I$(SRCDIR)/dbaccess/source/core/inc \
	-I$(WORKDIR)/YaccTarget/connectivity/source/parse \
))

$(eval $(call gb_Library_add_defs,dba,\
    -DOOO_DLLIMPLEMENTATION_DBA \
))

$(eval $(call gb_Library_use_external,dba,boost_headers))

$(eval $(call gb_Library_use_sdk_api,dba))

$(eval $(call gb_Library_use_libraries,dba,\
    comphelper \
    cppu \
    cppuhelper \
    dbtools \
    fwe \
    i18nlangtag \
    sal \
    salhelper \
	sax \
    sb \
    sfx \
    svl \
    svt \
    svxcore \
    tl \
    ucbhelper \
    utl \
    vcl \
    xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_set_componentfile,dba,dbaccess/util/dba))

$(eval $(call gb_Library_add_exception_objects,dba,\
    dbaccess/source/core/api/BookmarkSet \
    dbaccess/source/core/api/CacheSet \
    dbaccess/source/core/api/callablestatement \
    dbaccess/source/core/api/CIndexes \
    dbaccess/source/core/api/column \
    dbaccess/source/core/api/columnsettings \
    dbaccess/source/core/api/CRowSetColumn \
    dbaccess/source/core/api/CRowSetDataColumn \
    dbaccess/source/core/api/datacolumn \
    dbaccess/source/core/api/datasettings \
    dbaccess/source/core/api/definitioncolumn \
    dbaccess/source/core/api/FilteredContainer \
    dbaccess/source/core/api/HelperCollections \
    dbaccess/source/core/api/KeySet \
    dbaccess/source/core/api/OptimisticSet \
    dbaccess/source/core/api/preparedstatement \
    dbaccess/source/core/api/PrivateRow \
    dbaccess/source/core/api/query \
    dbaccess/source/core/api/querycomposer \
    dbaccess/source/core/api/querycontainer \
    dbaccess/source/core/api/querydescriptor \
    dbaccess/source/core/api/resultcolumn \
    dbaccess/source/core/api/resultset \
    dbaccess/source/core/api/RowSet \
    dbaccess/source/core/api/RowSetBase \
    dbaccess/source/core/api/RowSetCache \
    dbaccess/source/core/api/RowSetCacheIterator \
    dbaccess/source/core/api/SingleSelectQueryComposer \
    dbaccess/source/core/api/statement \
    dbaccess/source/core/api/StaticSet \
    dbaccess/source/core/api/table \
    dbaccess/source/core/api/tablecontainer \
    dbaccess/source/core/api/TableDeco \
    dbaccess/source/core/api/View \
    dbaccess/source/core/api/viewcontainer \
    dbaccess/source/core/api/WrappedResultSet \
    dbaccess/source/core/dataaccess/bookmarkcontainer \
    dbaccess/source/core/dataaccess/commandcontainer \
    dbaccess/source/core/dataaccess/commanddefinition \
    dbaccess/source/core/dataaccess/ComponentDefinition \
    dbaccess/source/core/dataaccess/connection \
    dbaccess/source/core/dataaccess/ContentHelper \
    dbaccess/source/core/dataaccess/dataaccessdescriptor \
    dbaccess/source/core/dataaccess/databasecontext \
    dbaccess/source/core/dataaccess/databasedocument \
    dbaccess/source/core/dataaccess/databaseregistrations \
    dbaccess/source/core/dataaccess/datasource \
    dbaccess/source/core/dataaccess/definitioncontainer \
    dbaccess/source/core/dataaccess/documentcontainer \
    dbaccess/source/core/dataaccess/documentdefinition \
    dbaccess/source/core/dataaccess/documenteventexecutor \
    dbaccess/source/core/dataaccess/documenteventnotifier \
    dbaccess/source/core/dataaccess/documentevents \
    dbaccess/source/core/dataaccess/intercept \
    dbaccess/source/core/dataaccess/ModelImpl \
    dbaccess/source/core/dataaccess/myucp_datasupplier \
    dbaccess/source/core/dataaccess/myucp_resultset \
    dbaccess/source/core/dataaccess/SharedConnection \
    dbaccess/source/core/misc/apitools \
    dbaccess/source/core/misc/ContainerMediator \
    dbaccess/source/core/misc/DatabaseDataProvider \
    dbaccess/source/core/misc/dsntypes \
    dbaccess/source/core/misc/module_dba \
    dbaccess/source/core/misc/objectnameapproval \
    dbaccess/source/core/misc/PropertyForward \
    dbaccess/source/core/misc/sdbcoretools \
    dbaccess/source/core/misc/services \
    dbaccess/source/core/misc/userinformation \
    dbaccess/source/core/misc/veto \
    dbaccess/source/core/recovery/dbdocrecovery \
    dbaccess/source/core/recovery/settingsimport \
    dbaccess/source/core/recovery/storagestream \
    dbaccess/source/core/recovery/storagetextstream \
    dbaccess/source/core/recovery/storagexmlstream \
    dbaccess/source/core/recovery/subcomponentloader \
    dbaccess/source/core/recovery/subcomponentrecovery \
    dbaccess/source/core/resource/core_resource \
))

# vim: set noet sw=4 ts=4:
