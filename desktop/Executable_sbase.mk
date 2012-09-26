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

$(eval $(call gb_Executable_Executable,sbase))

$(eval $(call gb_Executable_set_targettype_gui,sbase,YES))

$(eval $(call gb_Executable_add_defs,sbase,\
    -DUNICODE \
))

$(eval $(call gb_Executable_use_libraries,sbase,\
	$(gb_UWINAPI) \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Executable_add_libs,sbase,\
    $(call gb_CxxObject_get_target,desktop/win32/source/applauncher/launcher) \
))

$(call gb_Executable_get_target,sbase) : \
    $(call gb_CxxObject_get_target,desktop/win32/source/applauncher/launcher)

$(eval $(call gb_Executable_add_noexception_objects,sbase,\
    desktop/win32/source/applauncher/sbase \
))

$(eval $(call gb_Executable_add_nativeres,sbase,sbase/src))

# vim: set ts=4 sw=4 et:
