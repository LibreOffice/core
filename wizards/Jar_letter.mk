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



$(eval $(call gb_Jar_Jar,letter,SRCDIR))

$(eval $(call gb_Jar_add_jars,letter,\
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/unoil.jar \
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/juh.jar \
	$(OUTDIR)/bin/java_uno.jar \
	$(OUTDIR)/bin/commonwizards.jar \
))

$(eval $(call gb_Jar_set_manifest,letter,$(SRCDIR)/wizards/com/sun/star/wizards/letter/MANIFEST.MF))

$(eval $(call gb_Jar_set_packageroot,letter,com))

$(eval $(call gb_Jar_set_jarclasspath,letter,\
	commonwizards.jar \
))

$(eval $(call gb_Jar_add_sourcefiles,letter,\
	wizards/com/sun/star/wizards/letter/CallWizard \
	wizards/com/sun/star/wizards/letter/CGLetter \
	wizards/com/sun/star/wizards/letter/CGLetterWizard \
	wizards/com/sun/star/wizards/letter/CGPaperElementLocation \
	wizards/com/sun/star/wizards/letter/LetterDocument \
	wizards/com/sun/star/wizards/letter/LetterWizardDialog \
	wizards/com/sun/star/wizards/letter/LetterWizardDialogConst \
	wizards/com/sun/star/wizards/letter/LetterWizardDialogEvents \
	wizards/com/sun/star/wizards/letter/LetterWizardDialogImpl \
	wizards/com/sun/star/wizards/letter/LetterWizardDialogResources \
	wizards/com/sun/star/wizards/letter/LocaleCodes \
))

$(eval $(call gb_Jar_set_componentfile,letter,wizards/com/sun/star/wizards/letter/letter,OOO))

# vim: set noet sw=4 ts=4:
