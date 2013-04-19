# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_JunitTest_JunitTest,sc_unoapi))

$(eval $(call gb_JunitTest_add_classes,sc_unoapi, \
    org.openoffice.test.UnoApiTest \
))

$(eval $(call gb_JunitTest_use_jars,sc_unoapi, \
    OOoRunner \
    jurt \
    ridl \
    test \
    unoil \
))

$(eval $(call gb_JunitTest_set_defs,sc_unoapi,\
    $$(DEFS) \
    -Dorg.openoffice.test.arg.sce=$(SRCDIR)/sc/qa/unoapi/sc.sce \
    -Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/sc/qa/unoapi/testdocuments \
    -Dorg.openoffice.test.arg.xcl=$(SRCDIR)/sc/qa/unoapi/knownissues.xcl \
))

# vim: set noet sw=4 ts=4:
