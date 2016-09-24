# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,jurt))

ifneq ($(ENABLE_JAVA),)

$(eval $(call gb_Module_add_targets,jurt,\
    Jar_jurt \
    Library_jpipe \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,jurt,\
    Library_jpipx \
))
endif

$(eval $(call gb_Module_add_subsequentcheck_targets,jurt,\
    CustomTarget_test_urp \
    InternalUnoApi_test_urp \
    JunitTest_bridgefactory \
    JunitTest_connections \
    JunitTest_java \
    JunitTest_java_remote \
    JunitTest_remote \
    JunitTest_uno \
    JunitTest_urp \
    JunitTest_util \
))

endif

# vim:set noet sw=4 ts=4:
