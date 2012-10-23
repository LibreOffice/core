# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-

# WinResTarget class

gb_WinResTarget_DEFAULTDEFS := $(gb_RCDEFS)

define gb_WinResTarget_WinResTarget
$(call gb_WinResTarget_WinResTarget_init,$(1))
$$(eval $$(call gb_Module_register_target,$(call gb_WinResTarget_get_target,$(1)),$(call gb_WinResTarget_get_clean_target,$(1))))

endef

define gb_WinResTarget_WinResTarget_init
$(call gb_WinResTarget_get_target,$(1)) : DEFS := $(gb_WinResTarget_DEFAULTDEFS)
$(call gb_WinResTarget_get_target,$(1)) : INCLUDE := $(SOLARINC)
$(call gb_WinResTarget_get_clean_target,$(1)) : RCFILE :=
$(call gb_WinResTarget_get_target,$(1)) : RCFILE :=

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_WinResTarget_get_dep_target,$(1)) : DEFS := $$(gb_WinResTarget_DEFAULTDEFS)
$(call gb_WinResTarget_get_dep_target,$(1)) : INCLUDE := $$(gb_WinResTarget_INCLUDE)
$(call gb_WinResTarget_get_dep_target,$(1)) : RCFILE :=

-include $(call gb_WinResTarget_get_dep_target,$(1))
endif

endef

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_WinResTarget_get_dep_target,%) :
	$(call gb_WinResTarget__command_dep,$@,$*,$<)
endif


$(call gb_WinResTarget_get_target,%) :
	$(call gb_WinResTarget__command,$@,$*,$<)

$(call gb_WinResTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),RC ,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_WinResTarget_get_target,$*) \
			$(call gb_WinResTarget_get_dep_target,$*))

define gb_WinResTarget_set_defs
$$(call gb_Output_error,gb_WinResTarget_set_defs: use gb_WinResTarget_add_defs instead.)
endef

define gb_WinResTarget_add_defs
$(call gb_WinResTarget_get_target,$(1)) : DEFS += $(2)

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_WinResTarget_get_dep_target,$(1)) : DEFS += $(2)
endif

endef

define gb_WinResTarget__add_include
$(call gb_WinResTarget_get_target,$(1)) : INCLUDE += -I$(2)

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_WinResTarget_get_dep_target,$(1)) : INCLUDE += -I$(2)
endif

endef

define gb_WinResTarget_set_include
$(call gb_WinResTarget_get_target,$(1)) : INCLUDE := $(2)

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_WinResTarget_get_dep_target,$(1)) : INCLUDE := $(2)
endif

endef

define gb_WinResTarget_add_file
$$(call gb_Output_error,gb_WinResTarget_add_file: use gb_WinResTarget_set_rcfile instead.)
endef

define gb_WinResTarget_set_rcfile
$(call gb_WinResTarget_get_clean_target,$(1)) : RCFILE := $(SRCDIR)/$(strip $(2)).rc
$(call gb_WinResTarget_get_target,$(1)) : RCFILE := $(SRCDIR)/$(strip $(2)).rc
$(call gb_WinResTarget_get_target,$(1)) : $(SRCDIR)/$(strip $(2)).rc

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_WinResTarget_get_dep_target,$(1)) : RCFILE := $(SRCDIR)/$(strip $(2)).rc
endif

endef

define gb_WinResTarget_add_dependency
$(call gb_WinResTarget_get_target,$(1)) : $(SRCDIR)/$(strip $(2))

endef

define gb_WinResTarget_add_dependencies
$(foreach dep,$(2),$(call gb_WinResTarget_add_dependency,$(1),$(dep)))

endef

define gb_WinResTarget__use_custom_headers
$(call gb_WinResTarget_get_target,$(1)) : $(call gb_CustomTarget_get_target,$(2))
$(call gb_WinResTarget__add_include,$(1),$(call gb_CustomTarget_get_workdir,$(2)))

endef

define gb_WinResTarget_use_custom_headers
$(foreach customtarget,$(2),$(call gb_WinResTarget__use_custom_headers,$(1),$(customtarget)))

endef

# vim: set noet sw=4 ts=4:
