# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifneq ($(strip $(bridges_SELECTED_BRIDGE)),)
$(eval $(call gb_Output_error,cannot build bridge msvc_win32_x86-64: bridge $(bridges_SELECTED_BRIDGE) already selected))
endif
bridges_SELECTED_BRIDGE := msvc_win32_x86-64

$(eval $(call gb_Library_Library,mscx_uno))

$(eval $(call gb_Library_use_internal_comprehensive_api,mscx_uno,\
	udkapi \
))

$(eval $(call gb_Library_set_include,mscx_uno,\
	-I$(SRCDIR)/bridges/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,mscx_uno,\
	$(if $(filter YES,$(HAVE_POSIX_FALLOCATE)),\
		-DHAVE_POSIX_FALLOCATE \
	) \
	$(if $(cppu_no_leak)$(bndchk),,\
   		-DLEAK_STATIC_DATA \
	) \
))

$(eval $(call gb_Library_use_libraries,mscx_uno,\
	cppu \
	sal \
))

$(eval $(call gb_Library_add_asmobjects,mscx_uno,\
	bridges/source/cpp_uno/msvc_win32_x86-64/call \
))

$(eval $(call gb_Library_add_exception_objects,mscx_uno,\
	bridges/source/cpp_uno/msvc_win32_x86-64/cpp2uno \
	bridges/source/cpp_uno/msvc_win32_x86-64/dllinit \
	bridges/source/cpp_uno/msvc_win32_x86-64/uno2cpp \
	bridges/source/cpp_uno/shared/bridge \
	bridges/source/cpp_uno/shared/component \
	bridges/source/cpp_uno/shared/cppinterfaceproxy \
	bridges/source/cpp_uno/shared/types \
	bridges/source/cpp_uno/shared/unointerfaceproxy \
	bridges/source/cpp_uno/shared/vtablefactory \
	bridges/source/cpp_uno/shared/vtables \
))

$(eval $(call gb_Library_add_cxxobjects,mscx_uno,\
	bridges/source/cpp_uno/msvc_win32_x86-64/except \
	, $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))

# vim: set noet sw=4 ts=4:
