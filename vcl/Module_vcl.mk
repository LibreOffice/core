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
	Package_opengl \
	$(if $(filter WNT,$(OS)), \
		Package_opengl_blacklist ) \
    $(if $(filter DESKTOP,$(BUILD_TYPE)), \
        StaticLibrary_vclmain \
		$(if $(ENABLE_MACOSX_SANDBOX),, \
			$(if $(ENABLE_HEADLESS),, \
				Executable_ui-previewer)) \
		$(if $(filter LINUX MACOSX SOLARIS WNT %BSD,$(OS)), \
			Executable_outdevgrind \
			$(if $(ENABLE_HEADLESS),, \
				Executable_vcldemo \
				Executable_icontest \
				Executable_mtfdemo ))) \
    $(if $(filter-out ANDROID IOS WNT,$(OS)), \
        Executable_svdemo \
        Executable_fftester \
        Executable_svptest \
        Executable_svpclient) \
))

$(eval $(call gb_Module_add_l10n_targets,vcl,\
    AllLangResTarget_vcl \
    UIConfig_vcl \
))

ifeq ($(USING_X11),TRUE)
$(eval $(call gb_Module_add_targets,vcl,\
    Library_vclplug_gen \
    Library_desktop_detector \
    StaticLibrary_glxtest \
    Package_fontunxppds \
    Package_fontunxpsprint \
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
	CppunitTest_vcl_lifecycle \
	CppunitTest_vcl_bitmap_test \
	CppunitTest_vcl_fontcharmap \
	CppunitTest_vcl_font \
	CppunitTest_vcl_fontmetric \
	CppunitTest_vcl_complextext \
	CppunitTest_vcl_filters_test \
	CppunitTest_vcl_mapmode \
	CppunitTest_vcl_outdev \
	CppunitTest_vcl_app_test \
	CppunitTest_vcl_wmf_test \
))


ifeq ($(USING_X11),TRUE)
$(eval $(call gb_Module_add_check_targets,vcl,\
	CppunitTest_vcl_timer \
))
endif

ifeq ($(ENABLE_HEADLESS),TRUE)
$(eval $(call gb_Module_add_check_targets,vcl,\
	CppunitTest_vcl_timer \
))
endif

# Is any configuration missing?
ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_check_targets,vcl,\
	CppunitTest_vcl_timer \
	CppunitTest_vcl_blocklistparser_test \
))
endif

$(eval $(call gb_Module_add_slowcheck_targets,vcl,\
    CppunitTest_vcl_dialogs_test \
))

# vim: set noet sw=4 ts=4:
