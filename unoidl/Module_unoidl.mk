# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,unoidl))

# Executable_unoidl-check: !CROSS || ODK => !(CROSS && !ODK)

$(eval $(call gb_Module_add_targets,unoidl, \
    $(if $(filter DESKTOP,$(BUILD_TYPE)), \
        Executable_unoidl-read) \
    $(if $(and $(CROSS_COMPILING),$(filter-out ODK,$(BUILD_TYPE))),, \
        Executable_unoidl-check) \
    Library_unoidl \
))

$(eval $(call gb_Module_add_targets_for_build,unoidl, \
    Executable_unoidl-write \
))

$(eval $(call gb_Module_add_check_targets,unoidl, \
    CustomTarget_unoidl-write_test \
))
# vim: set noet sw=4 ts=4:
