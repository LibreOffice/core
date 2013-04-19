# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,solenv))

$(eval $(call gb_Module_add_targets_for_build,solenv,\
	Executable_concat-deps \
))

ifeq ($(COM),MSC)
$(eval $(call gb_Module_add_targets,solenv,\
	StaticLibrary_wrapper \
	Executable_gcc-wrapper \
	Executable_g++-wrapper \
))
endif


ifneq ($(filter-out WNT IOS ANDROID,$(OS)),)
$(eval $(call gb_Module_add_targets,solenv,\
	CustomTarget_gdb \
	Package_gdb \
))
endif

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
