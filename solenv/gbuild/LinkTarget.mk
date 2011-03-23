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


# CObject class

gb_CObject_REPOS := $(gb_REPOS)

gb_CObject_get_source = $(1)/$(2).c
# defined by platform
#  gb_CObject__command

# this rule generates an "always rebuild" dep file, to have something to include.
# the dep file will be overridden on the fly, when the object is compiled
ifeq ($(gb_FULLDEPS),$(true))
define gb_CObject__command_dep
mkdir -p $(dir $(1)) && \
    echo '$(call gb_CObject_get_target,$(2)) : $$(gb_Helper_PHONY)' > $(1)

endef
else
gb_CObject__command_dep =
endif

define gb_CObject__rules
$$(call gb_CObject_get_target,%) : $$(call gb_CObject_get_source,$(1),%)
    $$(call gb_CObject__command,$$@,$$*,$$<,$$(DEFS),$$(CFLAGS),$$(INCLUDE))

ifeq ($(gb_FULLDEPS),$(true))
$$(call gb_CObject_get_dep_target,%) : $$(call gb_CObject_get_source,$(1),%)
    $$(call gb_CObject__command_dep,$$@,$$*,$$<,$$(DEFS),$$(CFLAGS),$$(INCLUDE))
endif

endef

$(foreach repo,$(gb_CObject_REPOS),$(eval $(call gb_CObject__rules,$(repo))))

$(call gb_CObject_get_dep_target,%) :
    $(eval $(call gb_Output_error,Unable to find plain C file $(call gb_CObject_get_source,,$*) in the repositories: $(gb_CObject_REPOS)))

gb_CObject_CObject =


# CxxObject class

gb_CxxObject_REPOS := $(gb_REPOS)

gb_CxxObject_get_source = $(1)/$(2).cxx
# defined by platform
#  gb_CxxObject__command

# This rule generates an "always rebuild" dep file, to have something to
# include. The dep file will be overridden on the fly, when the object is
# compiled.
ifeq ($(gb_FULLDEPS),$(true))
define gb_CxxObject__command_dep
mkdir -p $(dir $(1)) && \
    echo '$(call gb_CxxObject_get_target,$(2)) : $$(gb_Helper_PHONY)' > $(1)

endef
else
gb_CxxObject__command_dep =
endif

# Only enable PCH if the PCH_CXXFLAGS and the PCH_DEFS (from the linktarget)
# are the same as the CXXFLAGS and DEFS we want to use for this object. This
# should usually be the case.  The DEFS/CXXFLAGS would have too be manually
# overridden for one object file for them to differ.  PCH_CXXFLAGS/PCH_DEFS
# should never be overridden on an object -- they should be the same as for the
# whole linktarget. In general it should be cleaner to use a static library
# compiled with different flags and link that in rather than mixing different
# flags in one linktarget.
define gb_CxxObject__set_pchflags
ifeq ($(gb_ENABLE_PCH),$(true))
ifneq ($(strip $$(PCH_NAME)),)
ifeq ($$(sort $$(PCH_CXXFLAGS) $$(PCH_DEFS) $$(gb_LinkTarget_EXCEPTIONFLAGS)),$$(sort $$(CXXFLAGS) $$(DEFS)))
$$@ : PCHFLAGS := $$(call gb_PrecompiledHeader_get_enableflags,$$(PCH_NAME))
else
ifeq ($$(sort $$(PCH_CXXFLAGS) $$(PCH_DEFS) $$(gb_LinkTarget_NOEXCEPTIONFLAGS)),$$(sort $$(CXXFLAGS) $$(DEFS)))
$$@ : PCHFLAGS := $$(call gb_NoexPrecompiledHeader_get_enableflags,$$(PCH_NAME))
else
$$(info No precompiled header available for $$*.)
$$(info precompiled header flags (  ex) : $$(sort $$(PCH_CXXFLAGS) $$(PCH_DEFS) $$(gb_LinkTarget_EXCEPTIONFLAGS)))
$$(info precompiled header flags (noex) : $$(sort $$(PCH_CXXFLAGS) $$(PCH_DEFS) $$(gb_LinkTarget_NOEXCEPTIONFLAGS)))
$$(info .           object flags        : $$(sort $$(CXXFLAGS) $$(DEFS)))
$$@ : PCHFLAGS := 
endif
endif
endif
endif
endef

define gb_CxxObject__rules
$$(call gb_CxxObject_get_target,%) : $$(call gb_CxxObject_get_source,$(1),%)
    $$(eval $$(gb_CxxObject__set_pchflags))
    $$(call gb_CxxObject__command,$$@,$$*,$$<)

ifeq ($(gb_FULLDEPS),$(true))
$$(call gb_CxxObject_get_dep_target,%) : $$(call gb_CxxObject_get_source,$(1),%)
    $$(eval $$(gb_CxxObject__set_pchflags))
    $$(call gb_CxxObject__command_dep,$$@,$$*,$$<)
endif

endef

$(foreach repo,$(gb_CxxObject_REPOS),$(eval $(call gb_CxxObject__rules,$(repo))))

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_CxxObject_get_dep_target,%) :
    $(eval $(call gb_Output_error,Unable to find C++ file $(call gb_CxxObject_get_source,,$*) in repositories: $(gb_CxxObject_REPOS)))

endif

gb_CxxObject_CxxObject =


# GenCxxObject class

gb_GenCxxObject_get_source = $(WORKDIR)/$(1).cxx
# defined by platform
#  gb_CxxObject__command

# This rule generates an "always rebuild" dep file, to have something to
# include. The dep file will be overridden on the fly, when the object is
# compiled.
ifeq ($(gb_FULLDEPS),$(true))
define gb_GenCxxObject__command_dep
mkdir -p $(dir $(1)) && \
    echo '$(call gb_GenCxxObject_get_target,$(2)) : $$(gb_Helper_PHONY)' > $(1)
endef
else
gb_GenCxxObject__command_dep =
endif

$(call gb_GenCxxObject_get_target,%) : $(call gb_GenCxxObject_get_source,%)
    $(call gb_CxxObject__command,$@,$*,$<)

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_GenCxxObject_get_dep_target,%) : $(call gb_GenCxxObject_get_source,%)
    $(call gb_GenCxxObject__command_dep,$@,$*,$<)

$(call gb_GenCxxObject_get_dep_target,%) :
    $(eval $(call gb_Output_error,Unable to find generated C++ file $(call gb_GenCxxObject_get_source,$*) in WORKDIR.))
endif

gb_GenCxxObject_GenCxxObject =


# ObjCxxObject class
#
gb_ObjCxxObject_REPOS := $(gb_REPOS)

gb_ObjCxxObject_get_source = $(1)/$(2).mm
# defined by platform
#  gb_ObjCxxObject__command

# this rule generates an "always rebuild" dep file, to have something to include.
# the dep file will be overridden on the fly, when the object is compiled
ifeq ($(gb_FULLDEPS),$(true))
define gb_ObjCxxObject__command_dep
mkdir -p $(dir $(1)) && \
    echo '$(call gb_ObjCxxObject_get_target,$(2)) : $$(gb_Helper_PHONY)' > $(1)

endef
else
gb_ObjCxxObject__command_dep =
endif

define gb_ObjCxxObject__rules
$$(call gb_ObjCxxObject_get_target,%) : $$(call gb_ObjCxxObject_get_source,$(1),%)
    $$(call gb_ObjCxxObject__command,$$@,$$*,$$<,$$(DEFS),$$(OBJCXXFLAGS),$$(INCLUDE_STL) $$(INCLUDE))

ifeq ($(gb_FULLDEPS),$(true))
$$(call gb_ObjCxxObject_get_dep_target,%) : $$(call gb_ObjCxxObject_get_source,$(1),%)
    $$(call gb_ObjCxxObject__command_dep,$$@,$$*,$$<,$$(DEFS),$$(OBJCXXFLAGS),$$(INCLUDE_STL) $$(INCLUDE))
endif

endef

$(foreach repo,$(gb_ObjCxxObject_REPOS),$(eval $(call gb_ObjCxxObject__rules,$(repo))))

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_ObjCxxObject_get_dep_target,%) :
    $(eval $(call gb_Output_error,Unable to find Objective C++ file $(call gb_ObjCxxObject_get_source,,$*) in repositories: $(gb_ObjCxxObject_REPOS)))
endif

gb_ObjCxxObject_ObjCxxObject =



# LinkTarget class

gb_LinkTarget_DEFAULTDEFS := $(gb_GLOBALDEFS)
# defined by platform
#  gb_LinkTarget_CXXFLAGS
#  gb_LinkTarget_LDFLAGS
#  gb_LinkTarget_INCLUDE 
#  gb_LinkTarget_INCLUDE_STL

.PHONY : $(call gb_LinkTarget_get_clean_target,%)
$(call gb_LinkTarget_get_clean_target,%) :
    $(call gb_Output_announce,$*,$(false),LNK,4)
    RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),200,\
        $(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
        $(foreach object,$(COBJECTS),$(call gb_CObject_get_dep_target,$(object))) \
        $(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
        $(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_dep_target,$(object))) \
        $(foreach object,$(OBJCXXOBJECTS),$(call gb_ObjCxxObject_get_target,$(object))) \
        $(foreach object,$(OBJCXXOBJECTS),$(call gb_ObjCxxObject_get_dep_target,$(object))) \
        $(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
        $(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_dep_target,$(object))) \
        $(call gb_LinkTarget_get_target,$*) \
        $(call gb_LinkTarget_get_dep_target,$*) \
        $(call gb_LinkTarget_get_headers_target,$*) \
        $(call gb_LinkTarget_get_external_headers_target,$*) \
        $(DLLTARGET) \
        $(AUXTARGETS)) && \
    cat $${RESPONSEFILE} /dev/null | xargs -n 200 rm -f && \
    rm -f $${RESPONSEFILE}


# cat the deps of all objects in one file, then we need only open that one file
define gb_LinkTarget__command_dep
$(call gb_Output_announce,LNK:$(2),$(true),DEP,1)
$(call gb_Helper_abbreviate_dirs,\
    mkdir -p $(dir $(1)) && \
    RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),200,\
        $(foreach object,$(3),$(call gb_CObject_get_dep_target,$(object))) \
        $(foreach object,$(4),$(call gb_CxxObject_get_dep_target,$(object))) \
        $(foreach object,$(5),$(call gb_ObjCxxObject_get_dep_target,$(object)))\
         $(foreach object,$(6),$(call gb_GenCxxObject_get_dep_target,$(object)))\
        ) && \
    cat $${RESPONSEFILE} /dev/null | xargs -n 200 cat > $(1)) && \
    rm -f $${RESPONSEFILE}

endef

$(call gb_LinkTarget_get_target,%) : $(call gb_LinkTarget_get_headers_target,%) $(gb_Helper_MISCDUMMY)
    $(call gb_LinkTarget__command,$@,$*)

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_target,%) : $(call gb_LinkTarget_get_dep_target,%)
$(call gb_LinkTarget_get_dep_target,%) : | $(call gb_LinkTarget_get_headers_target,%)
    $(call gb_LinkTarget__command_dep,$@,$*,$(COBJECTS),$(CXXOBJECTS),$(OBJCXXOBJECTS),$(GENCXXOBJECTS))
endif

# Ok, this is some dark voodoo: When declaring a linktarget with
# gb_LinkTarget_LinkTarget we set SELF in the headertarget to name of the
# target. When the rule for the headertarget is executed and SELF does not
# match the target name, we are depending on a linktarget that was never
# declared. In a full build exclusively in gbuild that should never happen.
# However, partial gbuild build will not know about how to build lower level
# linktargets, just as gbuild can not know about linktargets generated in the
# old build.pl/dmake system. Once all is migrated, gbuild should error out
# when is is told to depend on a linktarget it does not know about and not
# only warn.
define gb_LinkTarget__get_external_headers_check
ifneq ($$(SELF),$$*)
$$(eval $$(call gb_Output_info,LinkTarget $$* not defined: Assuming headers to be there!,ALL))
endif
$$@ : COMMAND := $$(call gb_Helper_abbreviate_dirs, mkdir -p $$(dir $$@) && touch $$@ && mkdir -p $(call gb_LinkTarget_get_target,)pdb/$$(dir $$*))

endef

$(call gb_LinkTarget_get_external_headers_target,%) :
    $(eval $(gb_LinkTarget__get_external_headers_check))
    $(COMMAND)

$(call gb_LinkTarget_get_headers_target,%) : $(call gb_LinkTarget_get_external_headers_target,%)
    $(call gb_Helper_abbreviate_dirs,\
        mkdir -p $(dir $@) && touch $@)

# Explanation of some of the targets:
# - gb_LinkTarget_get_external_headers_target is the targets that guarantees all
#   headers from linked against libraries are in OUTDIR.
# - gb_LinkTarget_get_headers_target is the target that guarantees all headers
#   from the linked against the libraries and the linktargets own headers
#   (including generated headers) are in the OUTDIR.
# - gb_LinkTarget_get_target links the objects into a file in WORKDIR.
# gb_LinkTarget_get_target depends on gb_LinkTarget_get_headers_target which in
# turn depends gb_LinkTarget_get_external_headers_target.
# gb_LinkTarget_get_target depends additionally on the objects, which in turn
# depend build-order only on the gb_LinkTarget_get_headers_target. The build
# order-only dependency ensures all headers to be there for compiling and
# dependency generation without causing all objects to be rebuild when one
# header changes. Only the ones with an explicit dependency in their generated
# dependency file will be rebuild.
#
# gb_LinkTarget_get_target is the target that links the objects into a file in
# WORKDIR
# Explanation of some of the variables:
# - AUXTARGETS are the additionally generated files that need to be cleaned out
#   on clean.
# - PCH_CXXFLAGS and PCH_DEFS are the flags that the precompiled headers will
#   be compiled with.  They should never be overridden in a single object
#   files.
# - TARGETTYPE is the type of linktarget as some platforms need very different
#   command to link different targettypes.
#
# Since most variables are set on the linktarget and not on the object, the
# object learns about these setting via GNU makes scoping of target variables.
# Therefore it is important that objects are only directly depended on by the
# linktarget. This for example means that you cannot build a single object
# alone, because then you would directly depend on the object.
define gb_LinkTarget_LinkTarget
$(call gb_LinkTarget_get_clean_target,$(1)) : AUXTARGETS :=
$(call gb_LinkTarget_get_external_headers_target,$(1)) : SELF := $(1)
$(call gb_LinkTarget_get_target,$(1)) : DLLTARGET := 
$(call gb_LinkTarget_get_clean_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : COBJECTS := 
$(call gb_LinkTarget_get_clean_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : CXXOBJECTS := 
$(call gb_LinkTarget_get_clean_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : OBJCXXOBJECTS :=
$(call gb_LinkTarget_get_clean_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : GENCXXOBJECTS :=
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : CFLAGS := $$(gb_LinkTarget_CFLAGS)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : CXXFLAGS := $$(gb_LinkTarget_CXXFLAGS)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : PCH_CXXFLAGS := $$(gb_LinkTarget_CXXFLAGS) $(gb_COMPILEROPTFLAGS)
$(call gb_LinkTarget_get_target,$(1)) : OBJCXXFLAGS := $$(gb_LinkTarget_OBJCXXFLAGS)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : DEFS := $$(gb_LinkTarget_DEFAULTDEFS)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : PCH_DEFS := $$(gb_LinkTarget_DEFAULTDEFS)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : INCLUDE := $$(gb_LinkTarget_INCLUDE)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : INCLUDE_STL := $$(gb_LinkTarget_INCLUDE_STL)
$(call gb_LinkTarget_get_target,$(1)) : LDFLAGS := $$(gb_LinkTarget_LDFLAGS)
$(call gb_LinkTarget_get_target,$(1)) : LINKED_LIBS := 
$(call gb_LinkTarget_get_target,$(1)) : LINKED_STATIC_LIBS := 
$(call gb_LinkTarget_get_target,$(1)) : TARGETTYPE := 
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : PCH_NAME :=
$(call gb_LinkTarget_get_target,$(1)) : PCHOBJS :=
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : PDBFILE :=
$(call gb_LinkTarget_get_target,$(1)) : NATIVERES :=

ifeq ($(gb_FULLDEPS),$(true))
ifneq ($(wildcard $(call gb_LinkTarget_get_dep_target,$(1))),)
include $(call gb_LinkTarget_get_dep_target,$(1))
else
$(firstword $(MAKEFILE_LIST)) : $(call gb_LinkTarget_get_dep_target,$(1))
endif
$(call gb_LinkTarget_get_dep_target,$(1)) : COBJECTS := 
$(call gb_LinkTarget_get_dep_target,$(1)) : CXXOBJECTS := 
$(call gb_LinkTarget_get_dep_target,$(1)) : OBJCXXOBJECTS :=
$(call gb_LinkTarget_get_dep_target,$(1)) : GENCXXOBJECTS :=
$(call gb_LinkTarget_get_dep_target,$(1)) : CFLAGS := $$(gb_LinkTarget_CFLAGS)
$(call gb_LinkTarget_get_dep_target,$(1)) : CXXFLAGS := $$(gb_LinkTarget_CXXFLAGS)
$(call gb_LinkTarget_get_dep_target,$(1)) : PCH_CXXFLAGS := $$(gb_LinkTarget_CXXFLAGS) $(gb_COMPILEROPTFLAGS)
$(call gb_LinkTarget_get_dep_target,$(1)) : OBJCXXFLAGS := $$(gb_LinkTarget_OBJCXXFLAGS)
$(call gb_LinkTarget_get_dep_target,$(1)) : DEFS := $$(gb_LinkTarget_DEFAULTDEFS)
$(call gb_LinkTarget_get_dep_target,$(1)) : PCH_DEFS := $$(gb_LinkTarget_DEFAULTDEFS)
$(call gb_LinkTarget_get_dep_target,$(1)) : INCLUDE := $$(gb_LinkTarget_INCLUDE)
$(call gb_LinkTarget_get_dep_target,$(1)) : INCLUDE_STL := $$(gb_LinkTarget_INCLUDE_STL)
$(call gb_LinkTarget_get_dep_target,$(1)) : TARGETTYPE := 
$(call gb_LinkTarget_get_dep_target,$(1)) : PCH_NAME :=
endif

endef

define gb_LinkTarget_set_defs
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : DEFS := $(2)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : PCH_DEFS := $(2)

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : DEFS := $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : PCH_DEFS := $(2)
endif

endef

define gb_LinkTarget_set_cflags
$(call gb_LinkTarget_get_target,$(1)) : CFLAGS := $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : CFLAGS := $(2)
endif

endef

define gb_LinkTarget_set_cxxflags
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : CXXFLAGS := $(2)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : PCH_CXXFLAGS := $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : CXXFLAGS := $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : PCH_CXXFLAGS := $(2)
endif

endef

define gb_LinkTarget_set_objcxxflags
$(call gb_LinkTarget_get_target,$(1)) : OBJCXXFLAGS := $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : OBJCXXFLAGS := $(2)
endif

endef

define gb_LinkTarget_set_include
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : INCLUDE := $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : INCLUDE := $(2)
endif

endef

define gb_LinkTarget_set_include_stl
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : INCLUDE_STL := $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : INCLUDE_STL := $(2)
endif

endef

define gb_LinkTarget_set_ldflags
$(call gb_LinkTarget_get_target,$(1)) : LDFLAGS := $(2)
endef

define gb_LinkTarget_add_linked_libs
ifneq (,$$(filter-out $(gb_Library_KNOWNLIBS),$(2)))
$$(eval $$(call gb_Output_info,currently known libraries are: $(sort $(gb_Library_KNOWNLIBS)),ALL))
$$(eval $$(call gb_Output_error,Cannot link against library/libraries $$(filter-out $(gb_Library_KNOWNLIBS),$(2)). Libraries must be registered in Repository.mk))
endif

$(call gb_LinkTarget_get_target,$(1)) : LINKED_LIBS += $(2)

$(call gb_LinkTarget_get_target,$(1)) : $$(foreach lib,$(2),$$(call gb_Library_get_target,$$(lib)))
$(call gb_LinkTarget_get_external_headers_target,$(1)) : \
$$(foreach lib,$(2),$$(call gb_Library_get_headers_target,$$(lib)))

endef

define gb_LinkTarget_add_linked_static_libs
ifneq (,$$(filter-out $(gb_StaticLibrary_KNOWNLIBS),$(2)))
$$(eval $$(call gb_Output_info, currently known static libraries are: $(sort $(gb_StaticLibrary_KNOWNLIBS)),ALL))
$$(eval $$(call gb_Output_error,Cannot link against static library/libraries $$(filter-out $(gb_StaticLibrary_KNOWNLIBS),$(2)). Static libraries must be registered in Repository.mk))
endif

$(call gb_LinkTarget_get_target,$(1)) : LINKED_STATIC_LIBS += $(2)

$(call gb_LinkTarget_get_target,$(1)) : $$(foreach lib,$(2),$$(call gb_StaticLibrary_get_target,$$(lib)))
$(call gb_LinkTarget_get_external_headers_target,$(1)) : \
$$(foreach lib,$(2),$$(call gb_StaticLibrary_get_headers_target,$$(lib)))

endef

define gb_LinkTarget_add_cobject
$(call gb_LinkTarget_get_target,$(1)) : COBJECTS += $(2)
$(call gb_LinkTarget_get_clean_target,$(1)) : COBJECTS += $(2)

$(call gb_LinkTarget_get_target,$(1)) : $(call gb_CObject_get_target,$(2))
$(call gb_CObject_get_target,$(2)) : | $(call gb_LinkTarget_get_headers_target,$(1))
$(call gb_CObject_get_target,$(2)) : CFLAGS += $(3)

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : COBJECTS += $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : $(call gb_CObject_get_dep_target,$(2))
endif

endef

define gb_LinkTarget_add_cxxobject
$(call gb_LinkTarget_get_target,$(1)) : CXXOBJECTS += $(2)
$(call gb_LinkTarget_get_clean_target,$(1)) : CXXOBJECTS += $(2)

$(call gb_LinkTarget_get_target,$(1)) : $(call gb_CxxObject_get_target,$(2))
$(call gb_CxxObject_get_target,$(2)) : | $(call gb_LinkTarget_get_headers_target,$(1))
$(call gb_CxxObject_get_target,$(2)) : CXXFLAGS += $(3)

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : CXXOBJECTS += $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : $(call gb_CxxObject_get_dep_target,$(2))
endif

endef

define gb_LinkTarget_add_objcxxobject
$(call gb_LinkTarget_get_target,$(1)) : OBJCXXOBJECTS += $(2)
$(call gb_LinkTarget_get_clean_target,$(1)) : OBJCXXOBJECTS += $(2)

$(call gb_LinkTarget_get_target,$(1)) : $(call gb_ObjCxxObject_get_target,$(2))
$(call gb_ObjCxxObject_get_target,$(2)) : | $(call gb_LinkTarget_get_headers_target,$(1))
$(call gb_ObjCxxObject_get_target,$(2)) : OBJCXXFLAGS += $(3)

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : OBJCXXOBJECTS += $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : $(call gb_ObjCxxObject_get_dep_target,$(2))
endif

endef

define gb_LinkTarget_add_generated_cxx_object
$(call gb_LinkTarget_get_target,$(1)) : GENCXXOBJECTS += $(2)
$(call gb_LinkTarget_get_clean_target,$(1)) : GENCXXOBJECTS += $(2)

$(call gb_LinkTarget_get_target,$(1)) : $(call gb_GenCxxObject_get_target,$(2))
$(call gb_GenCxxObject_get_source,$(2)) : | $(call gb_LinkTarget_get_headers_target,$(1))
$(call gb_GenCxxObject_get_target,$(2)) : CXXFLAGS += $(3) $(gb_COMPILEROPTFLAGS)

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : GENCXXOBJECTS += $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : $(call gb_GenCxxObject_get_dep_target,$(2))
endif

endef

define gb_LinkTarget_add_noexception_object
$(call gb_LinkTarget_add_cxxobject,$(1),$(2),$(gb_LinkTarget_NOEXCEPTIONFLAGS) $(gb_COMPILEROPTFLAGS))
endef

define gb_LinkTarget_add_exception_object
$(call gb_LinkTarget_add_cxxobject,$(1),$(2),$(gb_LinkTarget_EXCEPTIONFLAGS) $(gb_COMPILEROPTFLAGS))
endef

define gb_LinkTarget_add_cobjects
$(foreach obj,$(2),$(call gb_LinkTarget_add_cobject,$(1),$(obj),$(3)))
endef

define gb_LinkTarget_add_cxxobjects
$(foreach obj,$(2),$(call gb_LinkTarget_add_cxxobject,$(1),$(obj),$(3)))
endef

define gb_LinkTarget_add_objcxxobjects
$(foreach obj,$(2),$(call gb_LinkTarget_add_objcxxobject,$(1),$(obj),$(3)))
endef

define gb_LinkTarget_add_noexception_objects
$(foreach obj,$(2),$(call gb_LinkTarget_add_noexception_object,$(1),$(obj)))
endef

define gb_LinkTarget_add_exception_objects
$(foreach obj,$(2),$(call gb_LinkTarget_add_exception_object,$(1),$(obj)))
endef

define gb_LinkTarget_add_generated_exception_object
$(call gb_LinkTarget_add_generated_cxx_object,$(1),$(2),$(gb_LinkTarget_EXCEPTIONFLAGS))
endef

define gb_LinkTarget_add_generated_exception_objects
$(foreach obj,$(2),$(call gb_LinkTarget_add_generated_exception_object,$(1),$(obj)))
endef

define gb_LinkTarget_set_targettype
$(call gb_LinkTarget_get_target,$(1)) \
$(call gb_LinkTarget_get_dep_target,$(1)) : TARGETTYPE := $(2)
endef

define gb_LinkTarget_set_dlltarget
$(call gb_LinkTarget_get_clean_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : DLLTARGET := $(2)
endef

define gb_LinkTarget_set_auxtargets
$(call gb_LinkTarget_get_clean_target,$(1)) : AUXTARGETS := $(2)
endef

define gb_LinkTarget__add_internal_headers
$(call gb_LinkTarget_get_headers_target,$(1)) : $(2)
$(2) :|	$(call gb_LinkTarget_get_external_headers_target,$(1))

endef

define gb_LinkTarget_add_package_headers
$(foreach package,$(2),$(call gb_LinkTarget__add_internal_headers,$(1),$(call gb_Package_get_target,$(package))))
$(call gb_LinkTarget_get_clean_target,$(1)) : $(foreach package,$(2),$(call gb_Package_get_clean_target,$(package)))

endef

define gb_LinkTarget_add_sdi_headers
$(call gb_LinkTarget__add_internal_headers,$(1),$(foreach sdi,$(2),$(call gb_SdiTarget_get_target,$(sdi))))
$(call gb_LinkTarget_get_clean_target,$(1)) : $(foreach sdi,$(2),$(call gb_SdiTarget_get_clean_target,$(sdi)))
endef

define gb_LinkTarget__add_precompiled_header_impl
$(call gb_LinkTarget__add_internal_headers,$(1),$(call gb_PrecompiledHeader_get_target,$(3)))
$(call gb_LinkTarget_get_clean_target,$(1)) : $(call gb_PrecompiledHeader_get_clean_target,$(3))
$(call gb_PrecompiledHeader_get_target,$(3)) : $(2).cxx

$(call gb_LinkTarget__add_internal_headers,$(1),$(call gb_NoexPrecompiledHeader_get_target,$(3)))
$(call gb_LinkTarget_get_clean_target,$(1)) : $(call gb_NoexPrecompiledHeader_get_clean_target,$(3))
$(call gb_NoexPrecompiledHeader_get_target,$(3)) : $(2).cxx

$(call gb_LinkTarget_get_target,$(1)) : PCH_NAME := $(3)
$(call gb_LinkTarget_get_target,$(1)) : PCHOBJS = $(call gb_PrecompiledHeader_get_target,$(3)).obj $(call gb_NoexPrecompiledHeader_get_target,$(3)).obj

$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : DEFS := $$(DEFS) -DPRECOMPILED_HEADERS
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : PCH_DEFS = $$(DEFS)
ifeq ($(gb_FULLDEPS),$(true))
-include \
    $(call gb_PrecompiledHeader_get_dep_target,$(3)) \
    $(call gb_NoexPrecompiledHeader_get_dep_target,$(3))
$(call gb_LinkTarget_get_dep_target,$(1)) : DEFS := $$(DEFS) -DPRECOMPILED_HEADERS
$(call gb_LinkTarget_get_dep_target,$(1)) : PCH_DEFS = $$(DEFS)
endif

endef

define gb_LinkTarget_add_precompiled_header
ifeq ($(gb_ENABLE_PCH),$(true))
$(call gb_LinkTarget__add_precompiled_header_impl,$(1),$(2),$(notdir $(2)))
endif

endef

# vim: set noet sw=4 ts=4:
