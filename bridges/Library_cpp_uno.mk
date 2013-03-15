# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifeq ($(COM),GCC)
bridge_lib_name := gcc3_uno
else ifeq ($(COM),MSC)
ifeq ($(CPU),I)
bridge_lib_name := msci_uno
else ifeq ($(CPU),X)
bridge_lib_name := mscx_uno
endif # COM=WNT
endif

$(eval $(call gb_Library_Library,$(bridge_lib_name)))

ifeq ($(OS)$(CPU),AIXP)
bridges_SELECTED_BRIDGE := gcc3_aix_powerpc
bridge_exception_objects := except
bridge_cxx_objects := cpp2uno uno2cpp
else ifeq ($(CPU),R)
ifeq ($(OS),IOS)
bridges_SELECTED_BRIDGE := gcc3_ios_arm
bridge_asm_objects := helper
bridge_exception_objects := cpp2uno cpp2uno-arm cpp2uno-i386 except uno2cpp uno2cpp-arm uno2cpp-i386
$(eval $(call gb_Library_use_custom_headers,gcc3_uno,\
	bridges/source/cpp_uno/gcc3_ios_arm \
))
else ifneq ($(filter LINUX ANDROID,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_arm
bridge_noopt_objects := cpp2uno except uno2cpp
# HACK
$(call gb_LinkTarget_get_target,$(call gb_Library_get_linktargetname,gcc3_uno)) : \
	$(call gb_CustomTarget_get_workdir,bridges/source/cpp_uno/gcc3_linux_arm)/armhelper.objectlist
$(call gb_LinkTarget_get_target,$(call gb_Library_get_linktargetname,gcc3_uno)) : \
	EXTRAOBJECTLISTS += $(call gb_CustomTarget_get_workdir,bridges/source/cpp_uno/gcc3_linux_arm)/armhelper.objectlist
endif # CPU=R
else ifeq ($(OS)$(CPU),LINUXL)
bridges_SELECTED_BRIDGE := gcc3_linux_alpha
bridge_exception_objects := cpp2uno except uno2cpp
else ifeq ($(OS)$(CPU),LINUXH)
bridges_SELECTED_BRIDGE := gcc3_linux_hppa
bridge_asm_objects := call
bridge_noopt_objects := cpp2uno except uno2cpp
else ifeq ($(OS)$(CPU),LINUXA)
bridges_SELECTED_BRIDGE := gcc3_linux_ia64
bridge_asm_objects := call
bridge_exception_objects := except
bridge_noopt_objects := cpp2uno uno2cpp
else ifeq ($(CPU),I)
ifneq ($(filter ANDROID DRAGONFLY FREEBSD LINUX NETBSD OPENBSD,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_intel
bridge_asm_objects := call
bridge_exception_objects := callvirtualmethod cpp2uno except uno2cpp
else ifeq ($(OS),MACOSX)
bridges_SELECTED_BRIDGE := gcc3_macosx_intel
bridge_asm_objects := call
bridge_exception_objects := cpp2uno except uno2cpp
else ifeq ($(COM),MSC)
bridges_SELECTED_BRIDGE := msvc_win32_intel
bridge_exception_objects := cpp2uno dllinit uno2cpp
bridge_noopt_objects := except
endif # CPU=I
else ifeq ($(OS)$(CPU),LINUX6)
bridges_SELECTED_BRIDGE := gcc3_linux_m68k
bridge_noopt_objects := cpp2uno except uno2cpp
else ifeq ($(CPU),M)
ifneq ($(filter ANDROID LINUX,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_mips
bridge_noopt_objects := cpp2uno uno2cpp
bridge_exception_objects := except
endif # CPU=M
else ifeq ($(OS)$(CPUNAME),LINUXPOWERPC)
bridges_SELECTED_BRIDGE := gcc3_linux_powerpc
bridge_noopt_objects := uno2cpp
bridge_exception_objects := cpp2uno except
else ifeq ($(OS)$(CPUNAME),LINUXPOWERPC64)
bridges_SELECTED_BRIDGE := gcc3_linux_powerpc64
bridge_noopt_objects := cpp2uno uno2cpp
bridge_exception_objects := except
else ifeq ($(OS)$(CPUNAME),LINUXS390)
bridges_SELECTED_BRIDGE := gcc3_linux_s390
bridge_exception_objects := cpp2uno except uno2cpp
else ifeq ($(OS)$(CPUNAME),LINUXS390X)
bridges_SELECTED_BRIDGE := gcc3_linux_s390x
bridge_exception_objects := cpp2uno except uno2cpp
else ifeq ($(OS)$(CPU),LINUXS)
bridges_SELECTED_BRIDGE := gcc3_linux_sparc
bridge_asm_objects := call
bridge_noopt_objects := except
bridge_exception_objects := cpp2uno uno2cpp
else ifeq ($(CPU),X)
ifneq ($(filter DRAGONFLY FREEBSD LINUX NETBSD OPENBSD,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_x86-64
bridge_asm_objects := call
bridge_exception_objects := abi callvirtualmethod cpp2uno except uno2cpp
else ifeq ($(OS),MACOSX)
bridges_SELECTED_BRIDGE := gcc3_macosx_x86-64
bridge_exception_objects := abi call callvirtualmethod cpp2uno except uno2cpp
else ifeq ($(COM),MSC)
bridges_SELECTED_BRIDGE := msvc_win32_x86-64
bridge_exception_objects := cpp2uno dllinit uno2cpp
bridge_noopt_objects := except
bridge_asm_objects := call
endif # CPU=X
else ifeq ($(OS)$(CPU),MACOSXP)
bridges_SELECTED_BRIDGE := gcc3_macosx_powerpc
bridge_noopt_objects := uno2cpp
bridge_exception_objects := cpp2uno except
else ifeq ($(OS)$(CPU),SOLARISI)
bridges_SELECTED_BRIDGE := gcc3_solaris_intel
bridge_exception_objects := cpp2uno except uno2cpp
else ifeq ($(OS)$(CPU),SOLARISS)
bridges_SELECTED_BRIDGE := gcc3_solaris_sparc
bridge_noopt_objects := cpp2uno uno2cpp
bridge_exception_objects := except
else ifeq ($(OS)$(COM),WNTGCC)
bridges_SELECTED_BRIDGE := mingw_intel
bridge_noopt_objects := uno2cpp
bridge_exception_objects := callvirtualmethod cpp2uno dllinit except smallstruct
endif

$(eval $(call gb_Library_use_external,$(bridge_lib_name),boost_headers))

$(eval $(call gb_Library_use_internal_comprehensive_api,$(bridge_lib_name),\
	udkapi \
))

$(eval $(call gb_Library_set_include,$(bridge_lib_name),\
	-I$(SRCDIR)/bridges/inc \
	$$(INCLUDE) \
))

ifeq ($(HAVE_POSIX_FALLOCATE),YES)
$(eval $(call gb_Library_add_defs,$(bridge_lib_name),\
	-DHAVE_POSIX_FALLOCATE \
))
endif
ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_defs,$(bridge_lib_name),\
	$(if $(filter GCC,$(COM)),\
	$(if $(filter sjlj,$(EXCEPTIONS)),\
		-DBROKEN_ALLOCA \
	), \
	$(if $(cppu_no_leak)$(bndchk),,\
		-DLEAK_STATIC_DATA \
	)) \
))
endif

# In case someone enabled the non-standard -fomit-frame-pointer which does not
# work with the .cxx sources of this library.
# LTO causes crashes when enabled for this library
# In case the compiler supports AVX this code segfaults so specifically turn
# it off.
ifeq ($(COM),GCC)
$(eval $(call gb_Library_add_cxxflags,gcc3_uno,\
	-fno-omit-frame-pointer \
	-fno-strict-aliasing \
	$(if $(filter TRUE,$(ENABLE_LTO)),-fno-lto) \
	$(if $(filter I,$(OS)),\
		$(if $(filter TRUE,$(HAVE_GCC_AVX)),\
			-mno-avx)) \
))

ifneq ($(OS),ANDROID)
$(eval $(call gb_Library_add_libs,gcc3_uno,\
	-ldl \
))
endif
endif

$(eval $(call gb_Library_use_libraries,$(bridge_lib_name),\
	cppu \
	sal \
))

$(foreach obj,$(bridge_exception_objects),\
	$(eval $(call gb_Library_add_exception_objects,$(bridge_lib_name),\
	bridges/source/cpp_uno/$(bridges_SELECTED_BRIDGE)/$(obj))) \
)

$(foreach obj,$(bridge_noopt_objects),\
	$(eval $(call gb_Library_add_cxxobjects,$(bridge_lib_name),\
	bridges/source/cpp_uno/$(bridges_SELECTED_BRIDGE)/$(obj) \
	, $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS))) \
)
$(foreach obj,$(bridge_cxx_objects),\
	$(eval $(call gb_Library_add_cxxobjects,$(bridge_lib_name),\
	bridges/source/cpp_uno/$(bridges_SELECTED_BRIDGE)/$(obj))) \
)
$(foreach obj,$(bridge_asm_objects),\
$(eval $(call gb_Library_add_asmobjects,$(bridge_lib_name),\
	bridges/source/cpp_uno/$(bridges_SELECTED_BRIDGE)/$(obj))) \
)

$(eval $(call gb_Library_add_exception_objects,$(bridge_lib_name),\
	bridges/source/cpp_uno/shared/bridge \
	bridges/source/cpp_uno/shared/component \
	bridges/source/cpp_uno/shared/types \
	bridges/source/cpp_uno/shared/unointerfaceproxy \
	bridges/source/cpp_uno/shared/vtablefactory \
	bridges/source/cpp_uno/shared/vtables \
))

$(eval $(call gb_Library_add_cxxobjects,$(bridge_lib_name),\
	bridges/source/cpp_uno/shared/cppinterfaceproxy \
	, $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))

# vim: set noet sw=4 ts=4:
