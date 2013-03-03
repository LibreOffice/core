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

$(call gb_Pyuno_get_target,%) :
	$(call gb_Output_announce,$*,$(true),PYU,3)
	mkdir -p $(dir $@) && touch $@

.PHONY : $(call gb_Pyuno_get_clean_target,%)
$(call gb_Pyuno_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),PYU,3)
	rm -f $(call gb_Pyuno_get_target,$*) $(call gb_Pyuno_get_outdir_target,$*)

define gb_Pyuno_Pyuno
$(call gb_Zip_Zip,Pyuno/$(1),$(2))
$$(eval $$(call gb_Module_register_target,$(call gb_Pyuno_get_target,$(1)),$(call gb_Pyuno_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),Pyuno)
$(call gb_Pyuno_get_target,$(1)) : $(call gb_Pyuno_get_outdir_target,$(1))
$(call gb_Pyuno_get_outdir_target,$(1)) : $(call gb_Zip_get_target,Pyuno/$(1))
$(call gb_Pyuno_get_outdir_target,$(1)) :| $(dir $(call gb_Pyuno_get_outdir_target,$(1))).dir
$(call gb_Pyuno_get_clean_target,$(1)) : $(call gb_Zip_get_clean_target,Pyuno/$(1))

endef

define gb_Pyuno_add_file
$(call gb_Zip_add_file,Pyuno/$(1),$(2))

endef

define gb_Pyuno_add_files
$(foreach file,$(2),$(call gb_Pyuno_add_file,$(1),$(file)))

endef

gb_Pyuno__COMPONENTPREFIX := vnd.openoffice.pymodule:

define gb_Pyuno_set_componentfile_full
$(call gb_ComponentTarget_ComponentTarget,$(2),$(3),$(4))
$(call gb_Pyuno_get_target,$(1)) : $(call gb_ComponentTarget_get_outdir_target,$(2))
$(call gb_Pyuno_get_clean_target,$(1)) : $(call gb_ComponentTarget_get_clean_target,$(2))

endef

define gb_Pyuno_set_componentfile
$(call gb_Pyuno_set_componentfile_full,$(1),$(2),$(gb_Pyuno__COMPONENTPREFIX),$(1))

endef

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
