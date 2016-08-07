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



$(eval $(call gb_JunitTest_JunitTest,sm_unoapi,SRCDIR))

$(eval $(call gb_JunitTest_set_defs,sm_unoapi,\
	$$(DEFS) \
	-Dorg.openoffice.test.arg.sce=$(SRCDIR)/starmath/qa/unoapi/sm.sce \
	-Dorg.openoffice.test.arg.xcl=$(SRCDIR)/starmath/qa/unoapi/knownissues.xcl \
))

$(eval $(call gb_JunitTest_add_jars,sm_unoapi,\
	$(OUTDIR)/bin/OOoRunner.jar \
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/test.jar \
	$(OUTDIR)/bin/unoil.jar \
	$(OUTDIR)/bin/jurt.jar \
))

$(eval $(call gb_JunitTest_add_sourcefiles,sm_unoapi,\
	starmath/qa/unoapi/Test \
))

$(eval $(call gb_JunitTest_add_classes,sm_unoapi,\
	org.openoffice.starmath.qa.unoapi.Test \
))

# vim: set noet sw=4 ts=4:
