# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,unoidl))

$(eval $(call gb_Module_add_targets,unoidl, \
    $(if $(call gb_not,$(CROSS_COMPILING)), \
        Executable_unoidl-check \
        Executable_unoidl-read \
        Executable_unoidl-write) \
    Library_unoidl \
))

$(eval $(call gb_Module_add_check_targets,unoidl, \
    CustomTarget_unoidl-write_test \
    $(if $(filter ODK,$(BUILD_TYPE)),CustomTarget_unoidl-check_test) \
))
# vim: set noet sw=4 ts=4:
