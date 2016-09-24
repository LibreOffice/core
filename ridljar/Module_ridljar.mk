# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,ridljar))

ifneq ($(ENABLE_JAVA),)
$(eval $(call gb_Module_add_targets,ridljar,\
    CustomTarget_javamaker \
    Jar_ridl \
    Jar_unoloader \
))

# unfortunately, these tests depend on qadevOOo, which depends on
# ridl.jar for build...
$(eval $(call gb_Module_add_subsequentcheck_targets,ridljar,\
    JunitTest_typedesc \
    JunitTest_uno \
    JunitTest_util \
))
endif

# vim:set noet sw=4 ts=4:
