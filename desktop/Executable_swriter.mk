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

$(eval $(call gb_Executable_Executable,swriter))

$(eval $(call gb_Executable_set_targettype_gui,swriter,YES))

$(eval $(call gb_Executable_add_precompiled_header,swriter,desktop/inc/pch/precompiled_desktop.hxx))

$(eval $(call gb_Executable_set_include,swriter,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc/pch \
))

$(eval $(call gb_Executable_add_defs,swriter,\
    -DUNICODE \
))

$(eval $(call gb_Executable_add_noexception_objects,swriter,\
    desktop/win32/source/applauncher/launcher \
    desktop/win32/source/applauncher/swriter \
))

$(eval $(call gb_Executable_add_nativeres,swriter,src))

# vim: set ts=4 sw=4 et:
