# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,shell))

$(eval $(call gb_Module_add_targets,shell,\
	$(if $(filter-out MACOSX WNT,$(OS)),Library_desktopbe) \
	Library_localebe \
))

ifeq ($(OS),LINUX)
ifeq ($(ENABLE_GIO),TRUE)
$(eval $(call gb_Module_add_targets,shell,\
	Library_losessioninstall \
))
endif
endif

ifeq ($(ENABLE_GCONF),TRUE)
$(eval $(call gb_Module_add_targets,shell,\
	Library_gconfbe \
))
endif

ifeq ($(ENABLE_KDE4),TRUE)
$(eval $(call gb_Module_add_targets,shell,\
	Library_kde4be \
))
endif

ifeq ($(ENABLE_KDE),TRUE)
$(eval $(call gb_Module_add_targets,shell,\
	Library_kdebe \
))
endif

ifeq ($(ENABLE_TDE),TRUE)
$(eval $(call gb_Module_add_targets,shell,\
	Library_tdebe \
))
endif

ifneq ($(filter TRUE,$(ENABLE_KDE) $(ENABLE_TDE)),)
$(eval $(call gb_Module_add_targets,shell,\
))
endif

ifeq ($(OS),WNT)

$(eval $(call gb_Module_add_targets,shell,\
	Executable_regsvrex \
	Executable_senddoc \
	Library_smplmail \
	Library_syssh_win \
	Library_wininetbe \
	StaticLibrary_simplemapi \
))

ifeq ($(COM),MSC)
$(eval $(call gb_Module_add_targets,shell,\
	CustomTarget_shlxthdl_res \
	Library_ooofilt \
	Library_propertyhdl \
	Library_shlxthdl \
	StaticLibrary_shlxthandler_common \
	StaticLibrary_xmlparser \
	WinResTarget_shlxthdl \
))

$(eval $(call gb_Module_add_check_targets,shell,\
    CppunitTest_shell_zip \
))
endif

ifeq ($(BUILD_X64),TRUE)
$(eval $(call gb_Module_add_targets,shell,\
	CustomTarget_x64 \
	Library_ooofilt_x64 \
	Library_propertyhdl_x64 \
	Library_shlxthdl_x64 \
	StaticLibrary_shlxthandler_common_x64 \
	StaticLibrary_xmlparser_x64 \
))
endif

endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_Module_add_targets,shell,\
	Library_macbe \
))
endif

ifeq ($(filter DESKTOP,$(BUILD_TYPE)),DESKTOP)

$(eval $(call gb_Module_add_targets,shell,\
	Executable_lngconvex \
))

ifneq ($(OS),WNT)

$(eval $(call gb_Module_add_targets,shell,\
	Executable_gnome_open_url \
	Executable_uri_encode \
	Library_cmdmail \
	Library_syssh \
	Package_scripts \
	Package_scripts_gnome \
	Package_scripts_kde \
	Package_senddoc \
	StaticLibrary_xmlparser \
))

ifneq ($(filter-out MACOSX IOS ANDROID,$(OS)),)
$(eval $(call gb_Module_add_targets,shell,\
	Library_recentfile \
))
endif

endif

endif

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
