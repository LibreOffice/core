# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_JunitTest_JunitTest,forms_complex))

$(eval $(call gb_JunitTest_use_jars,forms_complex,\
    OOoRunner \
    ridl \
    test \
    unoil \
    jurt \
))

$(eval $(call gb_JunitTest_add_sourcefiles,forms_complex,\
    forms/qa/complex/forms/CheckOGroupBoxModel \
))

$(eval $(call gb_JunitTest_add_classes,forms_complex,\
    complex.forms.CheckOGroupBoxModel \
))

# vim: set noet sw=4 ts=4:
