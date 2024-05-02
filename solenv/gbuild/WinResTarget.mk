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

# WinResTarget class

gb_WinResTarget_DEFAULTDEFS := $(gb_RCDEFS)

define gb_WinResTarget_WinResTarget
$(call gb_WinResTarget_WinResTarget_init,$(1))
$$(eval $$(call gb_Module_register_target,$(call gb_WinResTarget_get_target,$(1)),$(call gb_WinResTarget_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),WinResTarget)

endef

define gb_WinResTarget_WinResTarget_init
$(call gb_WinResTarget_get_target,$(1)) : DEFS := $(gb_WinResTarget_DEFAULTDEFS)
$(call gb_WinResTarget_get_target,$(1)) : FLAGS := $(gb_RCFLAGS)
$(call gb_WinResTarget_get_target,$(1)) : INCLUDE := -I$(SRCDIR)/include $(subst -isystem,-I,$(SOLARINC)) -I$(BUILDDIR)/config_$(gb_Side)
$(call gb_WinResTarget_get_clean_target,$(1)) : RCFILE :=
$(call gb_WinResTarget_get_target,$(1)) : RCFILE :=

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_WinResTarget_get_dep_target,$(1)) : DEFS := $$(gb_WinResTarget_DEFAULTDEFS)
$(call gb_WinResTarget_get_dep_target,$(1)) : FLAGS := $$(gb_RCFLAGS)
$(call gb_WinResTarget_get_dep_target,$(1)) : INCLUDE := $$(gb_WinResTarget_INCLUDE) -I$(BUILDDIR)/config_$(gb_Side)
$(call gb_WinResTarget_get_dep_target,$(1)) : RCFILE :=

-include $(call gb_WinResTarget_get_dep_target,$(1))
endif

endef

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_WinResTarget_get_dep_target,%) : $(gb_WinResTarget__command_target)
	$(call gb_WinResTarget__command_dep,$@,$*,$(call gb_WinResTarget_get_target,$*))
endif


$(call gb_WinResTarget_get_target,%) :
	$(call gb_WinResTarget__command,$@,$*,$<)

$(call gb_WinResTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),RC ,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_WinResTarget_get_target,$*) \
			$(call gb_WinResTarget_get_dep_target,$*))

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
$(call gb_WinResTarget__add_include,$(1),$(gb_CustomTarget_workdir)/$(2))

endef

define gb_WinResTarget_use_custom_headers
$(foreach customtarget,$(2),$(call gb_WinResTarget__use_custom_headers,$(1),$(customtarget)))

endef

# vim: set noet sw=4 ts=4:
