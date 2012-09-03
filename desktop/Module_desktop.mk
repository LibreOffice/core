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
# Copyright (C) 2011 Red Hat, Inc., David Tardon <dtardon@redhat.com>
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

$(eval $(call gb_Module_Module,desktop))

$(eval $(call gb_Module_add_targets,desktop,\
    AllLangResTarget_deployment \
    AllLangResTarget_deploymentgui \
    AllLangResTarget_dkt \
    Library_deployment \
    Library_deploymentmisc \
    Library_offacc \
    Library_sofficeapp \
    Library_spl \
    Package_intro \
    Zip_brand \
    Zip_brand_dev \
))

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Module_add_targets,desktop,\
    Executable_unopkg.bin \
    Library_deploymentgui \
    Library_migrationoo2 \
    Library_migrationoo3 \
    Library_unopkgapp \
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

$(eval $(call gb_Module_add_targets,desktop,\
    Executable_officeloader \
))

else ifeq ($(OS),ANDROID)

else ifeq ($(OS),IOS)

else ifeq ($(GUIBASE),headless)

else

$(eval $(call gb_Module_add_targets,desktop,\
    Executable_oosplash \
    Library_spl_unx \
))

endif

# vim: set ts=4 sw=4 et:
