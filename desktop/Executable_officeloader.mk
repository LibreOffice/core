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

$(eval $(call gb_Executable_Executable,officeloader))

$(eval $(call gb_Executable_add_precompiled_header,officeloader,desktop/inc/pch/precompiled_desktop.hxx))

$(eval $(call gb_Executable_set_include,officeloader,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc/pch \
))

$(eval $(call gb_Executable_add_linked_libs,officeloader,\
    sal \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Executable_add_linked_libs,officeloader,\
    advapi32 \
    shell32 \
    shlwapi \
))
endif

ifeq ($(OS),MACOSX)

# I'm not suer wht was the intent, but that break the link
#$(eval $(call gb_Executable_set_ldflags,officeloader,\
#    $(filter -bind_at_load,$$(LDFLAGS)) \
#))

$(eval $(call gb_Executable_add_noexception_objects,officeloader,\
    desktop/unx/source/officeloader/officeloader \
))

$(eval $(call gb_Executable_add_cobjects,officeloader,\
    desktop/source/app/copyright_ascii_ooo \
))

endif

ifeq ($(GUI),WNT)

$(eval $(call gb_Executable_add_linked_libs,officeloader,\
    ooopathutils_hack \
))

$(eval $(call gb_Executable_add_noexception_objects,officeloader,\
    desktop/win32/source/extendloaderenvironment \
    desktop/win32/source/officeloader/officeloader \
))

endif

# vim: set ts=4 sw=4 et:
