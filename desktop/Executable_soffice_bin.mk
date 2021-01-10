# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,soffice_bin))

$(eval $(call gb_Executable_set_include,soffice_bin,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/source/inc \
))

$(eval $(call gb_Executable_use_libraries,soffice_bin,\
    sal \
    sofficeapp \
))

$(eval $(call gb_Executable_add_cobjects,soffice_bin,\
    desktop/source/app/main \
))

ifeq ($(OS),WNT)

$(eval $(call gb_Executable_set_targettype_gui,soffice_bin,NO))

$(eval $(call gb_Executable_add_nativeres,soffice_bin,sofficebin/officeloader))

$(eval $(call gb_Executable_add_default_nativeres,soffice_bin,$(PRODUCTNAME)))

ifeq ($(COM),MSC)

$(eval $(call gb_Executable_add_ldflags,soffice_bin,\
    /STACK:10000000 \
))

endif

endif

# vim: set ts=4 sw=4 et:
