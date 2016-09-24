# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,shlxthdl_x64))

$(eval $(call gb_Library_set_x64,shlxthdl_x64,YES))

$(eval $(call gb_Library_use_custom_headers,shlxthdl_x64,\
	shell/source \
))

$(eval $(call gb_Library_set_include,shlxthdl_x64,\
	-I$(SRCDIR)/shell/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,shlxthdl_x64,\
	-DISOLATION_AWARE_ENABLED \
	-DUNICODE \
	-DWIN32_LEAN_AND_MEAN \
	-D_UNICODE \
	-UNOMINMAX \
))

$(eval $(call gb_Library_use_externals,shlxthdl_x64,\
	expat_x64 \
	zlib_x64 \
))

$(eval $(call gb_Library_use_system_win32_libs,shlxthdl_x64,\
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
	shell32 \
	shlwapi \
	user32 \
	uuid \
))

$(eval $(call gb_Library_use_static_libraries,shlxthdl_x64,\
	shlxthandler_common_x64 \
	shell_xmlparser_x64 \
))

$(eval $(call gb_Library_add_nativeres,shlxthdl_x64,shlxthdl))

$(eval $(call gb_Library_add_ldflags,shlxthdl_x64,\
	/DEF:$(SRCDIR)/shell/source/win32/shlxthandler/shlxthdl.def \
	/NODEFAULTLIB:libcmt.lib \
))

$(eval $(call gb_Library_add_x64_generated_exception_objects,shlxthdl_x64,\
    CustomTarget/shell/source/win32/shlxthandler/classfactory \
    CustomTarget/shell/source/win32/shlxthandler/columninfo/columninfo \
    CustomTarget/shell/source/win32/shlxthandler/infotips/infotips \
    CustomTarget/shell/source/win32/shlxthandler/propsheets/document_statistic \
    CustomTarget/shell/source/win32/shlxthandler/propsheets/listviewbuilder \
    CustomTarget/shell/source/win32/shlxthandler/propsheets/propsheets \
    CustomTarget/shell/source/win32/shlxthandler/shlxthdl \
    CustomTarget/shell/source/win32/shlxthandler/thumbviewer/thumbviewer \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
