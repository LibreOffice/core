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



$(eval $(call gb_JunitTest_JunitTest,sfx2_complex,SRCDIR))

$(eval $(call gb_JunitTest_set_defs,sfx2_complex,\
    $$(DEFS) \
    -Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/sfx2/qa/complex/sfx2/testdocuments \
))

$(eval $(call gb_JunitTest_add_jars,sfx2_complex,\
    $(OUTDIR)/bin/OOoRunner.jar \
    $(OUTDIR)/bin/ridl.jar \
    $(OUTDIR)/bin/test.jar \
    $(OUTDIR)/bin/test-tools.jar \
    $(OUTDIR)/bin/unoil.jar \
    $(OUTDIR)/bin/jurt.jar \
))

$(eval $(call gb_JunitTest_add_sourcefiles,sfx2_complex,\
    sfx2/qa/complex/sfx2/tools/DialogThread \
    sfx2/qa/complex/sfx2/tools/WriterHelper \
    sfx2/qa/complex/sfx2/tools/TestDocument \
    sfx2/qa/complex/sfx2/GlobalEventBroadcaster \
    sfx2/qa/complex/sfx2/DocumentMetadataAccess \
    sfx2/qa/complex/sfx2/DocumentProperties \
    sfx2/qa/complex/sfx2/DocumentInfo \
    sfx2/qa/complex/sfx2/StandaloneDocumentInfo \
    sfx2/qa/complex/sfx2/UndoManager \
    sfx2/qa/complex/sfx2/JUnitBasedTest \
    sfx2/qa/complex/sfx2/DocumentEvents \
    sfx2/qa/complex/sfx2/standalonedocinfo/StandaloneDocumentInfoTest \
    sfx2/qa/complex/sfx2/standalonedocinfo/TestHelper \
    sfx2/qa/complex/sfx2/standalonedocinfo/Test01 \
    sfx2/qa/complex/sfx2/undo/CalcDocumentTest \
    sfx2/qa/complex/sfx2/undo/ChartDocumentTest \
    sfx2/qa/complex/sfx2/undo/DocumentTest \
    sfx2/qa/complex/sfx2/undo/DocumentTestBase \
    sfx2/qa/complex/sfx2/undo/DrawDocumentTest \
    sfx2/qa/complex/sfx2/undo/DrawingOrPresentationDocumentTest \
    sfx2/qa/complex/sfx2/undo/ImpressDocumentTest \
    sfx2/qa/complex/sfx2/undo/WriterDocumentTest \
))

$(eval $(call gb_JunitTest_add_classes,sfx2_complex,\
    complex.sfx2.DocumentInfo \
    complex.sfx2.DocumentProperties \
    complex.sfx2.DocumentMetadataAccess \
    complex.sfx2.UndoManager \
    complex.sfx2.DocumentEvents \
))
# #i115674# fails currently: misses some OnUnfocus event
#	complex.sfx2.GlobalEventBroadcaster \
# breaks because binfilter export has been removed
#	complex.sfx2.StandaloneDocumentInfo \

# vim: set noet sw=4 ts=4:
