# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ooofilt))

$(eval $(call gb_Library_set_include,ooofilt,\
	-I$(SRCDIR)/shell/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,ooofilt,\
	-DISOLATION_AWARE_ENABLED \
	-DUNICODE \
	-DWIN32_LEAN_AND_MEAN \
	-D_NTSDK \
	-D_UNICODE \
))
$(eval $(call gb_Library_add_defs,ooofilt,\
	-U_WIN32_IE \
	-D_WIN32_IE=0x501 \
	-U_WIN32_WINNT \
	-D_WIN32_WINNT=0x0501 \
	-UNOMINMAX \
))

$(eval $(call gb_Library_use_externals,ooofilt,\
	expat \
	zlib \
))

$(eval $(call gb_Library_use_system_win32_libs,ooofilt,\
	advapi32 \
	comctl32 \
	kernel32 \
	msvcprt \
	oldnames \
	ole32 \
	shell32 \
	uuid \
))

ifeq ($(ENABLE_DBGUTIL),TRUE)
$(eval $(call gb_Library_use_system_win32_libs,ooofilt,\
	msvcrt \
))
endif

$(eval $(call gb_Library_use_static_libraries,ooofilt,\
	shell_xmlparser \
	shlxthandler_common \
))

$(eval $(call gb_Library_add_ldflags,ooofilt,\
	/EXPORT:DllCanUnloadNow \
	/EXPORT:DllGetClassObject \
	/EXPORT:DllRegisterServer \
	/EXPORT:DllUnregisterServer \
	/NODEFAULTLIB:libcmt.lib \
))

$(eval $(call gb_Library_add_exception_objects,ooofilt,\
    shell/source/win32/shlxthandler/ooofilt/ooofilt \
    shell/source/win32/shlxthandler/ooofilt/propspec \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
