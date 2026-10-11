# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,desktop))

$(eval $(call gb_Module_add_targets,desktop,\
    Library_offacc \
    Library_sofficeapp \
    $(if $(or $(DISABLE_GUI),$(ENABLE_WASM_STRIP_SPLASH)),,Library_spl) \
    Package_branding \
    $(if $(CUSTOM_BRAND_DIR),Package_branding_custom) \
))

$(eval $(call gb_Module_add_l10n_targets,desktop,\
    AllLangMoTarget_dkt \
))

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Module_add_targets,desktop,\
    $(if $(filter-out EMSCRIPTEN MACOSX WNT,$(OS)),Executable_l10ntranslate) \
    Executable_soffice_bin \
    Package_scripts \
))

ifneq (,$(filter-out EMSCRIPTEN HAIKU MACOSX WNT,$(OS)))
$(eval $(call gb_Module_add_targets,desktop,\
    Package_soffice_sh \
))
endif

endif # DESKTOP

ifeq ($(OS),WNT)

$(eval $(call gb_Module_add_targets,desktop,\
    StaticLibrary_winloader \
    Executable_soffice_exe \
    Executable_soffice_com \
    Executable_unoinfo \
    WinResTarget_soffice \
    WinResTarget_sofficebin \
))

else ifeq (,$(filter MACOSX ANDROID iOS HAIKU EMSCRIPTEN,$(OS)))

ifeq (,$(filter FUZZERS,$(BUILD_TYPE)))
$(eval $(call gb_Module_add_targets,desktop,\
    Executable_oosplash \
))
endif

endif # $(OS)

ifeq ($(OS),EMSCRIPTEN)
$(eval $(call gb_Module_add_targets,desktop, \
    CustomTarget_soffice_bin-emscripten-exports \
))
endif

$(eval $(call gb_Module_add_check_targets,desktop, \
    CppunitTest_desktop_app \
))

ifneq ($(filter LINUX FREEBSD,$(OS))$(filter WNT-TRUE,$(OS)-$(USE_HEADLESS_CODE)),)
$(eval $(call gb_Module_add_check_targets,desktop, \
    CppunitTest_desktop_lib \
    CppunitTest_desktop_kitinit \
))
endif

# vim: set ts=4 sw=4 et:
