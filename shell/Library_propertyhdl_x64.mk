# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,propertyhdl_x64))

$(eval $(call gb_Library_set_x64,propertyhdl_x64,YES))

$(eval $(call gb_Library_use_custom_headers,propertyhdl_x64,\
	shell/source \
))

$(eval $(call gb_Library_set_include,propertyhdl_x64,\
	-I$(SRCDIR)/shell/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,propertyhdl_x64,\
	-DISOLATION_AWARE_ENABLED \
	-DUNICODE \
	-DWIN32_LEAN_AND_MEAN \
	-D_NTSDK \
	-D_UNICODE \
	-UNOMINMAX \
))
$(eval $(call gb_Library_add_defs,propertyhdl_x64,\
	-U_WIN32_IE \
	-D_WIN32_IE=0x501 \
	-U_WIN32_WINNT \
	-D_WIN32_WINNT=0x0501 \
))

$(eval $(call gb_Library_use_externals,propertyhdl_x64,\
	expat_utf16_x64 \
	zlib \
))

$(eval $(call gb_Library_use_system_win32_libs,propertyhdl_x64,\
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
	shlwapi \
	user32 \
	uuid \
))

$(eval $(call gb_Library_use_static_libraries,propertyhdl_x64,\
	shell_xmlparser_x64 \
	shlxthandler_common_x64 \
))

$(eval $(call gb_Library_add_ldflags,propertyhdl_x64,\
	/EXPORT:DllCanUnloadNow \
	/EXPORT:DllGetClassObject \
	/NODEFAULTLIB:libcmt.lib \
))

$(eval $(call gb_Library_add_x64_generated_exception_objects,propertyhdl_x64,\
    CustomTarget/shell/source/win32/shlxthandler/prophdl/propertyhdl \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
