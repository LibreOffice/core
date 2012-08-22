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
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
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

$(eval $(call gb_Module_Module,shell))

$(eval $(call gb_Module_add_targets,shell,\
	Library_desktopbe \
	Library_localebe \
))

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
	Package_inc \
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

ifeq ($(GUI),UNX)

$(eval $(call gb_Module_add_targets,shell,\
	Executable_gnome_open_url \
	Executable_uri_encode \
	Library_cmdmail \
	Library_syssh \
	Package_scripts \
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
