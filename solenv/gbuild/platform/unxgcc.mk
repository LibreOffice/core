# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# to block heavy exception handling that try to acquire the solarmutex
export LO_LEAN_EXCEPTION=1

include $(GBUILDDIR)/platform/com_GCC_defs.mk

gb_MKTEMP := mktemp -t gbuild.XXXXXX

ifneq ($(origin AR),default)
gb_AR := $(AR)
endif

gb_OSDEFS := \
	-D$(OS) \
	-D_PTHREADS \
	-DUNIX \
	-DUNX \
	$(PTHREAD_CFLAGS) \
	$(LFS_CFLAGS) \

gb_CFLAGS := \
	$(gb_CFLAGS_COMMON) \
	-fPIC \
	-Wdeclaration-after-statement \
	-Wshadow \

# At least libstdc++ needs -pthread when including various C++ headers like <thread>, see
# <https://gcc.gnu.org/onlinedocs/gcc-8.3.0/libstdc++/manual/manual/using.html
# #manual.intro.using.flags>:
ifneq ($(HAVE_LIBSTDCPP),)
gb_CXX_LINKFLAGS := -pthread
endif

gb_CXXFLAGS := \
	$(gb_CXXFLAGS_COMMON) \
	-fPIC \
	-Wshadow \
	-Woverloaded-virtual \
	$(CXXFLAGS_CXX11) \
	$(gb_CXX_LINKFLAGS) \


ifneq ($(strip $(SYSBASE)),)
gb_CXXFLAGS += --sysroot=$(SYSBASE)
gb_CFLAGS += --sysroot=$(SYSBASE)
gb_LinkTarget_LDFLAGS += \
	-Wl,--sysroot=$(SYSBASE)
endif

gb_LinkTarget_LDFLAGS += \
	-Wl,-rpath-link,$(SYSBASE)/lib:$(SYSBASE)/usr/lib \
	-Wl,-z,combreloc \

ifeq ($(HAVE_LD_HASH_STYLE),TRUE)
gb_LinkTarget_LDFLAGS += \
	-Wl,--hash-style=$(WITH_LINKER_HASH_STYLE) \

endif

ifneq ($(HAVE_LD_BSYMBOLIC_FUNCTIONS),)
gb_LinkTarget_LDFLAGS += -Wl,-Bsymbolic-functions
endif

gb_LINKEROPTFLAGS := -Wl,-O1
gb_LINKERSTRIPDEBUGFLAGS := -Wl,-S

# LinkTarget class

define gb_LinkTarget__get_rpath_for_layer
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_LinkTarget__RPATHS)))
endef

gb_LinkTarget__RPATHS := \
	URELIB:\dORIGIN \
	UREBIN:\dORIGIN \
	OOO:\dORIGIN \
	SDKBIN:\dORIGIN/../../program \
	OXT: \
	NONE:\dORIGIN/../Library \

gb_LinkTarget_CFLAGS := $(gb_CFLAGS)
gb_LinkTarget_CXXFLAGS := $(gb_CXXFLAGS)

gb_LinkTarget__cmd_lockfile = $(if $(LOCKFILE),$(LOCKFILE),$(call gb_Executable_get_command,lockfile))
gb_LinkTarget__Lock := $(WORKDIR)/LinkTarget/link.lock
gb_LinkTarget__WantLock = $(if $(and $(filter-out ANDROID MACOSX iOS WNT,$(OS)),$(filter TRUE,$(DISABLE_DYNLOADING)),$(filter CppunitTest Executable,$(TARGETTYPE))),$(true))

gb_LinkTarget__NeedsCxxLinker = $(if $(CXXOBJECTS)$(GENCXXOBJECTS)$(EXTRAOBJECTLISTS)$(filter-out XTRUE,X$(ENABLE_RUNTIME_OPTIMIZATIONS)),$(true))

# note that `cat $(extraobjectlist)` is needed to build with older gcc versions, e.g. 4.1.2 on SLED10
# we want to use @$(extraobjectlist) in the long run
# link with C compiler if there are no C++ files (pyuno_wrapper depends on this)
# But always link with C++ compiler e.g. under -fsanitize=undefined, as sal uses
# __ubsan_handle_dynamic_type_cache_miss_abort and __ubsan_vptr_type_cache from
# libclang_rt.ubsan_cxx-x86_64.a, and oosplash links against sal but itself only
# contains .c sources:
define gb_LinkTarget__command_dynamiclink
$(if $(gb_LinkTarget__WantLock),$(gb_LinkTarget__cmd_lockfile) -r -1 $(gb_LinkTarget__Lock))
$(call gb_Helper_abbreviate_dirs,\
	$(if $(call gb_LinkTarget__NeedsCxxLinker),$(or $(T_CXX),$(gb_CXX)) $(gb_CXX_LINKFLAGS),$(or $(T_CC),$(gb_CC))) \
		$(if $(filter Library CppunitTest,$(TARGETTYPE)),$(gb_Library_TARGETTYPEFLAGS)) \
		$(T_LTOFLAGS) \
		$(if $(SOVERSIONSCRIPT),-Wl$(COMMA)--soname=$(notdir $(1)) \
			-Wl$(COMMA)--version-script=$(SOVERSIONSCRIPT)) \
		$(subst \d,$$,$(RPATH)) \
		$(T_USE_LD) $(T_LDFLAGS) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(ASMOBJECTS),$(call gb_AsmObject_get_target,$(object))) \
		$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach object,$(GENNASMOBJECTS),$(call gb_GenNasmObject_get_target,$(object))) \
		$(foreach extraobjectlist,$(EXTRAOBJECTLISTS),`cat $(extraobjectlist)`) \
		$(if $(filter TRUE,$(DISABLE_DYNLOADING)), \
		    -Wl$(COMMA)--start-group \
		    $(patsubst lib%.a,-l%,$(patsubst lib%.so,-l%,$(patsubst %.$(gb_Library_UDK_MAJORVER),%,$(foreach lib,$(LINKED_LIBS),$(call gb_Library_get_filename,$(lib)))))) \
		    $(foreach lib,$(LINKED_STATIC_LIBS),$(call gb_StaticLibrary_get_target,$(lib))) \
		    $(T_LIBS) \
		    $(if $(call gb_LinkTarget__NeedsCxxLinker),$(T_STDLIBS_CXX)) \
		    -Wl$(COMMA)--end-group \
		, \
		    -Wl$(COMMA)--start-group \
		    $(foreach lib,$(LINKED_STATIC_LIBS),$(call gb_StaticLibrary_get_target,$(lib))) \
		    $(T_LIBS) \
		    $(if $(call gb_LinkTarget__NeedsCxxLinker),$(T_STDLIBS_CXX)) \
		    -Wl$(COMMA)--end-group \
		    -Wl$(COMMA)--no-as-needed \
		    $(patsubst lib%.a,-l%,$(patsubst lib%.so,-l%,$(patsubst %.$(gb_Library_UDK_MAJORVER),%,$(foreach lib,$(LINKED_LIBS),$(call gb_Library_get_filename,$(lib)))))) \
                ) \
		-o $(1) \
	$(if $(SOVERSIONSCRIPT),&& ln -sf ../../program/$(notdir $(1)) $(ILIBTARGET)) \
	$(if $(gb_LinkTarget__WantLock),; RC=$$? ; rm -f $(gb_LinkTarget__Lock); if test $$RC -ne 0; then exit $$RC; fi))

$(if $(filter Library,$(TARGETTYPE)), $(call gb_Helper_abbreviate_dirs,\
    $(READELF) -d $(1) | grep SONAME > $(WORKDIR)/LinkTarget/$(2).exports.tmp; \
    $(NM) $(gb_LTOPLUGINFLAGS) --dynamic --extern-only --defined-only --format=posix $(1) \
        | cut -d' ' -f1-2 >> $(WORKDIR)/LinkTarget/$(2).exports.tmp && \
    $(call gb_Helper_replace_if_different_and_touch,$(WORKDIR)/LinkTarget/$(2).exports.tmp, \
        $(WORKDIR)/LinkTarget/$(2).exports,$(1))))
endef

define gb_LinkTarget__command_staticlink
$(call gb_Helper_abbreviate_dirs,\
	rm -f $(1) && \
	$(gb_AR) $(gb_LTOPLUGINFLAGS) -rsu $(1) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(ASMOBJECTS),$(call gb_AsmObject_get_target,$(object))) \
		$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach object,$(GENNASMOBJECTS),$(call gb_GenNasmObject_get_target,$(object))) \
		$(foreach extraobjectlist,$(EXTRAOBJECTLISTS),@$(extraobjectlist)) \
		$(if $(findstring s,$(MAKEFLAGS)),2> /dev/null))
endef

define gb_LinkTarget__command
$(call gb_Output_announce,$(2),$(true),LNK,4)
	$(call gb_Trace_StartRange,$(2),LNK)
$(if $(filter CppunitTest Executable,$(TARGETTYPE)),$(call gb_LinkTarget__command_dynamiclink,$(1),$(2)))
$(if $(filter Library,$(TARGETTYPE)),$(if $(filter TRUE,$(DISABLE_DYNLOADING)),$(call gb_LinkTarget__command_staticlink,$(1)),$(call gb_LinkTarget__command_dynamiclink,$(1),$(2))))
$(if $(filter StaticLibrary,$(TARGETTYPE)),$(call gb_LinkTarget__command_staticlink,$(1)))
	$(call gb_Trace_EndRange,$(2),LNK)
endef


# Library class

gb_Library_DEFS :=
gb_Library_SYSPRE := lib
gb_Library_UNOVERPRE := $(gb_Library_SYSPRE)uno_

ifeq ($(DISABLE_DYNLOADING),TRUE)

gb_Library_PLAINEXT := .a
gb_Library_PLAINEXT_FOR_BUILD := .so
gb_Library_DLLEXT := .a

else

gb_Library_TARGETTYPEFLAGS := -shared -Wl,-z,noexecstack
gb_Library_UDK_MAJORVER := 3
gb_Library_PLAINEXT := .so
gb_Library_PLAINEXT_FOR_BUILD := .so
gb_Library_DLLEXT := .so

endif

gb_Library_RTEXT := gcc3$(gb_Library_PLAINEXT)

gb_Library_OOOEXT := $(gb_Library_DLLPOSTFIX)$(gb_Library_PLAINEXT)
gb_Library_UNOEXT := .uno$(gb_Library_PLAINEXT)

gb_Library_FILENAMES := \
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_OOOEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OXT),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PRIVATELIBS_URE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_OOOEXT)) \
	$(foreach lib,$(gb_Library_EXTENSIONLIBS),$(lib):$(lib)$(gb_Library_UNOEXT)) \

ifeq ($(DISABLE_DYNLOADING),TRUE)

gb_Library_FILENAMES += \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_RTEXT)) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(gb_Library_UNOVERPRE)$(lib)$(gb_Library_PLAINEXT)) \

else

gb_Library_FILENAMES += \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_RTEXT).$(gb_Library_UDK_MAJORVER)) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(gb_Library_UNOVERPRE)$(lib)$(gb_Library_PLAINEXT).$(gb_Library_UDK_MAJORVER)) \

endif

gb_Library_LAYER := \
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OXT),$(lib):OXT) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):NONE) \
	$(foreach lib,$(gb_Library_PRIVATELIBS_URE),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_EXTENSIONLIBS),$(lib):OXT) \

define gb_Library__get_rpath
$(if $(1),$(strip -Wl,-z,origin '-Wl,-rpath,$(1)' -Wl,-rpath-link,$(INSTDIR)/program))
endef

define gb_Library_get_rpath
$(call gb_Library__get_rpath,$(call gb_LinkTarget__get_rpath_for_layer,$(call gb_Library_get_layer,$(1))))
endef

define gb_Library_Library_platform
$(call gb_LinkTarget_get_target,$(2)) : RPATH := $(call gb_Library_get_rpath,$(1))

endef

gb_Library__set_soversion_script_platform = $(gb_Library__set_soversion_script)

gb_Library_get_sdk_link_dir = $(INSTDIR)/$(SDKDIRNAME)/lib

gb_Library_get_sdk_link_lib = $(gb_Library_get_versionlink_target)

# Executable class

gb_Executable_EXT :=

gb_Executable_LAYER := \
	$(foreach exe,$(gb_Executable_UREBIN),$(exe):UREBIN) \
	$(foreach exe,$(gb_Executable_SDK),$(exe):SDKBIN) \
	$(foreach exe,$(gb_Executable_OOO),$(exe):OOO) \
	$(foreach exe,$(gb_Executable_NONE),$(exe):NONE) \


define gb_Executable__get_rpath
$(strip -Wl,-z,origin $(if $(1),'-Wl$(COMMA)-rpath$(COMMA)$(1)') -Wl,-rpath-link,$(INSTDIR)/program)
endef

define gb_Executable_get_rpath
$(call gb_Executable__get_rpath,$(call gb_LinkTarget__get_rpath_for_layer,$(call gb_Executable_get_layer,$(1))))
endef

define gb_Executable_Executable_platform
$(call gb_LinkTarget_get_target,$(2)) : RPATH := $(call gb_Executable_get_rpath,$(1))

endef


# CppunitTest class

gb_CppunitTest_CPPTESTPRECOMMAND := \
    $(call gb_Helper_extend_ld_path,$(WORKDIR)/UnpackedTarball/cppunit/src/cppunit/.libs)
gb_CppunitTest_get_filename = libtest_$(1).so
gb_CppunitTest_get_ilibfilename = $(gb_CppunitTest_get_filename)
gb_CppunitTest_malloc_check := -ex 'set environment MALLOC_CHECK_=2; set environment MALLOC_PERTURB_=153'

define gb_CppunitTest_CppunitTest_platform
$(call gb_LinkTarget_get_target,$(2)) : RPATH := $(call gb_Library__get_rpath,$(call gb_LinkTarget__get_rpath_for_layer,NONE))

endef

define gb_CppunitTest_postprocess
$(SRCDIR)/solenv/bin/gdb-core-bt.sh $(1) $(2) $(3)
endef

# JunitTest class

ifneq ($(OOO_TEST_SOFFICE),)
gb_JunitTest_SOFFICEARG:=$(OOO_TEST_SOFFICE)
else
ifneq ($(gb_JunitTest_DEBUGRUN),)
gb_JunitTest_SOFFICEARG:=connect:pipe,name=$(USER)
else
gb_JunitTest_SOFFICEARG:=path:$(INSTROOT)/$(LIBO_BIN_FOLDER)/soffice
endif
endif

define gb_JunitTest_JunitTest_platform
$(call gb_JunitTest_get_target,$(1)) : DEFS := \
	-Dorg.openoffice.test.arg.env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
	-Dorg.openoffice.test.arg.user=$(call gb_Helper_make_url,$(call gb_JunitTest_get_userdir,$(1))) \
	-Dorg.openoffice.test.arg.workdir=$(call gb_JunitTest_get_userdir,$(1)) \
	-Dorg.openoffice.test.arg.postprocesscommand=$(SRCDIR)/solenv/bin/gdb-core-bt.sh \
	-Dorg.openoffice.test.arg.soffice="$(gb_JunitTest_SOFFICEARG)" \

endef

# PythonTest class

gb_PythonTest_PRECOMMAND := $(gb_CppunitTest_CPPTESTPRECOMMAND)

# Module class

define gb_Module_DEBUGRUNCOMMAND
OFFICESCRIPT=`mktemp` && \
printf 'if [ -e $(INSTROOT)/program/ooenv ]; then . $(INSTROOT)/program/ooenv; fi\n' > $${OFFICESCRIPT} && \
printf "PYTHONWARNINGS=default gdb $(INSTROOT)/$(LIBO_BIN_FOLDER)/soffice.bin" >> $${OFFICESCRIPT} && \
printf " -ex \"set args --norestore --nologo '--accept=pipe,name=$(USER);urp;' %s\"" \
	"$(subst ","\\\"",$(value gb_DBGARGS))" >> $${OFFICESCRIPT} && \
$(SHELL) $${OFFICESCRIPT} && \
rm $${OFFICESCRIPT}
endef

# InstallModuleTarget class

define gb_InstallModuleTarget_InstallModuleTarget_platform
$(call gb_InstallModuleTarget_add_defs,$(1),\
	$(gb_CPUDEFS) \
	$(gb_OSDEFS) \
	-DCOMID=gcc3 \
	-D_gcc3 \
)

endef

# InstallScript class

gb_InstallScript_EXT := .ins

# CliAssemblyTarget class

gb_CliAssemblyTarget_POLICYEXT :=
gb_CliAssemblyTarget_get_dll :=

# Extension class

gb_Extension_LICENSEFILE_DEFAULT := $(INSTROOT)/LICENSE

# UnpackedTarget class

gb_UnpackedTarget_TARFILE_LOCATION := $(TARFILE_LOCATION)

# UnoApiHeadersTarget class

ifeq ($(DISABLE_DYNLOADING),TRUE)
gb_UnoApiHeadersTarget_select_variant = $(if $(filter udkapi,$(1)),comprehensive,$(2))
else
gb_UnoApiHeadersTarget_select_variant = $(2)
endif

# UIMenubarTarget class

define gb_UIMenubarTarget__command
$(call gb_Output_announce,$(2),$(true),UIM,1)
$(call gb_Trace_StartRange,$(2),UIM)
cp $(3) $(1)
$(call gb_Trace_EndRange,$(2),UIM)

endef

gb_UIMenubarTarget_UIMenubarTarget_platform :=

# Python
gb_Python_PRECOMMAND := $(gb_Helper_set_ld_path) PYTHONHOME="$(INSTDIR)/program/python-core-$(PYTHON_VERSION)" PYTHONPATH="$${PYPATH:+$$PYPATH:}$(INSTDIR)/program/python-core-$(PYTHON_VERSION)/lib:$(INSTDIR)/program/python-core-$(PYTHON_VERSION)/lib/lib-dynload"
gb_Python_INSTALLED_EXECUTABLE := /bin/sh $(INSTROOT)/program/python
# this is passed to gdb as executable when running tests
gb_Python_INSTALLED_EXECUTABLE_GDB := $(INSTROOT)/program/python.bin

include $(GBUILDDIR)/platform/com_GCC_class.mk

# vim: set noet sw=4:
