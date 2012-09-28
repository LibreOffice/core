# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifneq ($(strip $(bridges_SELECTED_BRIDGE)),)
$(eval $(call gb_Output_error,cannot build bridge msvc_win32_intel: bridge $(bridges_SELECTED_BRIDGE) already selected))
endif
bridges_SELECTED_BRIDGE := msvc_win32_intel

$(eval $(call gb_Library_Library,msci_uno))

$(eval $(call gb_Library_use_internal_comprehensive_api,msci_uno,\
	udkapi \
))

$(eval $(call gb_Library_set_include,msci_uno,\
	-I$(SRCDIR)/bridges/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,msci_uno,\
	$(if $(filter YES,$(HAVE_POSIX_FALLOCATE)),\
		-DHAVE_POSIX_FALLOCATE \
	) \
	$(if $(cppu_no_leak)$(bndchk),,\
		-DLEAK_STATIC_DATA \
	) \
))

# In case someone enabled the non-standard -fomit-frame-pointer which does not
# work with the .cxx sources of this library.
$(eval $(call gb_Library_add_cxxflags,msci_uno,\
	-fno-omit-frame-pointer \
	-fno-strict-aliasing \
))

$(eval $(call gb_Library_use_libraries,msci_uno,\
	cppu \
	sal \
))

$(eval $(call gb_Library_add_exception_objects,msci_uno,\
	bridges/source/cpp_uno/msvc_win32_intel/cpp2uno \
	bridges/source/cpp_uno/msvc_win32_intel/dllinit \
	bridges/source/cpp_uno/msvc_win32_intel/uno2cpp \
	bridges/source/cpp_uno/shared/bridge \
	bridges/source/cpp_uno/shared/component \
	bridges/source/cpp_uno/shared/cppinterfaceproxy \
	bridges/source/cpp_uno/shared/types \
	bridges/source/cpp_uno/shared/unointerfaceproxy \
	bridges/source/cpp_uno/shared/vtablefactory \
	bridges/source/cpp_uno/shared/vtables \
))

$(eval $(call gb_Library_add_cxxobjects,msci_uno,\
	bridges/source/cpp_uno/msvc_win32_intel/except \
	, $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))

# vim: set noet sw=4 ts=4:
