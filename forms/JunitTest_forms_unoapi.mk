# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_JunitTest_JunitTest,forms_unoapi))

$(eval $(call gb_JunitTest_set_defs,forms_unoapi,\
    $$(DEFS) \
    -Dorg.openoffice.test.arg.sce=$(SRCDIR)/forms/qa/unoapi/forms.sce \
    -Dorg.openoffice.test.arg.xcl=$(SRCDIR)/forms/qa/unoapi/knownissues.xcl \
    -Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/forms/qa/unoapi/testdocuments \
))

$(eval $(call gb_JunitTest_use_jars,forms_unoapi,\
    OOoRunner \
    ridl \
    test \
    unoil \
    jurt \
))

$(eval $(call gb_JunitTest_add_classes,forms_unoapi,\
    org.openoffice.test.UnoApiTest \
))

# vim: set noet sw=4 ts=4:
