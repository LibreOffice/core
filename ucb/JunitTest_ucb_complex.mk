# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_JunitTest_JunitTest,ucb_complex))

$(eval $(call gb_JunitTest_set_defs,ucb_complex,\
    $$(DEFS) \
    -Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/ucb/qa/complex/testdocuments \
))

$(eval $(call gb_JunitTest_use_jars,ucb_complex,\
    OOoRunner \
    ridl \
    test \
    unoil \
    jurt \
))

$(eval $(call gb_JunitTest_add_sourcefiles,ucb_complex,\
	ucb/qa/complex/ucb/UCB \
))
$(eval $(call gb_JunitTest_add_classes,ucb_complex,\
    complex.ucb.UCB \
))

# vim: set noet sw=4 ts=4:
