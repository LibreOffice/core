# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,kit))

ifeq ($(OS), $(filter LINUX %BSD SOLARIS, $(OS)))

$(eval $(call gb_Module_add_check_targets,kit, \
    CppunitTest_kit_checkapi \
))

# tdf#113311 disabled because it can deadlock on shutdown
#$(eval $(call gb_Module_add_subsequentcheck_targets,kit,\
    CppunitTest_kit_tiledrendering \
))

ifneq ($(ENABLE_GTK3),)
$(eval $(call gb_Module_add_targets,kit,\
    Library_kitgtk \
    Executable_gtktiledviewer \
))
endif # ($(ENABLE_GTK3),)

$(eval $(call gb_Module_add_targets,kit,\
    $(if $(DISABLE_DYNLOADING),,Executable_tilebench) \
    Package_selectionhandles \
    UIConfig_kit \
))

else ifeq ($(OS),WNT)

ifneq ($(ENABLE_GTKTILEDVIEWER),)
$(eval $(call gb_Module_add_targets,kit,\
    Library_kitgtk \
    Executable_gtktiledviewer \
    Package_selectionhandles \
))
endif

endif

# vim: set ts=4 sw=4 et:
