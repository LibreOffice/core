# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifeq ($(OS),WNT)
desktop_UNOPKGBIN := unopkg_bin
else
desktop_UNOPKGBIN := unopkg.bin
endif

$(eval $(call gb_Executable_Executable,$(desktop_UNOPKGBIN)))

$(eval $(call gb_Executable_set_targettype_gui,$(desktop_UNOPKGBIN),YES))

$(eval $(call gb_Executable_set_include,$(desktop_UNOPKGBIN),\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/source/inc \
))

$(eval $(call gb_Executable_use_libraries,$(desktop_UNOPKGBIN),\
    comphelper \
    sal \
    tl \
    unopkgapp \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_add_cobjects,$(desktop_UNOPKGBIN),\
    desktop/source/pkgchk/unopkg/unopkg_main \
))

ifeq ($(OS),WNT)

# the resulting executable is called $(desktop_UNOPKGBIN).exe, copy it to $(desktop_UNOPKGBIN)
$(eval $(call gb_Package_Package,$(desktop_UNOPKGBIN),$(OUTDIR)/bin))
$(eval $(call gb_Package_add_file,$(desktop_UNOPKGBIN),bin/unopkg.bin,$(desktop_UNOPKGBIN).exe))

endif

# vim: set ts=4 sw=4 et:
