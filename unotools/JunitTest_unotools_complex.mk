# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_JunitTest_JunitTest,unotools_complex))

$(eval $(call gb_JunitTest_use_jars,unotools_complex,\
    OOoRunner \
    jurt \
    ridl \
    test-tools \
    test \
    unoil \
))

$(eval $(call gb_JunitTest_set_defs,unotools_complex,\
    $$(DEFS) \
))

$(eval $(call gb_JunitTest_add_classes,unotools_complex,\
    complex.tempfile.TempFileUnitTest \
))

$(eval $(call gb_JunitTest_add_sourcefiles,unotools_complex,\
    unotools/qa/complex/tempfile/TempFileTest \
    unotools/qa/complex/tempfile/TempFileUnitTest \
    unotools/qa/complex/tempfile/Test01 \
    unotools/qa/complex/tempfile/Test02 \
    unotools/qa/complex/tempfile/TestHelper \
))

# vim: set noet sw=4 ts=4:
