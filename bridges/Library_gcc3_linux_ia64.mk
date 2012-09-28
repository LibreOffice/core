# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifneq ($(strip $(bridges_SELECTED_BRIDGE)),)
$(eval $(call gb_Output_error,cannot build bridge gcc3_linux_ia64: bridge $(bridges_SELECTED_BRIDGE) already selected))
endif
bridges_SELECTED_BRIDGE := gcc3_linux_ia64

$(eval $(call gb_Library_Library,gcc3_uno))

$(eval $(call gb_Library_use_internal_comprehensive_api,gcc3_uno,\
	udkapi \
))

$(eval $(call gb_Library_set_include,gcc3_uno,\
	-I$(SRCDIR)/bridges/inc \
	$$(INCLUDE) \
))

ifeq ($(HAVE_POSIX_FALLOCATE),YES)
$(eval $(call gb_Library_add_defs,gcc3_uno,\
	-DHAVE_POSIX_FALLOCATE \
))
endif

# In case someone enabled the non-standard -fomit-frame-pointer which does not
# work with the .cxx sources of this library.
$(eval $(call gb_Library_add_cxxflags,gcc3_uno,\
	-fno-omit-frame-pointer \
	-fno-strict-aliasing \
))

$(eval $(call gb_Library_add_libs,gcc3_uno,\
	-ldl \
))

$(eval $(call gb_Library_use_libraries,gcc3_uno,\
	cppu \
	sal \
))

$(eval $(call gb_Library_add_asmobjects,gcc3_uno,\
	bridges/source/cpp_uno/gcc3_linux_ia64/call \
))

$(eval $(call gb_Library_add_exception_objects,gcc3_uno,\
	bridges/source/cpp_uno/gcc3_linux_ia64/except \
	bridges/source/cpp_uno/shared/bridge \
	bridges/source/cpp_uno/shared/component \
	bridges/source/cpp_uno/shared/types \
	bridges/source/cpp_uno/shared/unointerfaceproxy \
	bridges/source/cpp_uno/shared/vtablefactory \
	bridges/source/cpp_uno/shared/vtables \
))

$(eval $(call gb_Library_add_cxxobjects,gcc3_uno,\
	bridges/source/cpp_uno/gcc3_linux_ia64/cpp2uno \
	bridges/source/cpp_uno/gcc3_linux_ia64/uno2cpp \
	bridges/source/cpp_uno/shared/cppinterfaceproxy \
	, $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))

# vim: set noet sw=4 ts=4:
