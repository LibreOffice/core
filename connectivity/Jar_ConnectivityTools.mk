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
