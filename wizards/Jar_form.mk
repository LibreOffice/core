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



$(eval $(call gb_Jar_Jar,form,SRCDIR))

$(eval $(call gb_Jar_add_jars,form,\
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/unoil.jar \
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/juh.jar \
	$(OUTDIR)/bin/java_uno.jar \
	$(OUTDIR)/bin/commonwizards.jar \
))

$(eval $(call gb_Jar_set_manifest,form,$(SRCDIR)/wizards/com/sun/star/wizards/form/MANIFEST.MF))

$(eval $(call gb_Jar_set_packageroot,form,com))

$(eval $(call gb_Jar_set_jarclasspath,form,\
	commonwizards.jar \
))

$(eval $(call gb_Jar_add_sourcefiles,form,\
	wizards/com/sun/star/wizards/form/CallFormWizard \
	wizards/com/sun/star/wizards/form/DataEntrySetter \
	wizards/com/sun/star/wizards/form/FieldLinker \
	wizards/com/sun/star/wizards/form/Finalizer \
	wizards/com/sun/star/wizards/form/FormConfiguration \
	wizards/com/sun/star/wizards/form/FormControlArranger \
	wizards/com/sun/star/wizards/form/FormDocument \
	wizards/com/sun/star/wizards/form/FormWizard \
	wizards/com/sun/star/wizards/form/StyleApplier \
	wizards/com/sun/star/wizards/form/UIControlArranger \
))

$(eval $(call gb_Jar_set_componentfile,form,wizards/com/sun/star/wizards/form/form,OOO))

# vim: set noet sw=4 ts=4:
