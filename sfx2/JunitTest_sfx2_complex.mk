# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_JunitTest_JunitTest,sfx2_complex))

$(eval $(call gb_JunitTest_set_defs,sfx2_complex,\
    $$(DEFS) \
    -Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/sfx2/qa/complex/sfx2/testdocuments \
))

$(eval $(call gb_JunitTest_use_jars,sfx2_complex,\
    OOoRunner \
    ridl \
    test \
    test-tools \
    unoil \
    jurt \
))

$(eval $(call gb_JunitTest_add_sourcefiles,sfx2_complex,\
    sfx2/qa/complex/sfx2/DocumentInfo \
    sfx2/qa/complex/sfx2/DocumentMetadataAccess \
    sfx2/qa/complex/sfx2/DocumentProperties \
    sfx2/qa/complex/sfx2/GlobalEventBroadcaster \
    sfx2/qa/complex/sfx2/StandaloneDocumentInfo \
    sfx2/qa/complex/sfx2/UndoManager \
    sfx2/qa/complex/sfx2/JUnitBasedTest \
    sfx2/qa/complex/sfx2/DocumentEvents \
    sfx2/qa/complex/sfx2/standalonedocinfo/StandaloneDocumentInfoTest \
    sfx2/qa/complex/sfx2/standalonedocinfo/Test01 \
    sfx2/qa/complex/sfx2/standalonedocinfo/TestHelper \
    sfx2/qa/complex/sfx2/tools/DialogThread \
    sfx2/qa/complex/sfx2/tools/TestDocument \
    sfx2/qa/complex/sfx2/tools/WriterHelper \
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
    complex.sfx2.DocumentEvents \
    complex.sfx2.UndoManager \
))
# fd#35663 fails currently:
#    complex.sfx2.UndoManager \
# #i115674# fails currently: misses some OnUnfocus event
#	complex.sfx2.GlobalEventBroadcaster \
# breaks because binfilter export has been removed
#	complex.sfx2.StandaloneDocumentInfo \

# vim: set noet sw=4 ts=4:
