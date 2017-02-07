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

gb_MKTEMP := mktemp -t gbuild.XXXXXX

gb_CC := gcc
gb_CXX := g++
gb_GCCP := gcc
gb_AR := ar
gb_AWK := awk
gb_CLASSPATHSEP := :

# normalize setsolar and configure env.
ifeq ($(CPU),X)
CPUNAME := X86_64
endif

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
	$(PTHREAD_CFLAGS) \

ifeq ($(GXX_INCLUDE_PATH),)
GXX_INCLUDE_PATH=$(COMPATH)/include/c++/$(shell gcc -dumpversion)
endif

gb_COMPILERDEFS := \
	-D$(COM) \
	-DHAVE_GCC_VISIBILITY_FEATURE \
	-DCPPU_ENV=gcc3 \
	-DGXX_INCLUDE_PATH=$(GXX_INCLUDE_PATH) \

ifeq ($(CPUNAME),X86_64)
gb_CPUDEFS := -D$(CPUNAME)
else
gb_CPUDEFS := -DX86
endif

gb_CFLAGS := \
	-Wall \
	-Wendif-labels \
	-Wextra \
	-Wshadow \
	-fPIC \
	-fmessage-length=0 \
	-fno-common \
	-fno-strict-aliasing \
	-fvisibility=hidden \
	-pipe \

gb_CXXFLAGS := \
	-Wall \
	-Wendif-labels \
	-Wextra \
	-Wno-ctor-dtor-privacy \
	-Wno-non-virtual-dtor \
	-Wshadow \
	-fPIC \
	-fmessage-length=0 \
	-fno-common \
	-fno-strict-aliasing \
	-fuse-cxa-atexit \
	-fvisibility-inlines-hidden \
	-fvisibility=hidden \
	-pipe \

ifneq ($(EXTERNAL_WARNINGS_NOT_ERRORS),TRUE)
gb_CFLAGS_WERROR := -Werror
gb_CXXFLAGS_WERROR := -Werror
endif

ifneq ($(strip $(SYSBASE)),)
gb_CXXFLAGS += --sysroot=$(SYSBASE)
gb_CFLAGS += --sysroot=$(SYSBASE)
gb_LinkTarget_LDFLAGS := -Wl,--sysroot=$(SYSBASE)
endif
gb_LinkTarget_EXCEPTIONFLAGS := \
	-DEXCEPTIONS_ON \
	-fexceptions
ifeq ($(COM),GCC)
gb_LinkTarget_EXCEPTIONFLAGS +=  -fno-enforce-eh-specs
endif

gb_LinkTarget_NOEXCEPTIONFLAGS := \
	-DEXCEPTIONS_OFF \
	-fno-exceptions \

gb_LinkTarget_LDFLAGS += \
	-Wl,-rpath-link,$(SYSBASE)/lib:$(SYSBASE)/usr/lib \
	-Wl,-z,combreloc \
	-Wl,-z,defs \
	$(subst -L../lib , ,$(SOLARLIB)) \

ifeq ($(HAVE_LD_HASH_STYLE),TRUE)
gb_LinkTarget_LDFLAGS += \
	-Wl,--hash-style=both \

endif

ifneq ($(HAVE_LD_BSYMBOLIC_FUNCTIONS),)
gb_LinkTarget_LDFLAGS += \
	-Wl,--dynamic-list-cpp-new \
	-Wl,--dynamic-list-cpp-typeinfo \
	-Wl,-Bsymbolic-functions \

endif

ifeq ($(gb_DEBUGLEVEL),0)
gb_LinkTarget_LDFLAGS += -Wl,-O1
endif

ifeq ($(COM),CLANG)
ifeq ($(ENABLE_SYMBOLS),SMALL)
gb_DEBUG_CFLAGS := -ggdb1 -fno-inline
else
gb_DEBUG_CFLAGS := -ggdb3 -fno-inline
endif
else
ifeq ($(ENABLE_SYMBOLS),SMALL)
gb_DEBUG_CFLAGS := -ggdb1 -finline-limit=0 -fno-inline -fno-default-inline
else
gb_DEBUG_CFLAGS := -ggdb3 -finline-limit=0 -fno-inline -fno-default-inline
endif
endif

ifeq ($(gb_DEBUGLEVEL),2)
gb_COMPILEROPTFLAGS := -O0
else
gb_COMPILEROPTFLAGS := -Os
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

# CObject class

# $(call gb_CObject__command,object,relative-source,source,dep-file)
define gb_CObject__command
$(call gb_Output_announce,$(2),$(true),C  ,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && \
	$(gb_CC) \
		$(DEFS) \
		$(T_CFLAGS) \
		-c $(3) \
		-o $(1) \
		-MMD -MT $(1) \
		-MF $(4) \
		-I$(dir $(3)) \
		$(INCLUDE))
endef

# convert parametters filesystem root to native notation
# does some real work only on windows, make sure not to
# break the dummy implementations on unx*
define gb_Helper_convert_native
$(1)
endef

# CxxObject class

# $(call gb_CxxObject__command,object,relative-source,source,dep-file)
define gb_CxxObject__command
$(call gb_Output_announce,$(2),$(true),CXX,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && \
	$(gb_CXX) \
		$(DEFS) \
		$(T_CXXFLAGS) \
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
	URELIB:\dORIGIN \
	UREBIN:\dORIGIN/../lib:\dORIGIN \
	OOO:\dORIGIN:\dORIGIN/../ure-link/lib \
	BRAND:\dORIGIN:\dORIGIN/../basis-link/program:\dORIGIN/../basis-link/ure-link/lib \
	SDKBIN:\dORIGIN/../../ure-link/lib \
	NONEBIN:\dORIGIN/../lib:\dORIGIN \

gb_LinkTarget_CFLAGS := $(gb_CFLAGS) $(gb_CFLAGS_WERROR)
gb_LinkTarget_CXXFLAGS := $(gb_CXXFLAGS) $(gb_CXXFLAGS_WERROR)


gb_LinkTarget_INCLUDE := $(filter-out %/stl, $(subst -I. , ,$(SOLARINC)))
gb_LinkTarget_INCLUDE_STL := $(filter %/stl, $(subst -I. , ,$(SOLARINC)))

define gb_LinkTarget__command_dynamiclink
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(gb_CXX) \
		$(if $(filter Library,$(TARGETTYPE)),$(gb_Library_TARGETTYPEFLAGS)) \
		$(subst \d,$$,$(RPATH)) \
		$(T_LDFLAGS) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		-Wl$(COMMA)--start-group $(foreach lib,$(LINKED_STATIC_LIBS),$(call gb_StaticLibrary_get_target,$(lib))) -Wl$(COMMA)--end-group \
		$(patsubst lib%.so,-l%,$(foreach lib,$(LINKED_LIBS),$(call gb_Library_get_filename,$(lib)))) \
		$(patsubst %,-l%,$(EXTERNAL_LIBS)) \
		$(LIBS) \
		-lc \
		-o $(1))
endef

define gb_LinkTarget__command_staticlink
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(gb_AR) -rsu $(1) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		2> /dev/null)
endef

define gb_LinkTarget__command
$(call gb_Output_announce,$(2),$(true),LNK,4)
$(if $(filter Library GoogleTest Executable,$(TARGETTYPE)),$(call gb_LinkTarget__command_dynamiclink,$(1)))
$(if $(filter StaticLibrary,$(TARGETTYPE)),$(call gb_LinkTarget__command_staticlink,$(1)))
endef


# Library class

gb_Library_DEFS :=
gb_Library_TARGETTYPEFLAGS := -shared -Wl,-z,noexecstack
gb_Library_SYSPRE := lib
gb_Library_UNOVERPRE := $(gb_Library_SYSPRE)uno_
gb_Library_PLAINEXT := .so
gb_Library_RTEXT := gcc3$(gb_Library_PLAINEXT)
ifeq ($(gb_PRODUCT),$(true))
gb_Library_STLEXT := port_gcc$(gb_Library_PLAINEXT)
else
gb_Library_STLEXT := port_gcc_stldebug$(gb_Library_PLAINEXT)
endif

ifeq ($(CPUNAME),X86_64)
gb_Library_OOOEXT := $(gb_Library_PLAINEXT)
gb_Library_UNOEXT := .uno$(gb_Library_PLAINEXT)
else
gb_Library_OOOEXT := $(gb_Library_PLAINEXT)
gb_Library_UNOEXT := .uno$(gb_Library_PLAINEXT)
endif

gb_Library_PLAINLIBS_NONE += \
	fontconfig \
	Xrender \
	dl \
	freetype \
	m \
	pthread \
	X11 \
	Xext \
	SM \
	ICE

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
'-Wl,-rpath,$(call gb_LinkTarget__get_rpath_for_layer,$(call gb_Library_get_layer,$(1)))' \
'-Wl,-rpath-link,$(gb_Library_OUTDIRLOCATION)'
endef

define gb_Library_Library_platform
$(call gb_LinkTarget_get_target,$(2)) : RPATH := $(call gb_Library_get_rpath,$(1))

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

gb_Executable_LAYER := \
	$(foreach exe,$(gb_Executable_UREBIN),$(exe):UREBIN) \
	$(foreach exe,$(gb_Executable_SDK),$(exe):SDKBIN) \
	$(foreach exe,$(gb_Executable_OOO),$(exe):OOO) \
	$(foreach exe,$(gb_Executable_BRAND),$(exe):BRAND) \
	$(foreach exe,$(gb_Executable_NONE),$(exe):NONEBIN) \


define gb_Executable_get_rpath
'-Wl,-rpath,$(call gb_LinkTarget__get_rpath_for_layer,$(call gb_Executable_get_layer,$(1)))' \
-Wl,-rpath-link,$(gb_Library_OUTDIRLOCATION)
endef

define gb_Executable_Executable_platform
$(call gb_LinkTarget_get_target,$(2)) : RPATH := $(call gb_Executable_get_rpath,$(1))

endef


# GoogleTest class

gb_GoogleTest_GTESTPRECOMMAND := LD_LIBRARY_PATH=$(OUTDIR)/lib
gb_GoogleTest_get_filename = $(1)$(gb_Executable_EXT)

define gb_GoogleTest_GoogleTest_platform
$(call gb_LinkTarget_get_target,$(2)) : RPATH := -Wl,-rpath-link,$(gb_Library_OUTDIRLOCATION)
endef

# JunitTest class

define gb_JunitTest_JunitTest_platform
$(call gb_JunitTest_get_target,$(1)) : DEFS := \
	-Dorg.openoffice.test.arg.soffice="$$$${OOO_TEST_SOFFICE:-path:$(SRCDIR)/instsetoo_native/$(INPATH)/Apache_OpenOffice/installed/install/en-US/openoffice4/program/soffice}" \
    -Dorg.openoffice.test.arg.env=LD_LIBRARY_PATH \
    -Dorg.openoffice.test.arg.user=file://$(call gb_JunitTest_get_userdir,$(1)) \

endef

# SdiTarget class

gb_SdiTarget_SVIDLPRECOMMAND := LD_LIBRARY_PATH=$(OUTDIR)/lib

# SrsPartMergeTarget

gb_SrsPartMergeTarget_TRANSEXPRECOMMAND := LD_LIBRARY_PATH=$(OUTDIR)/lib

# SrsPartTarget class

gb_SrsPartTarget_RSCTARGET := $(OUTDIR)/bin/rsc
gb_SrsPartTarget_RSCCOMMAND := LD_LIBRARY_PATH=$(OUTDIR)/lib SOLARBINDIR=$(OUTDIR)/bin $(gb_SrsPartTarget_RSCTARGET)

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

gb_XSLTPROCPRECOMMAND := LD_LIBRARY_PATH=$(OUTDIR)/lib
gb_Library_COMPONENTPREFIXES := \
    OOO:vnd.sun.star.expand:\dOOO_BASE_DIR/program/ \
    URELIB:vnd.sun.star.expand:\dURE_INTERNAL_LIB_DIR/ \
    NONE:vnd.sun.star.expand:\dOOO_INBUILD_SHAREDLIB_DIR/ \

# UnoApiTarget

gb_UnoApiTarget_IDLCTARGET := $(OUTDIR)/bin/idlc
gb_UnoApiTarget_IDLCCOMMAND := LD_LIBRARY_PATH=$(OUTDIR)/lib SOLARBINDIR=$(OUTDIR)/bin $(gb_UnoApiTarget_IDLCTARGET)
gb_UnoApiTarget_REGMERGETARGET := $(OUTDIR)/bin/regmerge
gb_UnoApiTarget_REGMERGECOMMAND := LD_LIBRARY_PATH=$(OUTDIR)/lib SOLARBINDIR=$(OUTDIR)/bin $(gb_UnoApiTarget_REGMERGETARGET)
gb_UnoApiTarget_REGCOMPARETARGET := $(OUTDIR)/bin/regcompare
gb_UnoApiTarget_REGCOMPARECOMMAND := LD_LIBRARY_PATH=$(OUTDIR)/lib SOLARBINDIR=$(OUTDIR)/bin $(gb_UnoApiTarget_REGCOMPARETARGET)
gb_UnoApiTarget_CPPUMAKERTARGET := $(OUTDIR)/bin/cppumaker
gb_UnoApiTarget_CPPUMAKERCOMMAND := LD_LIBRARY_PATH=$(OUTDIR)/lib SOLARBINDIR=$(OUTDIR)/bin $(gb_UnoApiTarget_CPPUMAKERTARGET)
gb_UnoApiTarget_REGVIEWTARGET := $(OUTDIR)/bin/regview
gb_UnoApiTarget_REGVIEWCOMMAND := LD_LIBRARY_PATH=$(OUTDIR)/lib SOLARBINDIR=$(OUTDIR)/bin $(gb_UnoApiTarget_REGVIEWTARGET)

# vim: set noet sw=4 ts=4:
