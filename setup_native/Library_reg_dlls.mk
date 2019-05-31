# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,reg_dlls))

$(eval $(call gb_Library_add_defs,reg_dlls,\
	-U_DLL \
))

$(eval $(call gb_Library_add_cxxflags,reg_dlls,\
	$(if $(MSVC_USE_DEBUG_RUNTIME),/MTd,/MT) \
))

$(eval $(call gb_Library_add_ldflags,reg_dlls,\
	/DEF:$(SRCDIR)/setup_native/source/win32/customactions/reg_dlls/reg_dlls.def \
	/NODEFAULTLIB \
))

$(eval $(call gb_Library_add_exception_objects,reg_dlls,\
	setup_native/source/win32/customactions/reg_dlls/reg_dlls \
))

$(eval $(call gb_Library_use_system_win32_libs,reg_dlls,\
	libcmt \
	libcpmt \
	libucrt \
	libvcruntime \
	kernel32 \
	Ole32 \
	Shell32 \
	Msi \
))

# vim: set noet sw=4 ts=4:
