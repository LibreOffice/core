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

gb_SDKDIR := $(MACOSX_SDK_PATH)

include $(GBUILDDIR)/platform/com_GCC_defs.mk

# Darwin mktemp -t expects a prefix, not a pattern
gb_MKTEMP ?= /usr/bin/mktemp -t gbuild

gb_OSDEFS := \
	-D$(OS) \
	-D_PTHREADS \
	-DUNIX \
	-DUNX \
	-D_REENTRANT \
	-DNO_PTHREAD_PRIORITY \
	-DMAC_OS_X_VERSION_MIN_REQUIRED=$(MAC_OS_X_VERSION_MIN_REQUIRED) \
	-DMAC_OS_X_VERSION_MAX_ALLOWED=$(MAC_OS_X_VERSION_MAX_ALLOWED) \
	$(LFS_CFLAGS) \


gb_CFLAGS := \
	$(gb_CFLAGS_COMMON) \
	-fPIC \
	-fno-strict-aliasing \
	-Wshadow

gb_CXXFLAGS := \
	$(gb_CXXFLAGS_COMMON) \
	-fPIC \
	-Woverloaded-virtual \
	-Wshadow \
	-Wno-ctor-dtor-privacy \
	-fno-strict-aliasing \
	-fsigned-char \
	$(CXXFLAGS_CXX11)

	#-fsigned-char \ might be removed?

# these are to get g++ to switch to Objective-C++ mode
# (see toolkit module for a case where it is necessary to do it this way)
gb_OBJCXXFLAGS := -x objective-c++ -fobjc-exceptions

gb_OBJCFLAGS := -x objective-c -fobjc-exceptions

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

# $(call gb_LinkTarget__get_installname,libfilename,layerprefix)
define gb_LinkTarget__get_installname
$(if $(2),-install_name '$(2)$(1)')
endef

gb_LinkTarget_CFLAGS := $(gb_CFLAGS)
gb_LinkTarget_CXXFLAGS := $(gb_CXXFLAGS)
gb_LinkTarget_OBJCXXFLAGS := $(gb_CXXFLAGS) $(gb_OBJCXXFLAGS)
gb_LinkTarget_OBJCFLAGS := $(gb_CFLAGS) $(gb_OBJCFLAGS)

define gb_LinkTarget__get_layer
$(if $(filter Executable,$(1)),\
	$$(call gb_Executable_get_layer,$(2)),\
	$$(call gb_Library_get_layer,$(2)))
endef

# We cannot sign executables early since Mojave/Catalina would treat them as
# restricted binary and ignore any DYLD_LIBRARY_PATH setting - So all
# signing is handled by the solenv/bin/macosx-codesign-app-bundle script.
# And the soffice executable needs to be signed last in
# macosx-codesign-app-bundle, as codesign would fail complaining that other
# parts of the app have not yet been signed:

define gb_LinkTarget__command_dynamiclink
$(call gb_Helper_abbreviate_dirs,\
	FILELIST=$(call gb_var2file,$(shell $(gb_MKTEMP)),100, \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(ASMOBJECTS),$(call gb_AsmObject_get_target,$(object))) \
		$(foreach object,$(OBJCOBJECTS),$(call gb_ObjCObject_get_target,$(object))) \
		$(foreach object,$(OBJCXXOBJECTS),$(call gb_ObjCxxObject_get_target,$(object))) \
		$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach object,$(GENNASMOBJECTS),$(call gb_GenNasmObject_get_target,$(object))) \
		$(foreach object,$(GENOBJCOBJECTS),$(call gb_GenObjCObject_get_target,$(object))) \
		$(foreach object,$(GENOBJCXXOBJECTS),$(call gb_GenObjCxxObject_get_target,$(object))) \
		$(foreach extraobjectlist,$(EXTRAOBJECTLISTS),$(shell cat $(extraobjectlist)))) && \
	cat $${FILELIST} | tr "[:space:]" "\n" | grep -v '^$$' > $${FILELIST}.1 && \
	mv $${FILELIST}.1 $${FILELIST} && \
	$(if $(CXXOBJECTS)$(OBJCXXOBJECTS)$(GENCXXOBJECTS)$(EXTRAOBJECTLISTS),$(gb_CXX),$(gb_CC)) \
		$(if $(filter Executable,$(TARGETTYPE)),$(gb_Executable_TARGETTYPEFLAGS)) \
		$(if $(filter Bundle,$(TARGETTYPE)),$(gb_Bundle_TARGETTYPEFLAGS)) \
		$(if $(filter Library CppunitTest,$(TARGETTYPE)),$(gb_Library_TARGETTYPEFLAGS)) \
		$(subst \d,$$,$(RPATH)) \
		$(T_USE_LD) $(T_LDFLAGS) \
		$(patsubst lib%.dylib,-l%,$(patsubst %.$(gb_Library_UDK_MAJORVER),%,$(foreach lib,$(LINKED_LIBS),$(call gb_Library_get_filename,$(lib))))) \
		-Wl$(COMMA)-filelist$(COMMA)$${FILELIST} \
		$(T_LIBS) \
		$(foreach lib,$(LINKED_STATIC_LIBS),$(call gb_StaticLibrary_get_target,$(lib))) \
		-o $(1) && \
	rm -f $${FILELIST} && \
	$(if $(SOVERSIONSCRIPT),ln -sf $(1) $(ILIBTARGET),:) && \
	$(if $(filter Executable,$(TARGETTYPE)), \
		$(PERL) $(SRCDIR)/solenv/bin/macosx-change-install-names.pl app $(LAYER) $(1) &&) \
	$(if $(filter Library Bundle CppunitTest,$(TARGETTYPE)),\
		$(PERL) $(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl $(LAYER) $(1) &&) \
	$(if $(filter Library,$(TARGETTYPE)),\
		otool -l $(1) | grep -A 5 LC_ID_DYLIB \
			> $(WORKDIR)/LinkTarget/$(2).exports.tmp && \
		$(NM) -g -P $(1) | cut -d' ' -f1-2 | grep -v U$$ \
			>> $(WORKDIR)/LinkTarget/$(2).exports.tmp && \
		$(call gb_Helper_replace_if_different_and_touch,$(WORKDIR)/LinkTarget/$(2).exports.tmp, \
			$(WORKDIR)/LinkTarget/$(2).exports,$(1)) &&) \
	:)
endef

# parameters: 1-linktarget 2-cobjects 3-cxxobjects
define gb_LinkTarget__command_staticlink
$(call gb_Helper_abbreviate_dirs,\
	rm -f $(1) && \
	$(gb_AR) -rsu $(1) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(ASMOBJECTS),$(call gb_AsmObject_get_target,$(object))) \
		$(foreach object,$(OBJCOBJECTS),$(call gb_ObjCObject_get_target,$(object))) \
		$(foreach object,$(OBJCXXOBJECTS),$(call gb_ObjCxxObject_get_target,$(object))) \
		$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach object,$(GENNASMOBJECTS),$(call gb_GenNasmObject_get_target,$(object))) \
		$(foreach object,$(GENOBJCOBJECTS),$(call gb_GenObjCObject_get_target,$(object))) \
		$(foreach object,$(GENOBJCXXOBJECTS),$(call gb_GenObjCxxObject_get_target,$(object))) \
		$(foreach extraobjectlist,$(EXTRAOBJECTLISTS),$(shell cat $(extraobjectlist))) \
		$(if $(findstring s,$(MAKEFLAGS)),2> /dev/null))
endef

define gb_LinkTarget__command
$(call gb_Output_announce,$(2),$(true),LNK,4)
	$(call gb_Trace_StartRange,$(2),LNK)
$(if $(filter Library Bundle CppunitTest Executable,$(TARGETTYPE)),$(call gb_LinkTarget__command_dynamiclink,$(1),$(2)))
$(if $(filter StaticLibrary,$(TARGETTYPE)),$(call gb_LinkTarget__command_staticlink,$(1)))
	$(call gb_Trace_EndRange,$(2),LNK)
endef

define gb_LinkTarget_use_system_darwin_frameworks
$(call gb_LinkTarget_add_libs,$(1),$(foreach fw,$(2),-framework $(fw)))
$(if $(call gb_LinkTarget__is_merged,$(1)),\
  $(call gb_LinkTarget_add_libs,$(call gb_Library_get_linktarget,merged),$(foreach fw,$(2),-framework $(fw))))
endef


# Library class

gb_Library_DEFS :=
gb_Library_TARGETTYPEFLAGS := -dynamiclib -single_module
gb_Bundle_TARGETTYPEFLAGS := -bundle
gb_Library_UDK_MAJORVER := 3
gb_Library_SYSPRE := lib
gb_Library_UNOVERPRE := $(gb_Library_SYSPRE)uno_
gb_Library_PLAINEXT := .dylib
gb_Library_PLAINEXT_FOR_BUILD := .dylib
gb_Library_DLLEXT := .dylib
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
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_RTEXT).$(gb_Library_UDK_MAJORVER)) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(gb_Library_UNOVERPRE)$(lib)$(gb_Library_PLAINEXT).$(gb_Library_UDK_MAJORVER)) \
	$(foreach lib,$(gb_Library_EXTENSIONLIBS),$(lib):$(lib)$(gb_Library_UNOEXT)) \


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

define gb_Library_get_rpath
$(call gb_LinkTarget__get_installname,$(call gb_Library_get_filename,$(1)),$(call gb_LinkTarget__get_rpath_for_layer,$(call gb_Library_get_layer,$(1))))
endef

define gb_Library_Library_platform
$(call gb_LinkTarget_get_target,$(2)) : RPATH := $(call gb_Library_get_rpath,$(1))
$(call gb_LinkTarget_get_target,$(2)) : LAYER := $(call gb_Library_get_layer,$(1))

endef

gb_Library__set_soversion_script_platform = $(gb_Library__set_soversion_script)

gb_Library_get_sdk_link_dir = $(WORKDIR)/LinkTarget/Library

gb_Library_get_sdk_link_lib = $(gb_Library_get_versionlink_target)

# bundle is a special kind of library that exists only on Darwin/OSX
# set the TARGETTYPE to Bundle, and clear install_name(RPATH)
define gb_Library_Bundle
$(call gb_Library_Library,$(1))
$(call gb_LinkTarget_set_targettype,$(call gb_Library_get_linktarget,$(1)),Bundle)
$(call gb_LinkTarget_get_target,$(call gb_Library_get_linktarget,$(1))) : \
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

gb_CppunitTest_UNITTESTFAILED := $(GBUILDDIR)/platform/unittest-failed-MACOSX.sh
gb_CppunitTest_PYTHONDEPS := $(call gb_Library_get_target,pyuno_wrapper) $(if $(SYSTEM_PYTHON),,$(call gb_GeneratedPackage_get_target,python3))
gb_CppunitTest_CPPTESTPRECOMMAND := \
	$(call gb_Helper_extend_ld_path,$(gb_Library_DLLDIR):$(WORKDIR)/UnpackedTarball/cppunit/src/cppunit/.libs)
gb_CppunitTest_get_filename = libtest_$(1).dylib
gb_CppunitTest_get_ilibfilename = $(gb_CppunitTest_get_filename)
gb_CppunitTest_malloc_check := -o "env MallocScribble=1" -o "env MallocPreScribble=1"

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
gb_JunitTest_SOFFICEARG:=path:$(INSTROOT)/$(LIBO_BIN_FOLDER)/soffice
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

gb_PythonTest_UNITTESTFAILED := $(GBUILDDIR)/platform/unittest-failed-MACOSX.sh
gb_PythonTest_DEPS := $(call gb_GeneratedPackage_get_target,python3) $(call gb_Package_get_target,python_shell)
gb_PythonTest_PRECOMMAND := $(gb_Helper_LIBRARY_PATH_VAR)=$${$(gb_Helper_LIBRARY_PATH_VAR):+$$$(gb_Helper_LIBRARY_PATH_VAR):}$(INSTROOT)/$(LIBO_URE_LIB_FOLDER)
ifneq ($(LIBO_LIB_FOLDER),$(LIBO_URE_LIB_FOLDER))
gb_PythonTest_PRECOMMAND := $(gb_PythonTest_PRECOMMAND):$(INSTROOT)/$(LIBO_LIB_FOLDER)
endif
gb_PythonTest_PRECOMMAND := $(gb_PythonTest_PRECOMMAND):$(WORKDIR)/UnpackedTarball/cppunit/src/cppunit/.libs

# UITest class

gb_UITest_DEPS := $(call gb_GeneratedPackage_get_target,python3)

# Module class

define gb_Module_DEBUGRUNCOMMAND
lldb -f $(INSTROOT)/$(LIBO_BIN_FOLDER)/soffice -- --norestore --nologo \
    '"--accept=pipe,name=$(USER)\;urp"'
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

gb_Extension_LICENSEFILE_DEFAULT := $(INSTROOT)/Resources/LICENSE

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
$(call gb_ExternalExecutable_get_command,xsltproc) --nonet -o $(1) $(UI_MENUBAR_XSLT) $(3)
$(call gb_Trace_EndRange,$(2),UIM)

endef

define gb_UIMenubarTarget_UIMenubarTarget_platform
$(call gb_UIMenubarTarget_get_target,$(1)) : UI_MENUBAR_XSLT := $(SRCDIR)/solenv/bin/macosx_menubar_modification.xsl
$(call gb_UIMenubarTarget_get_target,$(1)) : $(SRCDIR)/solenv/bin/macosx_menubar_modification.xsl
$(call gb_UIMenubarTarget_get_target,$(1)) :| $(call gb_ExternalExecutable_get_dependencies,xsltproc)

endef

# Python
gb_Python_PRECOMMAND := PYTHONPATH="$$PYPATH"
gb_Python_INSTALLED_EXECUTABLE := $(INSTROOT)/$(LIBO_LIB_FOLDER)/LibreOfficePython.framework/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/Resources/Python.app/Contents/MacOS/LibreOfficePython
# this is passed to gdb as executable when running tests
gb_Python_INSTALLED_EXECUTABLE_GDB := $(gb_Python_INSTALLED_EXECUTABLE)

include $(GBUILDDIR)/platform/com_GCC_class.mk

# vim: set noet sw=4:
