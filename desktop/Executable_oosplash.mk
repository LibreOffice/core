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

$(eval $(call gb_Executable_Executable,oosplash))

$(eval $(call gb_Executable_set_targettype_gui,oosplash,YES))

$(eval $(call gb_Executable_set_include,oosplash,\
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_use_libraries,oosplash,\
    sal \
))

$(eval $(call gb_Executable_add_cobjects,oosplash,\
    desktop/unx/source/args \
    desktop/unx/source/file_image_unx \
    desktop/unx/source/pagein \
    desktop/unx/source/splashx \
    desktop/unx/source/start \
))

ifeq ($(USE_XINERAMA),YES)

$(eval $(call gb_Executable_add_defs,oosplash,\
    -DUSE_XINERAMA \
))

$(eval $(call gb_Executable_add_libs,oosplash,\
    -lXinerama \
))

endif

ifneq ($(GUIBASE),headless)
ifeq ($(GUI),UNX)
$(eval $(call gb_Executable_add_libs,oosplash,\
    -lX11 \
))
endif
endif

ifeq ($(ENABLE_QUICKSTART_LIBPNG),TRUE)

$(eval $(call gb_Executable_add_defs,oosplash,\
    -DENABLE_QUICKSTART_LIBPNG \
))

$(eval $(call gb_Executable_use_externals,oosplash,\
    png \
))

endif

ifeq ($(OS),SOLARIS)

$(eval $(call gb_Executable_add_libs,oosplash,\
    -lsocket \
))

endif

# vim: set ts=4 sw=4 et:
