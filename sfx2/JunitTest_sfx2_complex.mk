#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2009 by Sun Microsystems, Inc.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_JunitTest_JunitTest,sfx2_complex))

$(eval $(call gb_JunitTest_set_defs,sfx2_complex,\
    $$(DEFS) \
    -Dorg.openoffice.test.arg.docpath=$(SRCDIR)/sfx2/qa/complex/framework/testdocuments \
))
$(info $(call gb_JunitTest_set_defs,sfx2_complex,\
    -Dorg.openoffice.test.arg.soffice=path:$(OUTDIR)/installation/opt/openoffice.org3/program/soffice \
    -Dorg.openoffice.test.arg.env=LD_LIBRARY_PATH \
    -Dorg.openoffice.test.arg.user=file://$(call gb_JunitTest_get_userdir,$*) \
    -Dorg.openoffice.test.arg.docpath=$(SRCDIR)/sfx2/qa/complex/framework/testdocuments \
    $$(DEFS) \
))

$(eval $(call gb_JunitTest_add_sourcefiles,sfx2_complex,\
    sfx2/qa/complex/framework/CheckGlobalEventBroadcaster_writer1 \
    sfx2/qa/complex/framework/DocumentMetadataAccessTest \
    sfx2/qa/complex/framework/DialogThread \
    sfx2/qa/complex/framework/WriterHelper \
    sfx2/qa/complex/framework/TestDocument \
    sfx2/qa/complex/framework/DocumentPropertiesTest \
    sfx2/qa/complex/standalonedocumentinfo/StandaloneDocumentInfoTest \
    sfx2/qa/complex/standalonedocumentinfo/TestHelper \
    sfx2/qa/complex/standalonedocumentinfo/Test01 \
    sfx2/qa/complex/standalonedocumentinfo/StandaloneDocumentInfoUnitTest \
    sfx2/qa/complex/docinfo/DocumentProperties \
))

$(eval $(call gb_JunitTest_add_classes,sfx2_complex,\
    complex.docinfo.DocumentProperties \
    complex.framework.DocumentPropertiesTest \
    complex.framework.DocumentMetadataAccessTest \
    complex.framework.CheckGlobalEventBroadcaster_writer1 \
))
# vim: set noet sw=4 ts=4:
