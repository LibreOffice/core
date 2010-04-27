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


# CObject class

gb_CObject_get_source = $(SRCDIR)/$(1).c
# defined by platform
#  gb_CObject_command
#  gb_CObject_command_dep

$(call gb_CObject_get_target,%) : $(call gb_CObject_get_source,%)
    $(call gb_CObject_command,$@,$*,$<,$(DEFS),$(CFLAGS),$(INCLUDE))

$(call gb_CObject_get_dep_target,%) : $(call gb_CObject_get_source,%)
    $(call gb_CObject_command_dep,$@,$*,$<,$(DEFS),$(CFLAGS),$(INCLUDE))

$(call gb_CObject_get_dep_target,%) :
    $(error unable to find plain C file $(call gb_CObject_get_source,$*))


# CxxObject class

gb_CxxObject_get_source = $(SRCDIR)/$(1).cxx
# defined by platform
#  gb_CxxObject_command
#  gb_CxxObject_command_dep

$(call gb_CxxObject_get_target,%) : $(call gb_CxxObject_get_source,%)
    $(call gb_CxxObject_command,$@,$*,$<,$(DEFS),$(CXXFLAGS),$(INCLUDE_STL) $(INCLUDE))

$(call gb_CxxObject_get_dep_target,%) : $(call gb_CxxObject_get_source,%)
    $(call gb_CxxObject_command_dep,$@,$*,$<,$(DEFS),$(CXXFLAGS),$(INCLUDE_STL) $(INCLUDE))

$(call gb_CxxObject_get_dep_target,%) :
    $(error unable to find C++ file $(call gb_CxxObject_get_source,$*))


# LinkTarget class

gb_LinkTarget_DEFAULTDEFS := $(gb_GLOBALDEFS)
# defined by platform
#  gb_LinkTarget_CXXFLAGS
#  gb_LinkTarget_LDFLAGS
#  gb_LinkTarget_INCLUDE 
#  gb_LinkTarget_INCLUDE_STL

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
            $(call gb_LinkTarget_get_external_headers_target,$*) \
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

$(call gb_LinkTarget_get_target,%) : $(call gb_LinkTarget_get_headers_target,%) $(call gb_LinkTarget_get_dep_target,%)
    $(call gb_LinkTarget_command_dep,$(call gb_LinkTarget_get_dep_target,$*),$*,$(COBJECTS),$(CXXOBJECTS))
    $(call gb_LinkTarget_command,$@,$*,$(TARGETTYPE_FLAGS) $(LDFLAGS),$(LINKED_LIBS),$(LINKED_STATIC_LIBS),$(CXXOBJECTS),$(COBJECTS))

$(call gb_LinkTarget_get_dep_target,%) : $(call gb_LinkTarget_get_headers_target,%)
    $(call gb_LinkTarget_command_dep,$@,$*,$(COBJECTS),$(CXXOBJECTS))

define gb_LinkTarget_get_external_headers_check
ifneq ($$(SELF),$$*)
$$(info LinkTarget $$* not defined: Assuming headers to be there!)
endif
$$@ : COMMAND := $$(call gb_Helper_abbreviate_dirs, mkdir -p $$(dir $$@) && touch $$@)

endef

$(call gb_LinkTarget_get_external_headers_target,%) :
    $(eval $(gb_LinkTarget_get_external_headers_check))
    $(COMMAND)

$(call gb_LinkTarget_get_headers_target,%) : $(call gb_LinkTarget_get_external_headers_target,%)
    $(call gb_Helper_abbreviate_dirs,\
        mkdir -p $(dir $@) && touch $@)


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
$(call gb_LinkTarget_get_dep_target,$(1)) \
$(call gb_LinkTarget_get_target,$(1)) : INCLUDE_STL := $$(gb_LinkTarget_INCLUDE_STL)
$(call gb_LinkTarget_get_target,$(1)) : LDFLAGS := $$(gb_LinkTarget_LDFLAGS)
$(call gb_LinkTarget_get_target,$(1)) : LINKED_LIBS := 
$(call gb_LinkTarget_get_target,$(1)) : LINKED_STATIC_LIBS := 
$(call gb_LinkTarget_get_external_headers_target,$(1)) : SELF := $(1)
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
$$(error Cannot link against library/libraries $$(filter-out $(gb_Library_KNOWNLIBS),$(2)). These must be registered in $(GBUILDDIR)/libnames.mk)
endif
$(call gb_LinkTarget_get_target,$(1)) : LINKED_LIBS += $(2)

$(call gb_LinkTarget_get_target,$(1)) : $$(foreach lib,$(2),$$(call gb_Library_get_target,$$(lib)))
$(call gb_LinkTarget_get_external_headers_target,$(1)) : \
$$(foreach lib,$(2),$$(call gb_Library_get_headers_target,$$(lib)))

endef

define gb_LinkTarget_add_linked_static_libs
ifneq (,$$(filter-out $(gb_StaticLibrary_KNOWNLIBS),$(2)))
$$(info currently known static libraries are: $(sort $(gb_StaticLibrary_KNOWNLIBS)))
$$(error Cannot link against static library/libraries $$(filter-out $(gb_StaticLibrary_KNOWNLIBS),$(2)). These must be registered in $(GBUILDDIR)/inc/libnames.mk)
endif
$(call gb_LinkTarget_get_target,$(1)) : LINKED_STATIC_LIBS += $(2)

$(call gb_LinkTarget_get_target,$(1)) : $$(foreach lib,$(2),$$(call gb_StaticLibrary_get_target,$$(lib)))
$(call gb_LinkTarget_get_external_headers_target,$(1)) : \
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

define gb_LinkTarget__add_internal_headers
$(call gb_LinkTarget_get_headers_target,$(1)) : $(2)
$(2) :|	$(call gb_LinkTarget_get_external_headers_target,$(1))

endef

define gb_LinkTarget_add_package_headers
$(call gb_LinkTarget__add_internal_headers,$(1),$(foreach package,$(2),$(call gb_Package_get_target,$(package))))
$(call gb_LinkTarget_get_clean_target,$(1)) : $(foreach package,$(2),$(call gb_Package_get_clean_target,$(package)))

endef

define gb_LinkTarget_add_sdi_headers
$(call gb_LinkTarget__add_internal_headers,$(1),$(foreach sdi,$(2),$(call gb_SdiTarget_get_target,$(sdi))))
$(call gb_LinkTarget_get_clean_target,$(1)) : $(foreach sdi,$(2),$(call gb_SdiTarget_get_clean_target,$(sdi)))
endef

# vim: set noet sw=4 ts=4:
