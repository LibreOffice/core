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
    AllLangResTarget_deployment \
    AllLangResTarget_deploymentgui \
    AllLangResTarget_dkt \
    Library_deployment \
    Library_deploymentgui \
    Library_deploymentmisc \
    Library_offacc \
    Library_sofficeapp \
    $(if $(ENABLE_HEADLESS),,Library_spl) \
    Package_branding \
    $(if $(CUSTOM_BRAND_DIR),Package_branding_custom) \
    UIConfig_deployment \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Module_add_targets,desktop,\
	Library_libreoffice \
))
endif

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Module_add_targets,desktop,\
    Executable_unopkg.bin \
    Library_migrationoo2 \
    Library_migrationoo3 \
    Library_unopkgapp \
    $(if $(gb_RUNNABLE_INSTDIR),\
        Package_scripts_install \
    ) \
    Package_scripts \
    Pagein_calc \
    Pagein_common \
    Pagein_draw \
    Pagein_impress \
    Pagein_writer \
))

ifneq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,desktop,\
    Executable_soffice.bin \
))
endif

ifneq ($(OS),MACOSX)
ifneq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,desktop,\
    CustomTarget_soffice \
    Package_soffice_sh \
))
endif
endif
endif

ifeq ($(OS),WNT)

$(eval $(call gb_Module_add_targets,desktop,\
    StaticLibrary_winextendloaderenv \
    StaticLibrary_winlauncher \
    Executable_crashrep.com \
    Executable_quickstart \
    Executable_sbase \
    Executable_scalc \
    Executable_sdraw \
    Executable_simpress \
    Executable_smath \
    Executable_soffice \
    Executable_soffice_bin \
    Executable_sweb \
    Executable_swriter \
    Executable_unoinfo \
    Executable_unopkg \
    Executable_unopkg.com \
    Package_soffice_bin \
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

else ifeq ($(OS),MACOSX)

else ifeq ($(OS),ANDROID)

else ifeq ($(OS),IOS)

else

$(eval $(call gb_Module_add_targets,desktop,\
    Executable_oosplash \
    Library_spl_unx \
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

# vim: set ts=4 sw=4 et:
