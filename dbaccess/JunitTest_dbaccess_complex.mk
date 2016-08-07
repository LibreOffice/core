###############################################################
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
###############################################################



$(eval $(call gb_JunitTest_JunitTest,dbaccess_complex,SRCDIR))

$(eval $(call gb_JunitTest_set_defs,dbaccess_complex,\
	$$(DEFS) \
	-Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/dbaccess/qa/dbaccess \
))

$(eval $(call gb_JunitTest_add_sourcefiles,dbaccess_complex,\
	dbaccess/qa/complex/dbaccess/ApplicationController \
	dbaccess/qa/complex/dbaccess/Beamer \
	dbaccess/qa/complex/dbaccess/DataSource \
	dbaccess/qa/complex/dbaccess/DatabaseDocument \
	dbaccess/qa/complex/dbaccess/Parser \
	dbaccess/qa/complex/dbaccess/PropertyBag \
	dbaccess/qa/complex/dbaccess/Query \
	dbaccess/qa/complex/dbaccess/QueryInQuery \
	dbaccess/qa/complex/dbaccess/RowSet \
	dbaccess/qa/complex/dbaccess/SingleSelectQueryComposer \
	dbaccess/qa/complex/dbaccess/UISettings \
	dbaccess/qa/complex/dbaccess/CopyTableWizard \
	dbaccess/qa/complex/dbaccess/CRMBasedTestCase \
	dbaccess/qa/complex/dbaccess/CopyTableInterActionHandler \
	dbaccess/qa/complex/dbaccess/DatabaseApplication \
	dbaccess/qa/complex/dbaccess/FileHelper \
	dbaccess/qa/complex/dbaccess/RowSetEventListener \
	dbaccess/qa/complex/dbaccess/TestCase \
))

$(eval $(call gb_JunitTest_add_jars,dbaccess_complex,\
	$(OUTDIR)/bin/OOoRunner.jar \
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/test.jar \
	$(OUTDIR)/bin/unoil.jar \
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/ConnectivityTools.jar \
))

$(eval $(call gb_JunitTest_add_classes,dbaccess_complex,\
	complex.dbaccess.Beamer \
	complex.dbaccess.DatabaseDocument \
	complex.dbaccess.PropertyBag \
))
# deactivated since sb123;
# apparently most of these fail because OOo does not find JVM?
#	complex.dbaccess.ApplicationController \
	complex.dbaccess.DataSource \
	complex.dbaccess.Parser \
	complex.dbaccess.Query \
	complex.dbaccess.QueryInQuery \
	complex.dbaccess.RowSet \
	complex.dbaccess.SingleSelectQueryComposer \
	complex.dbaccess.UISettings \
	complex.dbaccess.CopyTableWizard \

# vim: set noet sw=4 ts=4:
