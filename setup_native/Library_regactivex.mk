# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,regactivex))

$(eval $(call gb_Library_add_defs,regactivex,\
	-U_DLL \
))

$(eval $(call gb_Library_add_cxxflags,regactivex,\
	$(if $(MSVC_USE_DEBUG_RUNTIME),/MTd,/MT) \
))

$(eval $(call gb_Library_add_ldflags,regactivex,\
	/DEF:$(SRCDIR)/setup_native/source/win32/customactions/regactivex/regactivex.def \
	/NODEFAULTLIB \
))

$(eval $(call gb_Library_add_exception_objects,regactivex,\
    setup_native/source/win32/customactions/regactivex/regactivex \
))

$(eval $(call gb_Library_use_system_win32_libs,regactivex,\
	libcmt \
	libcpmt \
	kernel32 \
	msi \
))

# vim: set noet sw=4 ts=4:
