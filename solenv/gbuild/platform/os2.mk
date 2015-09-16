#*************************************************************************
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
#*************************************************************************

GUI := OS2
COM := GCC

# set tmpdir to some mixed case path, suitable for native tools
#gb_TMPDIR:=$(if $(TMPDIR),$(shell cygpath -m $(TMPDIR)),$(shell cygpath -m /tmp))
#gb_MKTEMP := mktemp --tmpdir=$(gb_TMPDIR) gbuild.XXXXXX
gb_MKTEMP := mktemp -t gbuild.XXXXXX

gb_CC := gcc
gb_CXX := g++
gb_GCCP := gcc
gb_LINK := gcc
gb_AR := emxomfar
gb_AWK := awk
gb_CLASSPATHSEP := ;
gb_RC := wrc

# use CC/CXX if they are nondefaults
ifneq ($(origin CC),default)
gb_CC := $(CC)
gb_GCCP := $(CC)
endif
ifneq ($(origin CXX),default)
gb_CXX := $(CXX)
endif

# until we get a solution to libc ticket#251, use dmik's -q option.
EMXOMFOPT = -q
export EMXOMFOPT

gb_OSDEFS := \
	-D$(OS) \
	-D_PTHREADS \
	-DOS2 \
	$(PTHREAD_CFLAGS) \

ifeq ($(GXX_INCLUDE_PATH),)
GXX_INCLUDE_PATH=$(COMPATH)/include/c++/$(shell gcc -dumpversion)
endif

gb_COMPILERDEFS := \
	-D$(COM) \
	-DHAVE_GCC_VISIBILITY_FEATURE \
	-DCPPU_ENV=gcc3 \
	-DGXX_INCLUDE_PATH=$(GXX_INCLUDE_PATH) \

gb_CPUDEFS := -DINTEL -D_X86_=1 -DX86

gb_RCDEFS := \
	-DOS2 \
	-bt=os2 \
	-i$(PATH_EMXPGCC)\include \

gb_RCFLAGS := \
     -V

gb_CFLAGS := \
	-Zomf \
	-Wall \
	-Wendif-labels \
	-Wextra \
	-Wshadow \
	-fmessage-length=0 \
	-fno-common \
	-fno-strict-aliasing \
	-I$(JAVA_HOME)/include \
	-I$(JAVA_HOME)/include/os2

gb_CXXFLAGS := \
	-Zomf \
	-Wall \
	-Wendif-labels \
	-Wextra \
	-Wno-ctor-dtor-privacy \
	-Wno-non-virtual-dtor \
	-Wshadow \
	-fmessage-length=0 \
	-fno-common \
	-fno-strict-aliasing \
	-fno-use-cxa-atexit \
	-I$(JAVA_HOME)/include \
	-I$(JAVA_HOME)/include/os2

gb_STDLIBS = \
	z \
	stdc++

ifneq ($(EXTERNAL_WARNINGS_NOT_ERRORS),TRUE)
gb_CFLAGS_WERROR := -Werror
gb_CXXFLAGS_WERROR := -Werror
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
	-g -Zomf \
	-Zhigh-mem \
	-Zlinker "DISABLE 1121" \
	-Zmap \
	$(patsubst %,-LIBPATH:%,$(filter-out .,$(subst ;, ,$(subst \,/,$(ILIB))))) \
	$(subst -L../lib , ,$(SOLARLIB)) \

ifneq ($(ENABLE_CRASHDUMP),)
gb_LinkTarget_LDFLAGS += -DEBUG
gb_CFLAGS+=-Zi
gb_CXXFLAGS+=-Zi
endif

ifeq ($(gb_DEBUGLEVEL),2)
gb_COMPILEROPTFLAGS := -O0
#gb_LinkTarget_LDFLAGS += -DEBUG
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

define gb_CObject__command
$(call gb_Output_announce,$(2),$(true),C  ,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	mkdir -p $(dir $(call gb_CObject_get_dep_target,$(2))) && \
	$(gb_CC) \
		$(DEFS) $(CFLAGS) \
		-c $(3) \
		-o $(1) \
		-MMD -MT $(call gb_CObject_get_target,$(2)) \
		-MF $(call gb_CObject_get_dep_target,$(2)) \
		-I$(dir $(3)) \
		$(INCLUDE))
endef


# CxxObject class

define gb_CxxObject__command
$(call gb_Output_announce,$(2),$(true),CXX,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	mkdir -p $(dir $(call gb_CxxObject_get_dep_target,$(2))) && \
	$(gb_CXX) \
		$(DEFS) $(CXXFLAGS) \
		-c $(3) \
		-o $(1) \
		-MMD -MT $(call gb_CxxObject_get_target,$(2)) \
		-MF $(call gb_CxxObject_get_dep_target,$(2)) \
		-I$(dir $(3)) \
		$(INCLUDE_STL) $(INCLUDE))
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

ifeq ($(gb_DEBUGLEVEL),2)
gb_LinkTarget_CFLAGS += -g
gb_LinkTarget_CXXFLAGS += -g
gb_LinkTarget_OBJCXXFLAGS += -g
endif

gb_LinkTarget_INCLUDE :=\
	$(filter-out %/stl, $(subst -I. , ,$(SOLARINC))) \
	$(foreach inc,$(subst ;, ,$(JDKINC)),-I$(inc)) \

gb_LinkTarget_INCLUDE_STL := $(filter %/stl, $(subst -I. , ,$(SOLARINC)))

gb_LinkTarget_get_pdbfile = $(call gb_LinkTarget_get_target,)pdb/$(1).pdb

DLLBASE = $(notdir $(DLLTARGET:.dll=))
DLLDEF = $(dir $(DLLTARGET))$(DLLBASE).def

define gb_LinkTarget__command_dynamiclinkexecutable
$(call gb_Output_announce,$(2),$(true),LNK,4)
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	rm -f $(1) && \
	$(if $(DLLTARGET), echo LIBRARY	$(DLLBASE) INITINSTANCE TERMINSTANCE > $(DLLDEF) &&) \
	$(if $(DLLTARGET), echo DATA MULTIPLE >> $(DLLDEF) &&) \
	RESPONSEFILE=$(call var2filecr,$(shell $(gb_MKTEMP)),1, \
	    $(call gb_Helper_convert_native,$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(PCHOBJS))) && \
	$(if $(DLLTARGET), echo EXPORTS >> $(DLLDEF) &&) \
	$(if $(DLLTARGET), emxexp @$${RESPONSEFILE} | fix_exp_file | sort | uniq | fix_def_ord >> $(DLLDEF) &&) \
	$(gb_LINK) \
		$(if $(filter Library CppunitTest,$(TARGETTYPE)),$(gb_Library_TARGETTYPEFLAGS)) \
		$(if $(filter StaticLibrary,$(TARGETTYPE)),$(gb_StaticLibrary_TARGETTYPEFLAGS)) \
		$(if $(filter Executable,$(TARGETTYPE)),$(gb_Executable_TARGETTYPEFLAGS)) \
		$(LDFLAGS) \
		@$${RESPONSEFILE} \
		$(if $(DLLTARGET), $(DLLDEF)) \
		$(NATIVERES) \
		$(patsubst %.lib,-l%,$(foreach lib,$(LINKED_LIBS),$(call gb_Library_get_filename,$(lib)))) \
		$(patsubst %,-l%,$(EXTERNAL_LIBS)) \
		$(foreach lib,$(LINKED_STATIC_LIBS),$(call gb_StaticLibrary_get_target,$(lib))) \
		$(if $(DLLTARGET),-o $(DLLTARGET), -o $(1) ); \
		RC=$$?; rm $${RESPONSEFILE} \
	$(if $(DLLTARGET),; emximp -p2048 -o $(1) $(DLLDEF) ) \
	$(if $(DLLTARGET),; if [ ! -f $(DLLTARGET) ]; then rm -f $(1) && false; fi) ; exit $$RC)
endef


define gb_LinkTarget__command_staticlinklibrary
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	RESPONSEFILE=`$(gb_MKTEMP)` && \
	echo "$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) " > $${RESPONSEFILE} && \
	$(gb_AR) cru\
		$(1) \
		@$${RESPONSEFILE})
endef

define gb_LinkTarget__command
$(call gb_Output_announce,$(2),$(true),LNK,4)
$(if $(filter GoogleTest Executable,$(TARGETTYPE)),$(call gb_LinkTarget__command_dynamiclinkexecutable,$(1),$(2)))
$(if $(filter Library CppunitTest,$(TARGETTYPE)),$(call gb_LinkTarget__command_dynamiclinkexecutable,$(1),$(2)))
$(if $(filter StaticLibrary,$(TARGETTYPE)),$(call gb_LinkTarget__command_staticlinklibrary,$(1)))
endef


# Library class

gb_Library_DEFS := -D_DLL
gb_Library_TARGETTYPEFLAGS := -Zdll
gb_Library_get_rpath :=

gb_Library_SYSPRE := 
gb_Library_PLAINEXT := .lib

gb_Library_PLAINLIBS_NONE += \
	stdc++ \
	ft2lib \
	dl \
	freetype \
	jpeg \
	m \
	z \
	pthread \
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
	$(foreach lib,$(gb_Library_TARGETS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT))


gb_Library_DLLEXT := .dll
gb_Library_MAJORVER :=
gb_Library_RTEXT := $(gb_Library_DLLEXT)
ifeq ($(gb_PRODUCT),$(true))
gb_Library_STLEXT := stdc++$(gb_Library_DLLEXT)
else
gb_Library_STLEXT := stlp45_stldebug$(gb_Library_DLLEXT)
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

#$(call gb_LinkTarget_set_auxtargets,$(2),\
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

define gb_Library_add_default_nativeres_DISABLED
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
gb_StaticLibrary_SYSPRE :=
gb_StaticLibrary_PLAINEXT := .lib
gb_StaticLibrary_JPEGEXT := lib$(gb_StaticLibrary_PLAINEXT)

gb_StaticLibrary_FILENAMES := \
	$(foreach lib,$(gb_StaticLibrary_JPEGLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_JPEGEXT)) \
	$(foreach lib,$(gb_StaticLibrary_PLAINLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_PLAINEXT)) \

gb_StaticLibrary_StaticLibrary_platform =


# Executable class

gb_Executable_EXT := .exe
gb_Executable_TARGETTYPEFLAGS := 
#-RELEASE -BASE:0x1b000000 -OPT:NOREF -INCREMENTAL:NO -DEBUG
gb_Executable_get_rpath :=

define gb_Executable_Executable_platform
#$(call gb_LinkTarget_set_auxtargets,$(2),\
#	$(patsubst %.exe,%.pdb,$(call gb_LinkTarget_get_target,$(2))) \
#	$(call gb_LinkTarget_get_pdbfile,$(2)) \
#	$(call gb_LinkTarget_get_target,$(2)).manifest \
#)

#$(call gb_Executable_get_target,$(1)) \
#$(call gb_Executable_get_clean_target,$(1)) : AUXTARGETS := $(call gb_Executable_get_target,$(1)).manifest
#$(call gb_Deliver_add_deliverable,$(call gb_Executable_get_target,$(1)).manifest,$(call gb_LinkTarget_get_target,$(2)).manifest)

#$(call gb_LinkTarget_get_target,$(2)) \
#$(call gb_LinkTarget_get_headers_target,$(2)) : PDBFILE = $(call gb_LinkTarget_get_pdbfile,$(2))

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

#$(call gb_LinkTarget_set_auxtargets,$(2),\
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
		-fo=$(1) \
        -r $(RCFILE) )
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
