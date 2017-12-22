# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,twain_dsm))

ifeq ($(OS),WNT)

$(eval $(call gb_Module_add_targets,twain_dsm,\
    UnpackedTarball_twain_dsm \
    $(if $(filter X86_64,$(CPUNAME)), \
        ExternalProject_twain_dsm \
        ExternalPackage_twain_dsm \
    ) \
))

endif

# vim: set noet sw=4 ts=4:
