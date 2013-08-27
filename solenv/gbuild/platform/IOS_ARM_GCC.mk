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

gb_CPUDEFS := -DARM32

ifeq ($(CC),)
$(error You must set CC in the environment. See README.cross for example.)
endif
ifeq ($(CXX),)
$(error You must set CXX in the environment. See README.cross for example.)
endif

gb_COMPILERDEFAULTOPTFLAGS := -O3

include $(GBUILDDIR)/platform/com_GCC_defs.mk

# Darwin mktemp -t expects a prefix, not a pattern
gb_MKTEMP := /usr/bin/mktemp -t gbuild.

gb_OSDEFS := \
	-D$(OS) \
	-D_PTHREADS \
	-DUNIX \
	-DUNX \
	-D_REENTRANT \
	-DNO_PTHREAD_PRIORITY \
	$(EXTRA_CDEFS) \

gb_CFLAGS := \
	$(gb_CFLAGS_COMMON) \
	-Wshadow \
	-fno-strict-aliasing \

# For -Wno-non-virtual-dtor see <http://markmail.org/message/664jsoqe6n6smy3b>
# "Re: [dev] warnings01: -Wnon-virtual-dtor" message to dev@openoffice.org from
# Feb 1, 2006:
gb_CXXFLAGS := \
	$(gb_CXXFLAGS_COMMON) \
	-Wno-ctor-dtor-privacy \
	-Wno-non-virtual-dtor \
	-fno-strict-aliasing \
	-fsigned-char \
	$(CXXFLAGS_CXX11) \

# these are to get gcc to switch to Objective-C++ or Objective-C mode
gb_OBJC_OBJCXX_COMMON_FLAGS := -fobjc-abi-version=2 -fobjc-legacy-dispatch -D__IPHONE_OS_VERSION_MIN_REQUIRED=40300

gb_OBJCXXFLAGS := -x objective-c++ $(gb_OBJC_OBJCXX_COMMON_FLAGS)

gb_OBJCFLAGS := -x objective-c $(gb_OBJC_OBJCXX_COMMON_FLAGS)

gb_COMPILERDEFS += \
		-DBOOST_DETAIL_NO_CONTAINER_FWD

gb_LinkTarget_LDFLAGS := $(SOLARLIB) \
#man ld says: obsolete	-Wl,-multiply_defined,suppress \

gb_DEBUG_CFLAGS := -g -fno-inline

# LinkTarget class

gb_LinkTarget_CFLAGS := $(gb_CFLAGS)
gb_LinkTarget_CXXFLAGS := $(gb_CXXFLAGS)
gb_LinkTarget_OBJCXXFLAGS := $(gb_CXXFLAGS) $(gb_OBJCXXFLAGS)
gb_LinkTarget_OBJCFLAGS := $(gb_CFLAGS) $(gb_OBJCFLAGS)

ifeq ($(gb_SYMBOL),$(true))
gb_LinkTarget_CFLAGS += $(gb_DEBUG_CFLAGS)
gb_LinkTarget_CXXFLAGS += $(gb_DEBUG_CFLAGS)
gb_LinkTarget_OBJCXXFLAGS += $(gb_DEBUG_CFLAGS)
gb_LinkTarget_OBJCFLAGS += $(gb_DEBUG_CFLAGS)
endif

define gb_LinkTarget__get_liblinkflags
$(patsubst lib%.a,-l%,$(foreach lib,$(1),$(call gb_Library_get_filename,$(lib))))
endef

define gb_LinkTarget__get_layer
$(if $(filter Executable,$(1)),\
	$$(call gb_Executable_get_layer,$(2)),\
	$$(call gb_Library_get_layer,$(2)))
endef

# To not export anything: -Wl$(COMMA)-exported_symbols_list$(COMMA)/dev/null
# But for some reason that slows down ld significantly.

define gb_LinkTarget__command_dynamiclink
	$(if $(filter Library CppunitTest,$(TARGETTYPE)),@echo No dynamic libraries should be built for iOS && exit 1, \
	mkdir -p $(dir $(1)) && \
	$(if $(CXXOBJECTS)$(OBJCXXOBJECTS)$(GENCXXOBJECTS)$(EXTRAOBJECTLISTS),$(gb_CXX),$(gb_CC)) \
		$(gb_Executable_TARGETTYPEFLAGS) \
		$(subst \d,$$,$(RPATH)) \
		$(T_LDFLAGS) \
		-dead_strip -fobjc_link_runtime \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(ASMOBJECTS),$(call gb_AsmObject_get_target,$(object))) \
		$(foreach object,$(OBJCOBJECTS),$(call gb_ObjCObject_get_target,$(object))) \
		$(foreach object,$(OBJCXXOBJECTS),$(call gb_ObjCxxObject_get_target,$(object))) \
		$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach extraobjectlist,$(EXTRAOBJECTLISTS),`cat $(extraobjectlist)`) \
		$(foreach lib,$(LINKED_STATIC_LIBS),$(call gb_StaticLibrary_get_target,$(lib))) \
		$(call gb_LinkTarget__get_liblinkflags,$(LINKED_LIBS)) \
		$(wildcard $(OUTDIR)/lib/*.a) \
		$(LIBS) \
		-o $(1))
endef

# parameters: 1-linktarget 2-cobjects 3-cxxobjects
define gb_LinkTarget__command_staticlink
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	rm -f $(1) && \
	$(gb_AR) -rsu $(1) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(OBJCOBJECTS),$(call gb_ObjCObject_get_target,$(object))) \
		$(foreach object,$(OBJCXXOBJECTS),$(call gb_ObjCxxObject_get_target,$(object))) \
		$(foreach object,$(ASMOBJECTS),$(call gb_AsmObject_get_target,$(object))) \
		$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach extraobjectlist,$(EXTRAOBJECTLISTS),@$(extraobjectlist)) \
		$(if $(findstring s,$(MAKEFLAGS)),2> /dev/null))
endef

define gb_LinkTarget__command
$(call gb_Output_announce,$(2),$(true),LNK,4)
$(if $(filter Executable,$(TARGETTYPE)),$(call gb_LinkTarget__command_dynamiclink,$(1),$(2)))
$(if $(filter CppunitTest Library StaticLibrary,$(TARGETTYPE)),$(call gb_LinkTarget__command_staticlink,$(1)))
endef

define gb_LinkTarget_use_system_darwin_frameworks
$(call gb_LinkTarget_add_libs,$(1),$(foreach fw,$(2),-framework $(fw)))
endef


# Library class

gb_Library_DEFS :=
gb_Library_SYSPRE := lib
gb_Library_UNOVERPRE := $(gb_Library_SYSPRE)uno_
gb_Library_PLAINEXT := .a
gb_Library_DLLEXT := .a
gb_Library_RTEXT := gcc3$(gb_Library_PLAINEXT)

gb_Library_OOOEXT := $(gb_Library_DLLPOSTFIX)$(gb_Library_PLAINEXT)
gb_Library_UNOEXT := .uno$(gb_Library_PLAINEXT)

gb_Library_FILENAMES := \
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_OOOEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PRIVATELIBS_URE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_OOOEXT)) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_RTEXT)) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(gb_Library_UNOVERPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_EXTENSIONLIBS),$(lib):$(lib)$(gb_Library_UNOEXT)) \


gb_Library_LAYER := \
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):NONE) \
	$(foreach lib,$(gb_Library_PRIVATELIBS_URE),$(lib):OOO) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_EXTENSIONLIBS),$(lib):OXT) \

define gb_Library_Library_platform
$(call gb_LinkTarget_get_target,$(2)) : LAYER := $(call gb_Library_get_layer,$(1))

endef


# Executable class

gb_Executable_EXT :=
gb_Executable_TARGETTYPEFLAGS := 

gb_Executable_LAYER := \
	$(foreach exe,$(gb_Executable_UREBIN),$(exe):OOO) \
	$(foreach exe,$(gb_Executable_SDK),$(exe):OOO) \
	$(foreach exe,$(gb_Executable_OOO),$(exe):OOO) \
	$(foreach exe,$(gb_Executable_NONE),$(exe):OOO) \


define gb_Executable_Executable_platform
$(call gb_LinkTarget_get_target,$(2)) : LAYER := $(call gb_Executable_get_layer,$(1))

endef


# CppunitTest class

gb_CppunitTest_CPPTESTPRECOMMAND := :
gb_CppunitTest_SYSPRE := libtest_
gb_CppunitTest_EXT := .a
gb_CppunitTest_LIBDIR := $(gb_Helper_OUTDIRLIBDIR)
gb_CppunitTest_get_filename = $(gb_CppunitTest_SYSPRE)$(1)$(gb_CppunitTest_EXT)
gb_CppunitTest_get_libfilename = $(gb_CppunitTest_get_filename)

define gb_CppunitTest_CppunitTest_platform
$(call gb_LinkTarget_get_target,$(2)) : LAYER := NONE

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

gb_Extension_LICENSEFILE_DEFAULT := $(OUTDIR)/bin/osl/LICENSE

# UnpackedTarget class

gb_UnpackedTarget_TARFILE_LOCATION := $(TARFILE_LOCATION)

# UnoApiHeadersTarget class

ifeq ($(DISABLE_DYNLOADING),TRUE)
gb_UnoApiHeadersTarget_select_variant = $(if $(filter udkapi,$(1)),comprehensive,$(2))
else
gb_UnoApiHeadersTarget_select_variant = $(2)
endif

# Package class
gb_Package_SDKDIRNAME := sdk

# UIMenubarTarget class

define gb_UIMenubarTarget__command
$(call gb_Output_announce,$(2),$(true),UIM,1)
cp $(3) $(1)

endef

gb_UIMenubarTarget_UIMenubarTarget_platform :=

# Python
gb_PYTHON_PRECOMMAND := DYLD_LIBRARY_PATH=$(OUTDIR_FOR_BUILD)/lib

include $(GBUILDDIR)/platform/com_GCC_class.mk

# vim: set noet sw=4:
