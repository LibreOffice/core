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

define gb_Postprocess_Postprocess
$(call gb_Postprocess_get_target,$(1)) : POSTPROCESS_INFO := $(2)
$(call gb_Postprocess_get_target,$(1)) : POSTPROCESS_PREFIX := $(3)

$(call gb_Postprocess_get_target,$(1)) :| $(dir $(call gb_Postprocess_get_target,$(1))).dir

endef

define gb_Postprocess_make_targets
$(call gb_Postprocess_Postprocess,AllExecutables,All executables)
$(call gb_Postprocess_Postprocess,AllLibraries,All libraries)
$(call gb_Postprocess_Postprocess,AllModulesButInstsetNative,All modules but instset,$(WORKDIR)/Module/)
$(call gb_Postprocess_Postprocess,AllPackages,All packages,$(WORKDIR)/Package/)
$(call gb_Postprocess_Postprocess,AllResources,All resources,$(WORKDIR)/AllLangRes/)
$(call gb_Postprocess_Postprocess,AllUIs,All UI files,$(WORKDIR)/UI/)

endef

# vim: set noet sw=4 ts=4:
