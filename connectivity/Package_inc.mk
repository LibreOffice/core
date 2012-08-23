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

$(eval $(call gb_Package_Package,connectivity_inc,$(SRCDIR)/connectivity/inc/connectivity))

# sqlbison.hxx is generated and thus in another package
$(call gb_Package_get_target,connectivity_inc) : $(call gb_Package_get_target,connectivity_generated)

$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/BlobHelper.hxx,BlobHelper.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/CommonTools.hxx,CommonTools.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/conncleanup.hxx,conncleanup.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/ConnectionWrapper.hxx,ConnectionWrapper.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/dbcharset.hxx,dbcharset.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/dbconversion.hxx,dbconversion.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/dbexception.hxx,dbexception.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/dbmetadata.hxx,dbmetadata.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/dbtools.hxx,dbtools.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/dbtoolsdllapi.hxx,dbtoolsdllapi.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/DriversConfig.hxx,DriversConfig.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/filtermanager.hxx,filtermanager.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/formattedcolumnvalue.hxx,formattedcolumnvalue.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/FValue.hxx,FValue.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/IParseContext.hxx,IParseContext.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/OSubComponent.hxx,OSubComponent.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/ParameterCont.hxx,ParameterCont.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/parameters.hxx,parameters.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/paramwrapper.hxx,paramwrapper.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/PColumn.hxx,PColumn.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/predicateinput.hxx,predicateinput.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/sqlerror.hxx,sqlerror.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/sqliterator.hxx,sqliterator.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/sqlnode.hxx,sqlnode.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/sqlparse.hxx,sqlparse.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/SQLStatementHelper.hxx,SQLStatementHelper.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/standardsqlstate.hxx,standardsqlstate.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/statementcomposer.hxx,statementcomposer.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/StdTypeDefs.hxx,StdTypeDefs.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/TColumnsHelper.hxx,TColumnsHelper.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/TIndex.hxx,TIndex.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/TIndexColumns.hxx,TIndexColumns.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/TIndexes.hxx,TIndexes.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/TKey.hxx,TKey.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/TKeyColumns.hxx,TKeyColumns.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/TKeys.hxx,TKeys.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/TTableHelper.hxx,TTableHelper.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/virtualdbtools.hxx,virtualdbtools.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/warningscontainer.hxx,warningscontainer.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/sdbcx/IRefreshable.hxx,sdbcx/IRefreshable.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/sdbcx/VCatalog.hxx,sdbcx/VCatalog.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/sdbcx/VCollection.hxx,sdbcx/VCollection.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/sdbcx/VColumn.hxx,sdbcx/VColumn.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/sdbcx/VDescriptor.hxx,sdbcx/VDescriptor.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/sdbcx/VGroup.hxx,sdbcx/VGroup.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/sdbcx/VIndex.hxx,sdbcx/VIndex.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/sdbcx/VIndexColumn.hxx,sdbcx/VIndexColumn.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/sdbcx/VKey.hxx,sdbcx/VKey.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/sdbcx/VKeyColumn.hxx,sdbcx/VKeyColumn.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/sdbcx/VTable.hxx,sdbcx/VTable.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/sdbcx/VTypeDef.hxx,sdbcx/VTypeDef.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/sdbcx/VUser.hxx,sdbcx/VUser.hxx))
$(eval $(call gb_Package_add_file,connectivity_inc,inc/connectivity/sdbcx/VView.hxx,sdbcx/VView.hxx))

# vim: set noet sw=4 ts=4:
