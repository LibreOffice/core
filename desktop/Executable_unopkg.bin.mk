# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,unopkg_bin))

$(eval $(call gb_Executable_set_targettype_gui,unopkg_bin,YES))

$(eval $(call gb_Executable_set_include,unopkg_bin,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/source/inc \
))

$(eval $(call gb_Executable_use_libraries,unopkg_bin,\
    comphelper \
    sal \
    tl \
    unopkgapp \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_add_cobjects,unopkg_bin,\
    desktop/source/pkgchk/unopkg/unopkg_main \
))

# vim: set ts=4 sw=4 et:
