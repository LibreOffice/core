# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Module_Module,vcl))

$(eval $(call gb_Module_add_targets,vcl,\
    CustomTarget_afm_hash \
    Library_vcl \
    $(if $(filter DESKTOP,$(BUILD_TYPE)), \
		StaticLibrary_vclmain \
		Executable_ui-previewer) \
	$(if $(filter FREEBSD LINUX MACOSX WNT,$(OS)), \
		Library_vclopengl \
		Package_opengl \
	) \
))

$(eval $(call gb_Module_add_l10n_targets,vcl,\
    AllLangResTarget_vcl \
    UIConfig_vcl \
))

ifeq ($(GUIBASE),unx)
$(eval $(call gb_Module_add_targets,vcl,\
    Library_vclplug_svp \
    Library_vclplug_gen \
    Library_desktop_detector \
    StaticLibrary_headless \
))

ifneq ($(ENABLE_GTK),)
$(eval $(call gb_Module_add_targets,vcl,\
    Executable_xid_fullscreen_on_all_monitors \
    Library_vclplug_gtk \
))
endif
ifneq ($(ENABLE_GTK3),)
$(eval $(call gb_Module_add_targets,vcl,\
    Library_vclplug_gtk3 \
))
endif
ifneq ($(ENABLE_TDE),)
$(eval $(call gb_Module_add_targets,vcl,\
    CustomTarget_tde_moc \
    Executable_tdefilepicker \
    Library_vclplug_tde \
))
endif
ifneq ($(ENABLE_KDE),)
$(eval $(call gb_Module_add_targets,vcl,\
    CustomTarget_kde_moc \
    Executable_kdefilepicker \
    Library_vclplug_kde \
))
endif
ifneq ($(ENABLE_KDE4),)
$(eval $(call gb_Module_add_targets,vcl,\
    CustomTarget_kde4_moc \
    Library_vclplug_kde4 \
))
endif
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_Module_add_targets,vcl,\
    Package_osxres \
))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,vcl,\
    WinResTarget_vcl \
))
endif

$(eval $(call gb_Module_add_check_targets,vcl,\
	CppunitTest_vcl_complextext \
	CppunitTest_vcl_filters_test \
	CppunitTest_vcl_app_test \
))

# vim: set noet sw=4 ts=4:
