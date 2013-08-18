# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,jvmfwk))

ifeq ($(ENABLE_JAVA),TRUE)

$(eval $(call gb_Module_add_targets,jvmfwk,\
    Library_jvmfwk \
    Package_rcfiles \
))

$(eval $(call gb_Module_add_targets,jvmfwk,\
    CustomTarget_jreproperties \
    Library_sunjavaplugin \
    Package_jreproperties \
))

ifneq (,$(filter-out MACOSX WNT,$(OS)))
ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Module_add_targets,jvmfwk,\
    Executable_javaldx \
))
endif
endif

ifneq ($(gb_RUNNABLE_INSTDIR),)
$(eval $(call gb_Module_add_targets,jvmfwk,\
	Package_jreproperties_install \
	Package_rcfiles_install \
))
endif

endif

# vim:set noet sw=4 ts=4:
