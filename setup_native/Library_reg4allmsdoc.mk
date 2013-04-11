# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,reg4allmsdoc))

$(eval $(call gb_Library_add_defs,reg4allmsdoc,\
	-DUNICODE \
	-D_UNICODE \
	-U_DLL \
))

$(eval $(call gb_Library_add_cxxflags,reg4allmsdoc,\
	$(if $(MSVC_USE_DEBUG_RUNTIME),/MTd,/MT) \
))

$(eval $(call gb_Library_add_ldflags,reg4allmsdoc,\
	/DEF:$(SRCDIR)/setup_native/source/win32/customactions/reg4allmsdoc/reg4allmsdoc.def \
	/NODEFAULTLIB \
))

$(eval $(call gb_Library_add_exception_objects,reg4allmsdoc,\
    setup_native/source/win32/customactions/reg4allmsdoc/reg4allmsi \
))

$(eval $(call gb_Library_use_system_win32_libs,reg4allmsdoc,\
	libcmt \
	libcpmt \
	msi \
	kernel32 \
	user32 \
	advapi32 \
	shell32 \
	shlwapi \
))

# vim: set noet sw=4 ts=4:
