# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ooofilt_x64))

$(eval $(call gb_Library_set_x64,ooofilt_x64,YES))

$(eval $(call gb_Library_use_custom_headers,ooofilt_x64,\
	shell/source \
))

$(eval $(call gb_Library_set_include,ooofilt_x64,\
	-I$(SRCDIR)/shell/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,ooofilt_x64,\
	-DISOLATION_AWARE_ENABLED \
	-DUNICODE \
	-DWIN32_LEAN_AND_MEAN \
	-D_NTSDK \
	-D_UNICODE \
	-UNOMINMAX \
))
$(eval $(call gb_Library_add_defs,ooofilt_x64,\
	-U_WIN32_IE \
	-D_WIN32_IE=0x501 \
	-U_WIN32_WINNT \
	-D_WIN32_WINNT=0x0501 \
))

$(eval $(call gb_Library_use_externals,ooofilt_x64,\
	expat_x64 \
	zlib_x64 \
))

$(eval $(call gb_Library_use_system_win32_libs,ooofilt_x64,\
	advapi32 \
	comctl32 \
	gdi32 \
	gdiplus \
	kernel32 \
	msvcprt \
	msvcrt \
	oldnames \
	ole32 \
	oleaut32 \
	propsys \
	shell32 \
	user32 \
	uuid \
))

$(eval $(call gb_Library_use_static_libraries,ooofilt_x64,\
	shell_xmlparser_x64 \
	shlxthandler_common_x64 \
))

$(eval $(call gb_Library_add_ldflags,ooofilt_x64,\
	/EXPORT:DllCanUnloadNow \
	/EXPORT:DllGetClassObject \
	/EXPORT:DllRegisterServer \
	/EXPORT:DllUnregisterServer \
	/NODEFAULTLIB:libcmt.lib \
))

$(eval $(call gb_Library_add_x64_generated_exception_objects,ooofilt_x64,\
    CustomTarget/shell/source/win32/shlxthandler/ooofilt/ooofilt \
    CustomTarget/shell/source/win32/shlxthandler/ooofilt/propspec \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
