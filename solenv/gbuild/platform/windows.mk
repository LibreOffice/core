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
COM := MSC

# set tmpdir to some mixed case path, suitable for native tools
gb_TMPDIR:=$(if $(TMPDIR),$(shell cygpath -m $(TMPDIR)),$(shell cygpath -m /tmp))
gb_MKTEMP := mktemp --tmpdir=$(gb_TMPDIR) gbuild.XXXXXX

gb_CC := cl
gb_CXX := cl
gb_LINK := link
gb_AWK := awk
gb_CLASSPATHSEP := ;
gb_RC := rc

# use CC/CXX if they are nondefaults
ifneq ($(origin CC),default)
gb_CC := $(CC)
gb_GCCP := $(CC)
endif
ifneq ($(origin CXX),default)
gb_CXX := $(CXX)
endif

gb_OSDEFS := \
	-DWINVER=0x0500 \
	-D_WIN32_IE=0x0500 \
	-DNT351 \
	-DWIN32 \
	-DWNT \

gb_COMPILERDEFS := \
	-DMSC \
	-D_CRT_NON_CONFORMING_SWPRINTFS \
	-D_CRT_NONSTDC_NO_DEPRECATE \
	-D_CRT_SECURE_NO_DEPRECATE \
	-D_MT \
	-DBOOST_MEM_FN_ENABLE_CDECL \
	-DCPPU_ENV=msci \
	-DFULL_DESK \
	-DM1500 \

gb_CPUDEFS := -DINTEL -D_X86_=1

gb_RCDEFS := \
     -DWINVER=0x0400 \
	 -DWIN32 \

gb_RCFLAGS := \
     -V

gb_CFLAGS := \
	-Gd \
	-GR \
	-Gs \
	-GS \
	-Gy \
	-nologo \
	-Wall \
	-wd4005 \
	-wd4061 \
	-wd4127 \
	-wd4180 \
	-wd4189 \
	-wd4191 \
	-wd4217 \
	-wd4250 \
	-wd4251 \
	-wd4255 \
	-wd4275 \
	-wd4290 \
	-wd4294 \
	-wd4350 \
	-wd4355 \
	-wd4365 \
	-wd4503 \
	-wd4505 \
	-wd4511 \
	-wd4512 \
	-wd4514 \
	-wd4611 \
	-wd4619 \
	-wd4625 \
	-wd4626 \
	-wd4640 \
	-wd4668 \
	-wd4675 \
	-wd4692 \
	-wd4710 \
	-wd4711 \
	-wd4738 \
	-wd4786 \
	-wd4800 \
	-wd4820 \
	-wd4826 \
	-Zc:wchar_t- \
	-Zm500 \

gb_CXXFLAGS := \
	-Gd \
	-GR \
	-Gs \
	-GS \
	-Gy \
	-nologo \
	-Wall \
	-wd4005 \
	-wd4061 \
	-wd4127 \
	-wd4180 \
	-wd4189 \
	-wd4191 \
	-wd4217 \
	-wd4250 \
	-wd4251 \
	-wd4275 \
	-wd4290 \
	-wd4294 \
	-wd4350 \
	-wd4355 \
	-wd4365 \
	-wd4503 \
	-wd4505 \
	-wd4511 \
	-wd4512 \
	-wd4514 \
	-wd4611 \
	-wd4619 \
	-wd4625 \
	-wd4626 \
	-wd4640 \
	-wd4668 \
	-wd4675 \
	-wd4692 \
	-wd4710 \
	-wd4711 \
	-wd4738 \
	-wd4786 \
	-wd4800 \
	-wd4820 \
	-wd4826 \
	-Zc:wchar_t- \
	-Zm500 \

gb_STDLIBS := \
    uwinapi \
    kernel32 \
    msvcrt \
    oldnames \

ifneq ($(EXTERNAL_WARNINGS_NOT_ERRORS),TRUE)
gb_CFLAGS_WERROR := -WX
gb_CXXFLAGS_WERROR := -WX
endif

gb_LinkTarget_EXCEPTIONFLAGS := \
	-DEXCEPTIONS_ON \
	-EHa \

gb_PrecompiledHeader_EXCEPTIONFLAGS := $(gb_LinkTarget_EXCEPTIONFLAGS)


gb_LinkTarget_NOEXCEPTIONFLAGS := \
	-DEXCEPTIONS_OFF \

gb_NoexPrecompiledHeader_NOEXCEPTIONFLAGS := $(gb_LinkTarget_NOEXCEPTIONFLAGS)

gb_LinkTarget_LDFLAGS := \
	-MACHINE:IX86 \
	-NODEFAULTLIB \
	-SUBSYSTEM:CONSOLE \
	$(patsubst %,-LIBPATH:%,$(filter-out .,$(subst ;, ,$(subst \,/,$(ILIB))))) \

ifneq ($(ENABLE_CRASHDUMP),)
gb_LinkTarget_LDFLAGS += -DEBUG
gb_CFLAGS+=-Zi
gb_CXXFLAGS+=-Zi
endif

ifeq ($(gb_DEBUGLEVEL),2)
gb_CXXFLAGS +=-Zi
gb_CFLAGS +=-Zi
gb_COMPILEROPTFLAGS :=
gb_LinkTarget_LDFLAGS += -DEBUG
else
gb_COMPILEROPTFLAGS := -Ob1 -Oxs -Oy-
endif

gb_COMPILERNOOPTFLAGS := -Od


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
		$(filter-out -I$(COMPATH)% %/pch -I$(JAVA_HOME)%,$(6)) \
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
	unset INCLUDE && \
	$(gb_CC) \
		$(DEFS) $(CFLAGS)  -Fd$(PDBFILE) \
        $(PCHFLAGS) \
		-I$(dir $(3)) \
		$(INCLUDE) \
		-c $(3) \
		-Fo$(1))
$(call gb_CObject__command_deponcompile,$(1),$(2),$(3),$(DEFS),$(CFLAGS),$(INCLUDE))
endef


# CxxObject class

ifeq ($(gb_FULLDEPS),$(true))
define gb_CxxObject__command_deponcompile
$(call gb_Helper_abbreviate_dirs_native,\
	$(OUTDIR)/bin/makedepend$(gb_Executable_EXT) \
		$(filter-out -DPRECOMPILED_HEADERS,$(4)) $(5) \
		-I$(dir $(3)) \
		$(filter-out -I$(COMPATH)% %/pch -I$(JAVA_HOME)%,$(6)) \
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
	unset INCLUDE && \
	$(gb_CXX) \
		$(DEFS) $(CXXFLAGS) -Fd$(PDBFILE)\
        $(PCHFLAGS) \
		-I$(dir $(3)) \
		$(INCLUDE_STL) $(INCLUDE) \
		-c $(3) \
		-Fo$(1))
$(call gb_CxxObject__command_deponcompile,$(1),$(2),$(3),$(DEFS),$(CFLAGS),$(INCLUDE))
endef


# PrecompiledHeader class

gb_PrecompiledHeader_get_enableflags = -Yu$(1).hxx \
									   -Fp$(call gb_PrecompiledHeader_get_target,$(1))

ifeq ($(gb_FULLDEPS),$(true))
define gb_PrecompiledHeader__command_deponcompile
$(call gb_Helper_abbreviate_dirs_native,\
	$(OUTDIR)/bin/makedepend$(gb_Executable_EXT) \
		$(4) $(5) \
		-I$(dir $(3)) \
		$(filter-out -I$(COMPATH)% -I$(JAVA_HOME)%,$(6)) \
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
	unset INCLUDE && \
	$(gb_CXX) \
		$(4) $(5) -Fd$(PDBFILE) \
		-I$(dir $(3)) \
		$(6) \
		-c $(3) \
		-Yc$(notdir $(patsubst %.cxx,%.hxx,$(3))) -Fp$(1) -Fo$(1).obj)
$(call gb_PrecompiledHeader__command_deponcompile,$(1),$(2),$(3),$(4),$(5),$(6))
endef

# NoexPrecompiledHeader class

gb_NoexPrecompiledHeader_get_enableflags = -Yu$(1).hxx \
										   -Fp$(call gb_NoexPrecompiledHeader_get_target,$(1))

ifeq ($(gb_FULLDEPS),$(true))
define gb_NoexPrecompiledHeader__command_deponcompile
$(call gb_Helper_abbreviate_dirs_native,\
	$(OUTDIR)/bin/makedepend$(gb_Executable_EXT) \
		$(4) $(5) \
		-I$(dir $(3)) \
		$(filter-out -I$(COMPATH)% -I$(JAVA_HOME)%,$(6)) \
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
	unset INCLUDE && \
	$(gb_CXX) \
		$(4) $(5) -Fd$(PDBFILE) \
		-I$(dir $(3)) \
		$(6) \
		-c $(3) \
		-Yc$(notdir $(patsubst %.cxx,%.hxx,$(3))) -Fp$(1) -Fo$(1).obj)
$(call gb_NoexPrecompiledHeader__command_deponcompile,$(1),$(2),$(3),$(4),$(5),$(6))
endef

# LinkTarget class

gb_LinkTarget_CFLAGS := $(gb_CFLAGS) $(gb_CFLAGS_WERROR) $(gb_COMPILEROPTFLAGS)
gb_LinkTarget_CXXFLAGS := $(gb_CXXFLAGS) $(gb_CXXFLAGS_WERROR)

gb_LinkTarget_INCLUDE :=\
	$(filter-out %/stl, $(subst -I. , ,$(SOLARINC))) \
	$(foreach inc,$(subst ;, ,$(JDKINC)),-I$(inc)) \

gb_LinkTarget_INCLUDE_STL := $(filter %/stl, $(subst -I. , ,$(SOLARINC)))

gb_LinkTarget_get_pdbfile = $(call gb_LinkTarget_get_target,)pdb/$(1).pdb

define gb_LinkTarget__command
$(call gb_Output_announce,$(2),$(true),LNK,4)
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	rm -f $(1) && \
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),100, \
	    $(call gb_Helper_convert_native,$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(PCHOBJS) $(NATIVERES))) && \
	$(gb_LINK) \
		$(if $(filter Library CppunitTest,$(TARGETTYPE)),$(gb_Library_TARGETTYPEFLAGS)) \
		$(if $(filter StaticLibrary,$(TARGETTYPE)),$(gb_StaticLibrary_TARGETTYPEFLAGS)) \
		$(if $(filter GoogleTest Executable,$(TARGETTYPE)),$(gb_Executable_TARGETTYPEFLAGS)) \
		$(LDFLAGS) \
		@$${RESPONSEFILE} \
		$(foreach lib,$(LINKED_LIBS),$(call gb_Library_get_filename,$(lib))) \
		$(patsubst %,%.lib,$(EXTERNAL_LIBS)) \
		$(foreach lib,$(LINKED_STATIC_LIBS),$(call gb_StaticLibrary_get_filename,$(lib))) \
		$(if $(DLLTARGET),-out:$(DLLTARGET) -implib:$(1),-out:$(1)); RC=$$?; rm $${RESPONSEFILE} \
	$(if $(DLLTARGET),; if [ ! -f $(DLLTARGET) ]; then rm -f $(1) && false; fi) ; exit $$RC)
endef


# Library class

gb_Library_DEFS := -D_DLL
gb_Library_TARGETTYPEFLAGS := -DLL -OPT:NOREF -SAFESEH -NXCOMPAT -DYNAMICBASE
gb_Library_get_rpath :=

gb_Library_SYSPRE := i
gb_Library_PLAINEXT := .lib

gb_Library_PLAINLIBS_NONE += \
	advapi32 \
	gdi32 \
	gdiplus \
	gnu_getopt \
	imm32\
	kernel32 \
	msimg32 \
	msvcrt \
	mpr \
	oldnames \
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
gb_Library_RTEXT := MSC$(gb_Library_DLLEXT)

ifeq ($(USE_SYSTEM_STL),YES)
ifeq ($(gb_PRODUCT),$(true))
gb_Library_STLEXT := msvcprt.lib
else
gb_Library_STLEXT := msvcprt.lib
endif
else
ifeq ($(gb_PRODUCT),$(true))
gb_Library_STLEXT := port_vc7145$(gb_Library_DLLEXT)
else
gb_Library_STLEXT := port_vc7145_stldebug$(gb_Library_DLLEXT)
endif
endif

gb_Library_OOOEXT := $(gb_Library_DLLEXT)
gb_Library_UNOEXT := .uno$(gb_Library_DLLEXT)
gb_Library_UNOVEREXT := $(gb_Library_MAJORVER)$(gb_Library_DLLEXT)
gb_Library_RTVEREXT := $(gb_Library_MAJORVER)$(gb_Library_RTEXT)

gb_Library_DLLFILENAMES :=\
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):$(lib)$(gb_Library_OOOEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_RTLIBS),$(lib):$(lib)$(gb_Library_RTEXT)) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(lib)$(gb_Library_RTVEREXT)) \
	$(foreach lib,$(gb_Library_STLLIBS),$(lib):$(lib)$(gb_Library_STLEXT)) \
	$(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):$(lib)$(gb_Library_UNOEXT)) \
	$(foreach lib,$(gb_Library_UNOLIBS_OOO),$(lib):$(lib)$(gb_Library_UNOEXT)) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(lib)$(gb_Library_UNOVEREXT)) \

define gb_Library_Library_platform
$(call gb_LinkTarget_set_dlltarget,$(2),$(3))

$(call gb_LinkTarget_set_auxtargets,$(2),\
	$(patsubst %.lib,%.exp,$(call gb_LinkTarget_get_target,$(2))) \
	$(3).manifest \
	$(call gb_LinkTarget_get_pdbfile,$(2)) \
	$(patsubst %.dll,%.pdb,$(3)) \
	$(patsubst %.dll,%.ilk,$(3)) \
)

$(call gb_Library_get_target,$(1)) \
$(call gb_Library_get_clean_target,$(1)) : AUXTARGETS := $(OUTDIR)/bin/$(notdir $(3))

ifneq ($(ENABLE_CRASHDUMP),)
$(call gb_Library_get_target,$(1)) \
$(call gb_Library_get_clean_target,$(1)) : AUXTARGETS +=  \
		$(OUTDIR)/bin/$(notdir $(patsubst %.dll,%.pdb,$(3))) \
		$(OUTDIR)/bin/$(notdir $(patsubst %.dll,%.ilk,$(3))) \

$(call gb_Deliver_add_deliverable,$(OUTDIR)/bin/$(notdir $(patsubst %.dll,%.pdb,$(3))),$(patsubst %.dll,%.pdb,$(3)))
$(call gb_Deliver_add_deliverable,$(OUTDIR)/bin/$(notdir $(patsubst %.dll,%.ilk,$(3))),$(patsubst %.dll,%.ilk,$(3)))
endif

$(call gb_Deliver_add_deliverable,$(OUTDIR)/bin/$(notdir $(3)),$(3))

$(call gb_LinkTarget_get_target,$(2)) \
$(call gb_LinkTarget_get_headers_target,$(2)) : PDBFILE = $(call gb_LinkTarget_get_pdbfile,$(2))

endef

define gb_Library_add_default_nativeres
$(call gb_WinResTarget_WinResTarget_init,$(1)/$(2))
$(call gb_WinResTarget_add_file,$(1)/$(2),solenv/inc/shlinfo)
$(call gb_WinResTarget_set_defs,$(1)/$(2),\
        $$(DEFS) \
		-DADDITIONAL_VERINFO1 \
		-DADDITIONAL_VERINFO2 \
		-DADDITIONAL_VERINFO3 \
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
gb_StaticLibrary_TARGETTYPEFLAGS := -LIB
gb_StaticLibrary_SYSPRE :=
gb_StaticLibrary_PLAINEXT := .lib
gb_StaticLibrary_JPEGEXT := lib$(gb_StaticLibrary_PLAINEXT)

gb_StaticLibrary_FILENAMES := \
	$(foreach lib,$(gb_StaticLibrary_JPEGLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_JPEGEXT)) \
	$(foreach lib,$(gb_StaticLibrary_PLAINLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_PLAINEXT)) \

gb_StaticLibrary_FILENAMES := $(patsubst salcpprt:salcpprt%,salcpprt:cpprtl%,$(gb_StaticLibrary_FILENAMES))

define gb_StaticLibrary_StaticLibrary_platform
$(call gb_LinkTarget_get_target,$(2)) \
$(call gb_LinkTarget_get_headers_target,$(2)) : PDBFILE = $(call gb_LinkTarget_get_pdbfile,$(2))

$(call gb_LinkTarget_set_auxtargets,$(2),\
	$(call gb_LinkTarget_get_pdbfile,$(2)) \
)

endef

# Executable class

gb_Executable_EXT := .exe
gb_Executable_TARGETTYPEFLAGS := -RELEASE -BASE:0x1b000000 -OPT:NOREF -INCREMENTAL:NO -DEBUG -SAFESEH -NXCOMPAT -DYNAMICBASE
gb_Executable_get_rpath :=

define gb_Executable_Executable_platform
$(call gb_LinkTarget_set_auxtargets,$(2),\
	$(patsubst %.exe,%.pdb,$(call gb_LinkTarget_get_target,$(2))) \
	$(call gb_LinkTarget_get_pdbfile,$(2)) \
	$(call gb_LinkTarget_get_target,$(2)).manifest \
)

$(call gb_Executable_get_target,$(1)) \
$(call gb_Executable_get_clean_target,$(1)) : AUXTARGETS := $(call gb_Executable_get_target,$(1)).manifest
$(call gb_Deliver_add_deliverable,$(call gb_Executable_get_target,$(1)).manifest,$(call gb_LinkTarget_get_target,$(2)).manifest)

$(call gb_LinkTarget_get_target,$(2)) \
$(call gb_LinkTarget_get_headers_target,$(2)) : PDBFILE = $(call gb_LinkTarget_get_pdbfile,$(2))

endef

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
	$(patsubst %.lib,%.exp,$(call gb_LinkTarget_get_target,$(2))) \
	$(3).manifest \
	$(patsubst %.dll,%.pdb,$(3)) \
	$(call gb_LinkTarget_get_pdbfile,$(2)) \
	$(patsubst %.dll,%.ilk,$(3)) \
)

$(call gb_LinkTarget_get_target,$(2)) \
$(call gb_LinkTarget_get_headers_target,$(2)) : PDBFILE = $(call gb_LinkTarget_get_pdbfile,$(2))

endef

# JunitTest class

gb_defaultlangiso := en-US
gb_smoketest_instset := $(SRCDIR)/instsetoo_native/$(INPATH)/OpenOffice/archive/install/$(gb_defaultlangiso)/OOo_*_install-arc_$(gb_defaultlangiso).zip

ifeq ($(OOO_TEST_SOFFICE),)


# Work around Windows problems with long pathnames (see issue 50885) by
# installing into the temp directory instead of the module output tree (in which
# case $(target).instpath contains the path to the temp installation,
# which is removed after smoketest); can be removed once issue 50885 is fixed;
# on other platforms, a single installation to solver is created in
# smoketestoo_native.

# for now, no dependency on $(shell ls $(gb_smoketest_instset))
# because that doesn't work before the instset is built
# and there is not much of a benefit anyway (gbuild not knowing about smoketest)
define gb_JunitTest_JunitTest_platform_longpathname_hack
$(call gb_JunitTest_get_target,$(1)) : $(call gb_JunitTest_get_target,$(1)).instpath
$(call gb_JunitTest_get_target,$(1)) : CLEAN_CMD = $(call gb_Helper_abbreviate_dirs,rm -rf `cat $$@.instpath` $$@.instpath)

$(call gb_JunitTest_get_target,$(1)).instpath : 
	INST_DIR=$$$$(cygpath -m `mktemp -d -t testinst.XXXXXX`) \
	&& unzip -d "$$$${INST_DIR}"  $$(gb_smoketest_instset) \
	&& mv "$$$${INST_DIR}"/OOo_*_install-arc_$$(gb_defaultlangiso) "$$$${INST_DIR}"/opt\
	&& mkdir -p $$(dir $$@) \
	&& echo "$$$${INST_DIR}" > $$@

endef
else # OOO_TEST_SOFFICE
gb_JunitTest_JunitTest_platform_longpathname_hack =
endif # OOO_TEST_SOFFICE

define gb_JunitTest_JunitTest_platform
$(call gb_JunitTest_JunitTest_platform_longpathname_hack,$(1))

$(call gb_JunitTest_get_target,$(1)) : DEFS := \
	-Dorg.openoffice.test.arg.soffice="$$$${OOO_TEST_SOFFICE:-path:`cat $(call gb_JunitTest_get_target,$(1)).instpath`/opt/OpenOffice.org 3/program/soffice.exe}" \
    -Dorg.openoffice.test.arg.env=PATH \
    -Dorg.openoffice.test.arg.user=file:///$(call gb_JunitTest_get_userdir,$(1)) \

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

gb_WinResTarget_POSTFIX :=.res

define gb_WinResTarget__command
$(call gb_Output_announce,$(2),$(true),RES,3)
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	$(gb_RC) \
		$(DEFS) $(FLAGS) \
		-I$(dir $(3)) \
		$(INCLUDE) \
		-Fo$(1) \
        $(RCFILE) )
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
