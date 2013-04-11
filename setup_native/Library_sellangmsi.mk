# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,sellangmsi))

$(eval $(call gb_Library_use_packages,sellangmsi,\
	setup_native_spell \
))

$(eval $(call gb_Library_add_defs,sellangmsi,\
	-U_DLL \
))

$(eval $(call gb_Library_add_cxxflags,sellangmsi,\
	$(if $(MSVC_USE_DEBUG_RUNTIME),/MTd,/MT) \
))

$(eval $(call gb_Library_add_ldflags,sellangmsi,\
	/DEF:$(SRCDIR)/setup_native/source/win32/customactions/sellang/sellang.def \
	/NODEFAULTLIB \
))

$(eval $(call gb_Library_add_exception_objects,sellangmsi,\
    setup_native/source/win32/customactions/sellang/sellang \
    setup_native/source/win32/customactions/sellang/sorttree \
))

$(eval $(call gb_Library_use_system_win32_libs,sellangmsi,\
	libcmt \
	libcpmt \
	msi \
	kernel32 \
	user32 \
	advapi32 \
	shell32 \
))

# vim: set noet sw=4 ts=4:
