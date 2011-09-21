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
# Copyright (C) 2011 David Tardon, Red Hat Inc. <dtardon@redhat.com> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are licable
# instead of those above.

ifeq ($(OS_FOR_BUILD),WNT)
unopkgbin := unopkg_bin
else
unopkgbin := unopkg.bin
endif

$(eval $(call gb_Executable_Executable,$(unopkgbin)))

$(eval $(call gb_Executable_set_targettype_gui,$(unopkgbin),YES))

$(eval $(call gb_Executable_add_precompiled_header,$(unopkgbin),desktop/inc/pch/precompiled_desktop.hxx))

$(eval $(call gb_Executable_set_include,$(unopkgbin),\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc/pch \
    -I$(SRCDIR)/desktop/source/inc \
))

$(eval $(call gb_Executable_add_linked_libs,$(unopkgbin),\
    comphelper \
    sal \
    tl \
    unopkgapp \
))

$(eval $(call gb_Executable_add_cobjects,$(unopkgbin),\
    desktop/source/pkgchk/unopkg/unopkg_main \
))

ifeq ($(OS),WNT)

# the resulting executable is called $(unopkgbin).exe, copy it to $(unopkgbin)
$(eval $(call gb_Package_Package,$(unopkgbin),$(OUTDIR)/bin))
$(eval $(call gb_Package_add_file,$(unopkgbin),bin/unopkg.bin,$(unopkgbin).exe))

endif

# vim: set ts=4 sw=4 et:
