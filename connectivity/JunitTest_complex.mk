# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

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
