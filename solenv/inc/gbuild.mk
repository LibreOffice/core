#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2009 by Sun Microsystems, Inc.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

# vars needed from the env
# CVER
# DEBUG
# GXX_INCLUDE_PATH
# INPATH
# JAVA_HOME
# LIBXML_CFLAGS
# OS
# PRODUCT
# PTHREAD_CFLAGS (Linux)
# SOLARINC
# SOLARLIB
# SRC_ROOT
# STLPORT_VER
# SYSTEM_ICU
# SYSTEM_JPEG
# SYSTEM_LIBXML
# UPD
# USE_SYSTEM_STL

SHELL := /bin/sh

ifeq ($(strip $(SRC_ROOT)),)
$(error No environment set)
endif

SRCDIR := $(SOLARSRC)
OUTDIR := $(SOLARVERSION)/$(INPATH)
WORKDIR := $(SOLARVERSION)/$(INPATH)/workdir

# HACK
ifeq ($(OS),WNT)
SRCDIR := $(shell cygpath -u $(SRCDIR))
WORKDIR := $(shell cygpath -u $(WORKDIR))
OUTDIR := $(shell cygpath -u $(OUTDIR))
endif
true := T
false :=

ifneq ($(strip $(PRODUCT)),)
gb_PRODUCT := $(true)
else
gb_PRODUCT := $(false)
endif

ifneq ($(strip $(DEBUG)),)
gb_DEBUGLEVEL := 2
else
ifeq ($(gb_PRODUCT),$(true))
gb_DEBUGLEVEL := 0
else
gb_DEBUGLEVEL := 1
endif
endif

gb_FULLDEPS := $(true)
#gb_FULLDEPS := $(false)
ifeq ($(MAKECMDGOALS),clean)
gb_FULLDEPS := $(false)
endif

include $(SRCDIR)/solenv/inc/gbhelper.mk
include $(SRCDIR)/solenv/inc/libnames.mk
gb_Library_TARGETS := $(foreach namescheme,$(gb_Library_NAMESCHEMES),$(gb_Library_$(namescheme)LIBS))
gb_StaticLibrary_TARGETS := $(foreach namescheme,$(gb_StaticLibrary_NAMESCHEMES),$(gb_StaticLibrary_$(namescheme)LIBS))

ifeq ($(OS),LINUX)
include $(SRCDIR)/solenv/inc/platform/linux.mk
else
ifeq ($(OS),WNT)
include $(SRCDIR)/solenv/inc/platform/windows.mk
else
$(error unsupported OS: $(OS))
endif
endif

gb_Library_KNOWNLIBS := $(foreach namescheme,$(gb_Library_NAMESCHEMES),$(gb_Library_$(namescheme)LIBS))
gb_StaticLibrary_KNOWNLIBS := $(foreach namescheme,$(gb_StaticLibrary_NAMESCHEMES),$(gb_StaticLibrary_$(namescheme)LIBS))

ifeq ($(USE_SYSTEM_STL),YES)
gb_Library_TARGETS := $(filter-out stl,$(gb_Library_TARGETS))
endif

ifeq ($(SYSTEM_LIBXML),YES)
gb_Library_TARGETS := $(filter-out xml2,$(gb_Library_TARGETS))
endif

ifeq ($(SYSTEM_ICU),YES)
gb_Library_TARGETS := $(filter-out icuuc,$(gb_Library_TARGETS))
endif

gb_GLOBALDEFS := \
    -D__DMAKE \
    -D_REENTRANT \
    -D_USE_NAMESPACE=1 \
    -DCUI \
    -DENABLE_LAYOUT_EXPERIMENTAL=0 \
    -DENABLE_LAYOUT=0 \
    -DNEW_SOLAR \
    -DOSL_DEBUG_LEVEL=$(gb_DEBUGLEVEL) \
    -DSOLAR_JAVA \
    -DSTLPORT_VERSION=$(STLPORT_VER) \
    -DSUPD=$(UPD) \
    -DVCL \
    $(gb_OSDEFS) \
    $(gb_COMPILERDEFS) \
    $(gb_CPUDEFS) \

ifeq ($(gb_PRODUCT),$(true))
gb_GLOBALDEFS += \
    -DPRODUCT \
    -DPRODUCT_FULL \

else
gb_GLOBALDEFS += \
    -DDBG_UTIL \
    -D_STLP_DEBUG \

endif

ifeq ($(gb_DEBUGLEVEL),2)
gb_GLOBALDEFS += \
    -DDEBUG \

else
gb_GLOBALDEFS += \
    -DOPTIMIZE \
    -DNDEBUG \

endif

gb_GLOBALDEFS := $(sort $(gb_GLOBALDEFS))

# target patterns

gb_CObject_get_target = $(WORKDIR)/CObject/$(1).o
gb_CxxObject_get_target = $(WORKDIR)/CxxObject/$(1).o
gb_LinkTarget_get_target = $(WORKDIR)/LinkTarget/$(1)

define gb_Library_get_filename
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_Library_FILENAMES)))
endef

define gb_Library_get_target
$(patsubst $(1):%,$(gb_Library_OUTDIRLOCATION)/%,$(filter $(filter $(1),$(gb_Library_TARGETS)):%,$(gb_Library_FILENAMES)))
endef

define gb_Library_get_headers_target
$(patsubst $(1):%,$(WORKDIR)/Headers/Library/%,$(filter $(1):%,$(gb_Library_FILENAMES)))
endef

define gb_StaticLibrary_get_filename
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_StaticLibrary_FILENAMES)))
endef

define gb_StaticLibrary_get_target
$(patsubst $(1):%,$(gb_StaticLibrary_OUTDIRLOCATION)/%,$(filter $(filter $(1),$(gb_StaticLibrary_TARGETS)):%,$(gb_StaticLibrary_FILENAMES)))
endef

define gb_StaticLibrary_get_headers_target
$(patsubst $(1):%,$(WORKDIR)/Headers/StaticLibrary/%,$(filter $(1):%,$(gb_StaticLibrary_FILENAMES)))
endef

gb_Executable_get_target = $(OUTDIR)/bin/$(1)$(gb_Executable_EXT)
gb_SdiTarget_get_target = $(WORKDIR)/SdiTarget/$(1)
gb_SrsPartTarget_get_target = $(WORKDIR)/SrsPartTarget/$(1)
gb_SrsTarget_get_target = $(WORKDIR)/SrsTarget/$(1).srs
gb_ResTarget_get_target = $(OUTDIR)/misc/res/$(1)
gb_AllLangResTarget_get_target = $(OUTDIR)/misc/AllLangRes/$(1)
gb_PackagePart_get_target = $(OUTDIR)/$(1)
gb_Package_get_target = $(OUTDIR)/misc/Package/$(1)
gb_Module_get_target = $(OUTDIR)/misc/Module/$(1)

$(eval $(call gb_Helper_make_clean_targets,\
    LinkTarget \
    PackagePart \
    SdiTarget \
    SrsTarget \
))

$(eval $(call gb_Helper_make_outdir_clean_targets,\
    AllLangResTarget \
    Executable \
    Library \
    Module \
    Package \
    ResTarget \
))

$(eval $(call gb_Helper_make_dep_targets,\
    CObject \
    CxxObject \
    LinkTarget \
    SrsPartTarget \
    SrsTarget \
))

gb_Executable_get_headers_target = $(WORKDIR)/Headers/Executable/$(1)
gb_Library_get_linktargetname = Library/$(1)
gb_LinkTarget_get_headers_target = $(WORKDIR)/Headers/$(1)
gb_PackagePart_get_destinations = $(OUTDIR)/xml $(OUTDIR)/inc
gb_ResTarget_get_imagelist_target = $(OUTDIR)/res/img/$(1).ilst

# static members 

gb_CObject_get_source = $(SRCDIR)/$(1).c
# defined by platform
#  gb_CObject_command
#  gb_CObject_command_dep

gb_CxxObject_get_source = $(SRCDIR)/$(1).cxx
# defined by platform
#  gb_CxxObject_command
#  gb_CxxObject_command_dep

gb_LinkTarget_DEFAULTDEFS := $(gb_GLOBALDEFS)
# defined by platform
#  gb_LinkTarget_CXXFLAGS
#  gb_LinkTarget_LDFLAGS
#  gb_LinkTarget_INCLUDE 

gb_Library_OUTDIRLOCATION := $(OUTDIR)/lib
gb_Library_DLLDIR := $(WORKDIR)/LinkTarget/Library
# defined by platform
#  gb_Library_DEFS
#  gb_Library_DLLFILENAMES
#  gb_Library_FILENAMES
#  gb_Library_TARGETS
#  gb_Library_TARGETTYPEFLAGS
#  gb_Library_Library_platform

gb_StaticLibrary_OUTDIRLOCATION := $(OUTDIR)/lib
# defined by platform
#  gb_StaticLibrary_DEFS
#  gb_StaticLibrary_FILENAMES
#  gb_StaticLibrary_TARGETS
#  gb_StaticLibrary_TARGETTYPEFLAGS

# defined by platform
#  gb_Executable_TARGETTYPEFLAGS
#  gb_Executable_Executable_platform

gb_SdiTarget_SVIDLTARGET := $(call gb_Executable_get_target,svidl)
gb_SdiTarget_SVIDLCOMMAND := $(gb_SdiTarget_SVIDLPRECOMMAND) $(gb_SdiTarget_SVIDLTARGET)

gb_CppuTarget_CPPUMAKERTARGET := $(OUTDIR)/bin/cppumaker
gb_CppuTarget_CPPUMAKERCOMMAND := LD_LIBRARY_PATH=$(OUTDIR)/lib $(gb_CppuTarget_CPPUMAKERTARGET)

# defined by platform
#  gb_SrsPartTarget_RSCTARGET
#  gb_SrsPartTarget_RSCCOMMAND
#  gb_SrsPartTarget_command_dep

gb_SrsTarget_DEFAULTDEFS := $(gb_GLOBALDEFS)

gb_ResTarget_RSCTARGET := $(gb_SrsPartTarget_RSCTARGET)
gb_ResTarget_RSCCOMMAND := $(gb_SrsPartTarget_RSCCOMMAND)

gb_AllLangResTarget_LANGS := en-US

gb_Module_ALLMODULES :=

# rule templates
# We are using a set of scopes that we might as well call classes.


# CObject class

$(call gb_CObject_get_target,%) : $(call gb_CObject_get_source,%)
    $(call gb_CObject_command,$@,$*,$<,$(DEFS),$(CFLAGS),$(INCLUDE))

$(call gb_CObject_get_dep_target,%) : $(call gb_CObject_get_source,%)
    $(call gb_CObject_command_dep,$@,$*,$<,$(DEFS),$(CFLAGS),$(INCLUDE))

$(call gb_CObject_get_dep_target,%) :
    $(error unable to find plain C file $(call gb_CObject_get_source,$*))


# CxxObject class

$(call gb_CxxObject_get_target,%) : $(call gb_CxxObject_get_source,%)
    $(call gb_CxxObject_command,$@,$*,$<,$(DEFS),$(CXXFLAGS),$(INCLUDE))

$(call gb_CxxObject_get_dep_target,%) : $(call gb_CxxObject_get_source,%)
    $(call gb_CxxObject_command_dep,$@,$*,$<,$(DEFS),$(CXXFLAGS),$(INCLUDE))

$(call gb_CxxObject_get_dep_target,%) :
    $(error unable to find C++ file $(call gb_CxxObject_get_source,$*))


# LinkTarget class

.PHONY : $(call gb_LinkTarget_get_clean_target,%)
$(call gb_LinkTarget_get_clean_target,%) :
    $(call gb_Helper_announce,Cleaning up link $* ...)
    -$(call gb_Helper_abbreviate_dirs,\
        rm -f \
            $(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
            $(foreach object,$(COBJECTS),$(call gb_CObject_get_dep_target,$(object))) \
            $(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
            $(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_dep_target,$(object))) \
            $(call gb_LinkTarget_get_target,$*) \
            $(call gb_LinkTarget_get_dep_target,$*) \
            $(call gb_LinkTarget_get_headers_target,$*) \
            $(DLLTARGET) \
            $(AUXTARGETS))

define gb_LinkTarget_command_dep
$(call gb_Helper_announce,Collecting dependencies for link $(2) ...)
$(call gb_Helper_abbreviate_dirs,\
    mkdir -p $(dir $(1)) && \
    cat $(gb_Helper_NULLFILE)\
        $(foreach object,$(3),$(call gb_CObject_get_dep_target,$(object))) \
        $(foreach object,$(4),$(call gb_CxxObject_get_dep_target,$(object))) > $(1))
endef
$(call gb_LinkTarget_get_target,%) :$(call gb_LinkTarget_get_headers_target,%) $(call gb_LinkTarget_get_dep_target,%) $(gb_Helper_NULLFILE)
    $(call gb_LinkTarget_command_dep,$(call gb_LinkTarget_get_dep_target,$*),$*,$(COBJECTS),$(CXXOBJECTS))
    $(call gb_LinkTarget_command,$@,$*,$(TARGETTYPE_FLAGS) $(LDFLAGS),$(LINKED_LIBS),$(LINKED_STATIC_LIBS),$(CXXOBJECTS),$(COBJECTS))

$(call gb_LinkTarget_get_dep_target,%) : $(call gb_LinkTarget_get_headers_target,%) $(gb_Helper_NULLFILE)
    $(call gb_LinkTarget_command_dep,$@,$*,$(COBJECTS),$(CXXOBJECTS))

define gb_LinkTarget_get_headers_check
ifneq ($$(SELF),$$*)
$$(info LinkTarget $$* not defined: Assuming headers to be there!)
endif
$$@ : COMMAND := $$(call gb_Helper_abbreviate_dirs, mkdir -p $$(dir $$@) && touch $$@)

endef

$(call gb_LinkTarget_get_headers_target,%) :
    $(eval $(gb_LinkTarget_get_headers_check))
    $(COMMAND)

define gb_LinkTarget_LinkTarget
$(call gb_LinkTarget_get_clean_target,$(1)) : AUXTARGETS :=
$(call gb_LinkTarget_get_dep_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : CFLAGS := $$(gb_LinkTarget_CFLAGS)
$(call gb_LinkTarget_get_clean_target,$(1)) \
$(call gb_LinkTarget_get_dep_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : COBJECTS := 
$(call gb_LinkTarget_get_dep_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : CXXFLAGS := $$(gb_LinkTarget_CXXFLAGS)
$(call gb_LinkTarget_get_clean_target,$(1)) \
$(call gb_LinkTarget_get_dep_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : CXXOBJECTS := 
$(call gb_LinkTarget_get_dep_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : DEFS := $$(gb_LinkTarget_DEFAULTDEFS)
$(call gb_LinkTarget_get_target,$(1)) : DLLTARGET := 
$(call gb_LinkTarget_get_dep_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : INCLUDE := $$(gb_LinkTarget_INCLUDE)
$(call gb_LinkTarget_get_target,$(1)) : LDFLAGS := $$(gb_LinkTarget_LDFLAGS)
$(call gb_LinkTarget_get_target,$(1)) : LINKED_LIBS := 
$(call gb_LinkTarget_get_target,$(1)) : LINKED_STATIC_LIBS := 
$(call gb_LinkTarget_get_headers_target,$(1)) : SELF := $(1)
$(call gb_LinkTarget_get_dep_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : TARGETTYPE_FLAGS := 
ifeq ($(gb_FULLDEPS),$(true))
include $(call gb_LinkTarget_get_dep_target,$(1))
endif

endef

define gb_LinkTarget_set_defs
$(call gb_LinkTarget_get_target,$(1)) : DEFS := $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : DEFS := $(2)
endef

define gb_LinkTarget_set_cflags
$(call gb_LinkTarget_get_target,$(1)) \
$(call gb_LinkTarget_get_dep_target,$(1)) : CFLAGS := $(2)
endef

define gb_LinkTarget_set_cxxflags
$(call gb_LinkTarget_get_target,$(1)) \
$(call gb_LinkTarget_get_dep_target,$(1)) : CXXFLAGS := $(2)
endef

define gb_LinkTarget_set_include
$(call gb_LinkTarget_get_target,$(1)) \
$(call gb_LinkTarget_get_dep_target,$(1)) : INCLUDE := $(2)
endef

define gb_LinkTarget_set_ldflags
$(call gb_LinkTarget_get_target,$(1)) : LDFLAGS := $(2)
endef

define gb_LinkTarget_add_linked_libs
ifneq (,$$(filter-out $(gb_Library_KNOWNLIBS),$(2)))
$$(info currently known libraries are: $(sort $(gb_Library_KNOWNLIBS)))
$$(error Cannot link against library/libraries $$(filter-out $(gb_Library_KNOWNLIBS),$(2)). These must be registered in $(SRCDIR)/solenv/inc/libnames.mk)
endif
$(call gb_LinkTarget_get_target,$(1)) : LINKED_LIBS += $(2)

$(call gb_LinkTarget_get_target,$(1)) : $$(foreach lib,$(2),$$(call gb_Library_get_target,$$(lib)))
$(call gb_LinkTarget_get_headers_target,$(1)) : \
$$(foreach lib,$(2),$$(call gb_Library_get_headers_target,$$(lib)))

endef

define gb_LinkTarget_add_linked_static_libs
ifneq (,$$(filter-out $(gb_StaticLibrary_KNOWNLIBS),$(2)))
$$(info currently known static libraries are: $(sort $(gb_StaticLibrary_KNOWNLIBS)))
$$(error Cannot link against static library/libraries $$(filter-out $(gb_StaticLibrary_KNOWNLIBS),$(2)). These must be registered in $(SRCDIR)/solenv/inc/libnames.mk)
endif
$(call gb_LinkTarget_get_target,$(1)) : LINKED_STATIC_LIBS += $(2)

$(call gb_LinkTarget_get_target,$(1)) : $$(foreach lib,$(2),$$(call gb_StaticLibrary_get_target,$$(lib)))
$(call gb_LinkTarget_get_headers_target,$(1)) : \
$$(foreach lib,$(2),$$(call gb_StaticLibrary_get_headers_target,$$(lib)))

endef

define gb_LinkTarget_add_cobject
$(call gb_LinkTarget_get_target,$(1)) : COBJECTS += $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : COBJECTS += $(2)
$(call gb_LinkTarget_get_clean_target,$(1)) : COBJECTS += $(2)

$(call gb_LinkTarget_get_dep_target,$(1)) : $(call gb_CObject_get_dep_target,$(2))
$(call gb_LinkTarget_get_target,$(1)) : $(call gb_CObject_get_target,$(2))
$(call gb_CObject_get_target,$(2)) : | $(call gb_LinkTarget_get_headers_target,$(1))
$(call gb_CObject_get_target,$(2)) : CFLAGS += $(3)

endef
define gb_LinkTarget_add_cxxobject
$(call gb_LinkTarget_get_target,$(1)) : CXXOBJECTS += $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : CXXOBJECTS += $(2)
$(call gb_LinkTarget_get_clean_target,$(1)) : CXXOBJECTS += $(2)

$(call gb_LinkTarget_get_dep_target,$(1)) : $(call gb_CxxObject_get_dep_target,$(2))
$(call gb_LinkTarget_get_target,$(1)) : $(call gb_CxxObject_get_target,$(2))
$(call gb_CxxObject_get_target,$(2)) : | $(call gb_LinkTarget_get_headers_target,$(1))
$(call gb_CxxObject_get_target,$(2)) : CXXFLAGS += $(3)

endef

define gb_LinkTarget_add_noexception_object
$(call gb_LinkTarget_add_cxxobject,$(1),$(2),$(gb_LinkTarget_NOEXCEPTIONFLAGS))
endef

define gb_LinkTarget_add_exception_object
$(call gb_LinkTarget_add_cxxobject,$(1),$(2),$(gb_LinkTarget_EXCEPTIONFLAGS))
endef

define gb_LinkTarget_add_cobjects
$(foreach obj,$(2),$(call gb_LinkTarget_add_cobject,$(1),$(obj),$(3)))
endef

define gb_LinkTarget_add_cxxobjects
$(foreach obj,$(2),$(call gb_LinkTarget_add_cxxobject,$(1),$(obj),$(3)))
endef

define gb_LinkTarget_add_noexception_objects
$(foreach obj,$(2),$(call gb_LinkTarget_add_noexception_object,$(1),$(obj)))
endef

define gb_LinkTarget_add_exception_objects
$(foreach obj,$(2),$(call gb_LinkTarget_add_exception_object,$(1),$(obj)))
endef

define gb_LinkTarget_set_targettype_flags
$(call gb_LinkTarget_get_target,$(1)) \
$(call gb_LinkTarget_get_dep_target,$(1)) : TARGETTYPE_FLAGS := $(2)
endef

define gb_LinkTarget_set_dlltarget
$(call gb_LinkTarget_get_clean_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : DLLTARGET := $(2)
endef

define gb_LinkTarget_set_auxtargets
$(call gb_LinkTarget_get_clean_target,$(1)) : AUXTARGETS := $(2)
endef


# Library class

.PHONY : $(WORKDIR)/Clean/OutDir/lib/%$(gb_Library_PLAINEXT)
$(WORKDIR)/Clean/OutDir/lib/%$(gb_Library_PLAINEXT) : $(call gb_LinkTarget_get_clean_target,$(call gb_Library_get_linktargetname,%$(gb_Library_PLAINEXT)))
    $(call gb_Helper_abbreviate_dirs,\
        rm -f $(OUTDIR)/lib/$*$(gb_Library_PLAINEXT))

$(gb_Library_OUTDIRLOCATION)/%$(gb_Library_PLAINEXT) : 
    $(call gb_Helper_abbreviate_dirs,\
        mkdir -p $(dir $@) && cp -pf $< $@)


define gb_Library_Library
ifeq (,$$(findstring $(1),$$(gb_Library_KNOWNLIBS)))
$$(info currently known libraries are: $(sort $(gb_Library_KNOWNLIBS)))
$$(error Library $(1) must be registered in $(SRCDIR)/solenv/inc/libnames.mk)
endif
$(call gb_Library_Library__impl,$(1),$(call gb_Library_get_linktargetname,$(call gb_Library_get_filename,$(1))))
endef

define gb_Library_Library__impl
$(call gb_LinkTarget_LinkTarget,$(2))
$(call gb_LinkTarget_set_targettype_flags,$(2),$(gb_Library_TARGETTYPEFLAGS))
$(call gb_LinkTarget_set_defs,$(2),\
    $$(DEFS) \
    $(gb_Library_DEFS) \
)
$(call gb_Library_get_target,$(1)) : $(call gb_LinkTarget_get_target,$(2))
$(call gb_Library_Library_platform,$(1),$(2),$(gb_Library_DLLDIR)/$(call gb_Library_get_dllname,$(1)))

endef

define gb_Library_forward_to_Linktarget
gb_Library_$(1) = $$(call gb_LinkTarget_$(1),$(call gb_Library_get_linktargetname,$$(call gb_Library_get_filename,$$(1))),$$(2))

endef

$(eval $(foreach method,\
    add_cobject \
    add_cobjects \
    add_cxxobject \
    add_cxxobjects \
    add_exception_objects \
    add_noexception_objects \
    set_cxxflags \
    set_defs \
    set_include \
    set_ldflags \
    set_library_path_flags \
    add_linked_libs \
    add_linked_static_libs \
,\
    $(call gb_Library_forward_to_Linktarget,$(method))\
))


# Static Library class

.PHONY : $(WORKDIR)/Clean/OutDir/lib/%$(gb_StaticLibrary_PLAINEXT)
$(WORKDIR)/Clean/OutDir/lib/%$(gb_StaticLibrary_PLAINEXT) : $(call gb_LinkTarget_get_clean_target,$(call gb_Library_get_linktargetname,%$(gb_StaticLibrary_PLAINEXT)))
    $(call gb_Helper_abbreviate_dirs,\
        rm -f $(OUTDIR)/lib/$*$(gb_StaticLibrary_PLAINEXT))

$(gb_StaticLibrary_OUTDIRLOCATION)/%$(gb_StaticLibrary_PLAINEXT) : 
    $(call gb_Helper_abbreviate_dirs,\
        mkdir -p $(dir $@) && cp -pf $< $@)

define gb_StaticLibrary_StaticLibrary
ifeq (,$$(findstring $(1),$$(gb_StaticLibrary_KNOWNLIBS)))
$$(info currently known static libraries are: $(sort $(gb_StaticLibrary_KNOWNLIBS)))
$$(error Library $(1) must be registered in $(SRCDIR)/solenv/inc/libnames.mk)
endif
$(call gb_StaticLibrary_StaticLibrary__impl,$(1),$(call gb_StaticLibrary_get_linktargetname,$(call gb_StaticLibrary_get_filename,$(1))))

endef

define gb_StaticLibrary_StaticLibrary__impl
$(call gb_LinkTarget_LinkTarget,$(2))
$(call gb_LinkTarget_set_targettype_flags,$(2),$(gb_StaticLibrary_TARGETTYPEFLAGS))
$(call gb_LinkTarget_set_defs,$(2)),\
    $$(DEFS) \
    $(gb_StaticLibrary_DEFS) \
)
$(call gb_StaticLibrary_get_target,$(1)) : $(call gb_StaticLibrary_get_target,$(2))

endef

define gb_StaticLibrary_forward_to_Linktarget
gb_StaticLibrary_$(1) = $$(call gb_LinkTarget_$(1),$(call gb_Library_get_linktargetname,$$(call gb_Library_get_filename,$$(1))),$$(2))

endef

$(eval $(foreach method,\
    add_cobject \
    add_cobjects \
    add_cxxobject \
    add_cxxobjects \
    add_exception_objects \
    add_noexception_objects \
    set_cxxflags \
    set_defs \
    set_include \
    set_ldflags \
    set_library_path_flags \
    add_linked_libs \
    add_linked_static_libs \
,\
    $(call gb_StaticLibrary_forward_to_Linktarget,$(method))\
))


# Executable class

.PHONY : $(call gb_Executable_get_clean_target,%)
$(call gb_Executable_get_clean_target,%) :
    $(call gb_Helper_abbreviate_dirs,\
        rm -f $(call gb_Executable_get_target,$*))

$(call gb_Executable_get_target,%) :
    $(call gb_Helper_abbreviate_dirs,\
        mkdir -p $(dir $@) && cp -pf $< $@)

define gb_Executable_Executable
$(call gb_Executable_Executable__impl,$(1),Executable/$(1)$(gb_Executable_EXT))

endef

define gb_Executable_Executable__impl
$(call gb_LinkTarget_LinkTarget,$(2))
$(call gb_LinkTarget_set_targettype_flags,$(2),$(gb_Executable_TARGETTYPEFLAGS))
$(call gb_Executable_get_target,$(1)) : $(call gb_LinkTarget_get_target,$(2))
$(call gb_Executable_get_clean_target,$(1)) : $(call gb_LinkTarget_get_clean_target,$(2))
$(call gb_Executable_Executable_platform,$(1),$(2))

endef

define gb_Executable_forward_to_Linktarget
gb_Executable_$(1) = $$(call gb_LinkTarget_$(1),Executable/$$(1)$$(gb_Executable_EXT),$$(2))

endef

$(eval $(foreach method,\
    add_cobject \
    add_cobjects \
    add_cxxobject \
    add_cxxobjects \
    add_exception_objects \
    add_noexception_objects \
    set_cxxflags \
    set_defs \
    set_include \
    set_ldflags \
    set_library_path_flags \
    add_linked_libs \
    add_linked_static_libs \
,\
    $(call gb_Executable_forward_to_Linktarget,$(method))\
))


# SdiTarget class

$(call gb_SdiTarget_get_target,%) : $(SRCDIR)/%.sdi | $(gb_SdiTarget_SVIDLTARGET) $(call gb_Library_get_target,tl) $(call gb_Library_get_target,sal)
    $(call gb_Helper_announce,Processing sdi $* ...)
    $(call gb_Helper_abbreviate_dirs,\
        mkdir -p $(dir $(WORKDIR)/inc/$*) $(dir $@))
    $(call gb_Helper_abbreviate_dirs_native,\
        cd $(dir $<) && \
        $(gb_SdiTarget_SVIDLCOMMAND) -quiet \
            $(INCLUDE) \
            -fs$(WORKDIR)/inc/$*.hxx \
            -fd$(WORKDIR)/inc/$*.ilb \
            -fl$(WORKDIR)/inc/$*.lst \
            -fz$(WORKDIR)/inc/$*.sid \
            -fx$(EXPORTS) \
            -fm$@ \
            $<)

.PHONY : $(call gb_SdiTarget_get_clean_target,%)
$(call gb_SdiTarget_get_clean_target,%) :
    $(call gb_Helper_announce,Cleaning up sdi $* ...)
    -$(call gb_Helper_abbreviate_dirs,\
        rm -f $(WORKDIR)/inc/$*.hxx \
            $(WORKDIR)/inc/$*.ilb \
            $(WORKDIR)/inc/$*.lst \
            $(WORKDIR)/inc/$*.sid \
            $(call gb_SdiTarget_get_target,$*))

define gb_SdiTarget_SdiTarget
$(call gb_SdiTarget_get_target,$(1)) : INCLUDE := -I$(WORKDIR)/inc/ $$(subst -I. ,-I$$(dir $(SRCDIR)/$(1)) ,$$(SOLARINC))
$(call gb_SdiTarget_get_target,$(1)) : EXPORTS := $(SRCDIR)/$(2).sdi
$(WORKDIR)/inc/$(1).hxx \
$(WORKDIR)/inc/$(1).ilb \
$(WORKDIR)/inc/$(1).lst \
$(WORKDIR)/inc/$(1).sid : $(call gb_SdiTarget_get_target,$(1))

endef

define gb_SdiTarget_set_include
$(call gb_SdiTarget_get_target,$(1)) : INCLUDE := $(2)

endef


# SrsPartTarget class

$(call gb_SrsPartTarget_get_target,%) : $(SRCDIR)/% $(gb_Helper_MISCDUMMY) | $(gb_SrsPartTarget_RSCTARGET)
    $(call gb_SrsPartTarget_command_dep,$*,$<,$(INCLUDE),$(DEFS))
    $(call gb_Helper_abbreviate_dirs_native,\
        mkdir -p $(dir $@) && \
        RESPONSEFILE=$$(mktemp -p $(gb_Helper_MISC)) && \
        echo "-s \
            $(INCLUDE) \
            $(DEFS) \
            -fp=$@ \
            $<" > $${RESPONSEFILE} && \
        $(gb_SrsPartTarget_RSCCOMMAND) -presponse @$${RESPONSEFILE})

$(call gb_SrsPartTarget_get_dep_target,%) : $(SRCDIR)/%
    $(call gb_Helper_abbreviate_dirs,\
        mkdir -p $(dir $@) && \
        echo '$(call gb_SrsPartTarget_get_target,$*) : $(gb_Helper_PHONY)' > $@)

$(call gb_SrsPartTarget_get_dep_target,%) :
    $(error unable to find resource definition file $(SRCDIR)/$*)

define gb_SrsPartTarget_SrsPartTarget
endef


# SrsTarget class

.PHONY : $(call gb_SrsTarget_get_clean_target,%)
$(call gb_SrsTarget_get_clean_target,%) :
    $(call gb_Helper_announce,Cleaning up srs $* ...)
    -$(call gb_Helper_abbreviate_dirs,\
        rm -f $(call gb_SrsTarget_get_target,$*) \
            $(call gb_SrsTarget_get_dep_target,$*) \
            $(foreach part,$(PARTS),$(call gb_SrsPartTarget_get_target,$(part))) \
            $(foreach part,$(PARTS),$(call gb_SrsPartTarget_get_dep_target,$(part))))


define gb_SrsTarget_command_dep
$(call gb_Helper_announce,Collecting dependencies for srs $(2) ...)
$(call gb_Helper_abbreviate_dirs,\
    mkdir -p $(dir $(1)) && \
    cat $(3) > $(1))
endef

$(call gb_SrsTarget_get_target,%) :
    $(call gb_SrsTarget_command_dep,$(call gb_SrsTarget_get_dep_target,$*),$*,$(foreach part,$(PARTS),$(call gb_SrsPartTarget_get_dep_target,$(part))))
    $(call gb_Helper_announce,Processing srs $* ...)
    $(call gb_Helper_abbreviate_dirs,\
        mkdir -p $(dir $@) && \
        cat $^ > $@)

$(call gb_SrsTarget_get_dep_target,%) :
    $(call gb_SrsTarget_command_dep,$@,$*,$^)

define gb_SrsTarget_SrsTarget
$(call gb_SrsTarget_get_target,$(1)) : DEFS := $(gb_SrsTarget_DEFAULTDEFS)
$(call gb_SrsTarget_get_target,$(1)) : INCLUDE := $(SOLARINC)
$(call gb_SrsTarget_get_clean_target,$(1)) : PARTS :=
$(call gb_SrsTarget_get_target,$(1)) : PARTS :=
ifeq ($(gb_FULLDEPS),$(true))
include $(call gb_SrsTarget_get_dep_target,$(1))
endif

endef

define gb_SrsTarget_set_defs
$(call gb_SrsTarget_get_target,$(1)) : DEFS := $(2)
$(call gb_SrsTarget_get_dep_target,$(1)) : DEFS := $(2)

endef

define gb_SrsTarget_set_include
$(call gb_SrsTarget_get_target,$(1)) : INCLUDE := $(2)
$(call gb_SrsTarget_get_dep_target,$(1)) : INCLUDE := $(2)

endef

define gb_SrsTarget_add_file
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_SrsTarget_get_dep_target,$(1)) : $(call gb_SrsPartTarget_get_dep_target,$(2))
endif
$(call gb_SrsTarget_get_target,$(1)) : $(call gb_SrsPartTarget_get_target,$(2))
$(call gb_SrsTarget_get_clean_target,$(1)) : PARTS += $(2)
$(call gb_SrsTarget_get_target,$(1)) : PARTS += $(2)

endef

define gb_SrsTarget_add_files
$(foreach file,$(2),$(call gb_SrsTarget_add_file,$(1),$(file)))

endef


# ResTarget

$(call gb_ResTarget_get_clean_target,%) :
    $(call gb_Helper_announce,Cleaning up resource $* ...)
    $(call gb_Helper_abbreviate_dirs,\
        rm -f $(OUTDIR)/bin/$*.res $(call gb_ResTarget_get_target,$*))

$(call gb_ResTarget_get_target,%) : $(gb_Helper_MISCDUMMY) | $(gb_ResTarget_RSCTARGET)
    $(call gb_Helper_announce,Building resource $@ ...)
    $(call gb_Helper_abbreviate_dirs_native,\
        mkdir -p $(dir $@) $(OUTDIR)/bin && \
        RESPONSEFILE=$$(mktemp -p $(gb_Helper_MISC)) && \
        echo "-r -p \
            -lg$(LANGUAGE) \
            -fs=$(OUTDIR)/bin/$(LIBRARY)$(LANGUAGE).res \
            -lip=$(SRCDIR)/default_images/$(RESLOCATION)/imglst/$(LANGUAGE) \
            -lip=$(SRCDIR)/default_images/$(RESLOCATION)/imglst \
            -lip=$(SRCDIR)/default_images/$(RESLOCATION)/res/$(LANGUAGE) \
            -lip=$(SRCDIR)/default_images/$(RESLOCATION)/res \
            -lip=$(SRCDIR)/default_images/$(RESLOCATION) \
            -lip=$(SRCDIR)/default_images/res/$(LANGUAGE) \
            -lip=$(SRCDIR)/default_images/res \
            -subMODULE=$(SRCDIR)/default_images \
            -subGLOBALRES=$(SRCDIR)/default_images/res \
            -oil=$(dir $(call gb_ResTarget_get_imagelist_target,$(1))) \
            -ft=$@ \
            $(filter-out $(gb_Helper_MISCDUMMY),$^)" > $${RESPONSEFILE} && \
        $(gb_ResTarget_RSCCOMMAND) @$${RESPONSEFILE} && \
        rm -f $${RESPONSEFILE})

define gb_ResTarget_ResTarget
$(call gb_ResTarget_get_target,$(1)) : LIBRARY = $(2)
$(call gb_ResTarget_get_target,$(1)) : LANGUAGE = $(3)
$(call gb_ResTarget_get_target,$(1)) : RESLOCATION = $(2)
$(call gb_AllLangResTarget_get_target,$(2)) : $(call gb_ResTarget_get_target,$(1))
$(call gb_AllLangResTarget_get_clean_target,$(2)) : $(call gb_ResTarget_get_clean_target,$(1))
$(call gb_ResTarget_get_imagelist_target,$(1)) : $(call gb_ResTarget_get_target,$(1))

endef

define gb_ResTarget_add_file
$(call gb_ResTarget_get_target,$(1)) : $(2)

endef

define gb_ResTarget_add_one_srs
$(call gb_ResTarget_add_file,$(1),$(call gb_SrsTarget_get_target,$(2)))
$(call gb_ResTarget_get_clean_target,$(1)) : $(call gb_SrsTarget_get_clean_target,$(2))

endef

define gb_ResTarget_add_files
$(foreach file,$(2),\
    $(call gb_ResTarget_add_file,$(1),$(file)))

endef

define gb_ResTarget_add_srs
$(foreach srs,$(2),\
    $(call gb_ResTarget_add_one_srs,$(1),$(srs)))

endef

define gb_ResTarget_set_reslocation
$(call gb_ResTarget_get_target,$(1)) : RESLOCATION = $(2)

endef


# AllLangResTarget

define gb_AllLangResTarget_set_langs
gb_AllLangResTarget_LANGS := $(1)
endef

$(call gb_AllLangResTarget_get_clean_target,%) :
    $(call gb_Helper_abbreviate_dirs,\
        rm -f $(call gb_AllLangResTarget_get_target,$*))

$(call gb_AllLangResTarget_get_target,%) :
    $(call gb_Helper_abbreviate_dirs,\
        mkdir -p $(dir $@) && touch $@)

define gb_AllLangResTarget_AllLangResTarget
$(call gb_ResTarget_ResTarget,$(1)en-US,$(1),en-US)
endef

define gb_AllLangResTarget_add_file
$(foreach lang,$(gb_AllLangResTarget_LANGS),\
    $(call gb_ResTarget_add_file,$(1)$(lang),$(2)))

endef

define gb_AllLangResTarget_add_one_srs
$(foreach lang,$(gb_AllLangResTarget_LANGS),\
    $(call gb_ResTarget_add_one_srs,$(1)$(lang),$(2)))

endef

define gb_AllLangResTarget_add_files
$(foreach file,$(2),\
    $(call gb_AllLangResTarget_add_file,$(1),$(file)))

endef

define gb_AllLangResTarget_add_srs
$(foreach srs,$(2),\
    $(call gb_AllLangResTarget_add_one_srs,$(1),$(srs)))

endef

define gb_AllLangResTarget_set_reslocation
$(foreach lang,$(gb_AllLangResTarget_LANGS),\
    $(call gb_ResTarget_set_reslocation,$(1)$(lang),$(2)))

endef


# PackagePart class

$(foreach destination,$(call gb_PackagePart_get_destinations), $(destination)/%) :
    mkdir -p $(dir $@) && cp -pf $< $@

define gb_PackagePart_PackagePart
$(OUTDIR)/$(1) : $(2) 
endef


# Package class

.PHONY : $(call gb_Package_get_clean_target,%)
$(call gb_Package_get_clean_target,%) :
    $(call gb_Helper_announce,Cleaning up package $* ...)
    -$(call gb_Helper_abbreviate_dirs,\
        rm -f $(FILES))

$(call gb_Package_get_target,%) :
    $(call gb_Helper_announce,Copied all for package $* ...)
    mkdir -p $(dir $@) && touch $@

define gb_Package_Package
gb_TARGET_PACKAGE_$(1)_SOURCEDIR := $(2)
$(call gb_Package_get_clean_target,$(1)) : FILES := $(call gb_Package_get_target,$(1))

endef

define gb_Package_add_file
$(call gb_Package_get_target,$(1)) : $(OUTDIR)/$(2)
$(call gb_Package_get_clean_target,$(1)) : FILES += $(OUTDIR)/$(2)
$(call gb_PackagePart_PackagePart,$(2),$$(gb_TARGET_PACKAGE_$(1)_SOURCEDIR)/$(3))
$(OUTDIR)/$(2) : $$(gb_TARGET_PACKAGE_$(1)_SOURCEDIR)/$(3)

endef


# Module class

.PHONY : $(call gb_Module_get_clean_target,%)
$(call gb_Module_get_clean_target,%) :
    $(call gb_Helper_announce,Cleaning up module $* ...)
    -$(call gb_Helper_abbreviate_dirs,\
        rm -f $(call gb_Module_get_target,$*))

$(call gb_Module_get_target,%) :
    $(call gb_Helper_announce,Completed module $* ...)
    -$(call gb_Helper_abbreviate_dirs,\
        mkdir -p $(dir $@) && \
        touch $@)

define gb_Module_Module
$(call gb_Module_get_target,$(1)) : $(2)
$(call gb_Module_get_clean_target,$(1)) : $$(foreach target,$(2),$(call gb_Helper_get_outdir_clean_target,$$(target)))
gb_Module_ALLMODULES += $(1)

endef

define gb_Module_make_global_targets
include $(SRCDIR)/$(1)/prj/target_module_$(1).mk

.PHONY : all clean
all : $(call gb_Module_get_target,$(1))
clean : $(call gb_Module_get_clean_target,$(1))
.DEFAULT_GOAL := all

endef

define gb_Module_read_includes
include $$(foreach targetdef,$(2),$(SRCDIR)/$(1)/prj/target_$$(targetdef).mk)

endef

# vim: set noet sw=4 ts=4:
