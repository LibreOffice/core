#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_JunitTest_JunitTest,sfx2_complex))

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
    sfx2/qa/complex/sfx2/DocumentInfo \
    sfx2/qa/complex/sfx2/DocumentMetadataAccess \
    sfx2/qa/complex/sfx2/DocumentProperties \
    sfx2/qa/complex/sfx2/GlobalEventBroadcaster \
    sfx2/qa/complex/sfx2/StandaloneDocumentInfo \
    sfx2/qa/complex/sfx2/UndoManager \
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
))
# fd#35663 fails currently:
#    complex.sfx2.UndoManager \
# #i115674# fails currently: misses some OnUnfocus event
#	complex.sfx2.GlobalEventBroadcaster \
# breaks because binfilter export has been removed
#	complex.sfx2.StandaloneDocumentInfo \

# vim: set noet sw=4 ts=4:
