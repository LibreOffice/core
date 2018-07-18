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



GUI := UNX
#COM := S5ABI
COMID := s5abi

# Darwin mktemp -t expects a prefix, not a pattern
gb_MKTEMP ?= /usr/bin/mktemp -t gbuild.

gb_CC := cc
gb_CXX := g++
gb_GCCP := gcc
gb_AR := ar
gb_AWK := awk
gb_CLASSPATHSEP := :

# use CC/CXX if they are nondefaults
ifneq ($(origin CC),default)
gb_CC := $(CC)
gb_GCCP := $(CC)
endif
ifneq ($(origin CXX),default)
gb_CXX := $(CXX)
endif

gb_OSDEFS := \
	-D$(OS) \
	-D_PTHREADS \
	-DUNIX \
	-DUNX \
	-D_REENTRANT \
	-DNO_PTHREAD_PRIORITY \
	-DQUARTZ \
	$(EXTRA_CDEFS) \

gb_COMPILERDEFS := \
	-D$(COM) \
	-DHAVE_GCC_VISIBILITY_FEATURE \
	-DCPPU_ENV=$(COMNAME) \

gb_CPUDEFS := -D$(ALIGN) -D$(CPUNAME)
ifeq ($(CPUNAME),POWERPC)
gb_CPUDEFS += -DPOWERPC -DPPC
else ifeq ($(CPUNAME),INTEL)
gb_CPUDEFS += -DX86
endif

ifeq ($(strip $(SYSBASE)),)
	gb_SDKDIR := $(MACOSX_SDK_PATH)
else
	gb_SDKDIR := $(SYSBASE)/MacOSX$(MACOSX_DEPLOYMENT_TARGET).sdk
endif


gb_CFLAGS := \
	-isysroot $(gb_SDKDIR) \
	-Wall \
	-Wendif-labels \
	-Wextra \
	-Wshadow \
	-fPIC \
	-fmessage-length=0 \
	-fno-common \
	-fno-strict-aliasing \
	-pipe \

gb_CXXFLAGS := \
	-isysroot $(gb_SDKDIR) \
	-Wall \
	-Wendif-labels \
	-Wextra \
	-Wno-ctor-dtor-privacy \
	-Wno-non-virtual-dtor \
	-fPIC \
	-fmessage-length=0 \
	-fno-common \
	-fno-strict-aliasing \
	-fsigned-char \
	-pipe

ifneq ($(COM),GCC)
	gb_CXXFLAGS += -DHAVE_STL_INCLUDE_PATH -I../v1/
endif

ifeq ($(MACOSX_DEPLOYMENT_TARGET),$(filter $(MACOSX_DEPLOYMENT_TARGET), 10.7 10.8))
	gb_CXXFLAGS += -std=c++11 -stdlib=libc++
	gb_macos_LDFLAGS := -std=c++11 -stdlib=libc++
endif

# these are to get g++ to switch to Objective-C++ mode
# (see toolkit module for a case where it is necessary to do it this way)
gb_OBJCXXFLAGS := -x objective-c++ -fobjc-exceptions

ifneq ($(MACOSX_DEPLOYMENT_TARGET),)
	gb_CXXFLAGS += -DMAC_OS_X_VERSION_MAX_ALLOWED=MAC_OS_X_VERSION_$(subst .,_,$(MACOSX_DEPLOYMENT_TARGET))
endif

ifneq ($(EXTERNAL_WARNINGS_NOT_ERRORS),TRUE)
gb_CFLAGS_WERROR := -Werror -Wno-error=deprecated
gb_CXXFLAGS_WERROR := -Werror -Wno-error=deprecated
endif

gb_LinkTarget_EXCEPTIONFLAGS := \
	-DEXCEPTIONS_ON \
	-fexceptions

ifeq ($(COM),GCC)
    gb_LinkTarget_EXCEPTIONFLAGS += -fno-enforce-eh-specs
endif

gb_LinkTarget_NOEXCEPTIONFLAGS := \
	-DEXCEPTIONS_OFF \
	-fno-exceptions

gb_LinkTarget_LDFLAGS := \
	-Wl,-syslibroot,$(gb_SDKDIR) \
	$(subst -L../lib , ,$(SOLARLIB)) \
#man ld says: obsolete	-Wl,-multiply_defined,suppress \

ifeq ($(ENABLE_SYMBOLS),SMALL)
gb_DEBUG_CFLAGS := -g1
else
gb_DEBUG_CFLAGS := -g3
endif

ifeq ($(gb_DEBUGLEVEL),2)
gb_COMPILEROPTFLAGS := -O0
gb_COMPILEROPT1FLAGS := -O0
else
gb_COMPILEROPTFLAGS := -O2
gb_COMPILEROPT1FLAGS := -O1
endif

gb_COMPILERNOOPTFLAGS := -O0

# Helper class

gb_Helper_abbreviate_dirs_native = $(gb_Helper_abbreviate_dirs)

# convert parametters filesystem root to native notation
# does some real work only on windows, make sure not to
# break the dummy implementations on unx*
define gb_Helper_convert_native
$(1)
endef

# convert parametters filesystem root to native notation
# does some real work only on windows, make sure not to
# break the dummy implementations on unx*
define gb_Helper_convert_native
$(1)
endef


# CObject class

define gb_CObject__command
$(call gb_Output_announce,$(2),$(true),C  ,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && \
	$(gb_CC) \
		$(DEFS) \
		$(T_CFLAGS) \
		$(CFLAGS) \
		-c $(3) \
		-o $(1) \
		-MMD -MT $(1) \
		-MF $(4) \
		-I$(dir $(3)) \
		$(INCLUDE))
endef


# CxxObject class

# N.B: $(CXXFLAGS) may contain -x objective-c++, which must come before -c
define gb_CxxObject__command
$(call gb_Output_announce,$(2),$(true),CXX,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && \
	$(gb_CXX) \
		$(DEFS) \
		$(T_CXXFLAGS) \
		$(CXXFLAGS) \
		-c $(3) \
		-o $(1) \
		-MMD -MT $(1) \
		-MF $(4) \
		-I$(dir $(3)) \
		$(INCLUDE_STL) $(INCLUDE))
endef


# ObjCxxObject class

define gb_ObjCxxObject__command
$(call gb_Output_announce,$(2),$(true),OCX,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && \
	$(gb_CXX) \
		$(DEFS) \
		$(T_OBJCXXFLAGS) \
		$(OBJCXXFLAGS) \
		-c $(3) \
		-o $(1) \
		-MMD -MT $(1) \
		-MF $(4) \
		-I$(dir $(3)) \
		$(INCLUDE_STL) $(INCLUDE))
endef


# LinkTarget class

define gb_LinkTarget__get_rpath_for_layer
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_LinkTarget__RPATHS)))
endef

gb_LinkTarget__RPATHS := \
	URELIB:@__________________________________________________URELIB/ \
	UREBIN: \
	OOO:@__________________________________________________OOO/ \
	BRAND: \
	SDKBIN: \
	NONEBIN: \

define gb_LinkTarget__get_installname
$(if $(2),-install_name '$(2)$(1)',)
endef

gb_LinkTarget_CFLAGS := $(gb_CFLAGS) $(gb_CFLAGS_WERROR)
gb_LinkTarget_CXXFLAGS := $(gb_CXXFLAGS) $(gb_CXXFLAGS_WERROR)
gb_LinkTarget_OBJCXXFLAGS := $(gb_CXXFLAGS) $(gb_CXXFLAGS_WERROR) $(gb_OBJCXXFLAGS)

gb_LinkTarget_INCLUDE := $(filter-out %/stl, $(subst -I. , ,$(SOLARINC)))
gb_LinkTarget_INCLUDE_STL := $(filter %/stl, $(subst -I. , ,$(SOLARINC)))

# FIXME framework handling very hackish
define gb_LinkTarget__get_liblinkflags
$(patsubst lib%.dylib,-l%,$(foreach lib,$(filter-out $(gb_Library__FRAMEWORKS),$(1)),$(call gb_Library_get_filename,$(lib)))) \
$(addprefix -framework ,$(filter $(gb_Library__FRAMEWORKS),$(1)))
endef

define gb_LinkTarget__get_layer
$(if $(filter Executable,$(1)),\
	$$(call gb_Executable_get_layer,$(2)),\
	$$(call gb_Library_get_layer,$(2)))
endef

# FIXME the DYLIB_FILE mess is only necessary because
# solver layout is different from installation layout
define gb_LinkTarget__command_dynamiclink
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	DYLIB_FILE=`$(gb_MKTEMP)` && \
	$(PERL) $(SOLARENV)/bin/macosx-dylib-link-list.pl \
		$(if $(filter Executable,$(TARGETTYPE)),$(gb_Executable_TARGETTYPEFLAGS)) \
		$(if $(filter Library,$(TARGETTYPE)),$(gb_Library_TARGETTYPEFLAGS)) \
		$(subst \d,$$,$(RPATH)) \
		$(T_LDFLAGS) \
		$(patsubst lib%.dylib,-l%,$(foreach lib,$(LINKED_LIBS),$(call gb_Library_get_filename,$(lib)))) \
		$(patsubst %,-l%,$(EXTERNAL_LIBS)) \
		> $${DYLIB_FILE} && \
	$(gb_CXX) \
		$(if $(filter Executable,$(TARGETTYPE)),$(gb_Executable_TARGETTYPEFLAGS)) \
		$(if $(filter Library,$(TARGETTYPE)),$(gb_Library_TARGETTYPEFLAGS)) \
		$(subst \d,$$,$(RPATH)) \
		$(T_LDFLAGS) $(gb_macos_LDFLAGS) \
		$(call gb_LinkTarget__get_liblinkflags,$(LINKED_LIBS)) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(OBJCXXOBJECTS),$(call gb_ObjCxxObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach lib,$(LINKED_STATIC_LIBS),$(call gb_StaticLibrary_get_target,$(lib))) \
		$(LIBS) \
		-o $(if $(call gb_Library_is_udk_versioned,$(1)),$(1).$(gb_UDK_MAJOR),$(1)) \
		`cat $${DYLIB_FILE}` && \
	$(if $(filter Library,$(TARGETTYPE)),\
		$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl Library $(LAYER) $(1) && \
		ln -f $(if $(call gb_Library_is_udk_versioned,$(1)),$(1).$(gb_UDK_MAJOR),$(1)) \
		      $(patsubst %.dylib,%.jnilib,$(1)) &&) \
	rm -f $${DYLIB_FILE})
endef

define gb_LinkTarget__command_symlink_udk_versioned_library
	$(if $(call gb_Library_is_udk_versioned,$(1)),
		$(call gb_Helper_abbreviate_dirs,\
			rm -f $(1) && ln -s $(notdir $(1)).$(gb_UDK_MAJOR) $(1)))
endef

# parameters: 1-linktarget 2-cobjects 3-cxxobjects
define gb_LinkTarget__command_staticlink
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(gb_AR) -rsu $(1) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(OBJCXXOBJECTS),$(call gb_ObjCxxObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		2> /dev/null)
endef

define gb_LinkTarget__command
$(call gb_Output_announce,$(2),$(true),LNK,4)
$(if $(filter Library GoogleTest Executable,$(TARGETTYPE)),$(call gb_LinkTarget__command_dynamiclink,$(1),$(2)))
$(if $(filter Library,$(TARGETTYPE)),$(call gb_LinkTarget__command_symlink_udk_versioned_library,$(1)))
$(if $(filter StaticLibrary,$(TARGETTYPE)),$(call gb_LinkTarget__command_staticlink,$(1)))
endef


# Library class

gb_Library_DEFS :=
gb_Library_TARGETTYPEFLAGS := -dynamiclib -single_module
gb_Library_SYSPRE := lib
gb_Library_UNOVERPRE := $(gb_Library_SYSPRE)uno_
gb_Library_PLAINEXT := .dylib
gb_Library_RTEXT := $(COMID)$(gb_Library_PLAINEXT)

gb_Library_OOOEXT := $(gb_Library_PLAINEXT)
gb_Library_UNOEXT := .uno$(gb_Library_PLAINEXT)

gb_Library__FRAMEWORKS := \
	Cocoa \

gb_Library_PLAINLIBS_NONE += \
	Cocoa \
	objc \
	m \
	pthread

gb_Library_FILENAMES := \
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_OOOEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_RTLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_RTEXT)) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_RTEXT)) \
	$(foreach lib,$(gb_Library_STLLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_STLEXT)) \
	$(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):$(lib)$(gb_Library_UNOEXT)) \
	$(foreach lib,$(gb_Library_UNOLIBS_OOO),$(lib):$(lib)$(gb_Library_UNOEXT)) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(gb_Library_UNOVERPRE)$(lib)$(gb_Library_PLAINEXT)) \


gb_Library_LAYER := \
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):OOO) \
	$(foreach lib,$(gb_Library_RTLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_STLLIBS),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_UNOLIBS_OOO),$(lib):OOO) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):URELIB) \

define gb_Library_get_rpath
$(call gb_LinkTarget__get_installname,$(call gb_Library_get_filename,$(1)),$(call gb_LinkTarget__get_rpath_for_layer,$(call gb_Library_get_layer,$(1))))
endef

define gb_Library_Library_platform
$(call gb_LinkTarget_get_target,$(2)) : RPATH := $(call gb_Library_get_rpath,$(1))
$(call gb_LinkTarget_get_target,$(2)) : LAYER := $(call gb_Library_get_layer,$(1))
ifneq (,$(call gb_Library_is_udk_versioned,$(call gb_Library_get_target,$(1))))
$(call gb_Library_get_target,$(1)) \
$(call gb_Library_get_clean_target,$(1)) : AUXTARGETS +=  \
	$(call gb_Library_get_target,$(1)).$(gb_UDK_MAJOR)
endif

endef


# StaticLibrary class

gb_StaticLibrary_DEFS :=
gb_StaticLibrary_SYSPRE := lib
gb_StaticLibrary_PLAINEXT := .a
gb_StaticLibrary_JPEGEXT := lib$(gb_StaticLibrary_PLAINEXT)

gb_StaticLibrary_FILENAMES := \
	$(foreach lib,$(gb_StaticLibrary_JPEGLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_JPEGEXT)) \
	$(foreach lib,$(gb_StaticLibrary_PLAINLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_PLAINEXT)) \

gb_StaticLibrary_StaticLibrary_platform =


# Executable class

gb_Executable_EXT :=
gb_Executable_TARGETTYPEFLAGS := -bind_at_load

gb_Executable_LAYER := \
	$(foreach exe,$(gb_Executable_UREBIN),$(exe):UREBIN) \
	$(foreach exe,$(gb_Executable_SDK),$(exe):SDKBIN) \
	$(foreach exe,$(gb_Executable_OOO),$(exe):OOO) \
	$(foreach exe,$(gb_Executable_BRAND),$(exe):BRAND) \
	$(foreach exe,$(gb_Executable_NONE),$(exe):NONEBIN) \


define gb_Executable_get_rpath
$(call gb_LinkTarget__get_installname,$(1),$(call gb_LinkTarget__get_rpath_for_layer,$(call gb_Executable_get_layer,$(1))))
endef

define gb_Executable_Executable_platform
$(call gb_LinkTarget_get_target,$(2)) : RPATH := $(call gb_Executable_get_rpath,$(1))
$(call gb_LinkTarget_get_target,$(2)) : LAYER := $(call gb_Executable_get_layer,$(1))

endef


# GoogleTest class

gb_GoogleTest_GTESTPRECOMMAND := DYLD_LIBRARY_PATH=$(OUTDIR)/lib
gb_GoogleTest_get_filename = $(1)$(gb_Executable_EXT)

define gb_GoogleTest_GoogleTest_platform
$(call gb_LinkTarget_get_target,$(2)) : RPATH :=
endef

# JunitTest class

define gb_JunitTest_JunitTest_platform
$(call gb_JunitTest_get_target,$(1)) : DEFS := \
	-Dorg.openoffice.test.arg.soffice="$$$${OOO_TEST_SOFFICE:-path:$(SRCDIR)/instsetoo_native/$(INPATH)/Apache_OpenOffice/installed/install/en-US/openoffice4/program/soffice}" \
    -Dorg.openoffice.test.arg.env=DYLD_LIBRARY_PATH \
    -Dorg.openoffice.test.arg.user=file://$(call gb_JunitTest_get_userdir,$(1)) \

endef

# SdiTarget class

gb_SdiTarget_SVIDLPRECOMMAND := DYLD_LIBRARY_PATH=$(OUTDIR)/lib

# SrsPartMergeTarget

gb_SrsPartMergeTarget_TRANSEXPRECOMMAND := DYLD_LIBRARY_PATH=$(OUTDIR)/lib

# SrsPartTarget class

gb_SrsPartTarget_RSCTARGET := $(OUTDIR)/bin/rsc
gb_SrsPartTarget_RSCCOMMAND := DYLD_LIBRARY_PATH=$(OUTDIR)/lib SOLARBINDIR=$(OUTDIR)/bin $(gb_SrsPartTarget_RSCTARGET)

define gb_SrsPartTarget__command_dep
$(call gb_Helper_abbreviate_dirs,\
	$(gb_GCCP) \
		-MM -MT $(call gb_SrsPartTarget_get_target,$(1)) \
		$(INCLUDE) \
		$(DEFS) \
		-c -x c++-header $(2) \
		-o $(call gb_SrsPartTarget_get_dep_target,$(1)))
endef


# ComponentTarget

gb_XSLTPROCPRECOMMAND := DYLD_LIBRARY_PATH=$(OUTDIR)/lib
gb_Library_COMPONENTPREFIXES := \
    OOO:vnd.sun.star.expand:\dOOO_BASE_DIR/program/ \
    URELIB:vnd.sun.star.expand:\dURE_INTERNAL_LIB_DIR/ \
    NONE:vnd.sun.star.expand:\dOOO_INBUILD_SHAREDLIB_DIR/ \

# UnoApiTarget

gb_UnoApiTarget_IDLCTARGET := $(OUTDIR)/bin/idlc
gb_UnoApiTarget_IDLCCOMMAND := DYLD_LIBRARY_PATH=$(OUTDIR)/lib SOLARBINDIR=$(OUTDIR)/bin $(gb_UnoApiTarget_IDLCTARGET)
gb_UnoApiTarget_REGMERGETARGET := $(OUTDIR)/bin/regmerge
gb_UnoApiTarget_REGMERGECOMMAND := DYLD_LIBRARY_PATH=$(OUTDIR)/lib SOLARBINDIR=$(OUTDIR)/bin $(gb_UnoApiTarget_REGMERGETARGET)
gb_UnoApiTarget_REGCOMPARETARGET := $(OUTDIR)/bin/regcompare
gb_UnoApiTarget_REGCOMPARECOMMAND := DYLD_LIBRARY_PATH=$(OUTDIR)/lib SOLARBINDIR=$(OUTDIR)/bin $(gb_UnoApiTarget_REGCOMPARETARGET)
gb_UnoApiTarget_CPPUMAKERTARGET := $(OUTDIR)/bin/cppumaker
gb_UnoApiTarget_CPPUMAKERCOMMAND := DYLD_LIBRARY_PATH=$(OUTDIR)/lib SOLARBINDIR=$(OUTDIR)/bin $(gb_UnoApiTarget_CPPUMAKERTARGET)
gb_UnoApiTarget_REGVIEWTARGET := $(OUTDIR)/bin/regview
gb_UnoApiTarget_REGVIEWCOMMAND := DYLD_LIBRARY_PATH=$(OUTDIR)/lib SOLARBINDIR=$(OUTDIR)/bin $(gb_UnoApiTarget_REGVIEWTARGET)


# vim: set noet sw=4 ts=4:
