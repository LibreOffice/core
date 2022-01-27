# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,desktop))

$(eval $(call gb_Module_add_targets,desktop,\
    CustomTarget_desktop_unopackages_install \
    GeneratedPackage_desktop_unopackages_install \
    Library_deployment \
    Library_deploymentgui \
    Library_deploymentmisc \
    Library_offacc \
    Library_sofficeapp \
    $(if $(ENABLE_BREAKPAD), \
        Library_crashreport \
        ) \
    $(if $(or $(DISABLE_GUI),$(ENABLE_WASM_STRIP_SPLASH)),,Library_spl) \
    Package_branding \
    $(if $(CUSTOM_BRAND_DIR),Package_branding_custom) \
    UIConfig_deployment \
))

$(eval $(call gb_Module_add_l10n_targets,desktop,\
    AllLangMoTarget_dkt \
))

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Module_add_targets,desktop,\
    Executable_soffice_bin \
    $(call gb_CondExeUnopkg,Executable_unopkg_bin) \
    $(if $(ENABLE_BREAKPAD),Executable_minidump_upload) \
    Library_migrationoo2 \
    Library_migrationoo3 \
    $(call gb_CondExeUnopkg,Library_unopkgapp) \
    Package_scripts \
    $(if $(ENABLE_PAGEIN), \
        Pagein_calc \
        Pagein_common \
        Pagein_draw \
        Pagein_impress \
        Pagein_writer \
    ) \
    $(if $(filter-out MACOSX WNT,$(OS)),CustomTarget_soffice) \
))

ifeq ($(USING_X11),TRUE)
$(eval $(call gb_Module_add_targets,desktop,\
    Package_sbase_sh \
    Package_scalc_sh \
    Package_sdraw_sh \
    Package_simpress_sh \
    Package_smath_sh \
    Package_swriter_sh \
    Package_soffice_sh \
))
endif
endif # DESKTOP

ifeq ($(OS),WNT)

$(eval $(call gb_Module_add_targets,desktop,\
    StaticLibrary_winloader \
    StaticLibrary_winlauncher \
    Executable_quickstart \
    Executable_sbase \
    Executable_scalc \
    Executable_sdraw \
    Executable_simpress \
    Executable_smath \
    Executable_soffice_exe \
    Executable_soffice_com \
    Executable_soffice_safe \
    Executable_sweb \
    Executable_swriter \
    Executable_unoinfo \
    $(call gb_CondExeUnopkg, \
        Executable_unopkg \
        Executable_unopkg_com \
    ) \
    WinResTarget_quickstart \
    WinResTarget_sbase \
    WinResTarget_scalc \
    WinResTarget_sdraw \
    WinResTarget_simpress \
    WinResTarget_soffice \
    WinResTarget_sofficebin \
    WinResTarget_smath \
    WinResTarget_sweb \
    WinResTarget_swriter \
))

else ifeq (,$(filter MACOSX ANDROID iOS HAIKU EMSCRIPTEN,$(OS)))

ifeq (,$(filter FUZZERS,$(BUILD_TYPE)))
$(eval $(call gb_Module_add_targets,desktop,\
    Executable_oosplash \
))
endif

endif # $(OS)

ifneq (,$(filter Extension_test-active,$(MAKECMDGOALS)))
$(eval $(call gb_Module_add_targets,desktop, \
    Extension_test-active \
    Jar_active_java \
    Library_active_native \
))
endif

ifneq (,$(filter Extension_test-passive,$(MAKECMDGOALS)))
$(eval $(call gb_Module_add_targets,desktop, \
    Extension_test-passive \
    Jar_passive_java \
    Library_passive_native \
    Pyuno_passive_python \
    Rdb_passive_generic \
    Rdb_passive_platform \
))
endif

ifneq (,$(filter Extension_test-crashextension,$(MAKECMDGOALS)))
$(eval $(call gb_Module_add_targets,desktop, \
    Extension_test-crashextension \
    Library_crashextension \
    Rdb_crashextension \
))
endif

$(eval $(call gb_Module_add_check_targets,desktop, \
    CppunitTest_desktop_app \
    CppunitTest_desktop_version \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Module_add_check_targets,desktop, \
    CppunitTest_desktop_lib \
    CppunitTest_desktop_lokinit \
))
endif

# screenshots
$(eval $(call gb_Module_add_screenshot_targets,desktop,\
    CppunitTest_desktop_dialogs_test \
))

# vim: set ts=4 sw=4 et:
