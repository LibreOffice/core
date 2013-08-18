# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,accessibility))

$(eval $(call gb_Module_add_targets,accessibility,\
    AllLangResTarget_acc \
    Library_acc \
))

ifneq ($(ENABLE_JAVA),)
ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,accessibility,\
    CustomTarget_bridge \
    CustomTarget_bridge_inc \
    Jar_uno_accessbridge \
    Library_java_uno_accessbridge \
))
endif
endif

# vim:set noet sw=4 ts=4:
