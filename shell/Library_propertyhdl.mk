# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,propertyhdl))

$(eval $(call gb_Library_set_include,propertyhdl,\
	-I$(SRCDIR)/shell/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,propertyhdl,\
	-DISOLATION_AWARE_ENABLED \
	-DUNICODE \
	-DWIN32_LEAN_AND_MEAN \
	-D_NTSDK \
	-D_UNICODE \
	-UNOMINMAX \
))
$(eval $(call gb_Library_add_defs,propertyhdl,\
	-U_WIN32_IE \
	-D_WIN32_IE=0x501 \
	-U_WIN32_WINNT \
	-D_WIN32_WINNT=0x0501 \
))

$(eval $(call gb_Library_use_externals,propertyhdl,\
	expat \
	zlib \
))

$(eval $(call gb_Library_use_system_win32_libs,propertyhdl,\
	advapi32 \
	comctl32 \
	gdi32 \
	gdiplus \
	kernel32 \
	msvcprt \
	ole32 \
	oleaut32 \
	propsys \
	shell32 \
	shlwapi \
	uuid \
))

$(eval $(call gb_Library_use_static_libraries,propertyhdl,\
	shell_xmlparser \
	shlxthandler_common \
))

$(eval $(call gb_Library_add_ldflags,propertyhdl,\
	/EXPORT:DllCanUnloadNow \
	/EXPORT:DllGetClassObject \
	/NODEFAULTLIB:libcmt.lib \
))

$(eval $(call gb_Library_add_exception_objects,propertyhdl,\
    shell/source/win32/shlxthandler/prophdl/propertyhdl \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
