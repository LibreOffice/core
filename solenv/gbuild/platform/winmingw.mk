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



GUI := WNT
COM := GCC

# set tmpdir to some mixed case path, suitable for native tools
gb_TMPDIR:=$(if $(TMPDIR),$(shell cygpath -m $(TMPDIR)),$(shell cygpath -m /tmp))
gb_MKTEMP := mktemp --tmpdir=$(gb_TMPDIR) gbuild.XXXXXX

gb_CC := $(CC)
gb_CXX := $(CXX)
gb_LINK := $(shell $(CC) -print-prog-name=ld)
gb_AR := $(shell $(CC) -print-prog-name=ar)
gb_AWK := awk
gb_RC := rc
ifeq ($(USE_MINGW),cygwin)
gb_MINGWLIBDIR := $(COMPATH)/lib/mingw
else
ifeq ($(USE_MINGW),cygwin-w64-mingw32)
gb_MINGWLIBDIR := $(COMPATH)/usr/i686-w64-mingw32/sys-root/mingw/lib
else
gb_MINGWLIBDIR := $(COMPATH)/lib
endif
endif
ifeq ($(MINGW_SHARED_GXXLIB),YES)
gb_MINGW_LIBSTDCPP := $(subst -l,,$(MINGW_SHARED_LIBSTDCPP))
else
gb_MINGW_LIBSTDCPP := \
		stdc++ \
		moldname
endif
ifeq ($(MINGW_SHARED_GCCLIB),YES)
gb_MINGW_LIBGCC := \
		gcc_s \
		gcc
else
ifeq ($(MINGW_GCCLIB_EH),YES)
gb_MINGW_LIBGCC := \
		gcc \
		gcc_eh
else
gb_MINGW_LIBGCC := gcc
endif
endif

gb_OSDEFS := \
	-DWINVER=0x0500 \
	-D_WIN32_IE=0x0500 \
	-D_WIN32_WINNT=0x0600 \
	-DNT351 \
	-DWIN32 \
	-DWNT \

ifeq ($(GXX_INCLUDE_PATH),)
GXX_INCLUDE_PATH=$(COMPATH)/include/c++/$(shell gcc -dumpversion)
endif

gb_COMPILERDEFS := \
	-DGCC \
	-D$(CVER) \
	-DCVER=$(CVER) \
	-DGLIBC=2 \
	-DGXX_INCLUDE_PATH=$(GXX_INCLUDE_PATH) \
	-DCPPU_ENV=gcc3 \
	-D_MT \
	-D_NATIVE_WCHAR_T_DEFINED \
	-D_MSC_EXTENSIONS \
	-D_FORCENAMELESSUNION \

ifeq ($(USE_MINGW),cygwin-w64-mingw32)
gb_COMPILERDEFS +=-D_declspec=__declspec
endif

gb_CPUDEFS := \
	-DINTEL \
	-D_M_IX86 \

gb_RCDEFS := \
     -DWIN32 \
     -D__GNUC__ \

gb_RCFLAGS := \
     -V

gb_CFLAGS := \
	-Wall \
	-Wendif-labels \
	-Wextra \
	-fmessage-length=0 \
	-fno-strict-aliasing \
	-pipe \
	-nostdinc \

gb_CXXFLAGS := \
	-Wall \
	-Wendif-labels \
	-Wextra \
	-Wno-ctor-dtor-privacy \
	-Wno-non-virtual-dtor \
	-Wreturn-type \
	-Wshadow \
	-Wuninitialized \
	-fmessage-length=0 \
	-fno-strict-aliasing \
	-fno-use-cxa-atexit \
	-pipe \
	-nostdinc \

ifneq ($(EXTERNAL_WARNINGS_NOT_ERRORS),TRUE)
gb_CFLAGS_WERROR := -Werror
gb_CXXFLAGS_WERROR := -Werror
endif

ifneq ($(SYSBASE),)
gb_CXXFLAGS += --sysroot=$(SYSBASE)
gb_CFLAGS += --sysroot=$(SYSBASE)
endif
gb_LinkTarget_EXCEPTIONFLAGS := \
	-DEXCEPTIONS_ON \
	-fexceptions \
	-fno-enforce-eh-specs \

gb_PrecompiledHeader_EXCEPTIONFLAGS := $(gb_LinkTarget_EXCEPTIONFLAGS)


gb_LinkTarget_NOEXCEPTIONFLAGS := \
	-DEXCEPTIONS_OFF \
	-fno-exceptions \

gb_NoexPrecompiledHeader_NOEXCEPTIONFLAGS := $(gb_LinkTarget_NOEXCEPTIONFLAGS)

gb_LinkTarget_LDFLAGS := \
	--export-all-symbols \
	--kill-at \
	--subsystem console \
	--exclude-libs ALL \
	--enable-stdcall-fixup \
	--enable-runtime-pseudo-reloc-v2 \
	-L$(gb_Library_DLLDIR) \
	$(patsubst %,-L%,$(filter-out .,$(subst ;, ,$(subst \,/,$(ILIB))))) \

ifeq ($(MINGW_GCCLIB_EH),YES)
gb_LinkTarget_LDFLAGS += -shared-libgcc
endif

ifeq ($(gb_DEBUGLEVEL),2)
gb_COMPILEROPTFLAGS := -O0
else
gb_COMPILEROPTFLAGS := -Os
endif

gb_COMPILERNOOPTFLAGS := -O0

gb_STDLIBS := \
	mingwthrd \
	$(gb_MINGW_LIBSTDCPP) \
	mingw32 \
	$(gb_MINGW_LIBGCC) \
	uwinapi \
	moldname \
	mingwex \
    kernel32 \
	msvcrt \


# Helper class

gb_Helper_SRCDIR_NATIVE := $(shell cygpath -m $(SRCDIR) | $(gb_AWK) -- '{ print tolower(substr($$0,1,1)) substr($$0,2) }')
gb_Helper_WORKDIR_NATIVE := $(shell cygpath -m $(WORKDIR) | $(gb_AWK) -- '{ print tolower(substr($$0,1,1)) substr($$0,2) }')
gb_Helper_OUTDIR_NATIVE := $(shell cygpath -m $(OUTDIR) | $(gb_AWK) -- '{ print tolower(substr($$0,1,1)) substr($$0,2) }')
gb_Helper_REPODIR_NATIVE := $(shell cygpath -m $(REPODIR) | $(gb_AWK) -- '{ print tolower(substr($$0,1,1)) substr($$0,2) }')

define gb_Helper_abbreviate_dirs_native
R=$(gb_Helper_REPODIR_NATIVE) && $(subst $(REPODIR)/,$$R/,$(subst $(gb_Helper_REPODIR_NATIVE)/,$$R/,O=$(gb_Helper_OUTDIR_NATIVE) && W=$(gb_Helper_WORKDIR_NATIVE) && S=$(gb_Helper_SRCDIR_NATIVE))) && \
$(subst $(REPODIR)/,$$R/,$(subst $(SRCDIR)/,$$S/,$(subst $(OUTDIR)/,$$O/,$(subst $(WORKDIR)/,$$W/,$(subst $(gb_Helper_REPODIR_NATIVE)/,$$R/,$(subst $(gb_Helper_SRCDIR_NATIVE)/,$$S/,$(subst $(gb_Helper_OUTDIR_NATIVE)/,$$O/,$(subst $(gb_Helper_WORKDIR_NATIVE)/,$$W/,$(1)))))))))
endef

# convert parametters filesystem root to native notation
# does some real work only on windows, make sure not to
# break the dummy implementations on unx*
define gb_Helper_convert_native
$(patsubst -I$(OUTDIR)%,-I$(gb_Helper_OUTDIR_NATIVE)%, \
$(patsubst $(OUTDIR)%,$(gb_Helper_OUTDIR_NATIVE)%, \
$(patsubst $(WORKDIR)%,$(gb_Helper_WORKDIR_NATIVE)%, \
$(patsubst $(SRCDIR)%,$(gb_Helper_SRCDIR_NATIVE)%, \
$(1)))))
endef

# CObject class

ifeq ($(gb_FULLDEPS),$(true))
define gb_CObject__command_deponcompile
$(call gb_Helper_abbreviate_dirs_native,\
	$(OUTDIR)/bin/makedepend$(gb_Executable_EXT) \
		$(filter-out -DPRECOMPILED_HEADERS,$(4)) $(5) \
		-I$(dir $(3)) \
		$(filter-out -I$(COMPATH)% %/pch -I$(JAVA_HOME),$(6)) \
		$(3) \
		-f - \
	| $(gb_AWK) -f $(GBUILDDIR)/processdeps.awk \
		-v OBJECTFILE=$(1) \
		-v OUTDIR=$(OUTDIR)/ \
		-v WORKDIR=$(WORKDIR)/ \
		-v SRCDIR=$(SRCDIR)/ \
		-v REPODIR=$(REPODIR)/ \
	> $(call gb_CObject_get_dep_target,$(2)))
endef
else
CObject__command_deponcompile =
endif

define gb_CObject__command
$(call gb_Output_announce,$(2),$(true),C  ,3)
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	$(gb_CC) \
		$(DEFS) $(CFLAGS) \
		-c $(3) \
		-o $(1) \
		-I$(dir $(3)) \
		$(INCLUDE))
$(call gb_CObject__command_deponcompile,$(1),$(2),$(3),$(DEFS),$(CFLAGS),$(INCLUDE))
endef



# CxxObject class

ifeq ($(gb_FULLDEPS),$(true))
define gb_CxxObject__command_deponcompile
$(call gb_Helper_abbreviate_dirs_native,\
	$(OUTDIR)/bin/makedepend$(gb_Executable_EXT) \
		$(filter-out -DPRECOMPILED_HEADERS,$(4)) $(5) \
		-I$(dir $(3)) \
		$(filter-out -I$(COMPATH)% %/pch -I$(JAVA_HOME),$(6)) \
		$(3) \
		-f - \
	| $(gb_AWK) -f $(GBUILDDIR)/processdeps.awk \
		-v OBJECTFILE=$(1) \
		-v OUTDIR=$(OUTDIR)/ \
		-v WORKDIR=$(WORKDIR)/ \
		-v SRCDIR=$(SRCDIR)/ \
		-v REPODIR=$(REPODIR)/ \
	> $(call gb_CxxObject_get_dep_target,$(2)))
endef
else
gb_CxxObject__command_deponcompile =
endif

define gb_CxxObject__command
$(call gb_Output_announce,$(2),$(true),CXX,3)
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	$(gb_CXX) \
		$(DEFS) $(CXXFLAGS) \
		-c $(3) \
		-o $(1) \
		-I$(dir $(3)) \
		$(INCLUDE_STL) $(INCLUDE))
$(call gb_CxxObject__command_deponcompile,$(1),$(2),$(3),$(DEFS),$(CXXFLAGS),$(INCLUDE_STL) $(INCLUDE))
endef


# PrecompiledHeader class

gb_PrecompiledHeader_EXT := .gch

gb_PrecompiledHeader_get_enableflags = -I$(WORKDIR)/PrecompiledHeader/$(gb_PrecompiledHeader_DEBUGDIR) \
					-DPRECOMPILED_HEADERS \
					-Winvalid-pch \

ifeq ($(gb_FULLDEPS),$(true))
define gb_PrecompiledHeader__command_deponcompile
$(call gb_Helper_abbreviate_dirs_native,\
	$(OUTDIR)/bin/makedepend$(gb_Executable_EXT) \
		$(4) $(5) \
		-I$(dir $(3)) \
		$(filter-out -I$(COMPATH)% -I$(JAVA_HOME),$(6)) \
		$(3) \
		-f - \
	| $(gb_AWK) -f $(GBUILDDIR)/processdeps.awk \
		-v OBJECTFILE=$(1) \
		-v OUTDIR=$(OUTDIR)/ \
		-v WORKDIR=$(WORKDIR)/ \
		-v SRCDIR=$(SRCDIR)/ \
		-v REPODIR=$(REPODIR)/ \
	> $(call gb_PrecompiledHeader_get_dep_target,$(2)))
endef
else
gb_PrecompiledHeader__command_deponcompile =
endif

define gb_PrecompiledHeader__command
$(call gb_Output_announce,$(2),$(true),PCH,1)
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) $(dir $(call gb_PrecompiledHeader_get_dep_target,$(2))) && \
	$(gb_CXX) \
		-x c++-header \
		$(4) $(5) \
		-I$(dir $(3)) \
		$(6) \
		-c $(3) \
		-o$(1))
$(call gb_PrecompiledHeader__command_deponcompile,$(1),$(2),$(3),$(4),$(5),$(6))

endef

# NoexPrecompiledHeader class

gb_NoexPrecompiledHeader_EXT := .gch

gb_NoexPrecompiledHeader_get_enableflags = -I$(WORKDIR)/NoexPrecompiledHeader/$(gb_NoexPrecompiledHeader_DEBUGDIR) \
					-Winvalid-pch \

ifeq ($(gb_FULLDEPS),$(true))
define gb_NoexPrecompiledHeader__command_deponcompile
$(call gb_Helper_abbreviate_dirs_native,\
	$(OUTDIR)/bin/makedepend$(gb_Executable_EXT) \
		$(4) $(5) \
		-I$(dir $(3)) \
		$(filter-out -I$(COMPATH)% -I$(JAVA_HOME),$(6)) \
		$(3) \
		-f - \
	| $(gb_AWK) -f $(GBUILDDIR)/processdeps.awk \
		-v OBJECTFILE=$(1) \
		-v OUTDIR=$(OUTDIR)/ \
		-v WORKDIR=$(WORKDIR)/ \
		-v SRCDIR=$(SRCDIR)/ \
		-v REPODIR=$(REPODIR)/ \
	> $(call gb_NoexPrecompiledHeader_get_dep_target,$(2)))
endef
else
gb_NoexPrecompiledHeader__command_deponcompile =
endif

define gb_NoexPrecompiledHeader__command
$(call gb_Output_announce,$(2),$(true),PCH,1)
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) $(dir $(call gb_NoexPrecompiledHeader_get_dep_target,$(2))) && \
	$(gb_CXX) \
		-x c++-header \
		$(4) $(5) \
		-I$(dir $(3)) \
		$(6) \
		-c $(3) \
		-o$(1))
$(call gb_NoexPrecompiledHeader__command_deponcompile,$(1),$(2),$(3),$(4),$(5),$(6))

endef


# LinkTarget class

gb_LinkTarget_CFLAGS := $(gb_CFLAGS) $(gb_CFLAGS_WERROR) $(gb_COMPILEROPTFLAGS)
gb_LinkTarget_CXXFLAGS := $(gb_CXXFLAGS) $(gb_CXXFLAGS_WERROR)

ifeq ($(gb_DEBUGLEVEL),2)
gb_LinkTarget_CXXFLAGS += -ggdb3 -finline-limit=0 -fno-inline -fno-default-inline
gb_LinkTarget_CFLAGS += -ggdb3 -finline-limit=0 -fno-inline -fno-default-inline

endif

gb_LinkTarget_INCLUDE :=\
	$(filter-out %/stl, $(subst -I. , ,$(SOLARINC))) \
	$(foreach inc,$(subst ;, ,$(JDKINC)),-I$(inc)) \

gb_LinkTarget_INCLUDE_STL := $(filter %/stl, $(subst -I. , ,$(SOLARINC)))

define gb_LinkTarget__command_dynamiclinkexecutable
$(call gb_Output_announce,$(2),$(true),LNK,4)
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	RESPONSEFILE=`$(gb_MKTEMP)` && \
	echo "$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(NATIVERES) " > $${RESPONSEFILE} && \
	$(gb_LINK) \
		$(gb_Executable_TARGETTYPEFLAGS) \
		$(LDFLAGS) \
		$(gb_MINGWLIBDIR)/crt2.o \
		$(MINGW_CLIB_DIR)/crtbegin.o \
		@$${RESPONSEFILE} \
		--start-group $(foreach lib,$(LINKED_STATIC_LIBS),$(call gb_StaticLibrary_get_target,$(lib))) --end-group \
		--start-group $(patsubst %.dll,-l%,$(foreach lib,$(LINKED_LIBS),$(call gb_Library_get_dllname,$(lib)))) --end-group \
		--start-group $(patsubst %,-l%,$(EXTERNAL_LIBS)) --end-group \
		$(MINGW_CLIB_DIR)/crtend.o \
		-Map $(basename $(1)).map \
		-o $(1))
endef

define gb_LinkTarget__command_dynamiclinklibrary
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	rm -f $(1) && \
	RESPONSEFILE=`$(gb_MKTEMP)` && \
	echo "$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(NATIVERES) " > $${RESPONSEFILE} && \
	$(gb_LINK) \
		$(gb_Library_TARGETTYPEFLAGS) \
		$(LDFLAGS) \
		--enable-auto-image-base \
		-e _DllMainCRTStartup@12 \
		$(gb_MINGWLIBDIR)/dllcrt2.o \
		$(MINGW_CLIB_DIR)/crtbegin.o \
		@$${RESPONSEFILE} \
		--start-group $(foreach lib,$(LINKED_STATIC_LIBS),$(call gb_StaticLibrary_get_target,$(lib))) --end-group \
		--start-group $(patsubst %.dll,-l%,$(foreach lib,$(LINKED_LIBS),$(call gb_Library_get_dllname,$(lib)))) --end-group \
		$(MINGW_CLIB_DIR)/crtend.o \
		-Map $(basename $(DLLTARGET)).map \
		-o $(DLLTARGET) && touch $(1))
endef

define gb_LinkTarget__command_staticlinklibrary
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	RESPONSEFILE=`$(gb_MKTEMP)` && \
	echo "$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) " > $${RESPONSEFILE} && \
	$(gb_AR) -rsu\
		$(1) \
		@$${RESPONSEFILE})
endef

define gb_LinkTarget__command
$(call gb_Output_announce,$(2),$(true),LNK,4)
$(if $(filter GoogleTest Executable,$(TARGETTYPE)),$(call gb_LinkTarget__command_dynamiclinkexecutable,$(1),$(2)))
$(if $(filter Library CppunitTest,$(TARGETTYPE)),$(call gb_LinkTarget__command_dynamiclinklibrary,$(1),$(2)))
$(if $(filter StaticLibrary,$(TARGETTYPE)),$(call gb_LinkTarget__command_staticlinklibrary,$(1)))
endef


# Library class

gb_Library_DEFS := -D_DLL
gb_Library_TARGETTYPEFLAGS := -shared
gb_Library_get_rpath :=

gb_Library_SYSPRE := i
gb_Library_PLAINEXT := .lib

gb_Library_PLAINLIBS_NONE += \
	mingwthrd \
	mingw32 \
	mingwex \
	$(gb_MINGW_LIBSTDCPP) \
	$(gb_MINGW_LIBGCC) \
	advapi32 \
	gdi32 \
	gdiplus \
	gnu_getopt \
	imm32\
	kernel32 \
	msimg32 \
	msvcrt \
	mpr \
	moldname \
	ole32 \
	oleaut32 \
	shell32 \
	user32 \
	uuid \
	uwinapi \
	winspool \
	z \
	cppunit

gb_Library_LAYER := \
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):OOO) \
	$(foreach lib,$(gb_Library_RTLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_STLLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):OOO) \
	$(foreach lib,$(gb_Library_UNOLIBS_OOO),$(lib):OOO) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):OOO) \

gb_Library_FILENAMES :=\
	$(foreach lib,$(gb_Library_TARGETS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \

gb_Library_DLLEXT := .dll
gb_Library_MAJORVER := 3
gb_Library_RTEXT := gcc3$(gb_Library_DLLEXT)
ifeq ($(gb_PRODUCT),$(true))
gb_Library_STLEXT := port_gcc$(gb_Library_DLLEXT)
else
gb_Library_STLEXT := port_gcc_stldebug$(gb_Library_DLLEXT)
endif
gb_Library_OOOEXT := $(gb_Library_DLLEXT)
gb_Library_UNOEXT := .uno$(gb_Library_DLLEXT)
gb_Library_UNOVEREXT := $(gb_Library_MAJORVER)$(gb_Library_DLLEXT)
gb_Library_RTVEREXT := $(gb_Library_MAJORVER)$(gb_Library_RTEXT)

gb_Library_DLLFILENAMES := \
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):$(lib)$(gb_Library_OOOEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_RTLIBS),$(lib):$(lib)$(gb_Library_RTEXT)) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(lib)$(gb_Library_RTVEREXT)) \
	$(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):$(lib)$(gb_Library_UNOEXT)) \
	$(foreach lib,$(gb_Library_UNOLIBS_OOO),$(lib):$(lib)$(gb_Library_UNOEXT)) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(lib)$(gb_Library_UNOVEREXT)) \

gb_Library_IARCSYSPRE := lib
gb_Library_IARCEXT := .a

gb_Library_ILIBEXT := .lib

define gb_Library_Library_platform
$(call gb_LinkTarget_set_dlltarget,$(2),$(3))

$(call gb_LinkTarget_set_auxtargets,$(2),\
	$(patsubst %.dll,%.map,$(3)) \
)

$(call gb_Library_get_target,$(1)) \
$(call gb_Library_get_clean_target,$(1)) : AUXTARGETS := $(OUTDIR)/bin/$(notdir $(3))

$(call gb_Deliver_add_deliverable,$(OUTDIR)/bin/$(notdir $(3)),$(3))

endef

define gb_Library_add_default_nativeres
$(call gb_WinResTarget_WinResTarget_init,$(1)/$(2))
$(call gb_WinResTarget_add_file,$(1)/$(2),solenv/inc/shlinfo)
$(call gb_WinResTarget_set_defs,$(1)/$(2),\
        $$(DEFS) \
		-DADDITIONAL_VERINFO1= \
		-DADDITIONAL_VERINFO2= \
		-DADDITIONAL_VERINFO3= \
)
$(call gb_Library_add_nativeres,$(1),$(2))
$(call gb_Library_get_clean_target,$(1)) : $(call gb_WinResTarget_get_clean_target,$(1)/$(2))

endef

define gb_Library_add_nativeres
$(call gb_LinkTarget_get_target,$(call gb_Library__get_linktargetname,$(1))) : $(call gb_WinResTarget_get_target,$(1)/$(2))
$(call gb_LinkTarget_get_target,$(call gb_Library__get_linktargetname,$(1))) : NATIVERES += $(call gb_WinResTarget_get_target,$(1)/$(2))

endef

define gb_Library_get_dllname
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_Library_DLLFILENAMES)))
endef


# StaticLibrary class

gb_StaticLibrary_DEFS :=
gb_StaticLibrary_SYSPRE := lib
gb_StaticLibrary_PLAINEXT := .a
gb_StaticLibrary_JPEGEXT := lib$(gb_StaticLibrary_PLAINEXT)

gb_StaticLibrary_FILENAMES := \
	$(foreach lib,$(gb_StaticLibrary_JPEGLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_JPEGEXT)) \
	$(foreach lib,$(gb_StaticLibrary_PLAINLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_PLAINEXT)) \

gb_StaticLibrary_FILENAMES := $(patsubst salcpprt:salcpprt%,salcpprt:cpprtl%,$(gb_StaticLibrary_FILENAMES))

gb_StaticLibrary_StaticLibrary_platform =

# Executable class

gb_Executable_EXT := .exe
gb_Executable_TARGETTYPEFLAGS :=
gb_Executable_get_rpath :=
gb_Executable_Executable_platform =

# GoogleTest class

gb_GoogleTest_GTESTPRECOMMAND := PATH="$${PATH}:$(OUTDIR)/bin"
gb_GoogleTest_get_filename = $(1)$(gb_Executable_EXT)

define gb_GoogleTest_GoogleTest_platform
endef

# CppunitTest class

gb_CppunitTest_CPPTESTPRECOMMAND :=
gb_CppunitTest_SYSPRE := itest_
gb_CppunitTest_EXT := .lib
gb_CppunitTest_get_filename = $(gb_CppunitTest_SYSPRE)$(1)$(gb_CppunitTest_EXT)
gb_CppunitTest_get_libfilename = test_$(1).dll

define gb_CppunitTest_CppunitTest_platform
$(call gb_LinkTarget_set_dlltarget,$(2),$(3))

$(call gb_LinkTarget_set_auxtargets,$(2),\
	$(patsubst %.dll,%.map,$(3)) \
)

endef

# SdiTarget class

gb_SdiTarget_SVIDLPRECOMMAND := PATH="$${PATH}:$(OUTDIR)/bin"

# SrsPartMergeTarget

gb_SrsPartMergeTarget_TRANSEXPRECOMMAND := PATH="$${PATH}:$(OUTDIR)/bin"

# SrsPartTarget class

gb_SrsPartTarget_RSCTARGET := $(OUTDIR)/bin/rsc.exe
gb_SrsPartTarget_RSCCOMMAND := SOLARBINDIR=$(OUTDIR)/bin $(gb_SrsPartTarget_RSCTARGET)

ifeq ($(gb_FULLDEPS),$(true))
define gb_SrsPartTarget__command_dep
$(call gb_Helper_abbreviate_dirs_native,\
	$(OUTDIR)/bin/makedepend$(gb_Executable_EXT) \
		$(INCLUDE) \
		$(DEFS) \
		$(2) \
		-f - \
	| $(gb_AWK) -f $(GBUILDDIR)/processdeps.awk \
		-v OBJECTFILE=$(call gb_SrsPartTarget_get_target,$(1)) \
		-v OUTDIR=$(OUTDIR)/ \
		-v WORKDIR=$(WORKDIR)/ \
		-v SRCDIR=$(SRCDIR)/ \
		-v REPODIR=$(REPODIR)/ \
	> $(call gb_SrsPartTarget_get_dep_target,$(1)))
endef
else
gb_SrsPartTarget__command_dep =
endif

# WinResTarget class

gb_WinResTarget_POSTFIX :=_res.o

define gb_WinResTarget__command
$(call gb_Output_announce,$(2),$(true),RES,3)
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	$(gb_RC) \
		$(DEFS) $(FLAGS) \
		-I$(dir $(3)) \
		$(INCLUDE) \
		-Fo$(patsubst %_res.o,%.res,$(1)) \
        $(RCFILE) )
$(call gb_Helper_abbreviate_dirs_native,\
    windres $(patsubst %_res.o,%.res,$(1)) $(1))
	rm $(patsubst %_res.o,%.res,$(1))
endef

$(eval $(call gb_Helper_make_dep_targets,\
	WinResTarget \
))

ifeq ($(gb_FULLDEPS),$(true))
define gb_WinResTarget__command_dep
$(call gb_Helper_abbreviate_dirs_native,\
	$(OUTDIR)/bin/makedepend$(gb_Executable_EXT) \
		$(INCLUDE) \
		$(DEFS) \
		$(2) \
		-f - \
	| $(gb_AWK) -f $(GBUILDDIR)/processdeps.awk \
		-v OBJECTFILE=$(call gb_WinResTarget_get_target,$(1)) \
		-v OUTDIR=$(OUTDIR)/ \
		-v WORKDIR=$(WORKDIR)/ \
		-v SRCDIR=$(SRCDIR)/ \
		-v REPODIR=$(REPODIR)/ \
	> $(call gb_WinResTarget_get_dep_target,$(1)))
endef
else
gb_WinResTarget__command_dep =
endif

# ComponentTarget

gb_XSLTPROCPRECOMMAND := PATH="$${PATH}:$(OUTDIR)/bin"
gb_Library_COMPONENTPREFIXES := \
    OOO:vnd.sun.star.expand:\dOOO_BASE_DIR/program/ \
    URELIB:vnd.sun.star.expand:\dURE_INTERNAL_LIB_DIR/ \

# vim: set noet sw=4 ts=4:
