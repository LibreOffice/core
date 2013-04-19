# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,shlxthdl))

$(eval $(call gb_Library_set_include,shlxthdl,\
	-I$(SRCDIR)/shell/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,shlxthdl,\
	-DISOLATION_AWARE_ENABLED \
	-DUNICODE \
	-DWIN32_LEAN_AND_MEAN \
	-D_NTSDK \
	-D_UNICODE \
	-UNOMINMAX \
))
$(eval $(call gb_Library_add_defs,shlxthdl,\
	-U_WIN32_IE \
	-D_WIN32_IE=0x501 \
	-U_WIN32_WINNT \
	-D_WIN32_WINNT=0x0501 \
))

$(eval $(call gb_Library_use_externals,shlxthdl,\
	expat \
	zlib \
))

$(eval $(call gb_Library_use_system_win32_libs,shlxthdl,\
	advapi32 \
	comctl32 \
	gdi32 \
	gdiplus \
	kernel32 \
	ole32 \
	oleaut32 \
	shell32 \
	shlwapi \
	uuid \
))

$(eval $(call gb_Library_use_static_libraries,shlxthdl,\
	shlxthandler_common \
	shell_xmlparser \
))

$(eval $(call gb_Library_add_nativeres,shlxthdl,shlxthdl))

$(eval $(call gb_Library_add_ldflags,shlxthdl,\
	/EXPORT:DllCanUnloadNow \
	/EXPORT:DllGetClassObject \
	/EXPORT:DllRegisterServer \
	/EXPORT:DllUnregisterServer \
	/NODEFAULTLIB:libcmt.lib \
))

$(eval $(call gb_Library_add_exception_objects,shlxthdl,\
    shell/source/win32/shlxthandler/classfactory \
    shell/source/win32/shlxthandler/columninfo/columninfo \
    shell/source/win32/shlxthandler/infotips/infotips \
    shell/source/win32/shlxthandler/propsheets/document_statistic \
    shell/source/win32/shlxthandler/propsheets/listviewbuilder \
    shell/source/win32/shlxthandler/propsheets/propsheets \
    shell/source/win32/shlxthandler/shlxthdl \
    shell/source/win32/shlxthandler/thumbviewer/thumbviewer \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
