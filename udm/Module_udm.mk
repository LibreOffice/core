# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
$(eval $(call gb_Module_Module,udm))

ifneq (,$(filter ODK,$(BUILD_TYPE)))

$(eval $(call gb_Module_add_targets,udm,\
    Package_inc \
    StaticLibrary_udm \
))

endif

# vim:set shiftwidth=4 softtabstop=4 expandtab:
