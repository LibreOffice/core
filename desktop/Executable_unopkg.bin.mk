# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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
