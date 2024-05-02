# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,$(CPPU_ENV)_uno))

ifeq ($(CPUNAME),ARM)

ifneq ($(filter ANDROID DRAGONFLY FREEBSD LINUX NETBSD OPENBSD,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_arm
bridge_noopt_objects := cpp2uno except uno2cpp
# HACK
$(call gb_Library_get_linktarget_target,gcc3_uno) : \
	$(gb_CustomTarget_workdir)/bridges/source/cpp_uno/gcc3_linux_arm/armhelper.objectlist
$(call gb_Library_get_linktarget_target,gcc3_uno) : \
	EXTRAOBJECTLISTS += $(gb_CustomTarget_workdir)/bridges/source/cpp_uno/gcc3_linux_arm/armhelper.objectlist
endif

else ifeq ($(CPUNAME),AARCH64)

ifneq ($(filter ANDROID DRAGONFLY FREEBSD LINUX MACOSX NETBSD OPENBSD,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_aarch64
bridge_asm_objects := vtableslotcall
bridge_exception_objects := abi cpp2uno uno2cpp

$(eval $(call gb_Library_add_exception_objects,$(CPPU_ENV)_uno, \
    bridges/source/cpp_uno/$(bridges_SELECTED_BRIDGE)/callvirtualfunction, \
    $(if $(HAVE_GCC_STACK_CLASH_PROTECTION),-fno-stack-clash-protection) \
	$(if $(COM_IS_CLANG),-fasynchronous-unwind-tables) \
))

else ifeq ($(OS),iOS)
bridges_SELECTED_BRIDGE := gcc3_ios
bridge_noopt_objects := cpp2uno except uno2cpp
bridge_asm_objects := ios64_helper

else ifeq ($(COM),MSC)
bridges_SELECTED_BRIDGE := msvc_win32_arm64
bridge_exception_objects := cpp2uno uno2cpp abi
bridge_noopt_objects := except
bridge_asm_objects := callvirtualfunction vtableslotcall

endif

else ifeq ($(CPUNAME),AXP)

ifneq ($(filter DRAGONFLY FREEBSD LINUX NETBSD OPENBSD,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_alpha
bridge_exception_objects := cpp2uno except uno2cpp
endif

else ifeq ($(CPUNAME),HPPA)

ifneq ($(filter DRAGONFLY FREEBSD LINUX NETBSD OPENBSD,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_hppa
bridge_noopt_objects := call cpp2uno except uno2cpp
endif

else ifeq ($(CPUNAME),IA64)

ifneq ($(filter DRAGONFLY FREEBSD LINUX NETBSD OPENBSD,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_ia64
bridge_asm_objects := call
bridge_exception_objects := except
bridge_noopt_objects := cpp2uno uno2cpp
endif

else ifeq ($(CPUNAME),INTEL)

ifneq ($(filter ANDROID DRAGONFLY FREEBSD LINUX NETBSD OPENBSD HAIKU,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_intel
bridge_asm_objects := call
bridge_exception_objects := cpp2uno except uno2cpp
bridge_noncallexception_objects := callvirtualmethod
else ifeq ($(COM),MSC)
bridges_SELECTED_BRIDGE := msvc_win32_intel
bridge_exception_objects := cpp2uno uno2cpp
bridge_noopt_objects := except
else ifeq ($(OS),EMSCRIPTEN)
bridges_SELECTED_BRIDGE := gcc3_wasm
bridge_noopt_objects := cpp2uno except uno2cpp
endif

else ifeq ($(CPUNAME),M68K)

ifneq ($(filter DRAGONFLY FREEBSD LINUX NETBSD OPENBSD,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_m68k
bridge_noopt_objects := cpp2uno except uno2cpp
endif

else ifeq ($(CPUNAME),MIPS)

ifneq ($(filter LINUX,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_mips
bridge_noopt_objects := cpp2uno uno2cpp
bridge_exception_objects := except
endif

else ifeq ($(CPUNAME),MIPS64)

ifneq ($(filter LINUX,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_mips64
bridge_asm_objects := call
bridge_noopt_objects := cpp2uno uno2cpp
bridge_exception_objects := except
endif

else ifeq ($(CPUNAME),LOONGARCH64)

ifneq ($(filter LINUX,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_loongarch64
bridge_asm_objects := call
bridge_noopt_objects := abi cpp2uno uno2cpp
bridge_exception_objects := except
endif

else ifeq ($(CPUNAME),RISCV64)

ifneq ($(filter LINUX,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_riscv64
bridge_asm_objects := call
bridge_noopt_objects := abi cpp2uno uno2cpp
bridge_exception_objects := except
endif

else ifeq ($(CPUNAME),POWERPC)

ifneq ($(filter DRAGONFLY FREEBSD LINUX NETBSD OPENBSD,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_powerpc
bridge_noopt_objects := uno2cpp
bridge_exception_objects := cpp2uno except
endif

else ifeq ($(CPUNAME),POWERPC64)

ifneq ($(filter DRAGONFLY FREEBSD LINUX NETBSD OPENBSD,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_powerpc64
bridge_noopt_objects := cpp2uno uno2cpp
bridge_exception_objects := except
endif

else ifeq ($(CPUNAME),S390X)

ifneq ($(filter DRAGONFLY FREEBSD LINUX NETBSD OPENBSD,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_s390x
bridge_exception_objects := cpp2uno except uno2cpp
endif

else ifeq ($(CPUNAME),SPARC)

ifneq ($(filter DRAGONFLY FREEBSD LINUX NETBSD OPENBSD,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_sparc
bridge_asm_objects := call
bridge_noopt_objects := except
bridge_exception_objects := cpp2uno uno2cpp
endif

else ifeq ($(OS)-$(CPUNAME),LINUX-SPARC64)

bridges_SELECTED_BRIDGE := gcc3_linux_sparc64
bridge_asm_objects := call
bridge_noopt_objects := cpp2uno uno2cpp
bridge_exception_objects := except

else ifeq ($(CPUNAME),X86_64)

ifneq ($(filter ANDROID DRAGONFLY FREEBSD LINUX NETBSD OPENBSD HAIKU,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_linux_x86-64
bridge_asm_objects := call
bridge_noncallexception_noopt_objects := callvirtualmethod
bridge_exception_objects := abi cpp2uno except rtti uno2cpp
else ifneq ($(filter MACOSX iOS,$(OS)),)
bridges_SELECTED_BRIDGE := gcc3_macosx_x86-64
bridge_exception_objects := abi call cpp2uno except uno2cpp
bridge_noncallexception_noopt_objects := callvirtualmethod
else ifeq ($(COM),MSC)
bridges_SELECTED_BRIDGE := msvc_win32_x86-64
bridge_exception_objects := cpp2uno uno2cpp
bridge_noopt_objects := except
bridge_asm_objects := call
endif

endif

$(eval $(call gb_Library_use_internal_comprehensive_api,$(CPPU_ENV)_uno,\
	udkapi \
))

ifeq ($(OS),MACOSX)
ifeq ($(CPUNAME),AARCH64)
$(eval $(call gb_Library_use_internal_comprehensive_api,$(CPPU_ENV)_uno,\
	offapi \
))
endif
endif

$(eval $(call gb_Library_set_include,$(CPPU_ENV)_uno,\
	-I$(SRCDIR)/bridges/inc \
	$$(INCLUDE) \
))

ifeq ($(HAVE_POSIX_FALLOCATE),YES)
$(eval $(call gb_Library_add_defs,$(CPPU_ENV)_uno,\
	-DHAVE_POSIX_FALLOCATE \
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
	-fno-lto \
	$(if $(filter TRUE,$(HAVE_GCC_AVX)),-mno-avx) \
))

ifeq ($(filter ANDROID WNT DRAGONFLY FREEBSD NETBSD OPENBSD MACOSX iOS HAIKU,$(OS)),)
$(eval $(call gb_Library_add_libs,gcc3_uno,\
	-ldl \
))
endif
endif

ifeq ($(COM),GCC)
ifneq ($(COM_IS_CLANG),TRUE)
bridges_NON_CALL_EXCEPTIONS_FLAGS := -fnon-call-exceptions
endif
endif

$(eval $(call gb_Library_use_libraries,$(CPPU_ENV)_uno,\
	cppu \
	sal \
))

$(foreach obj,$(bridge_exception_objects),\
	$(eval $(call gb_Library_add_exception_objects,$(CPPU_ENV)_uno,\
	bridges/source/cpp_uno/$(bridges_SELECTED_BRIDGE)/$(obj))) \
)
$(foreach obj,$(bridge_noncallexception_objects),\
	$(eval $(call gb_Library_add_exception_objects,$(CPPU_ENV)_uno,\
	bridges/source/cpp_uno/$(bridges_SELECTED_BRIDGE)/$(obj) \
	, $(bridges_NON_CALL_EXCEPTIONS_FLAGS) )) \
)
$(foreach obj,$(bridge_noopt_objects),\
		$(eval $(call gb_Library_add_exception_objects,$(CPPU_ENV)_uno,\
				bridges/source/cpp_uno/$(bridges_SELECTED_BRIDGE)/$(obj) \
				, $(gb_COMPILERNOOPTFLAGS))) \
 )
$(foreach obj,$(bridge_noncallexception_noopt_objects),\
		$(eval $(call gb_Library_add_exception_objects,$(CPPU_ENV)_uno,\
				bridges/source/cpp_uno/$(bridges_SELECTED_BRIDGE)/$(obj) \
				, $(gb_COMPILERNOOPTFLAGS) $(bridges_NON_CALL_EXCEPTIONS_FLAGS) )) \
 )
$(foreach obj,$(bridge_cxx_objects),\
		$(eval $(call gb_Library_add_cxxobjects,$(CPPU_ENV)_uno,\
				bridges/source/cpp_uno/$(bridges_SELECTED_BRIDGE)/$(obj))) \
 )
$(foreach obj,$(bridge_asm_objects),\
		$(eval $(call gb_Library_add_asmobjects,$(CPPU_ENV)_uno,\
				bridges/source/cpp_uno/$(bridges_SELECTED_BRIDGE)/$(obj))) \
 )

$(eval $(call gb_Library_add_exception_objects,$(CPPU_ENV)_uno,\
			bridges/source/cpp_uno/shared/bridge \
			bridges/source/cpp_uno/shared/component \
			bridges/source/cpp_uno/shared/types \
			bridges/source/cpp_uno/shared/unointerfaceproxy \
			bridges/source/cpp_uno/shared/vtablefactory \
			bridges/source/cpp_uno/shared/vtables \
	))

ifeq ($(COM),MSC)
$(eval $(call gb_Library_add_exception_objects,$(CPPU_ENV)_uno,\
    bridges/source/cpp_uno/msvc_shared/cpp2uno \
    bridges/source/cpp_uno/msvc_shared/except \
))
endif

$(eval $(call gb_Library_add_cxxobjects,$(CPPU_ENV)_uno,\
			bridges/source/cpp_uno/shared/cppinterfaceproxy \
			, $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
	))

ifeq ($(DISABLE_DYNLOADING),TRUE)
$(eval $(call gb_Library_set_plugin_for_nodep,$(CPPU_ENV)_uno,cppu))
endif

# vim: set noet sw=4 ts=4:
