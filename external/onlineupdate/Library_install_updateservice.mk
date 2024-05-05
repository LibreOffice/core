# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,install_updateservice))

$(eval $(call gb_Library_use_unpacked,install_updateservice,onlineupdate))

$(eval $(call gb_Library_set_include,install_updateservice, \
    -I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/source/update/common \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,install_updateservice, \
    -U_DLL \
))

$(eval $(call gb_Library_add_cxxflags,install_updateservice, \
    $(if $(MSVC_USE_DEBUG_RUNTIME),/MTd,/MT) \
    $(if $(filter -fsanitize=%,$(CC)),,/fno-sanitize-address-vcasan-lib) \
))

$(eval $(call gb_Library_add_ldflags,install_updateservice, \
    /DEF:$(SRCDIR)/external/onlineupdate/install_updateservice.def \
    /NODEFAULTLIB \
))

$(eval $(call gb_Library_add_exception_objects,install_updateservice, \
    external/onlineupdate/install_updateservice \
))

$(eval $(call gb_Library_use_static_libraries,install_updateservice, \
    updatehelper \
))

$(eval $(call gb_Library_use_system_win32_libs,install_updateservice, \
    libcmt \
    libcpmt \
    libucrt \
    libvcruntime \
    msi \
    kernel32 \
    user32 \
    advapi32 \
    shell32 \
    shlwapi \
))

# vim: set noet sw=4 ts=4:
