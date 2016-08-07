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



$(eval $(call gb_Jar_Jar,query,SRCDIR))

$(eval $(call gb_Jar_add_jars,query,\
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/unoil.jar \
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/juh.jar \
	$(OUTDIR)/bin/java_uno.jar \
	$(OUTDIR)/bin/commonwizards.jar \
))

$(eval $(call gb_Jar_set_manifest,query,$(SRCDIR)/wizards/com/sun/star/wizards/query/MANIFEST.MF))

$(eval $(call gb_Jar_set_packageroot,query,com))

$(eval $(call gb_Jar_set_jarclasspath,query,\
	commonwizards.jar \
))

$(eval $(call gb_Jar_add_sourcefiles,query,\
	wizards/com/sun/star/wizards/query/CallQueryWizard \
	wizards/com/sun/star/wizards/query/Finalizer \
	wizards/com/sun/star/wizards/query/QuerySummary \
	wizards/com/sun/star/wizards/query/QueryWizard \
))

$(eval $(call gb_Jar_set_componentfile,query,wizards/com/sun/star/wizards/query/query,OOO))

# vim: set noet sw=4 ts=4:
