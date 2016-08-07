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



$(eval $(call gb_Jar_Jar,web,SRCDIR))

$(eval $(call gb_Jar_add_jars,web,\
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/unoil.jar \
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/juh.jar \
	$(OUTDIR)/bin/java_uno.jar \
	$(OUTDIR)/bin/commonwizards.jar \
))

$(eval $(call gb_Jar_set_packageroot,web,com))

$(eval $(call gb_Jar_set_manifest,web,$(SRCDIR)/wizards/com/sun/star/wizards/web/MANIFEST.MF))

$(eval $(call gb_Jar_set_jarclasspath,web,\
	commonwizards.jar \
))

$(eval $(call gb_Jar_add_sourcefiles,web,\
	wizards/com/sun/star/wizards/web/AbstractErrorHandler \
	wizards/com/sun/star/wizards/web/BackgroundsDialog \
	wizards/com/sun/star/wizards/web/CallWizard \
	wizards/com/sun/star/wizards/web/ErrorHandler \
	wizards/com/sun/star/wizards/web/ExtensionVerifier \
	wizards/com/sun/star/wizards/web/FTPDialog \
	wizards/com/sun/star/wizards/web/FTPDialogResources \
	wizards/com/sun/star/wizards/web/IconsDialog \
	wizards/com/sun/star/wizards/web/ImageListDialog \
	wizards/com/sun/star/wizards/web/LogTaskListener \
	wizards/com/sun/star/wizards/web/Process \
	wizards/com/sun/star/wizards/web/ProcessErrorHandler \
	wizards/com/sun/star/wizards/web/ProcessErrors \
	wizards/com/sun/star/wizards/web/ProcessStatusRenderer \
	wizards/com/sun/star/wizards/web/StatusDialog \
	wizards/com/sun/star/wizards/web/StylePreview \
	wizards/com/sun/star/wizards/web/TOCPreview \
	wizards/com/sun/star/wizards/web/WebWizard \
	wizards/com/sun/star/wizards/web/WebWizardConst \
	wizards/com/sun/star/wizards/web/WebWizardDialog \
	wizards/com/sun/star/wizards/web/WebWizardDialogResources \
	wizards/com/sun/star/wizards/web/WWD_Events \
	wizards/com/sun/star/wizards/web/WWD_General \
	wizards/com/sun/star/wizards/web/WWD_Startup \
	wizards/com/sun/star/wizards/web/WWHID \
	wizards/com/sun/star/wizards/web/data/CGArgument \
	wizards/com/sun/star/wizards/web/data/CGContent \
	wizards/com/sun/star/wizards/web/data/CGDesign \
	wizards/com/sun/star/wizards/web/data/CGDocument \
	wizards/com/sun/star/wizards/web/data/CGExporter \
	wizards/com/sun/star/wizards/web/data/CGFilter \
	wizards/com/sun/star/wizards/web/data/CGGeneralInfo \
	wizards/com/sun/star/wizards/web/data/CGIconSet \
	wizards/com/sun/star/wizards/web/data/CGImage \
	wizards/com/sun/star/wizards/web/data/CGLayout \
	wizards/com/sun/star/wizards/web/data/CGPublish \
	wizards/com/sun/star/wizards/web/data/CGSession \
	wizards/com/sun/star/wizards/web/data/CGSessionName \
	wizards/com/sun/star/wizards/web/data/CGSettings \
	wizards/com/sun/star/wizards/web/data/CGStyle \
	wizards/com/sun/star/wizards/web/data/ConfigSetItem \
	wizards/com/sun/star/wizards/web/data/TypeDetection \
	wizards/com/sun/star/wizards/web/export/AbstractExporter \
	wizards/com/sun/star/wizards/web/export/ConfiguredExporter \
	wizards/com/sun/star/wizards/web/export/CopyExporter \
	wizards/com/sun/star/wizards/web/export/Exporter \
	wizards/com/sun/star/wizards/web/export/FilterExporter \
	wizards/com/sun/star/wizards/web/export/ImpressHTMLExporter \
	wizards/com/sun/star/wizards/web/status/ErrorLog \
	wizards/com/sun/star/wizards/web/status/ErrorReporter \
	wizards/com/sun/star/wizards/web/status/LogTaskListener \
	wizards/com/sun/star/wizards/web/status/Task \
	wizards/com/sun/star/wizards/web/status/TaskEvent \
	wizards/com/sun/star/wizards/web/status/TaskListener \
))

$(eval $(call gb_Jar_set_componentfile,web,wizards/com/sun/star/wizards/web/web,OOO))

# vim: set noet sw=4 ts=4:
