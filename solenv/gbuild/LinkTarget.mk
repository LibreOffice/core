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



#the following user-defined variables are supported:
# CPPFLAGS
# CFLAGS
# CXXFLAGS
# OBJCXXFLAGS
# JAVAFLAGS
# LDFLAGS

# CFLAGS from environment override debug/optimization flags

ifeq ($(gb_DEBUGGING),TRUE)
CFLAGS ?= $(gb_COMPILEROPTFLAGS) $(gb_DEBUG_CFLAGS)
CXXFLAGS ?= $(gb_COMPILEROPTFLAGS) $(gb_DEBUG_CFLAGS)
OBJCXXFLAGS ?= $(gb_COMPILEROPTFLAGS) $(gb_DEBUG_CFLAGS)
JAVAFLAGS ?= -g
else
CFLAGS ?= $(gb_COMPILEROPTFLAGS)
CXXFLAGS ?= $(gb_COMPILEROPTFLAGS)
OBJCXXFLAGS ?= $(gb_COMPILEROPTFLAGS)
endif


# For every object there is a dep file (if gb_FULLDEPS is active).
# The dep file depends on the object: the Object__command also updates the
# dep file as a side effect.
# In the dep file rule just touch it so it's newer than the object.

# The gb_Object__command_dep generates an "always rebuild" dep file;
# It is _only_ used in case the user deletes the object dep file.
ifeq ($(gb_FULLDEPS),$(true))
define gb_Object__command_dep
mkdir -p $(dir $(1)) && \
	echo '$(2) : $$(gb_Helper_PHONY)' > $(1)

endef
else
gb_Object__command_dep = \
 $(call gb_Output_error,gb_Object__command_dep is only for gb_FULLDEPS)
endif


# CObject class

gb_CObject_REPOS := $(gb_REPOS)

gb_CObject_get_source = $(1)/$(2).c
# defined by platform
#  gb_CObject__command

define gb_CObject__rules
$$(call gb_CObject_get_target,%) : $$(call gb_CObject_get_source,$(1),%)
	$$(call gb_CObject__command,$$@,$$*,$$<,$$(call gb_CObject_get_dep_target,$$*))

ifeq ($(gb_FULLDEPS),$(true))
$$(call gb_CObject_get_dep_target,%) : $$(call gb_CObject_get_target,%)
	$$(if $$(wildcard $$@),touch $$@,\
	  $$(call gb_Object__command_dep,$$@,$$(call gb_CObject_get_target,$$*)))
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

# Only enable PCH if the PCH_CXXFLAGS and the PCH_DEFS (from the linktarget)
# are the same as the T_CXXFLAGS and DEFS we want to use for this object. This
# should usually be the case.  The DEFS/T_CXXFLAGS would have too be manually
# overridden for one object file for them to differ.  PCH_CXXFLAGS/PCH_DEFS
# should never be overridden on an object -- they should be the same as for the
# whole linktarget. In general it should be cleaner to use a static library
# compiled with different flags and link that in rather than mixing different
# flags in one linktarget.
define gb_CxxObject__set_pchflags
ifeq ($(gb_ENABLE_PCH),$(true))
ifneq ($(strip $$(PCH_NAME)),)
ifeq ($$(sort $$(PCH_CXXFLAGS) $$(PCH_DEFS) $$(gb_LinkTarget_EXCEPTIONFLAGS)),$$(sort $$(T_CXXFLAGS) $$(CXXFLAGS) $$(DEFS)))
$$@ : PCHFLAGS := $$(call gb_PrecompiledHeader_get_enableflags,$$(PCH_NAME))
else
ifeq ($$(sort $$(PCH_CXXFLAGS) $$(PCH_DEFS) $$(gb_LinkTarget_NOEXCEPTIONFLAGS)),$$(sort $$(T_CXXFLAGS) $$(CXXFLAGS) $$(DEFS)))
$$@ : PCHFLAGS := $$(call gb_NoexPrecompiledHeader_get_enableflags,$$(PCH_NAME))
else
$$(info No precompiled header available for $$*.)
$$(info precompiled header flags (  ex) : $$(sort $$(PCH_CXXFLAGS) $$(PCH_DEFS) $$(gb_LinkTarget_EXCEPTIONFLAGS)))
$$(info precompiled header flags (noex) : $$(sort $$(PCH_CXXFLAGS) $$(PCH_DEFS) $$(gb_LinkTarget_NOEXCEPTIONFLAGS)))
$$(info .           object flags        : $$(sort $$(T_CXXFLAGS) $$(DEFS)))
$$@ : PCHFLAGS := 
endif
endif
endif
endif
endef

define gb_CxxObject__rules
$$(call gb_CxxObject_get_target,%) : $$(call gb_CxxObject_get_source,$(1),%)
	$$(eval $$(gb_CxxObject__set_pchflags))
	$$(call gb_CxxObject__command,$$@,$$*,$$<,$$(call gb_CxxObject_get_dep_target,$$*))

ifeq ($(gb_FULLDEPS),$(true))
$$(call gb_CxxObject_get_dep_target,%) : $$(call gb_CxxObject_get_target,%)
	$$(if $$(wildcard $$@),touch $$@,\
	  $$(eval $$(gb_CxxObject__set_pchflags))\
	  $$(call gb_Object__command_dep,$$@,$$(call gb_CxxObject_get_target,$$*)))
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

$(call gb_GenCxxObject_get_target,%) : $(call gb_GenCxxObject_get_source,%)
	$(call gb_CxxObject__command,$@,$*,$<,$(call gb_GenCxxObject_get_dep_target,$*))

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_GenCxxObject_get_dep_target,%) : $(call gb_GenCxxObject_get_target,%)
	$(if $(wildcard $@),touch $@,\
	  $(call gb_Object__command_dep,$@,$(call gb_GenCxxObject_get_target,$*)))
endif

gb_GenCxxObject_GenCxxObject =


# ObjCxxObject class
#
gb_ObjCxxObject_REPOS := $(gb_REPOS)

gb_ObjCxxObject_get_source = $(1)/$(2).mm
# defined by platform
#  gb_ObjCxxObject__command

define gb_ObjCxxObject__rules
$$(call gb_ObjCxxObject_get_target,%) : $$(call gb_ObjCxxObject_get_source,$(1),%)
	$$(call gb_ObjCxxObject__command,$$@,$$*,$$<,$$(call gb_ObjCxxObject_get_dep_target,$$*))

ifeq ($(gb_FULLDEPS),$(true))
$$(call gb_ObjCxxObject_get_dep_target,%) : $$(call gb_ObjCxxObject_get_target,%)
	$$(if $$(wildcard $$@),touch $$@,\
	  $$(call gb_Object__command_dep,$$@,$$(call gb_ObjCxxObject_get_target,$$*)))
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
# - VERSIONMAP is the linker script, usually used to version a dynamic
#   library's symbols (on *nix/Mac).
#
# Since most variables are set on the linktarget and not on the object, the
# object learns about these setting via GNU makes scoping of target variables.
# Therefore it is important that objects are only directly depended on by the
# linktarget. This for example means that you cannot build a single object
# alone, because then you would directly depend on the object.
#
# A note about flags: because the overriding the global variables with a target
# local variable of the same name is considered obscure, the target local
# variables have a T_ prefix.
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
$(call gb_LinkTarget_get_target,$(1)) : T_CFLAGS := $$(gb_LinkTarget_CFLAGS)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : T_CXXFLAGS := $$(gb_LinkTarget_CXXFLAGS)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : PCH_CXXFLAGS := $$(gb_LinkTarget_CXXFLAGS) $$(CXXFLAGS)
$(call gb_LinkTarget_get_target,$(1)) : T_OBJCXXFLAGS := $$(gb_LinkTarget_OBJCXXFLAGS)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : DEFS := $$(gb_LinkTarget_DEFAULTDEFS) $(CPPFLAGS)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : PCH_DEFS := $$(gb_LinkTarget_DEFAULTDEFS) $(CPPFLAGS)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : INCLUDE := $$(gb_LinkTarget_INCLUDE)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : INCLUDE_STL := $$(gb_LinkTarget_INCLUDE_STL)
$(call gb_LinkTarget_get_target,$(1)) : T_LDFLAGS := $$(gb_LinkTarget_LDFLAGS) $(LDFLAGS)
$(call gb_LinkTarget_get_target,$(1)) : LINKED_LIBS := 
$(call gb_LinkTarget_get_target,$(1)) : LINKED_STATIC_LIBS := 
$(call gb_LinkTarget_get_target,$(1)) : EXTERNAL_LIBS := 
$(call gb_LinkTarget_get_target,$(1)) : LIBS :=
$(call gb_LinkTarget_get_target,$(1)) : TARGETTYPE := 
$(call gb_LinkTarget_get_target,$(1)) : VERSIONMAP := 
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : PCH_NAME :=
$(call gb_LinkTarget_get_target,$(1)) : PCHOBJS :=
#$(call gb_LinkTarget_get_headers_target,$(1)) \
#$(call gb_LinkTarget_get_target,$(1)) : PDBFILE :=
$(call gb_LinkTarget_get_target,$(1)) : NATIVERES :=

ifeq ($(gb_FULLDEPS),$(true))
-include $(call gb_LinkTarget_get_dep_target,$(1))
$(call gb_LinkTarget_get_dep_target,$(1)) : COBJECTS := 
$(call gb_LinkTarget_get_dep_target,$(1)) : CXXOBJECTS := 
$(call gb_LinkTarget_get_dep_target,$(1)) : OBJCXXOBJECTS :=
$(call gb_LinkTarget_get_dep_target,$(1)) : GENCXXOBJECTS :=
$(call gb_LinkTarget_get_dep_target,$(1)) : T_CFLAGS := $$(gb_LinkTarget_CFLAGS)
$(call gb_LinkTarget_get_dep_target,$(1)) : T_CXXFLAGS := $$(gb_LinkTarget_CXXFLAGS)
$(call gb_LinkTarget_get_dep_target,$(1)) : PCH_CXXFLAGS := $$(gb_LinkTarget_CXXFLAGS) $$(CXXFLAGS)
$(call gb_LinkTarget_get_dep_target,$(1)) : T_OBJCXXFLAGS := $$(gb_LinkTarget_OBJCXXFLAGS)
$(call gb_LinkTarget_get_dep_target,$(1)) : DEFS := $$(gb_LinkTarget_DEFAULTDEFS) $(CPPFLAGS)
$(call gb_LinkTarget_get_dep_target,$(1)) : PCH_DEFS := $$(gb_LinkTarget_DEFAULTDEFS) $(CPPFLAGS)
$(call gb_LinkTarget_get_dep_target,$(1)) : INCLUDE := $$(gb_LinkTarget_INCLUDE)
$(call gb_LinkTarget_get_dep_target,$(1)) : INCLUDE_STL := $$(gb_LinkTarget_INCLUDE_STL)
$(call gb_LinkTarget_get_dep_target,$(1)) : TARGETTYPE := 
$(call gb_LinkTarget_get_dep_target,$(1)) : VERSIONMAP := 
$(call gb_LinkTarget_get_dep_target,$(1)) : PCH_NAME :=
endif

endef

define gb_LinkTarget_add_defs
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : DEFS += $(2)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : PCH_DEFS += $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : DEFS += $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : PCH_DEFS += $(2)
endif
endef

define gb_LinkTarget_set_defs
ifeq (,)
$$(call gb_Output_error,\
 gb_LinkTarget_set_defs: use gb_LinkTarget_add_defs instead.)
else
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : DEFS := $(2)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : PCH_DEFS := $(2)

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : DEFS := $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : PCH_DEFS := $(2)
endif
endif

endef

define gb_LinkTarget_add_cflags
$(call gb_LinkTarget_get_target,$(1)) : T_CFLAGS += $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : T_CFLAGS += $(2)
endif

endef

define gb_LinkTarget_set_cflags
ifeq (,)
$$(call gb_Output_error,\
 gb_LinkTarget_set_cflags: use gb_LinkTarget_add_cflags instead.)
else
$(call gb_LinkTarget_get_target,$(1)) : T_CFLAGS := $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : T_CFLAGS := $(2)
endif
endif

endef

define gb_LinkTarget_add_cxxflags
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : T_CXXFLAGS += $(2)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : PCH_CXXFLAGS += $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : T_CXXFLAGS += $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : PCH_CXXFLAGS += $(2)
endif
endef

define gb_LinkTarget_set_cxxflags
ifeq (,)
$$(call gb_Output_error,\
 gb_LinkTarget_set_cxxflags: use gb_LinkTarget_add_cxxflags instead.)
else
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : T_CXXFLAGS := $(2)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : PCH_CXXFLAGS := $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : T_CXXFLAGS := $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : PCH_CXXFLAGS := $(2)
endif
endif

endef

define gb_LinkTarget_add_objcxxflags
$(call gb_LinkTarget_get_target,$(1)) : T_OBJCXXFLAGS += $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : T_OBJCXXFLAGS += $(2)
endif
endef

define gb_LinkTarget_set_objcxxflags
ifeq (,)
$$(call gb_Output_error,\
 gb_LinkTarget_set_objcxxflags: use gb_LinkTarget_add_objcxxflags instead.)
else
$(call gb_LinkTarget_get_target,$(1)) : T_OBJCXXFLAGS := $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : T_OBJCXXFLAGS := $(2)
endif
endif
endef

define gb_LinkTarget_set_c_optimization
$(foreach object,$(1),$(eval $(call gb_CObject_get_target,$(object)) : CFLAGS := $(filter-out $(gb_COMPILEROPTFLAGS),$(CFLAGS)) $(2)))
endef

define gb_LinkTarget_set_cxx_optimization
$(foreach object,$(1),$(eval $(call gb_CxxObject_get_target,$(object)) : CXXFLAGS := $(filter-out $(gb_COMPILEROPTFLAGS),$(CXXFLAGS)) $(2)))
endef

define gb_LinkTarget_set_gencxx_optimization
$(foreach object,$(1),$(eval $(call gb_GenCxxObject_get_target,$(object)) : CXXFLAGS := $(filter-out $(gb_COMPILEROPTFLAGS),$(CXXFLAGS)) $(2)))
endef

define gb_LinkTarget_set_objcxx_optimization
$(foreach object,$(1),$(eval $(call gb_ObjCxxObject_get_target,$(object)) : OBJCXXFLAGS := $(filter-out $(gb_COMPILEROPTFLAGS),$(OBJCXXFLAGS)) $(2)))
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

define gb_LinkTarget_add_ldflags
$(call gb_LinkTarget_get_target,$(1)) : T_LDFLAGS += $(2)
endef

# real use in RepositoryExternal.mk
define gb_LinkTarget_set_ldflags
$(call gb_LinkTarget_get_target,$(1)) : T_LDFLAGS := $(2)
endef

define gb_LinkTarget_add_api
$(call gb_LinkTarget_get_external_headers_target,$(1)) :| \
	$$(foreach api,$(2),$$(call gb_Package_get_target,$$(api)_inc))
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : INCLUDE += $$(foreach api,$(2),-I$(OUTDIR)/inc/$$(api))
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : INCLUDE += $$(foreach api,$(2),-I$(OUTDIR)/inc/$$(api))
endif

endef

define gb_LinkTarget_add_private_api
$(call gb_LinkTarget_get_external_headers_target,$(1)) :| \
	$(call gb_UnoPrivateApiTarget_get_target,$(1)/idl.cppumaker.flag)
$(call gb_LinkTarget_get_headers_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : INCLUDE += -I$(call gb_UnoPrivateApiTarget_get_target,$(1)/inc)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : INCLUDE += -I$(call gb_UnoPrivateApiTarget_get_target,$(1)/inc)
endif

$(call gb_UnoPrivateApiTarget_get_target,$(1)/idl.cppumaker.flag): $(2)
	$(call gb_Output_announce,$@,$(true),PVTIDL,2)
	-$$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $$(call gb_UnoPrivateApiTarget_get_target,$(1)/urd) && \
		mkdir -p $$(call gb_UnoPrivateApiTarget_get_target,$(1)/rdb) && \
		mkdir -p $$(call gb_UnoPrivateApiTarget_get_target,$(1)/inc) && \
		$$(gb_UnoApiTarget_IDLCCOMMAND) -I$$(OUTDIR)/idl -O $$(call gb_UnoPrivateApiTarget_get_target,$(1)/urd) \
			-verbose -cid -we $(2) && \
		$$(gb_UnoApiTarget_REGMERGECOMMAND) $$(call gb_UnoPrivateApiTarget_get_target,$(1)/rdb/registry.rdb) /UCR \
			$(patsubst %.idl,%.urd,$$(call gb_UnoPrivateApiTarget_get_target,$(1)/urd)/$(notdir $(2))) && \
		$(gb_UnoApiTarget_CPPUMAKERCOMMAND) \
			-O $$(call gb_UnoPrivateApiTarget_get_target,$(1)/inc) \
			-BUCR \
			-C \
			$$(call gb_UnoPrivateApiTarget_get_target,$(1)/rdb/registry.rdb) \
			$$(OUTDIR)/bin/udkapi.rdb && \
		touch $(call gb_UnoPrivateApiTarget_get_target,$(1)/idl.cppumaker.flag))

$(call gb_LinkTarget_get_clean_target,$(1)) :
	rm -rf $(call gb_UnoPrivateApiTarget_get_target,$(1))

endef

# FIXME: multiple??
define gb_LinkTarget_set_private_api
$(foreach api,$(2),$(call gb_LinkTarget_add_private_api,$(1),$(api)))

endef

define gb_LinkTarget_add_libs
$(call gb_LinkTarget_get_target,$(1)) : LIBS += $(2)
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

#
# Add external libs for linking.  External libaries are not built by any module.
#
# The list of libraries is used as is, ie it is not filtered with gb_Library_KNOWNLIBS.
#
# An error is signaled, when any of the library names does not look like
# a base name, ie is prefixed by -l or is folled by .lib or .so.
# 
# @param target
# @param libraries
#     A list of (base names of) libraries that will be added to the target
#     local EXTERNAL_LIBS variable and eventually linked in when the
#     target is made.
#
define gb_LinkTarget_add_external_libs

# Make sure that all libraries are given as base names.
ifneq (,$$(filter -l% %.so %.lib, $(2)))
$$(eval $$(call gb_Output_announce,ERROR: Please give only library basenames to gb_LinkTarget_add_external_libs))
$$(eval $$(call gb_Output_announce,ERROR:    (no prefixes -l% or lib%, no suffixes %.so or %.lib)))
$$(eval $$(call gb_Output_announce,ERROR:    libraries given: $(2)))
$$(eval $$(call gb_Output_announce,ERROR:    offending: $$(filter -l% lib% %.so %.lib, $(2))))
$$(eval $$(call gb_Output_error,  ))
endif

$(call gb_LinkTarget_get_target,$(1)) : EXTERNAL_LIBS += $(2)

$(call gb_LinkTarget_get_target,$(1)) : $$(foreach lib,$(2),$$(call gb_Library_get_target,$$(lib)))
$(call gb_LinkTarget_get_external_headers_target,$(1)) : \
$$(foreach lib,$(2),$$(call gb_Library_get_headers_target,$$(lib)))

endef


define gb_LinkTarget_add_cobject
$(call gb_LinkTarget_get_target,$(1)) : COBJECTS += $(2)
$(call gb_LinkTarget_get_clean_target,$(1)) : COBJECTS += $(2)

$(call gb_LinkTarget_get_target,$(1)) : $(call gb_CObject_get_target,$(2))
$(call gb_CObject_get_target,$(2)) : | $(call gb_LinkTarget_get_headers_target,$(1))
$(call gb_CObject_get_target,$(2)) : T_CFLAGS += $(3)

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
$(call gb_CxxObject_get_target,$(2)) : T_CXXFLAGS += $(3)

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
$(call gb_ObjCxxObject_get_target,$(2)) : T_OBJCXXFLAGS += $(3)

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
$(call gb_GenCxxObject_get_target,$(2)) : T_CXXFLAGS += $(3)

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_LinkTarget_get_dep_target,$(1)) : GENCXXOBJECTS += $(2)
$(call gb_LinkTarget_get_dep_target,$(1)) : $(call gb_GenCxxObject_get_dep_target,$(2))
endif

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

define gb_LinkTarget_set_versionmap
$(call gb_LinkTarget_get_target,$(1)) \
$(call gb_LinkTarget_get_dep_target,$(1)) : VERSIONMAP := $(2)
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
$(call gb_LinkTarget_get_target,$(1)) : PCH_DEFS = $$(DEFS)
ifeq ($(gb_FULLDEPS),$(true))
-include \
	$(call gb_PrecompiledHeader_get_dep_target,$(3)) \
	$(call gb_NoexPrecompiledHeader_get_dep_target,$(3))
$(call gb_LinkTarget_get_dep_target,$(1)) : PCH_NAME := $(3)
$(call gb_LinkTarget_get_dep_target,$(1)) : PCH_DEFS = $$(DEFS)
endif

endef

define gb_LinkTarget_add_precompiled_header
ifeq ($(gb_ENABLE_PCH),$(true))
$(call gb_LinkTarget__add_precompiled_header_impl,$(1),$(2),$(notdir $(2)))
endif

endef

# this forwards to functions that must be defined in RepositoryExternal.mk.
# $(call gb_LinkTarget_use_external,library,external)
define gb_LinkTarget_use_external
$(eval $(if $(value gb_LinkTarget__use_$(2)),\
  $(call gb_LinkTarget__use_$(2),$(1)),\
  $(error gb_LinkTarget_use_external: unknown external: $(2))))
endef

# $(call gb_LinkTarget_use_externals,library,externals)
gb_LinkTarget_use_externals = \
 $(foreach external,$(2),$(call gb_LinkTarget_use_external,$(1),$(external)))


# vim: set noet sw=4 ts=4:
