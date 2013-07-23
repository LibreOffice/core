# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(dir $(call gb_Postprocess_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_Postprocess_get_target,%) :
	$(call gb_Output_announce,$(POSTPROCESS_INFO): $(if $(POSTPROCESS_PREFIX),$(subst $(POSTPROCESS_PREFIX),,$^),$^),$(true),ALL)
	touch $@

.PHONY : $(call gb_Postprocess_get_clean_target,%)
$(call gb_Postprocess_get_clean_target,%) :
	$(call gb_Output_announce,$(POSTPROCESS_INFO): $(if $(POSTPROCESS_PREFIX),$(subst $(POSTPROCESS_PREFIX),,$^),$^),$(false),ALL)
	rm -f $(call gb_Postprocess_get_target,$*)

define gb_Postprocess_Postprocess
$(call gb_Postprocess_get_target,$(1)) : POSTPROCESS_INFO := $(2)
$(call gb_Postprocess_get_target,$(1)) : POSTPROCESS_PREFIX := $(3)
$(call gb_Postprocess_get_clean_target,$(1)) : POSTPROCESS_INFO := $(2)
$(call gb_Postprocess_get_clean_target,$(1)) : POSTPROCESS_PREFIX := $(subst $(WORKDIR),$(WORKDIR)/Clean,$(3))

$(call gb_Postprocess_get_target,$(1)) :| $(dir $(call gb_Postprocess_get_target,$(1))).dir

$(call gb_Helper_make_userfriendly_targets,$(1),Postprocess)

endef

# gb_Postprocess_register_target category class targetname
define gb_Postprocess_register_target
$(call gb_Postprocess_get_target,$(1)) : $(call gb_$(2)_get_target,$(3))
$(call gb_Postprocess_get_clean_target,$(1)) : $(call gb_$(2)_get_clean_target,$(3))

endef

define gb_Postprocess_make_targets
$(call gb_Postprocess_Postprocess,AllExecutables,All executables)
$(call gb_Postprocess_Postprocess,AllLibraries,All libraries)
$(call gb_Postprocess_Postprocess,AllModulesButInstsetNative,All modules but instset,$(WORKDIR)/Module/)
$(call gb_Postprocess_Postprocess,AllPackages,All packages,$(WORKDIR)/Package/)
$(call gb_Postprocess_Postprocess,AllResources,All resources,$(WORKDIR)/AllLangRes/)
$(call gb_Postprocess_Postprocess,AllUIConfigs,All UI configuration files,$(WORKDIR)/UIConfig/)
$(call gb_Postprocess_Postprocess,AllModuleTests,All modules' tests,$(WORKDIR)/Module/check/)

endef

# vim: set noet sw=4 ts=4:
