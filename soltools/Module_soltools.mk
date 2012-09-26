# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,soltools))

ifneq ($(CROSS_COMPILING),YES)
$(eval $(call gb_Module_add_targets,soltools,\
    Executable_checkdll \
    Executable_cpp \
    Executable_javadep \
    Executable_makedepend \
))
endif

ifeq ($(OS)$(COM),SOLARISC52)
$(eval $(call gb_Module_add_targets,soltools,\
    Executable_adjustvisibility \
))
endif

ifeq ($(GUI)$(COM),WNTMSC)
$(eval $(call gb_Module_add_targets,soltools,\
    Package_inc \
))
endif

# vim:set shiftwidth=4 softtabstop=4 expandtab:
