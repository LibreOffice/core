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
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Executable_Executable,soffice.bin))

$(eval $(call gb_Executable_set_targettype_gui,soffice.bin,YES))

$(eval $(call gb_Executable_add_precompiled_header,soffice.bin,desktop/inc/pch/precompiled_desktop.hxx))

$(eval $(call gb_Executable_set_include,soffice.bin,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc/pch \
    -I$(SRCDIR)/desktop/source/inc \
))

$(eval $(call gb_Executable_add_linked_libs,soffice.bin,\
    sal \
    sofficeapp \
    user32 \
))

$(eval $(call gb_Executable_add_cobjects,soffice.bin,\
    desktop/source/app/copyright_ascii_ooo \
    desktop/source/app/main \
))

ifeq ($(OS),WNT)

$(eval $(call gb_Executable_add_linked_static_libs,soffice.bin,\
    ooopathutils \
))

ifeq ($(COM),MSC)

$(eval $(call gb_Executable_add_ldflags,soffice.bin,\
    /STACK:10000000 \
))

endif

$(eval $(call gb_Executable_add_noexception_objects,soffice.bin,\
    desktop/win32/source/extendloaderenvironment \
))

# the resulting executable is called soffice.bin.exe, copy it to soffice.bin
$(eval $(call gb_Package_Package,soffice.bin,$(OUTDIR)/bin))
$(eval $(call gb_Package_add_file,soffice.bin,bin/soffice.bin,soffice.bin.exe))

endif

ifeq ($(OS),MACOSX)

$(eval $(call gb_Executable_set_ldflags,\
    $(filter-out -bind_at_load,$$(LDFLAGS)) \
))

endif

# vim: set ts=4 sw=4 et:
