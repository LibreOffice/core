#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Library_Library,dba))
$(eval $(call gb_Library_add_package_headers,dba,dbaccess_inc))
$(eval $(call gb_Library_add_precompiled_header,dba,$(SRCDIR)/dbaccess/inc/pch/precompiled_dbaccess))

$(eval $(call gb_Library_set_componentfile,dba,dbaccess/util/dba))

$(eval $(call gb_Library_set_include,dba,\
	-I$(SRCDIR)/dbaccess/inc \
	-I$(SRCDIR)/dbaccess/source/core/inc \
	-I$(SRCDIR)/dbaccess/source/inc \
	-I$(SRCDIR)/dbaccess/inc/pch \
	$$(SOLARINC) \
	-I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_set_defs,dba,\
	$$(DEFS) \
	-DOOO_DLLIMPLEMENTATION_DBA \
))

$(eval $(call gb_Library_add_linked_libs,dba,\
	sb \
	tl \
	svt \
	svl \
	cppuhelper \
	cppu \
	comphelper \
	dbtools \
	utl \
	vcl \
	sal \
	stl \
	i18nisolang1 \
	ucbhelper \
	sfx \
	salhelper \
	xo \
	fwe \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,dba,\
	dbaccess/source/core/api/querydescriptor \
	dbaccess/source/core/api/FilteredContainer \
	dbaccess/source/core/api/TableDeco \
	dbaccess/source/core/api/RowSetCacheIterator \
	dbaccess/source/core/api/RowSet \
	dbaccess/source/core/api/column \
	dbaccess/source/core/api/RowSetBase \
	dbaccess/source/core/api/CRowSetDataColumn \
	dbaccess/source/core/api/CRowSetColumn \
	dbaccess/source/core/api/querycomposer \
	dbaccess/source/core/api/CIndexes \
	dbaccess/source/core/api/BookmarkSet \
	dbaccess/source/core/api/KeySet \
	dbaccess/source/core/api/StaticSet \
	dbaccess/source/core/api/CacheSet \
	dbaccess/source/core/api/RowSetCache \
	dbaccess/source/core/api/definitioncolumn \
	dbaccess/source/core/api/resultcolumn \
	dbaccess/source/core/api/datacolumn \
	dbaccess/source/core/api/resultset \
	dbaccess/source/core/api/statement \
	dbaccess/source/core/api/preparedstatement \
	dbaccess/source/core/api/callablestatement \
	dbaccess/source/core/api/query \
	dbaccess/source/core/api/querycontainer \
	dbaccess/source/core/api/table \
	dbaccess/source/core/api/viewcontainer \
	dbaccess/source/core/api/tablecontainer \
	dbaccess/source/core/api/SingleSelectQueryComposer \
	dbaccess/source/core/api/HelperCollections \
	dbaccess/source/core/api/datasettings \
	dbaccess/source/core/api/View \
	dbaccess/source/core/api/WrappedResultSet \
	dbaccess/source/core/api/OptimisticSet \
	dbaccess/source/core/api/PrivateRow \
	dbaccess/source/core/api/columnsettings \
	dbaccess/source/core/dataaccess/SharedConnection \
	dbaccess/source/core/dataaccess/ContentHelper \
	dbaccess/source/core/dataaccess/bookmarkcontainer \
	dbaccess/source/core/dataaccess/definitioncontainer \
	dbaccess/source/core/dataaccess/commanddefinition \
	dbaccess/source/core/dataaccess/documentcontainer \
	dbaccess/source/core/dataaccess/commandcontainer \
	dbaccess/source/core/dataaccess/documentdefinition \
	dbaccess/source/core/dataaccess/ComponentDefinition \
	dbaccess/source/core/dataaccess/databasecontext \
	dbaccess/source/core/dataaccess/connection \
	dbaccess/source/core/dataaccess/datasource \
	dbaccess/source/core/dataaccess/databaseregistrations \
	dbaccess/source/core/dataaccess/intercept \
	dbaccess/source/core/dataaccess/myucp_datasupplier \
	dbaccess/source/core/dataaccess/myucp_resultset \
	dbaccess/source/core/dataaccess/databasedocument \
	dbaccess/source/core/dataaccess/dataaccessdescriptor \
	dbaccess/source/core/dataaccess/ModelImpl \
	dbaccess/source/core/dataaccess/documentevents \
	dbaccess/source/core/dataaccess/documenteventexecutor \
	dbaccess/source/core/dataaccess/documenteventnotifier \
	dbaccess/source/core/misc/sdbcoretools \
	dbaccess/source/core/misc/services \
	dbaccess/source/core/misc/PropertyForward \
	dbaccess/source/core/misc/ContainerMediator \
	dbaccess/source/core/misc/userinformation \
	dbaccess/source/core/misc/ContainerListener \
	dbaccess/source/core/misc/objectnameapproval \
	dbaccess/source/core/misc/DatabaseDataProvider \
	dbaccess/source/core/misc/module_dba \
	dbaccess/source/core/misc/dsntypes \
	dbaccess/source/core/misc/veto \
	dbaccess/source/core/misc/apitools \
	dbaccess/source/core/misc/dbastrings \
	dbaccess/source/core/recovery/dbdocrecovery \
	dbaccess/source/core/recovery/subcomponentloader \
	dbaccess/source/core/recovery/storagestream \
	dbaccess/source/core/recovery/storagexmlstream \
	dbaccess/source/core/recovery/storagetextstream \
	dbaccess/source/core/recovery/subcomponentrecovery \
	dbaccess/source/core/recovery/settingsimport \
	dbaccess/source/core/resource/core_resource \
))

# vim: set noet sw=4 ts=4:
