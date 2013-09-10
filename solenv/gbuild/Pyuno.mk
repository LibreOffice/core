# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# class Pyuno
#
# Handles creation and delivery of Python UNO components.
#
# Provides one filelist, called Pyuno/<name>.

$(dir $(call gb_Pyuno_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_Pyuno_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_Pyuno_get_target,%) :
	$(call gb_Output_announce,$*,$(true),PYU,3)
	touch $@

.PHONY : $(call gb_Pyuno_get_clean_target,%)
$(call gb_Pyuno_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),PYU,3)
	rm -f $(call gb_Pyuno_get_target,$*)

gb_Pyuno_get_packagename = Pyuno/$(1)

# gb_Pyuno_Pyuno component
define gb_Pyuno_Pyuno
$(call gb_Package_Package_internal,$(call gb_Pyuno_get_packagename,$(1)),$(2))
$(call gb_Package_set_outdir,$(call gb_Pyuno_get_packagename,$(1)),$(gb_INSTROOT))

$(call gb_Pyuno_get_target,$(1)) : $(call gb_Package_get_target,$(call gb_Pyuno_get_packagename,$(1)))
$(call gb_Pyuno_get_target,$(1)) :| $(dir $(call gb_Pyuno_get_target,$(1))).dir
$(call gb_Pyuno_get_clean_target,$(1)) : $(call gb_Package_get_clean_target,$(call gb_Pyuno_get_packagename,$(1)))

$$(eval $$(call gb_Module_register_target,$(call gb_Pyuno_get_target,$(1)),$(call gb_Pyuno_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),Pyuno)

endef

# gb_Pyuno_add_file component destination source
define gb_Pyuno_add_file
$(call gb_Package_add_file,$(call gb_Pyuno_get_packagename,$(1)),$(LIBO_LIB_PYUNO_FOLDER)/$(2),$(3))

endef

# gb_Pyuno_add_files component destdir source
define gb_Pyuno_add_files
$(foreach file,$(3),$(call gb_Pyuno_add_file,$(1),$(if $(strip $(2)),$(strip $(2))/)$(file),$(file)))

endef

gb_Pyuno__COMPONENTPREFIX := vnd.openoffice.pymodule:

define gb_Pyuno_set_componentfile_full
$(call gb_ComponentTarget_ComponentTarget,$(2),$(3),$(4))
$(call gb_Pyuno_get_target,$(1)) : $(call gb_ComponentTarget_get_target,$(2))
$(call gb_Pyuno_get_clean_target,$(1)) : $(call gb_ComponentTarget_get_clean_target,$(2))

endef

# Set .component file for the component.
define gb_Pyuno_set_componentfile
$(call gb_Pyuno_set_componentfile_full,$(1),$(2),$(gb_Pyuno__COMPONENTPREFIX),$(1))

endef

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
