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



$(eval $(call gb_Jar_Jar,agenda,SRCDIR))

$(eval $(call gb_Jar_add_jars,agenda,\
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/unoil.jar \
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/juh.jar \
	$(OUTDIR)/bin/java_uno.jar \
	$(OUTDIR)/bin/commonwizards.jar \
))

$(eval $(call gb_Jar_set_manifest,agenda,$(SRCDIR)/wizards/com/sun/star/wizards/agenda/MANIFEST.MF))

$(eval $(call gb_Jar_set_jarclasspath,agenda,\
	commonwizards.jar \
))

$(eval $(call gb_Jar_set_packageroot,agenda,com))

$(eval $(call gb_Jar_add_sourcefiles,agenda,\
	wizards/com/sun/star/wizards/agenda/AgendaTemplate \
	wizards/com/sun/star/wizards/agenda/AgendaWizardDialog \
	wizards/com/sun/star/wizards/agenda/AgendaWizardDialogConst \
	wizards/com/sun/star/wizards/agenda/AgendaWizardDialogImpl \
	wizards/com/sun/star/wizards/agenda/AgendaWizardDialogResources \
	wizards/com/sun/star/wizards/agenda/CallWizard \
	wizards/com/sun/star/wizards/agenda/CGAgenda \
	wizards/com/sun/star/wizards/agenda/CGTopic \
	wizards/com/sun/star/wizards/agenda/TemplateConsts \
	wizards/com/sun/star/wizards/agenda/TopicsControl \
))

$(eval $(call gb_Jar_set_componentfile,agenda,wizards/com/sun/star/wizards/agenda/agenda,OOO))

# vim: set noet sw=4 ts=4:
