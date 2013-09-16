# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,$(gb_CPPU_ENV)_uno))

ifeq ($(OS)$(CPU),AIXP)

bridges_SELECTED_BRIDGE := gcc3_aix_powerpc
bridge_exception_objects := except
bridge_cxx_objects := cpp2uno uno2cpp

else ifeq ($(CPU),R)

ifeq ($(OS),IOS)
$(eval $(call gb_Library_use_sdk_api,gcc3_uno))
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
$(call gb_LinkTarget_get_target,$(call gb_Library_get_linktarget,gcc3_uno)) : \
	$(call gb_CustomTarget_get_workdir,bridges/source/cpp_uno/gcc3_linux_arm)/armhelper.objectlist
$(call gb_LinkTarget_get_target,$(call gb_Library_get_linktarget,gcc3_uno)) : \
	EXTRAOBJECTLISTS += $(call gb_CustomTarget_get_workdir,bridges/source/cpp_uno/gcc3_linux_arm)/armhelper.objectlist
endif

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
bridge_exception_objects := cpp2uno except uno2cpp
bridge_noncallexception_objects := callvirtualmethod
else ifeq ($(OS),MACOSX)
bridges_SELECTED_BRIDGE := gcc3_macosx_intel
bridge_asm_objects := call
bridge_exception_objects := cpp2uno except uno2cpp
else ifeq ($(COM),MSC)
bridges_SELECTED_BRIDGE := msvc_win32_intel
bridge_exception_objects := cpp2uno dllinit uno2cpp
bridge_noopt_objects := except
else ifeq ($(OS)$(COM),WNTGCC)
bridges_SELECTED_BRIDGE := mingw_intel
bridge_asm_objects := call
bridge_noopt_objects := uno2cpp
bridge_exception_objects := callvirtualmethod cpp2uno dllinit except smallstruct
endif

else ifeq ($(OS)$(CPU),LINUX6)

bridges_SELECTED_BRIDGE := gcc3_linux_m68k
bridge_noopt_objects := cpp2uno except uno2cpp

else ifeq ($(CPU),M)

ifneq ($(filter ANDROID LINUX,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_mips
bridge_noopt_objects := cpp2uno uno2cpp
bridge_exception_objects := except
endif

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
bridge_noncallexception_noopt_objects := callvirtualmethod
bridge_exception_objects := abi cpp2uno except uno2cpp
else ifeq ($(OS),MACOSX)
bridges_SELECTED_BRIDGE := gcc3_macosx_x86-64
bridge_exception_objects := abi call cpp2uno except uno2cpp
bridge_noncallexception_noopt_objects := callvirtualmethod
else ifeq ($(COM),MSC)
bridges_SELECTED_BRIDGE := msvc_win32_x86-64
bridge_exception_objects := cpp2uno dllinit uno2cpp
bridge_noopt_objects := except
bridge_asm_objects := call
else ifeq ($(OS)$(COM),WNTGCC)
bridges_SELECTED_BRIDGE := mingw_x86-64
bridge_asm_objects := call
bridge_noncallexception_noopt_objects := callvirtualmethod
bridge_exception_objects := abi cpp2uno except uno2cpp
endif

else ifeq ($(OS)$(CPU),SOLARISI)

bridges_SELECTED_BRIDGE := gcc3_solaris_intel
bridge_exception_objects := cpp2uno except uno2cpp

else ifeq ($(OS)$(CPU),SOLARISS)

bridges_SELECTED_BRIDGE := gcc3_solaris_sparc
bridge_noopt_objects := cpp2uno uno2cpp
bridge_exception_objects := except

endif

$(eval $(call gb_Library_use_external,$(gb_CPPU_ENV)_uno,boost_headers))

$(eval $(call gb_Library_use_internal_comprehensive_api,$(gb_CPPU_ENV)_uno,\
	udkapi \
))

$(eval $(call gb_Library_set_include,$(gb_CPPU_ENV)_uno,\
	-I$(SRCDIR)/bridges/inc \
	$$(INCLUDE) \
))

ifeq ($(HAVE_POSIX_FALLOCATE),YES)
$(eval $(call gb_Library_add_defs,$(gb_CPPU_ENV)_uno,\
	-DHAVE_POSIX_FALLOCATE \
))
endif
ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_defs,$(gb_CPPU_ENV)_uno,\
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
	$(if $(filter TRUE,$(HAVE_GCC_AVX)),-mno-avx) \
))

ifeq ($(filter ANDROID WNT,$(OS)),)
$(eval $(call gb_Library_add_libs,gcc3_uno,\
	-ldl \
))
endif
endif

ifeq ($(COM),GCC)
ifneq ($(COM_GCC_IS_CLANG),TRUE)
bridges_NON_CALL_EXCEPTIONS_FLAGS := -fnon-call-exceptions
endif
endif

$(eval $(call gb_Library_use_libraries,$(gb_CPPU_ENV)_uno,\
	cppu \
	sal \
))

$(foreach obj,$(bridge_exception_objects),\
	$(eval $(call gb_Library_add_exception_objects,$(gb_CPPU_ENV)_uno,\
	bridges/source/cpp_uno/$(bridges_SELECTED_BRIDGE)/$(obj))) \
)
$(foreach obj,$(bridge_noncallexception_objects),\
	$(eval $(call gb_Library_add_cxxobjects,$(gb_CPPU_ENV)_uno,\
	bridges/source/cpp_uno/$(bridges_SELECTED_BRIDGE)/$(obj) \
    , $(bridges_NON_CALL_EXCEPTIONS_FLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS))) \
)
$(foreach obj,$(bridge_noopt_objects),\
	$(eval $(call gb_Library_add_cxxobjects,$(gb_CPPU_ENV)_uno,\
	bridges/source/cpp_uno/$(bridges_SELECTED_BRIDGE)/$(obj) \
	, $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS))) \
)
$(foreach obj,$(bridge_noncallexception_noopt_objects),\
	$(eval $(call gb_Library_add_cxxobjects,$(gb_CPPU_ENV)_uno,\
	bridges/source/cpp_uno/$(bridges_SELECTED_BRIDGE)/$(obj) \
	, $(gb_COMPILERNOOPTFLAGS) $(bridges_NON_CALL_EXCEPTIONS_FLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS))) \
)
$(foreach obj,$(bridge_cxx_objects),\
	$(eval $(call gb_Library_add_cxxobjects,$(gb_CPPU_ENV)_uno,\
	bridges/source/cpp_uno/$(bridges_SELECTED_BRIDGE)/$(obj))) \
)
$(foreach obj,$(bridge_asm_objects),\
$(eval $(call gb_Library_add_asmobjects,$(gb_CPPU_ENV)_uno,\
	bridges/source/cpp_uno/$(bridges_SELECTED_BRIDGE)/$(obj))) \
)

$(eval $(call gb_Library_add_exception_objects,$(gb_CPPU_ENV)_uno,\
	bridges/source/cpp_uno/shared/bridge \
	bridges/source/cpp_uno/shared/component \
	bridges/source/cpp_uno/shared/types \
	bridges/source/cpp_uno/shared/unointerfaceproxy \
	bridges/source/cpp_uno/shared/vtablefactory \
	bridges/source/cpp_uno/shared/vtables \
))

$(eval $(call gb_Library_add_cxxobjects,$(gb_CPPU_ENV)_uno,\
	bridges/source/cpp_uno/shared/cppinterfaceproxy \
	, $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))

# vim: set noet sw=4 ts=4:
