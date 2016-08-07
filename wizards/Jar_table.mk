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



$(eval $(call gb_Jar_Jar,table,SRCDIR))

$(eval $(call gb_Jar_add_jars,table,\
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/unoil.jar \
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/juh.jar \
	$(OUTDIR)/bin/java_uno.jar \
	$(OUTDIR)/bin/commonwizards.jar \
))

$(eval $(call gb_Jar_set_packageroot,table,com))

$(eval $(call gb_Jar_set_manifest,table,$(SRCDIR)/wizards/com/sun/star/wizards/table/MANIFEST.MF))

$(eval $(call gb_Jar_set_jarclasspath,table,\
	commonwizards.jar \
))

$(eval $(call gb_Jar_add_sourcefiles,table,\
	wizards/com/sun/star/wizards/table/CallTableWizard \
	wizards/com/sun/star/wizards/table/CGCategory \
	wizards/com/sun/star/wizards/table/CGTable \
	wizards/com/sun/star/wizards/table/FieldDescription \
	wizards/com/sun/star/wizards/table/FieldFormatter \
	wizards/com/sun/star/wizards/table/Finalizer \
	wizards/com/sun/star/wizards/table/PrimaryKeyHandler \
	wizards/com/sun/star/wizards/table/ScenarioSelector \
	wizards/com/sun/star/wizards/table/TableWizard \
))

$(eval $(call gb_Jar_set_componentfile,table,wizards/com/sun/star/wizards/table/table,OOO))

# vim: set noet sw=4 ts=4:
