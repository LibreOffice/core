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



$(eval $(call gb_JunitTest_JunitTest,sw_complex,SRCDIR))

$(eval $(call gb_JunitTest_set_defs,sw_complex,\
    $$(DEFS) \
    -Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/sw/qa/complex/writer/testdocuments \
))

$(eval $(call gb_JunitTest_add_sourcefiles,sw_complex,\
    sw/qa/complex/checkColor/CheckChangeColor \
    sw/qa/complex/indeterminateState/CheckIndeterminateState \
    sw/qa/complex/writer/CheckCrossReferences \
    sw/qa/complex/writer/LoadSaveTest \
    sw/qa/complex/writer/CheckNamedPropertyValues \
    sw/qa/complex/writer/CheckIndexedPropertyValues \
    sw/qa/complex/writer/TestDocument \
    sw/qa/complex/writer/CheckBookmarks \
    sw/qa/complex/writer/CheckFlies \
    sw/qa/complex/writer/TextPortionEnumerationTest \
    sw/qa/complex/accessibility/AccessibleRelationSet \
))

$(eval $(call gb_JunitTest_add_jars,sw_complex,\
    $(OUTDIR)/bin/OOoRunner.jar \
    $(OUTDIR)/bin/ridl.jar \
    $(OUTDIR)/bin/test.jar \
    $(OUTDIR)/bin/unoil.jar \
    $(OUTDIR)/bin/jurt.jar \
))

$(eval $(call gb_JunitTest_add_classes,sw_complex,\
    complex.accessibility.AccessibleRelationSet \
    complex.checkColor.CheckChangeColor \
    complex.writer.CheckCrossReferences \
    complex.writer.CheckFlies \
    complex.writer.CheckIndexedPropertyValues \
    complex.writer.CheckNamedPropertyValues \
    complex.writer.TextPortionEnumerationTest \
))

# CheckBookmarks currently fails on windows because the hashes are different
ifneq ($(OS),WNT)
$(eval $(call gb_JunitTest_add_classes,sw_complex,\
    complex.writer.CheckBookmarks \
))
endif

# vim: set noet sw=4 ts=4:
