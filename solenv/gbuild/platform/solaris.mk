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
COM := C52

gb_MKTEMP := mktemp -t gbuild.XXXXXX

gb_CC := cc
gb_CXX := CC
gb_GCCP := cc
gb_AR := ar
gb_AWK := /usr/xpg4/bin/awk
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
	-D$(GUI) \
	-DSYSV \
	-DSUN \
	-DSUN4 \
	-D_REENTRANT \
	-D_POSIX_PTHREAD_SEMANTICS \
	-D_PTHREADS \
	-DUNIX \

gb_COMPILERDEFS := \
	-D$(COM) \
	-DCPPU_ENV=sunpro5 \

gb_CPUDEFS := -D$(CPUNAME)
ifeq ($(CPUNAME),SPARC)
gb_CPUDEFS += -D__sparcv8plus
endif

gb_CFLAGS := \
	-temp=/tmp \
	-KPIC \
	-mt \
	-xldscope=hidden \
	-xCC \
	-xc99=none \

gb_CXXFLAGS := \
	-temp=/tmp \
	-KPIC \
	-mt \
	-xldscope=hidden \
	-features=no%altspell \
	-library=no%Cstd \
	+w2 \
	-erroff=doubunder,identexpected,inllargeuse,inllargeint,notemsource,reftotemp,truncwarn,wnoretvalue,anonnotype \

ifneq ($(EXTERNAL_WARNINGS_NOT_ERRORS),TRUE)
gb_CFLAGS_WERROR := -errwarn=%all
gb_CXXFLAGS_WERROR := -xwe
endif

gb_LinkTarget_EXCEPTIONFLAGS := \
	-DEXCEPTIONS_ON \

gb_LinkTarget_NOEXCEPTIONFLAGS := \
	-DEXCEPTIONS_OFF \
	-noex \

gb_LinkTarget_LDFLAGS := \
	$(subst -L../lib , ,$(SOLARLIB)) \
	-temp=/tmp \
	-w \
	-mt \
	-Bdirect \
	-z defs \
	-z combreloc \
	-norunpath \
	-PIC \
	-library=no%Cstd \

ifeq ($(gb_DEBUGLEVEL),2)
gb_COMPILEROPTFLAGS :=
else
ifeq ($(CPUNAME),INTEL)
gb_COMPILEROPTFLAGS := -xarch=generic -xO3
else # ifeq ($(CPUNAME),SPARC)
#  -m32 -xarch=sparc        restrict target to 32 bit sparc
#  -xO3                     optimization level 3
#  -xspace                  don't do optimizations which do increase binary size
#  -xprefetch=yes           do prefetching (helps on UltraSparc III)
gb_COMPILEROPTFLAGS := -m32 -xarch=sparc -xO3 -xspace -xprefetch=yes
endif
endif

gb_COMPILERNOOPTFLAGS :=

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
	rm -f $(call gb_CObject_get_dep_target,$(2)) && \
	mkdir -p $(dir $(call gb_CObject_get_dep_target,$(2))) && \
	$(gb_CC) \
		-c $(3) \
		-o $(1) \
		-xMMD \
		-xMF $(call gb_CObject_get_dep_target,$(2)) \
		$(DEFS) $(CFLAGS) \
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
		-xMMD \
		-xMF $(call gb_CxxObject_get_dep_target,$(2)) \
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

gb_LinkTarget_CFLAGS := $(gb_CFLAGS) $(gb_CFLAGS_WERROR) $(gb_COMPILEROPTFLAGS)
gb_LinkTarget_CXXFLAGS := $(gb_CXXFLAGS) $(gb_CXXFLAGS_WERROR)

ifeq ($(gb_DEBUGLEVEL),2)
gb_LinkTarget_CXXFLAGS += -g
gb_LinkTarget_CFLAGS += -g
endif

gb_LinkTarget_INCLUDE := $(filter-out %/stl, $(subst -I. , ,$(SOLARINC)))
gb_LinkTarget_INCLUDE_STL := $(filter %/stl, $(subst -I. , ,$(SOLARINC)))

define gb_LinkTarget__command_dynamiclink
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(gb_CXX) \
		$(if $(filter Library CppunitTest,$(TARGETTYPE)),$(gb_Library_TARGETTYPEFLAGS)) \
		$(subst \d,$$,$(RPATH)) $(LDFLAGS) \
		$(patsubst lib%.so,-l%,$(foreach lib,$(LINKED_LIBS),$(call gb_Library_get_filename,$(lib)))) \
		$(patsubst %,-l%,$(EXTERNAL_LIBS)) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach lib,$(LINKED_STATIC_LIBS),$(call gb_StaticLibrary_get_target,$(lib))) \
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
$(if $(filter Library CppunitTest GoogleTest Executable,$(TARGETTYPE)),$(call gb_LinkTarget__command_dynamiclink,$(1)))
$(if $(filter StaticLibrary,$(TARGETTYPE)),$(call gb_LinkTarget__command_staticlink,$(1)))
endef

# Library class

gb_Library_DEFS :=
gb_Library_TARGETTYPEFLAGS := -Bdynamic -z text -G
gb_Library_SYSPRE := lib
gb_Library_UNOVERPRE := $(gb_Library_SYSPRE)uno_
gb_Library_PLAINEXT := .so
gb_Library_RTEXT := C52$(gb_Library_PLAINEXT)
ifeq ($(gb_PRODUCT),$(true))
gb_Library_STLEXT := port_sunpro$(gb_Library_PLAINEXT)
else
gb_Library_STLEXT := port_sunpro_debug$(gb_Library_PLAINEXT)
endif

ifeq ($(CPUNAME),INTEL)
gb_Library_OOOEXT := $(gb_Library_PLAINEXT)
gb_Library_UNOEXT := .uno$(gb_Library_PLAINEXT)
else # ifeq ($(CPUNAME),SPARC)
gb_Library_OOOEXT := $(gb_Library_PLAINEXT)
gb_Library_UNOEXT := .uno$(gb_Library_PLAINEXT)
endif

gb_STDLIBS := \
	Crun \
	m \
	c \

gb_Library_PLAINLIBS_NONE += \
	$(gb_STDLIBS) \
	dl \
    freetype \
	jpeg \
	pthread \
	X11 \
    Xext \
    SM \
    ICE \
	z \
	cppunit

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
'-R$(call gb_LinkTarget__get_rpath_for_layer,$(call gb_Library_get_layer,$(1)))'
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
'-R$(call gb_LinkTarget__get_rpath_for_layer,$(call gb_Executable_get_layer,$(1)))'
endef

define gb_Executable_Executable_platform
$(call gb_LinkTarget_get_target,$(2)) : RPATH := $(call gb_Executable_get_rpath,$(1))

endef


# GoogleTest class

gb_GoogleTest_GTESTPRECOMMAND := LD_LIBRARY_PATH=$(OUTDIR)/lib
gb_GoogleTest_get_filename = $(1)$(gb_Executable_EXT)

define gb_GoogleTest_GoogleTest_platform
$(call gb_LinkTarget_get_target,$(2)) : RPATH :=
endef

# CppunitTest class

gb_CppunitTest_CPPTESTPRECOMMAND := LD_LIBRARY_PATH=$(OUTDIR)/lib
gb_CppunitTest_SYSPRE := libtest_
gb_CppunitTest_EXT := .so
gb_CppunitTest_get_filename = $(gb_CppunitTest_SYSPRE)$(1)$(gb_CppunitTest_EXT)
gb_CppunitTest_get_libfilename = $(gb_CppunitTest_get_filename)

define gb_CppunitTest_CppunitTest_platform
$(call gb_LinkTarget_get_target,$(2)) : RPATH :=

endef

# JunitTest class

define gb_JunitTest_JunitTest_platform
$(call gb_JunitTest_get_target,$(1)) : DEFS := \
	-Dorg.openoffice.test.arg.soffice="$$$${OOO_TEST_SOFFICE:-path:$(OUTDIR)/installation/opt/openoffice.org3/program/soffice}" \
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

# Sun cc/CC support -xM1/-xMF flags, but unfortunately refuse input files that
# do not have the right suffix, so use makedepend here...
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


# ComponentTarget

gb_XSLTPROCPRECOMMAND := LD_LIBRARY_PATH=$(OUTDIR)/lib
gb_Library_COMPONENTPREFIXES := \
    OOO:vnd.sun.star.expand:\dOOO_BASE_DIR/program/ \
    URELIB:vnd.sun.star.expand:\dURE_INTERNAL_LIB_DIR/ \


# vim: set noet sw=4 ts=4:
