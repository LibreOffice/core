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

$(eval $(call gb_Executable_Executable,crashrep.com))

$(eval $(call gb_Executable_set_targettype_gui,crashrep.com,NO))

$(eval $(call gb_Executable_add_precompiled_header,crashrep.com,desktop/inc/pch/precompiled_desktop.hxx))

$(eval $(call gb_Executable_set_include,crashrep.com,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc/pch \
))

$(eval $(call gb_Executable_add_defs,crashrep.com,\
    $(LFS_CFLAGS) \
))

$(eval $(call gb_Executable_add_linked_libs,crashrep.com,\
    user32 \
))

$(eval $(call gb_Executable_add_exception_objects,crashrep.com,\
    desktop/win32/source/guistdio/guistdio \
))

# the resulting executable is called soffice.bin.exe, copy it to soffice.bin
$(eval $(call gb_Package_Package,crashrep.com,$(OUTDIR)/bin))
$(eval $(call gb_Package_add_file,crashrep.com,bin/crashrep.com,crashrep.com.exe))

# vim: set ts=4 sw=4 et:
