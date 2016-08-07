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



$(eval $(call gb_Jar_Jar,reportbuilderwizard,SRCDIR))

$(eval $(call gb_Jar_add_jars,reportbuilderwizard,\
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/unoil.jar \
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/juh.jar \
	$(OUTDIR)/bin/java_uno.jar \
	$(OUTDIR)/bin/commonwizards.jar \
	$(OUTDIR)/bin/report.jar \
))

$(eval $(call gb_Jar_set_packageroot,reportbuilderwizard,com))

$(eval $(call gb_Jar_set_manifest,reportbuilderwizard,$(SRCDIR)/wizards/com/sun/star/wizards/reportbuilder/Manifest.mf))

$(eval $(call gb_Jar_set_jarclasspath,reportbuilderwizard,\
	commonwizards.jar \
	report.jar \
))

$(eval $(call gb_Jar_add_sourcefiles,reportbuilderwizard,\
	wizards/com/sun/star/wizards/reportbuilder/ReportBuilderImplementation \
	wizards/com/sun/star/wizards/reportbuilder/layout/ColumnarSingleColumn \
	wizards/com/sun/star/wizards/reportbuilder/layout/ColumnarThreeColumns \
	wizards/com/sun/star/wizards/reportbuilder/layout/ColumnarTwoColumns \
	wizards/com/sun/star/wizards/reportbuilder/layout/DesignTemplate \
	wizards/com/sun/star/wizards/reportbuilder/layout/InBlocksLabelsAbove \
	wizards/com/sun/star/wizards/reportbuilder/layout/InBlocksLabelsLeft \
	wizards/com/sun/star/wizards/reportbuilder/layout/LayoutConstants \
	wizards/com/sun/star/wizards/reportbuilder/layout/ReportBuilderLayouter \
	wizards/com/sun/star/wizards/reportbuilder/layout/SectionEmptyObject \
	wizards/com/sun/star/wizards/reportbuilder/layout/SectionLabel \
	wizards/com/sun/star/wizards/reportbuilder/layout/SectionObject \
	wizards/com/sun/star/wizards/reportbuilder/layout/SectionTextField \
	wizards/com/sun/star/wizards/reportbuilder/layout/Tabular \
))

# vim: set noet sw=4 ts=4:
