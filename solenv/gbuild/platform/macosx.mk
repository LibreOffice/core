# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

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
	-DQUARTZ \
	-DMAC_OS_X_VERSION_MIN_REQUIRED=$(MAC_OS_X_VERSION_MIN_REQUIRED) \
	-DMAC_OS_X_VERSION_MAX_ALLOWED=$(MAC_OS_X_VERSION_MAX_ALLOWED) \
	-DMACOSX_SDK_VERSION=$(MACOSX_SDK_VERSION) \
	$(EXTRA_CDEFS) \


gb_CFLAGS := \
	-isysroot $(gb_SDKDIR) \
	$(gb_CFLAGS_COMMON) \
	-fPIC \
	-fno-strict-aliasing \
    #-Wshadow \ break in compiler headers already

# For -Wno-non-virtual-dtor see <http://markmail.org/message/664jsoqe6n6smy3b>
# "Re: [dev] warnings01: -Wnon-virtual-dtor" message to dev@openoffice.org from
# Feb 1, 2006:
gb_CXXFLAGS := \
	-isysroot $(gb_SDKDIR) \
	$(gb_CXXFLAGS_COMMON) \
	-fPIC \
	-Wno-ctor-dtor-privacy \
	-Wno-non-virtual-dtor \
	-fno-strict-aliasing \
	-fsigned-char \
	$(if $(filter TRUE,$(COM_GCC_IS_CLANG)),,-malign-natural) \
	#-Wshadow \ break in compiler headers already
	#-fsigned-char \ might be removed?
	#-malign-natural \ might be removed?

ifeq ($(HAVE_GCC_VISIBILITY_FEATURE),TRUE)
gb_COMPILERDEFS += \
	-DHAVE_GCC_VISIBILITY_FEATURE \

gb_CFLAGS += \
    -fvisibility=hidden

gb_CXXFLAGS += \
	-fvisibility=hidden \

ifneq ($(HAVE_GCC_VISIBILITY_BROKEN),TRUE)
gb_CXXFLAGS += \
    -fvisibility-inlines-hidden \

endif

endif

ifeq ($(HAVE_SFINAE_ANONYMOUS_BROKEN),TRUE)
gb_COMPILERDEFS += \
        -DHAVE_SFINAE_ANONYMOUS_BROKEN \

endif

ifeq ($(HAVE_GCC_NO_LONG_DOUBLE),TRUE)
gb_CXXFLAGS += -Wno-long-double
endif

ifneq ($(HAVE_THREADSAFE_STATICS),TRUE)
gb_CXXFLAGS += -fno-threadsafe-statics
endif

# these are to get g++ to switch to Objective-C++ mode
# (see toolkit module for a case where it is necessary to do it this way)
gb_OBJCXXFLAGS := -x objective-c++ -fobjc-exceptions

gb_OBJCFLAGS := -x objective-c -fobjc-exceptions

gb_LinkTarget_LDFLAGS := \
	$(if $(filter 1040 1050,$(MAC_OS_X_VERSION_MIN_REQUIRED)),-Wl$(COMMA)-syslibroot$(COMMA)$(gb_SDKDIR)) \
	$(subst -L../lib , ,$(SOLARLIB)) \
#man ld says: obsolete	-Wl,-multiply_defined,suppress \

gb_DEBUG_CFLAGS := -g
gb_COMPILEROPTFLAGS := -O2
gb_COMPILERNOOPTFLAGS := -O0

# ObjCxxObject class

define gb_ObjCxxObject__command
$(call gb_Output_announce,$(2),$(true),OCX,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && \
	$(gb_CXX) \
		$(DEFS) \
		$(if $(WARNINGS_NOT_ERRORS),,$(gb_CXXFLAGS_WERROR)) \
		$(T_OBJCXXFLAGS) \
		-c $(3) \
		-o $(1) \
		-MMD -MT $(1) \
		-MP -MF $(4) \
		-I$(dir $(3)) \
		$(INCLUDE_STL) $(INCLUDE))
endef

# ObjCObject class

define gb_ObjCObject__command
$(call gb_Output_announce,$(2),$(true),OCC,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) $(dir $(4)) && \
	$(gb_CC) \
		$(DEFS) \
		$(if $(WARNINGS_NOT_ERRORS),,$(gb_CFLAGS_WERROR)) \
		$(T_OBJCFLAGS) \
		-c $(3) \
		-o $(1) \
		-MMD -MT $(1) \
		-MP -MF $(4) \
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
	SDKBIN: \
	OXT: \
	NONE:@__VIA_LIBRARY_PATH__@ \

# $(call gb_LinkTarget__get_installname,libfilename,soversion,layerprefix)
define gb_LinkTarget__get_installname
$(if $(3),-install_name '$(3)$(1)$(if $(2),.$(2))')
endef

gb_LinkTarget_CFLAGS := $(gb_CFLAGS)
gb_LinkTarget_CXXFLAGS := $(gb_CXXFLAGS)
gb_LinkTarget_OBJCXXFLAGS := $(gb_CXXFLAGS) $(gb_OBJCXXFLAGS)
gb_LinkTarget_OBJCFLAGS := $(gb_CFLAGS) $(gb_OBJCFLAGS)

ifeq ($(gb_SYMBOL),$(true))
gb_LinkTarget_CFLAGS += -g
gb_LinkTarget_CXXFLAGS += -g
gb_LinkTarget_OBJCFLAGS += -g
gb_LinkTarget_OBJCXXFLAGS += -g
endif

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
		$(if $(filter Bundle,$(TARGETTYPE)),$(gb_Bundle_TARGETTYPEFLAGS)) \
		$(if $(filter Library CppunitTest,$(TARGETTYPE)),$(gb_Library_TARGETTYPEFLAGS)) \
		$(if $(filter Library,$(TARGETTYPE)),$(gb_Library_LTOFLAGS)) \
		$(subst \d,$$,$(RPATH)) \
		$(T_LDFLAGS) \
		$(patsubst lib%.dylib,-l%,$(foreach lib,$(LINKED_LIBS),$(call gb_Library_get_filename,$(lib)))) > $${DYLIB_FILE} && \
	$(gb_CXX) \
		$(if $(filter Executable,$(TARGETTYPE)),$(gb_Executable_TARGETTYPEFLAGS)) \
		$(if $(filter Bundle,$(TARGETTYPE)),$(gb_Bundle_TARGETTYPEFLAGS)) \
		$(if $(filter Library CppunitTest,$(TARGETTYPE)),$(gb_Library_TARGETTYPEFLAGS)) \
		$(if $(filter Library,$(TARGETTYPE)),$(gb_Library_LTOFLAGS)) \
		$(subst \d,$$,$(RPATH)) \
		$(T_LDFLAGS) \
		$(call gb_LinkTarget__get_liblinkflags,$(LINKED_LIBS)) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(OBJCOBJECTS),$(call gb_ObjCObject_get_target,$(object))) \
		$(foreach object,$(OBJCXXOBJECTS),$(call gb_ObjCxxObject_get_target,$(object))) \
		$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach extraobjectlist,$(EXTRAOBJECTLISTS),`cat $(extraobjectlist)`) \
		$(foreach lib,$(LINKED_STATIC_LIBS),$(call gb_StaticLibrary_get_target,$(lib))) \
		$(LIBS) \
		-o $(if $(SOVERSION),$(1).$(SOVERSION),$(1)) \
		`cat $${DYLIB_FILE}` && \
	$(if $(SOVERSION),ln -sf $(notdir $(1)).$(SOVERSION) $(1),:) && \
    $(if $(filter Executable,$(TARGETTYPE)), \
        $(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl Executable \
            $(LAYER) $(1) &&) \
	$(if $(filter Library Bundle CppunitTest,$(TARGETTYPE)),\
		$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl Library $(LAYER) $(if $(SOVERSION),$(1).$(SOVERSION),$(1)) && \
		ln -sf $(notdir $(1)) $(basename $(1)).jnilib &&) \
	rm -f $${DYLIB_FILE})
endef

# parameters: 1-linktarget 2-cobjects 3-cxxobjects
define gb_LinkTarget__command_staticlink
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(gb_AR) -rsu $(1) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
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

gb_Library__FRAMEWORKS := \
	Cocoa \
	GLUT \
	OpenGL \

gb_Library_PLAINLIBS_NONE += \
	GLUT \
	Cocoa \
	objc \
	OpenGL \
	m \
	pthread \

gb_Library_FILENAMES := \
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_OOOEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_RTLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_RTEXT)) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_RTEXT)) \
	$(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):$(lib)$(gb_Library_UNOEXT)) \
	$(foreach lib,$(gb_Library_UNOLIBS_OOO),$(lib):$(lib)$(gb_Library_UNOEXT)) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(gb_Library_UNOVERPRE)$(lib)$(gb_Library_PLAINEXT)) \
	$(foreach lib,$(gb_Library_EXTENSIONLIBS),$(lib):$(lib)$(gb_Library_UNOEXT)) \


gb_Library_LAYER := \
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):NONE) \
	$(foreach lib,$(gb_Library_RTLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_UNOLIBS_OOO),$(lib):OOO) \
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
	$(foreach exe,$(gb_Executable_NONE),$(exe):NONE) \


gb_Executable_get_rpath :=

define gb_Executable_Executable_platform
$(call gb_LinkTarget_get_target,$(2)) : RPATH :=
$(call gb_LinkTarget_get_target,$(2)) : LAYER := $(call gb_Executable_get_layer,$(1))

endef


# CppunitTest class

ifeq ($(CROSS_COMPILING),YES)
gb_CppunitTest_CPPTESTPRECOMMAND := :
else
gb_CppunitTest_CPPTESTPRECOMMAND := $(gb_Helper_set_ld_path)
endif

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

define gb_JunitTest_JunitTest_platform
$(call gb_JunitTest_get_target,$(1)) : DEFS := \
	-Dorg.openoffice.test.arg.soffice="$$$${OOO_TEST_SOFFICE:-path:$(DEVINSTALLDIR)/opt/LibreOffice.app/Contents/MacOS/soffice}" \
	-Dorg.openoffice.test.arg.env=DYLD_LIBRARY_PATH \
	-Dorg.openoffice.test.arg.user=file://$(call gb_JunitTest_get_userdir,$(1)) \
	-Dorg.openoffice.test.arg.workdir=$(call gb_JunitTest_get_userdir,$(1)) \

endef

# InstallModuleTarget class

define gb_InstallModuleTarget_InstallModuleTarget_platform
$(call gb_InstallModuleTarget_add_defs,$(1),\
	$(gb_CPUDEFS) \
	$(gb_OSDEFS) \
	-DCOMID=gcc3 \
	-DCOMNAME=gcc3 \
	-D_gcc3 \
	$(if $(filter TRUE,$(SOLAR_JAVA)),-DSOLAR_JAVA) \
)

$(call gb_InstallModuleTarget_set_include,$(1),\
	$(SOLARINC) \
	$(SCP_INCLUDE) \
)

endef

# ScpConvertTarget class

gb_ScpConvertTarget_ScpConvertTarget_platform :=

# InstallScript class

gb_InstallScript_EXT := .ins

# ExtensionTarget class

gb_ExtensionTarget_LICENSEFILE_DEFAULT := $(OUTDIR)/bin/osl/LICENSE

# UnpackedTarget class

gb_UnpackedTarget_TARFILE_LOCATION := $(TARFILE_LOCATION)

# Python
gb_PYTHON_PRECOMMAND := DYLD_LIBRARY_PATH=$(OUTDIR)/lib

include $(GBUILDDIR)/platform/com_GCC_class.mk

# vim: set noet sw=4:
