#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



$(eval $(call gb_Library_Library,$(COMNAME)_uno))

$(eval $(call gb_Library_add_precompiled_header,$(COMNAME)_uno,$(SRCDIR)/bridges/inc/pch/precompiled_bridges))

$(eval $(call gb_Library_set_include,$(COMNAME)_uno,\
	$$(INCLUDE) \
	-I$(SRCDIR) \
	-I$(SRCDIR)/bridges/inc \
	-I$(SRCDIR)/bridges/inc/pch \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_set_private_extract_of_public_api,$(COMNAME)_uno,$(OUTDIR)/bin/udkapi.rdb,\
	com.sun.star.uno.XInterface \
	com.sun.star.uno.TypeClass \
))

#$(eval $(call gb_Library_add_api,$(COMNAME)_uno, \
#        udkapi \
#	offapi \
#))

$(eval $(call gb_Library_set_versionmap,$(COMNAME)_uno,$(SRCDIR)/bridges/source/bridge_exports.map))

$(eval $(call gb_Library_add_linked_libs,$(COMNAME)_uno,\
	cppu \
	sal \
	stl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/shared/bridge \
	bridges/source/cpp_uno/shared/component \
	bridges/source/cpp_uno/shared/cppinterfaceproxy \
	bridges/source/cpp_uno/shared/types \
	bridges/source/cpp_uno/shared/unointerfaceproxy \
	bridges/source/cpp_uno/shared/vtablefactory \
	bridges/source/cpp_uno/shared/vtables \
))

# Disable optimization for cppinterfaceproxy.cxx -
# attribute constructor / destructor do not get called otherwise.
ifeq ($(COM),GCC)
$(eval $(call gb_LinkTarget_set_cxx_optimization, \
	bridges/source/cpp_uno/shared/cppinterfaceproxy, $(gb_COMPILERNOOPTFLAGS) \
))
endif


###################################################
ifeq ($(OS)-$(CPUNAME)-$(COMNAME),FREEBSD-ARM-gcc3)
###################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_freebsd_arm/except \
	bridges/source/cpp_uno/gcc3_freebsd_arm/cpp2uno \
	bridges/source/cpp_uno/gcc3_freebsd_arm/uno2cpp \
))

$(eval $(call gb_LinkTarget_set_cxx_optimization, \
	bridges/source/cpp_uno/gcc3_freebsd_arm/except \
	bridges/source/cpp_uno/gcc3_freebsd_arm/cpp2uno \
	bridges/source/cpp_uno/gcc3_freebsd_arm/uno2cpp \
, $(gb_COMPILERNOOPTFLAGS) \
))

$(eval $(call gb_Library_add_asmobjects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_freebsd_arm/armhelper \
))

##########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),FREEBSD-INTEL-gcc3)
##########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_freebsd_intel/except \
	bridges/source/cpp_uno/gcc3_freebsd_intel/cpp2uno \
	bridges/source/cpp_uno/gcc3_freebsd_intel/uno2cpp \
))

$(eval $(call gb_Library_add_asmobjects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_freebsd_intel/call \
))

############################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),FREEBSD-POWERPC-gcc3)
############################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
        bridges/source/cpp_uno/gcc3_freebsd_powerpc/except \
        bridges/source/cpp_uno/gcc3_freebsd_powerpc/cpp2uno \
        bridges/source/cpp_uno/gcc3_freebsd_powerpc/uno2cpp \
))

$(eval $(call gb_LinkTarget_set_cxx_optimization, \
        bridges/source/cpp_uno/gcc3_freebsd_powerpc/uno2cpp \
, $(gb_COMPILERNOOPTFLAGS) \
))

##############################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),FREEBSD-POWERPC64-gcc3)
##############################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
        bridges/source/cpp_uno/gcc3_freebsd_powerpc64/except \
        bridges/source/cpp_uno/gcc3_freebsd_powerpc64/cpp2uno \
        bridges/source/cpp_uno/gcc3_freebsd_powerpc64/uno2cpp \
))

$(eval $(call gb_LinkTarget_set_cxx_optimization, \
        bridges/source/cpp_uno/gcc3_freebsd_powerpc64/cpp2uno \
        bridges/source/cpp_uno/gcc3_freebsd_powerpc64/uno2cpp \
, $(gb_COMPILERNOOPTFLAGS) \
))

###########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),FREEBSD-X86_64-gcc3)
###########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_freebsd_x86-64/abi \
	bridges/source/cpp_uno/gcc3_freebsd_x86-64/except \
	bridges/source/cpp_uno/gcc3_freebsd_x86-64/cpp2uno \
	bridges/source/cpp_uno/gcc3_freebsd_x86-64/uno2cpp \
))

$(eval $(call gb_Library_add_asmobjects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_freebsd_x86-64/call \
))

#########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),LINUX-ALPHA_-gcc3)
#########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
        bridges/source/cpp_uno/gcc3_linux_alpha/except \
        bridges/source/cpp_uno/gcc3_linux_alpha/cpp2uno \
        bridges/source/cpp_uno/gcc3_linux_alpha/uno2cpp \
))

$(eval $(call gb_Library_add_libs,$(COMNAME)_uno,\
	-ldl \
))

######################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),LINUX-ARM-gcc3)
######################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_linux_arm/except \
	bridges/source/cpp_uno/gcc3_linux_arm/cpp2uno \
	bridges/source/cpp_uno/gcc3_linux_arm/uno2cpp \
))

$(eval $(call gb_LinkTarget_set_cxx_optimization, \
	bridges/source/cpp_uno/gcc3_linux_arm/except \
	bridges/source/cpp_uno/gcc3_linux_arm/cpp2uno \
	bridges/source/cpp_uno/gcc3_linux_arm/uno2cpp \
, $(gb_COMPILERNOOPTFLAGS) \
))

$(eval $(call gb_Library_add_asmobjects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_linux_arm/armhelper \
))

$(eval $(call gb_Library_add_libs,$(COMNAME)_uno,\
	-ldl \
))

#######################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),LINUX-HPPA-gcc3)
#######################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_linux_hppa/call \
	bridges/source/cpp_uno/gcc3_linux_hppa/except \
	bridges/source/cpp_uno/gcc3_linux_hppa/cpp2uno \
	bridges/source/cpp_uno/gcc3_linux_hppa/uno2cpp \
))

$(eval $(call gb_LinkTarget_set_cxx_optimization, \
	bridges/source/cpp_uno/gcc3_linux_hppa/call \
	bridges/source/cpp_uno/gcc3_linux_hppa/except \
	bridges/source/cpp_uno/gcc3_linux_hppa/cpp2uno \
	bridges/source/cpp_uno/gcc3_linux_hppa/uno2cpp \
, $(gb_COMPILERNOOPTFLAGS) \
))

$(eval $(call gb_Library_add_libs,$(COMNAME)_uno,\
	-ldl \
))

#######################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),LINUX-IA64-gcc3)
#######################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_linux_ia64/except \
	bridges/source/cpp_uno/gcc3_linux_ia64/cpp2uno \
	bridges/source/cpp_uno/gcc3_linux_ia64/uno2cpp \
))

$(eval $(call gb_LinkTarget_set_cxx_optimization, \
	bridges/source/cpp_uno/gcc3_linux_ia64/cpp2uno \
	bridges/source/cpp_uno/gcc3_linux_ia64/uno2cpp \
, $(gb_COMPILERNOOPTFLAGS) \
))

$(eval $(call gb_Library_add_asmobjects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_linux_ia64/call \
))

$(eval $(call gb_Library_add_libs,$(COMNAME)_uno,\
	-ldl \
))

########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),LINUX-INTEL-gcc3)
########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_linux_intel/abi \
	bridges/source/cpp_uno/gcc3_linux_intel/except \
	bridges/source/cpp_uno/gcc3_linux_intel/cpp2uno \
	bridges/source/cpp_uno/gcc3_linux_intel/uno2cpp \
))

$(eval $(call gb_Library_add_asmobjects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_linux_intel/call \
))


$(eval $(call gb_Library_add_libs,$(COMNAME)_uno,\
	-ldl \
))

#######################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),LINUX-M68K-gcc3)
#######################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_linux_m68k/except \
	bridges/source/cpp_uno/gcc3_linux_m68k/cpp2uno \
	bridges/source/cpp_uno/gcc3_linux_m68k/uno2cpp \
))

$(eval $(call gb_LinkTarget_set_cxx_optimization, \
	bridges/source/cpp_uno/gcc3_linux_m68k/except \
	bridges/source/cpp_uno/gcc3_linux_m68k/cpp2uno \
	bridges/source/cpp_uno/gcc3_linux_m68k/uno2cpp \
, $(gb_COMPILERNOOPTFLAGS) \
))


$(eval $(call gb_Library_add_libs,$(COMNAME)_uno,\
	-ldl \
))

#########################################################
# It's a MIPS, apparently...
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),LINUX-GODSON-gcc3)
#########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_linux_mips/except \
	bridges/source/cpp_uno/gcc3_linux_mips/cpp2uno \
	bridges/source/cpp_uno/gcc3_linux_mips/uno2cpp \
))

$(eval $(call gb_LinkTarget_set_cxx_optimization, \
	bridges/source/cpp_uno/gcc3_linux_mips/cpp2uno \
	bridges/source/cpp_uno/gcc3_linux_mips/uno2cpp \
, $(gb_COMPILERNOOPTFLAGS) \
))


$(eval $(call gb_Library_add_libs,$(COMNAME)_uno,\
	-ldl \
))

##########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),LINUX-POWERPC-gcc3)
##########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_linux_powerpc/except \
	bridges/source/cpp_uno/gcc3_linux_powerpc/cpp2uno \
	bridges/source/cpp_uno/gcc3_linux_powerpc/uno2cpp \
))

$(eval $(call gb_LinkTarget_set_cxx_optimization, \
	bridges/source/cpp_uno/gcc3_linux_powerpc/uno2cpp \
, $(gb_COMPILERNOOPTFLAGS) \
))


$(eval $(call gb_Library_add_libs,$(COMNAME)_uno,\
	-ldl \
))

############################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),LINUX-POWERPC64-gcc3)
############################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_linux_powerpc64/except \
	bridges/source/cpp_uno/gcc3_linux_powerpc64/cpp2uno \
	bridges/source/cpp_uno/gcc3_linux_powerpc64/uno2cpp \
))

$(eval $(call gb_LinkTarget_set_cxx_optimization, \
	bridges/source/cpp_uno/gcc3_linux_powerpc64/cpp2uno \
	bridges/source/cpp_uno/gcc3_linux_powerpc64/uno2cpp \
, $(gb_COMPILERNOOPTFLAGS) \
))


$(eval $(call gb_Library_add_libs,$(COMNAME)_uno,\
	-ldl \
))

#######################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),LINUX-S390-gcc3)
#######################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_linux_s390/except \
	bridges/source/cpp_uno/gcc3_linux_s390/cpp2uno \
	bridges/source/cpp_uno/gcc3_linux_s390/uno2cpp \
))


$(eval $(call gb_Library_add_libs,$(COMNAME)_uno,\
	-ldl \
))

########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),LINUX-S390X-gcc3)
########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_linux_s390x/except \
	bridges/source/cpp_uno/gcc3_linux_s390x/cpp2uno \
	bridges/source/cpp_uno/gcc3_linux_s390x/uno2cpp \
))


$(eval $(call gb_Library_add_libs,$(COMNAME)_uno,\
	-ldl \
))

########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),LINUX-SPARC-gcc3)
########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_linux_sparc/except \
	bridges/source/cpp_uno/gcc3_linux_sparc/cpp2uno \
	bridges/source/cpp_uno/gcc3_linux_sparc/uno2cpp \
))

$(eval $(call gb_LinkTarget_set_cxx_optimization, \
	bridges/source/cpp_uno/gcc3_linux_sparc/cpp2uno \
	bridges/source/cpp_uno/gcc3_linux_sparc/uno2cpp \
, $(gb_COMPILERNOOPTFLAGS) \
))

$(eval $(call gb_Library_add_asmobjects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_linux_sparc/call \
))

$(eval $(call gb_Library_add_libs,$(COMNAME)_uno,\
	-ldl \
))

#########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),LINUX-X86_64-gcc3)
#########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_linux_x86-64/abi \
	bridges/source/cpp_uno/gcc3_linux_x86-64/except \
	bridges/source/cpp_uno/gcc3_linux_x86-64/cpp2uno \
	bridges/source/cpp_uno/gcc3_linux_x86-64/uno2cpp \
))

$(eval $(call gb_Library_add_asmobjects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_linux_x86-64/call \
))

$(eval $(call gb_Library_add_libs,$(COMNAME)_uno,\
	-ldl \
))

#########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),MACOSX-INTEL-gcc3)
#########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_macosx_intel/except \
	bridges/source/cpp_uno/gcc3_macosx_intel/cpp2uno \
	bridges/source/cpp_uno/gcc3_macosx_intel/uno2cpp \
))

$(eval $(call gb_Library_add_asmobjects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_macosx_intel/call \
))

###########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),MACOSX-POWERPC-gcc3)
###########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_macosx_powerpc/except \
	bridges/source/cpp_uno/gcc3_macosx_powerpc/cpp2uno \
	bridges/source/cpp_uno/gcc3_macosx_powerpc/uno2cpp \
))

$(eval $(call gb_LinkTarget_set_cxx_optimization, \
	bridges/source/cpp_uno/gcc3_macosx_powerpc/uno2cpp \
, $(gb_COMPILERNOOPTFLAGS) \
))

$(eval $(call gb_Library_add_linked_libs,$(COMNAME)_uno,\
        dl \
))

###########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),MACOSX-X86_64-s5abi)
###########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/s5abi_macosx_x86_64/abi \
	bridges/source/cpp_uno/s5abi_macosx_x86_64/except \
	bridges/source/cpp_uno/s5abi_macosx_x86_64/cpp2uno \
	bridges/source/cpp_uno/s5abi_macosx_x86_64/uno2cpp \
))

#########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),NETBSD-INTEL-gcc3)
#########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_netbsd_intel/except \
	bridges/source/cpp_uno/gcc3_netbsd_intel/cpp2uno \
	bridges/source/cpp_uno/gcc3_netbsd_intel/uno2cpp \
))

#########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),OS2-INTEL-gcc3)
#########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_os2_intel/except \
	bridges/source/cpp_uno/gcc3_os2_intel/cpp2uno \
	bridges/source/cpp_uno/gcc3_os2_intel/uno2cpp \
))

#########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COM),SOLARIS-INTEL-C50)
#########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/cc50_solaris_intel/except \
	bridges/source/cpp_uno/cc50_solaris_intel/cpp2uno \
	bridges/source/cpp_uno/cc50_solaris_intel/uno2cpp \
))

$(eval $(call gb_Library_add_asmobjects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/cc50_solaris_intel/call \
))

#########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COM),SOLARIS-INTEL-C52)
#########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/cc50_solaris_intel/except \
	bridges/source/cpp_uno/cc50_solaris_intel/cpp2uno \
	bridges/source/cpp_uno/cc50_solaris_intel/uno2cpp \
))

$(eval $(call gb_Library_add_asmobjects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/cc50_solaris_intel/call \
))

##########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),SOLARIS-INTEL-gcc3)
##########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_solaris_intel/except \
	bridges/source/cpp_uno/gcc3_solaris_intel/cpp2uno \
	bridges/source/cpp_uno/gcc3_solaris_intel/uno2cpp \
))

#########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COM),SOLARIS-SPARC-C52)
#########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/cc50_solaris_sparc/except \
	bridges/source/cpp_uno/cc50_solaris_sparc/cpp2uno \
	bridges/source/cpp_uno/cc50_solaris_sparc/uno2cpp \
))

$(eval $(call gb_Library_add_asmobjects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/cc50_solaris_sparc/call \
))

##########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),SOLARIS-SPARC-gcc3)
##########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/gcc3_solaris_sparc/except \
	bridges/source/cpp_uno/gcc3_solaris_sparc/cpp2uno \
	bridges/source/cpp_uno/gcc3_solaris_sparc/uno2cpp \
))

$(eval $(call gb_LinkTarget_set_cxx_optimization, \
	bridges/source/cpp_uno/gcc3_solaris_sparc/cpp2uno \
	bridges/source/cpp_uno/gcc3_solaris_sparc/uno2cpp \
, $(gb_COMPILERNOOPTFLAGS) \
))

#########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COM),SOLARIS-SPARC64-C52)
#########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/cc5_solaris_sparc64/cpp2uno \
	bridges/source/cpp_uno/cc5_solaris_sparc64/exceptions \
	bridges/source/cpp_uno/cc5_solaris_sparc64/isdirectreturntype \
	bridges/source/cpp_uno/cc5_solaris_sparc64/uno2cpp \
))

$(eval $(call gb_Library_add_asmobjects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/cc5_solaris_sparc64/callvirtualmethod \
	bridges/source/cpp_uno/cc5_solaris_sparc64/fp \
	bridges/source/cpp_uno/cc5_solaris_sparc64/vtableslotcall \
))

#########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),WNT-INTEL-msci)
#########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/msvc_win32_intel/cpp2uno \
	bridges/source/cpp_uno/msvc_win32_intel/dllinit \
	bridges/source/cpp_uno/msvc_win32_intel/except \
	bridges/source/cpp_uno/msvc_win32_intel/uno2cpp \
))

$(eval $(call gb_LinkTarget_set_cxx_optimization, \
	bridges/source/cpp_uno/msvc_win32_intel/except \
, $(gb_COMPILERNOOPTFLAGS) \
))

#########################################################
else ifeq ($(OS)-$(CPUNAME)-$(COMNAME),WNT-INTEL-gcc3)
#########################################################

$(eval $(call gb_Library_add_exception_objects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/mingw_intel/cpp2uno \
	bridges/source/cpp_uno/mingw_intel/dllinit \
	bridges/source/cpp_uno/mingw_intel/except \
	bridges/source/cpp_uno/mingw_intel/smallstruct \
	bridges/source/cpp_uno/mingw_intel/uno2cpp \
))

$(eval $(call gb_LinkTarget_set_cxx_optimization, \
	bridges/source/cpp_uno/msvc_win32_intel/uno2cpp \
, $(gb_COMPILERNOOPTFLAGS) \
))

$(eval $(call gb_Library_add_asmobjects,$(COMNAME)_uno,\
	bridges/source/cpp_uno/mingw_intel/call \
))

ifeq ($(EXCEPTIONS),sjlj)
$(eval $(call gb_Library_add_defs,$(COMNAME)_uno,\
	-DBROKEN_ALLOCA \
))
endif

#####################################
else
#####################################

$(call gb_Output_error,Unsupported OS-platform-ABI $(OS)-$(CPUNAME)-$(COMNAME) - please add a bridge under main/bridges/source/cpp_uno)

#####################################
endif
#####################################


# vim: set noet sw=4 ts=4:

