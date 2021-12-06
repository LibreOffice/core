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
	Executable_gbuildtojson \
))

ifeq (,$(LOCKFILE))
$(eval $(call gb_Module_add_targets_for_build,solenv,\
    $(call gb_CondBuildLockfile,Executable_lockfile) \
))
endif

ifeq ($(COM),MSC)
$(eval $(call gb_Module_add_targets,solenv,\
	StaticLibrary_wrapper \
	Executable_gcc-wrapper \
	Executable_g++-wrapper \
))
endif

ifneq ($(DISABLE_PYTHON),TRUE)
ifneq ($(MAKE_VERSION),3.81) # gbuildtojson requires 3.82+
ifneq ($(OS),WNT) # disable on Windows for now, causes gerrit/jenkins failures
ifneq ($(OS),MACOSX) # disable on macOS too, fails at least for me and would be pointless anyway surely
$(eval $(call gb_Module_add_subsequentcheck_targets,solenv,\
	CustomTarget_gbuildtesttools \
	PythonTest_solenv_python \
))
endif
endif
endif
endif

ifneq ($(COMPILER_PLUGINS),)
ifeq ($(COMPILER_EXTERNAL_TOOL)$(COMPILER_PLUGIN_TOOL),)
$(eval $(call gb_Module_add_check_targets,solenv, \
    CompilerTest_compilerplugins_clang \
))
endif
endif

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
