# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

# class Pyuno
#
# Handles creation and delivery of Python UNO components.
#
# Provides one filelist, called Pyuno/<name>.

# platform
#  gb_Pyuno_PROGRAMDIRNAME

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
$(call gb_Package_set_outdir,$(call gb_Pyuno_get_packagename,$(1)),$(INSTDIR))

$(call gb_Pyuno_get_target,$(1)) : $(call gb_Package_get_target,$(call gb_Pyuno_get_packagename,$(1)))
$(call gb_Pyuno_get_target,$(1)) :| $(dir $(call gb_Pyuno_get_target,$(1))).dir
$(call gb_Pyuno_get_clean_target,$(1)) : $(call gb_Package_get_clean_target,$(call gb_Pyuno_get_packagename,$(1)))

$$(eval $$(call gb_Module_register_target,$(call gb_Pyuno_get_target,$(1)),$(call gb_Pyuno_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),Pyuno)

endef

# gb_Pyuno_add_file component destination source
define gb_Pyuno_add_file
$(call gb_Package_add_file,$(call gb_Pyuno_get_packagename,$(1)),$(gb_Pyuno_PROGRAMDIRNAME)/$(2),$(3))

endef

# gb_Pyuno_add_files component destdir source
define gb_Pyuno_add_files
$(foreach file,$(3),$(call gb_Pyuno_add_file,$(1),$(if $(strip $(2)),$(strip $(2))/)$(file),$(file)))

endef

gb_Pyuno__COMPONENTPREFIX := vnd.openoffice.pymodule:

define gb_Pyuno_set_componentfile_full
$(call gb_ComponentTarget_ComponentTarget,$(2),$(3),$(4))
$(call gb_Pyuno_get_target,$(1)) : $(call gb_ComponentTarget_get_outdir_target,$(2))
$(call gb_Pyuno_get_clean_target,$(1)) : $(call gb_ComponentTarget_get_clean_target,$(2))

endef

# Set .component file for the component.
define gb_Pyuno_set_componentfile
$(call gb_Pyuno_set_componentfile_full,$(1),$(2),$(gb_Pyuno__COMPONENTPREFIX),$(1))

endef

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
