# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,shlxtmsi))

$(eval $(call gb_Library_use_external,shlxtmsi,boost_headers))

$(eval $(call gb_Library_add_defs,shlxtmsi,\
	-U_DLL \
))

$(eval $(call gb_Library_add_cxxflags,shlxtmsi,\
	/MT \
))

$(eval $(call gb_Library_add_ldflags,shlxtmsi,\
	/DEF:$(SRCDIR)/setup_native/source/win32/customactions/shellextensions/shlxtmsi.def \
	/NODEFAULTLIB \
))

$(eval $(call gb_Library_add_exception_objects,shlxtmsi,\
    setup_native/source/win32/customactions/shellextensions/startmenuicon \
    setup_native/source/win32/customactions/shellextensions/upgrade \
    setup_native/source/win32/customactions/shellextensions/iconcache \
    setup_native/source/win32/customactions/shellextensions/migrateinstallpath \
    setup_native/source/win32/customactions/shellextensions/completeinstallpath \
    setup_native/source/win32/customactions/shellextensions/checkdirectory \
    setup_native/source/win32/customactions/shellextensions/layerlinks \
    setup_native/source/win32/customactions/shellextensions/vistaspecial \
    setup_native/source/win32/customactions/shellextensions/checkpatches \
))

$(eval $(call gb_Library_use_static_libraries,shlxtmsi,\
	seterror \
))

$(eval $(call gb_Library_use_system_win32_libs,shlxtmsi,\
	libcmt \
	libcpmt \
	msi \
	kernel32 \
	advapi32 \
	shell32 \
	psapi \
))

# vim: set noet sw=4 ts=4:
