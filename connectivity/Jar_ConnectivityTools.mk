# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Jar_Jar,ConnectivityTools))

$(eval $(call gb_Jar_use_jars,ConnectivityTools,\
	ridl \
	unoil \
	jurt \
	juh \
	java_uno \
	OOoRunnerLight \
))

$(eval $(call gb_Jar_set_packageroot,ConnectivityTools,connectivity))

$(eval $(call gb_Jar_add_sourcefiles,ConnectivityTools,\
	connectivity/qa/connectivity/tools/AbstractDatabase \
	connectivity/qa/connectivity/tools/CRMDatabase \
	connectivity/qa/connectivity/tools/CsvDatabase \
	connectivity/qa/connectivity/tools/DatabaseAccess \
	connectivity/qa/connectivity/tools/DataSource \
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
