# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,shell))

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Module_add_targets_for_build,shell,\
	$(if $(filter WNT,$(OS)),Executable_lngconvex) \
))
endif

ifeq ($(gb_Side),host)

$(eval $(call gb_Module_add_targets,shell,\
	$(if $(filter-out MACOSX WNT,$(OS)),Library_desktopbe) \
	Library_localebe \
))

ifneq ($(filter DRAGONFLY FREEBSD LINUX NETBSD OPENBSD,$(OS)),)
ifeq ($(ENABLE_GIO),TRUE)
$(eval $(call gb_Module_add_targets,shell,\
	Library_losessioninstall \
))
endif
endif

ifeq ($(OS),SOLARIS)
ifeq ($(ENABLE_GIO),TRUE)
$(eval $(call gb_Module_add_targets,shell,\
    Library_losessioninstall \
))
endif
endif

ifeq ($(ENABLE_KF5),TRUE)
$(eval $(call gb_Module_add_targets,shell,\
	Library_kf5be \
))
endif

ifeq ($(OS),WNT)

$(eval $(call gb_Module_add_targets,shell,\
	Executable_regsvrex \
	Executable_senddoc \
	Library_smplmail \
	Library_wininetbe \
	Executable_spsupp_helper \
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
	CustomTarget_spsupp_idl \
	WinResTarget_spsupp \
	WinResTarget_spsupp_dlg \
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

ifneq ($(CXX_X64_BINARY),)
$(eval $(call gb_Module_add_targets,shell,\
	Library_spsupp_x64 \
))
endif

ifneq ($(CXX_X86_BINARY),)
$(eval $(call gb_Module_add_targets,shell,\
	Library_spsupp_x86 \
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
	Library_syssh \
))

ifneq ($(OS),WNT)

$(eval $(call gb_Module_add_targets,shell,\
	StaticLibrary_xmlparser \
	Executable_uri_encode \
	Library_cmdmail \
	$(if $(ENABLE_MACOSX_SANDBOX),,Package_senddoc) \
))

endif

endif

$(eval $(call gb_Module_add_l10n_targets,shell,\
    AllLangMoTarget_shell \
))

endif # $(gb_Side) == host

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
