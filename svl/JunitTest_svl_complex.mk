#**************************************************************
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
#**************************************************************



$(eval $(call gb_JunitTest_JunitTest,svl_complex,SRCDIR))

$(eval $(call gb_JunitTest_set_defs,svl_complex,\
    $$(DEFS) \
    -Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/svl/qa/complex/broken_document/test_documents \
))

$(eval $(call gb_JunitTest_add_jars,svl_complex,\
    $(OUTDIR)/bin/OOoRunner.jar \
    $(OUTDIR)/bin/ridl.jar \
    $(OUTDIR)/bin/test.jar \
    $(OUTDIR)/bin/unoil.jar \
    $(OUTDIR)/bin/jurt.jar \
    $(OUTDIR)/bin/ConnectivityTools.jar \
))

$(eval $(call gb_JunitTest_add_sourcefiles,svl_complex,\
    svl/qa/complex/ConfigItems/CheckConfigItems \
    svl/qa/complex/passwordcontainer/PasswordContainerUnitTest \
    svl/qa/complex/passwordcontainer/TestHelper \
    svl/qa/complex/passwordcontainer/Test03 \
    svl/qa/complex/passwordcontainer/Test02 \
    svl/qa/complex/passwordcontainer/Test01 \
    svl/qa/complex/passwordcontainer/PasswordContainerTest \
    svl/qa/complex/passwordcontainer/MasterPasswdHandler \
))

$(eval $(call gb_JunitTest_add_classes,svl_complex,\
    complex.passwordcontainer.PasswordContainerUnitTest \
))

# vim: set noet sw=4 ts=4:
