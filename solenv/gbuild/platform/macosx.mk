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

gb_DEVINSTALLROOT := $(DEVINSTALLDIR)/opt/LibreOffice.app/Contents

gb_PROGRAMDIRNAME := MacOS

gb_SDKDIR := $(MACOSX_SDK_PATH)

include $(GBUILDDIR)/platform/com_GCC_defs.mk

# Darwin mktemp -t expects a prefix, not a pattern
gb_MKTEMP ?= /usr/bin/mktemp -t gbuild.

gb_OSDEFS := \
	-D$(OS) \
	-D_PTHREADS \
	-DUNIX \
	-DUNX \
	-D_REENTRANT \
	-DNO_PTHREAD_PRIORITY \
	-DMAC_OS_X_VERSION_MIN_REQUIRED=$(MAC_OS_X_VERSION_MIN_REQUIRED) \
	-DMAC_OS_X_VERSION_MAX_ALLOWED=$(MAC_OS_X_VERSION_MAX_ALLOWED) \
	-DMACOSX_SDK_VERSION=$(MACOSX_SDK_VERSION) \
	$(EXTRA_CDEFS) \


gb_CFLAGS := \
	$(gb_CFLAGS_COMMON) \
	-fPIC \
	-fno-strict-aliasing \
    #-Wshadow \ break in compiler headers already

# For -Wno-non-virtual-dtor see <http://markmail.org/message/664jsoqe6n6smy3b>
# "Re: [dev] warnings01: -Wnon-virtual-dtor" message to dev@openoffice.org from
# Feb 1, 2006:
gb_CXXFLAGS := \
	$(gb_CXXFLAGS_COMMON) \
	-fPIC \
	-Wno-ctor-dtor-privacy \
	-Wno-non-virtual-dtor \
	-fno-strict-aliasing \
	-fsigned-char \
	$(if $(filter TRUE,$(COM_GCC_IS_CLANG)),$(CXXFLAGS_CXX11),-malign-natural) \

	#-Wshadow \ break in compiler headers already
	#-fsigned-char \ might be removed?
	#-malign-natural \ might be removed?

# Without this I get struct/class clashes for "complex" when compiling
# some source files in vcl, at least with the 10.7 SDK.
ifneq ($(filter 1070,$(MACOSX_SDK_VERSION)),)
gb_COMPILERDEFS += \
		-DBOOST_DETAIL_NO_CONTAINER_FWD \

endif

ifneq ($(filter 1060,$(MACOSX_SDK_VERSION)),)
gb_COMPILERNOOPTFLAGS := -O0 -fstrict-overflow

endif

ifeq ($(HAVE_GCC_NO_LONG_DOUBLE),TRUE)
gb_CXXFLAGS += -Wno-long-double
endif

# these are to get g++ to switch to Objective-C++ mode
# (see toolkit module for a case where it is necessary to do it this way)
gb_OBJCXXFLAGS := -x objective-c++ -fobjc-exceptions

gb_OBJCFLAGS := -x objective-c -fobjc-exceptions

gb_LinkTarget_LDFLAGS := \
	$(SOLARLIB) \

# LinkTarget class

define gb_LinkTarget__get_rpath_for_layer
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_LinkTarget__RPATHS)))
endef

gb_LinkTarget__RPATHS := \
	URELIB:@__________________________________________________URELIB/ \
	UREBIN: \
	OOO:@__________________________________________________OOO/ \
	SDKBIN: \
	OXT: \
	NONE:@__________________________________________________NONE/ \

# $(call gb_LinkTarget__get_installname,libfilename,soversion,layerprefix)
define gb_LinkTarget__get_installname
$(if $(3),-install_name '$(3)$(1)$(if $(2),.$(2))')
endef

gb_LinkTarget_CFLAGS := $(gb_CFLAGS)
gb_LinkTarget_CXXFLAGS := $(gb_CXXFLAGS)
gb_LinkTarget_OBJCXXFLAGS := $(gb_CXXFLAGS) $(gb_OBJCXXFLAGS)
gb_LinkTarget_OBJCFLAGS := $(gb_CFLAGS) $(gb_OBJCFLAGS)

ifeq ($(gb_SYMBOL),$(true))
gb_LinkTarget_CFLAGS += $(gb_DEBUG_CFLAGS)
gb_LinkTarget_CXXFLAGS += $(gb_DEBUG_CFLAGS)
gb_LinkTarget_OBJCFLAGS += $(gb_DEBUG_CFLAGS)
gb_LinkTarget_OBJCXXFLAGS += $(gb_DEBUG_CFLAGS)
endif

define gb_LinkTarget__get_layer
$(if $(filter Executable,$(1)),\
	$$(call gb_Executable_get_layer,$(2)),\
	$$(call gb_Library_get_layer,$(2)))
endef

define gb_LinkTarget__command_dynamiclink
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(if $(CXXOBJECTS)$(OBJCXXOBJECTS)$(GENCXXOBJECTS)$(EXTRAOBJECTLISTS),$(gb_CXX),$(gb_CC)) \
		$(if $(filter Executable,$(TARGETTYPE)),$(gb_Executable_TARGETTYPEFLAGS)) \
		$(if $(filter Bundle,$(TARGETTYPE)),$(gb_Bundle_TARGETTYPEFLAGS)) \
		$(if $(filter Library CppunitTest,$(TARGETTYPE)),$(gb_Library_TARGETTYPEFLAGS)) \
		$(if $(filter Library,$(TARGETTYPE)),$(gb_Library_LTOFLAGS)) \
		$(subst \d,$$,$(RPATH)) \
		$(T_LDFLAGS) \
		$(patsubst lib%.dylib,-l%,$(foreach lib,$(LINKED_LIBS),$(call gb_Library_get_filename,$(lib)))) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(ASMOBJECTS),$(call gb_AsmObject_get_target,$(object))) \
		$(foreach object,$(OBJCOBJECTS),$(call gb_ObjCObject_get_target,$(object))) \
		$(foreach object,$(OBJCXXOBJECTS),$(call gb_ObjCxxObject_get_target,$(object))) \
		$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach extraobjectlist,$(EXTRAOBJECTLISTS),`cat $(extraobjectlist)`) \
		$(LIBS) \
		$(foreach lib,$(LINKED_STATIC_LIBS),$(call gb_StaticLibrary_get_target,$(lib))) \
		-o $(if $(SOVERSION),$(1).$(SOVERSION),$(1)) && \
	$(if $(SOVERSION),ln -sf $(notdir $(1)).$(SOVERSION) $(1),:) && \
	$(if $(filter Executable,$(TARGETTYPE)), \
		$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl app $(LAYER) $(1) &&) \
	$(if $(filter Library Bundle CppunitTest,$(TARGETTYPE)),\
		$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl shl $(LAYER) $(if $(SOVERSION),$(1).$(SOVERSION),$(1)) && \
		ln -sf $(notdir $(1)) $(basename $(1)).jnilib &&) \
	$(if $(MACOSX_CODESIGNING_IDENTITY), \
		$(if $(filter Executable,$(TARGETTYPE)), \
			codesign --identifier=$(MACOSX_BUNDLE_IDENTIFIER).$(notdir $(1)) --sign $(MACOSX_CODESIGNING_IDENTITY) $(1) &&)) \
	$(if $(filter Library,$(TARGETTYPE)),\
		otool -l $(1) | grep -A 5 LC_ID_DYLIB > $(1).exports.tmp && \
		$(NM) -g -P $(1) | cut -d' ' -f1-2 | grep -v U$$ \
			>> $(1).exports.tmp && \
		if cmp -s $(1).exports.tmp $(1).exports; \
			then rm $(1).exports.tmp; \
			else mv $(1).exports.tmp $(1).exports; touch -r $(1) $(1).exports; \
		fi &&) \
	:)
endef

# parameters: 1-linktarget 2-cobjects 3-cxxobjects
define gb_LinkTarget__command_staticlink
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	rm -f $(1) && \
	$(gb_AR) -rsu $(1) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(ASMOBJECTS),$(call gb_AsmObject_get_target,$(object))) \
		$(foreach object,$(OBJCOBJECTS),$(call gb_ObjCObject_get_target,$(object))) \
		$(foreach object,$(OBJCXXOBJECTS),$(call gb_ObjCxxObject_get_target,$(object))) \
		$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach extraobjectlist,$(EXTRAOBJECTLISTS),@$(extraobjectlist)) \
		$(if $(findstring s,$(MAKEFLAGS)),2> /dev/null))
endef

define gb_LinkTarget__command
$(call gb_Output_announce,$(2),$(true),LNK,4)
$(if $(filter Library Bundle CppunitTest Executable,$(TARGETTYPE)),$(call gb_LinkTarget__command_dynamiclink,$(1),$(2)))
$(if $(filter StaticLibrary,$(TARGETTYPE)),$(call gb_LinkTarget__command_staticlink,$(1)))
endef

define gb_LinkTarget_use_system_darwin_frameworks
$(call gb_LinkTarget_add_libs,$(1),$(foreach fw,$(2),-framework $(fw)))
endef


# Library class

gb_Library_DEFS :=
gb_Library_TARGETTYPEFLAGS := -dynamiclib -single_module
gb_Bundle_TARGETTYPEFLAGS := -bundle
gb_Library_SYSPRE := lib
gb_Library_UNOVERPRE := $(gb_Library_SYSPRE)uno_
gb_Library_PLAINEXT := .dylib
gb_Library_DLLEXT := .dylib
gb_Library_RTEXT := gcc3$(gb_Library_PLAINEXT)

gb_Library_OOOEXT := $(gb_Library_DLLPOSTFIX)$(gb_Library_PLAINEXT)
gb_Library_UNOEXT := .uno$(gb_Library_PLAINEXT)

gb_Library_FILENAMES := \
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_OOOEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_RTEXT)) \
	$(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):$(lib)$(gb_Library_UNOEXT)) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(gb_Library_UNOVERPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_EXTENSIONLIBS),$(lib):$(lib)$(gb_Library_UNOEXT)) \


gb_Library_LAYER := \
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):NONE) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_EXTENSIONLIBS),$(lib):OXT) \

define gb_Library_get_rpath
$(call gb_LinkTarget__get_installname,$(call gb_Library_get_filename,$(1)),$(2),$(call gb_LinkTarget__get_rpath_for_layer,$(call gb_Library_get_layer,$(1))))
endef

# RPATH def is delayed until the link command to get current value of SOVERSION
define gb_Library_Library_platform
$(call gb_LinkTarget_get_target,$(2)) : \
	RPATH = $$(call gb_Library_get_rpath,$(1),$$(SOVERSION))
$(call gb_LinkTarget_get_target,$(2)) : LAYER := $(call gb_Library_get_layer,$(1))

endef

gb_Library__set_soversion_script_platform = $(gb_Library__set_soversion_script)

# bundle is a special kind of library that exists only on Darwin/OSX
# set the TARGETTYPE to Bundle, and clear install_name(RPATH)
define gb_Library_Bundle
$(call gb_Library_Library,$(1))
$(call gb_LinkTarget_set_targettype,$(call gb_Library_get_linktargetname,$(1)),Bundle)
$(call gb_LinkTarget_get_target,$(call gb_Library_get_linktargetname,$(1))) : \
	RPATH :=
endef

# Executable class

gb_Executable_EXT :=
gb_Executable_TARGETTYPEFLAGS := -bind_at_load

gb_Executable_LAYER := \
	$(foreach exe,$(gb_Executable_UREBIN),$(exe):UREBIN) \
	$(foreach exe,$(gb_Executable_SDK),$(exe):SDKBIN) \
	$(foreach exe,$(gb_Executable_OOO),$(exe):OOO) \
	$(foreach exe,$(gb_Executable_NONE),$(exe):NONE) \


gb_Executable_get_rpath :=

define gb_Executable_Executable_platform
$(call gb_LinkTarget_get_target,$(2)) : RPATH :=
$(call gb_LinkTarget_get_target,$(2)) : LAYER := $(call gb_Executable_get_layer,$(1))

endef


# CppunitTest class

gb_CppunitTest_CPPTESTPRECOMMAND := $(gb_Helper_set_ld_path)
gb_CppunitTest_SYSPRE := libtest_
gb_CppunitTest_EXT := .dylib
gb_CppunitTest_LIBDIR := $(gb_Helper_OUTDIRLIBDIR)
gb_CppunitTest_get_filename = $(gb_CppunitTest_SYSPRE)$(1)$(gb_CppunitTest_EXT)
gb_CppunitTest_get_libfilename = $(gb_CppunitTest_get_filename)

define gb_CppunitTest_CppunitTest_platform
$(call gb_LinkTarget_get_target,$(2)) : RPATH :=
$(call gb_LinkTarget_get_target,$(2)) : LAYER := NONE

endef

# JunitTest class

ifneq ($(OOO_TEST_SOFFICE),)
gb_JunitTest_SOFFICEARG:=$(OOO_TEST_SOFFICE)
else
ifneq ($(gb_JunitTest_DEBUGRUN),)
gb_JunitTest_SOFFICEARG:=connect:pipe,name=$(USER)
else
gb_JunitTest_SOFFICEARG:=path:$(gb_DEVINSTALLROOT)/MacOS/soffice
endif
endif

define gb_JunitTest_JunitTest_platform
$(call gb_JunitTest_get_target,$(1)) : DEFS := \
	-Dorg.openoffice.test.arg.soffice="$(gb_JunitTest_SOFFICEARG)" \
	-Dorg.openoffice.test.arg.env=DYLD_LIBRARY_PATH"$$$${DYLD_LIBRARY_PATH+=$$$$DYLD_LIBRARY_PATH}" \
	-Dorg.openoffice.test.arg.user=$(call gb_Helper_make_url,$(call gb_JunitTest_get_userdir,$(1))) \
	-Dorg.openoffice.test.arg.workdir=$(call gb_JunitTest_get_userdir,$(1)) \

endef

# PythonTest class

gb_PythonTest_PRECOMMAND := $(gb_Helper_LIBRARY_PATH_VAR)=$${$(gb_Helper_LIBRARY_PATH_VAR):+$$$(gb_Helper_LIBRARY_PATH_VAR):}$(gb_DEVINSTALLROOT)/ure-link/lib:$(gb_DEVINSTALLROOT)/program:$(OUTDIR)/lib

# Module class

define gb_Module_DEBUGRUNCOMMAND
OFFICESCRIPT=$$($(gb_MKTEMP)) && \
printf '%s\n' "set args --norestore --nologo '--accept=pipe,name=$(USER);urp;' -env:UserInstallation=$(call gb_Helper_make_url,$(DEVINSTALLDIR)/)" > $${OFFICESCRIPT} && \
gdb -x $${OFFICESCRIPT} $(gb_DEVINSTALLROOT)/MacOS/soffice && \
rm $${OFFICESCRIPT}
endef

# InstallModuleTarget class

define gb_InstallModuleTarget_InstallModuleTarget_platform
$(call gb_InstallModuleTarget_add_defs,$(1),\
	$(gb_CPUDEFS) \
	$(gb_OSDEFS) \
	-DCOMID=gcc3 \
	-D_gcc3 \
	$(if $(filter TRUE,$(SOLAR_JAVA)),-DSOLAR_JAVA) \
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

# It seems that when using the latest Xcode and Clang for OS X, we
# also neeed to always generate comprehensive headers for
# udkapi. Otherwise we get assertion failures in saxparser when doing
# i18npool, at least.
ifneq ($(filter TRUE,$(COM_GCC_IS_CLANG) $(DISABLE_DYNLOADING)),)
gb_UnoApiHeadersTarget_select_variant = $(if $(filter udkapi,$(1)),comprehensive,$(2))
else
gb_UnoApiHeadersTarget_select_variant = $(2)
endif

# Package class
gb_Package_PROGRAMDIRNAME := MacOS
gb_Package_SDKDIRNAME := LibreOffice$(PRODUCTVERSION)_SDK

# UIMenubarTarget class

define gb_UIMenubarTarget__command
$(call gb_Output_announce,$(2),$(true),UIM,1)
$(call gb_ExternalExecutable_get_command,xsltproc) -o $(1) $(UI_MENUBAR_XSLT) $(3)

endef

define gb_UIMenubarTarget_UIMenubarTarget_platform
$(call gb_UIMenubarTarget_get_target,$(1)) : UI_MENUBAR_XSLT := $(SRCDIR)/solenv/bin/macosx_menubar_modification.xsl
$(call gb_UIMenubarTarget_get_target,$(1)) : $(SRCDIR)/solenv/bin/macosx_menubar_modification.xsl
$(call gb_UIMenubarTarget_get_target,$(1)) :| $(call gb_ExternalExecutable_get_dependencies,xsltproc)

endef

# Python
gb_Python_PRECOMMAND := DYLD_LIBRARY_PATH=$(OUTDIR)/lib
gb_Python_INSTALLED_EXECUTABLE := $(gb_DEVINSTALLROOT)/program/LibreOfficePython.framework/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/Resources/Python.app/Contents/MacOS/LibreOfficePython
# this is passed to gdb as executable when running tests
gb_Python_INSTALLED_EXECUTABLE_GDB := $(gb_Python_INSTALLED_EXECUTABLE)

include $(GBUILDDIR)/platform/com_GCC_class.mk

# vim: set noet sw=4:
