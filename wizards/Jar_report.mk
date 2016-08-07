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



$(eval $(call gb_Jar_Jar,report,SRCDIR))

$(eval $(call gb_Jar_add_jars,report,\
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/unoil.jar \
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/juh.jar \
	$(OUTDIR)/bin/java_uno.jar \
	$(OUTDIR)/bin/commonwizards.jar \
))

$(eval $(call gb_Jar_set_manifest,report,$(SRCDIR)/wizards/com/sun/star/wizards/report/MANIFEST.MF))

$(eval $(call gb_Jar_set_packageroot,report,com))

$(eval $(call gb_Jar_set_jarclasspath,report,\
	commonwizards.jar \
))

$(eval $(call gb_Jar_add_sourcefiles,report,\
	wizards/com/sun/star/wizards/report/CallReportWizard \
	wizards/com/sun/star/wizards/report/Dataimport \
	wizards/com/sun/star/wizards/report/DBColumn \
	wizards/com/sun/star/wizards/report/GroupFieldHandler \
	wizards/com/sun/star/wizards/report/IReportBuilderLayouter \
	wizards/com/sun/star/wizards/report/IReportDefinitionReadAccess \
	wizards/com/sun/star/wizards/report/IReportDocument \
	wizards/com/sun/star/wizards/report/RecordTable \
	wizards/com/sun/star/wizards/report/ReportFinalizer \
	wizards/com/sun/star/wizards/report/ReportImplementationHelper \
	wizards/com/sun/star/wizards/report/ReportLayouter \
	wizards/com/sun/star/wizards/report/ReportTextDocument \
	wizards/com/sun/star/wizards/report/ReportTextImplementation \
	wizards/com/sun/star/wizards/report/ReportWizard \
))

$(eval $(call gb_Jar_set_componentfile,report,wizards/com/sun/star/wizards/report/report,OOO))

# vim: set noet sw=4 ts=4:
