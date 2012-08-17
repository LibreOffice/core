# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
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

$(eval $(call gb_JunitTest_JunitTest,connectivity_complex))

$(eval $(call gb_JunitTest_set_defs,connectivity_complex,\
	$$(DEFS) \
	-Dorg.openoffice.test.arg.sce=$(SRCDIR)/connectivity/qa/scenearios.sce \
))

$(eval $(call gb_JunitTest_use_jars,connectivity_complex,\
	jurt \
	OOoRunner \
	ridl \
	test \
	unoil \
))

$(eval $(call gb_JunitTest_use_externals,connectivity_complex,\
	hsqldb \
))

$(eval $(call gb_JunitTest_add_classes,connectivity_complex,\
	org.openoffice.test.UnoApiTest \
))

$(eval $(call gb_JunitTest_add_sourcefiles,connectivity_complex,\
	connectivity/qa/complex/connectivity/DBaseDriverTest \
	connectivity/qa/complex/connectivity/FlatFileAccess \
	connectivity/qa/complex/connectivity/HsqlDriverTest \
	connectivity/qa/complex/connectivity/JdbcLongVarCharTest \
	connectivity/qa/complex/connectivity/SubTestCase \
	connectivity/qa/complex/connectivity/TestCase \
	connectivity/qa/complex/connectivity/dbase/DBaseDateFunctions \
	connectivity/qa/complex/connectivity/dbase/DBaseNumericFunctions \
	connectivity/qa/complex/connectivity/dbase/DBaseSqlTests \
	connectivity/qa/complex/connectivity/dbase/DBaseStringFunctions \
	connectivity/qa/complex/connectivity/hsqldb/DatabaseMetaData \
	connectivity/qa/complex/connectivity/hsqldb/TestCacheSize \
	connectivity/qa/connectivity/tools/AbstractDatabase \
	connectivity/qa/connectivity/tools/CRMDatabase \
	connectivity/qa/connectivity/tools/CsvDatabase \
	connectivity/qa/connectivity/tools/DataSource \
	connectivity/qa/connectivity/tools/DatabaseAccess \
	connectivity/qa/connectivity/tools/DbaseDatabase \
	connectivity/qa/connectivity/tools/FlatFileDatabase \
	connectivity/qa/connectivity/tools/HsqlColumnDescriptor \
	connectivity/qa/connectivity/tools/HsqlDatabase \
	connectivity/qa/connectivity/tools/HsqlTableDescriptor \
	connectivity/qa/connectivity/tools/QueryDefinition \
	connectivity/qa/connectivity/tools/RowSet \
	connectivity/qa/connectivity/tools/sdb/Connection \
))

# vim: set noet sw=4 ts=4:
