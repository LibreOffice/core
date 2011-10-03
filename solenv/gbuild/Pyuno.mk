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
# Copyright (C) 2011 David Tardon, Red Hat Inc. <dtardon@redhat.com> (initial developer)
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

ifeq ($(OS),WNT)
gb_Pyuno__get_outdir_path = bin/pyuno/$(1)
else
gb_Pyuno__get_outdir_path = lib/pyuno/$(1)
endif

$(call gb_Pyuno_get_target,%) :
	$(call gb_Output_announce,$*,$(true),PYU,3)
	mkdir -p $(dir $@) && touch $@

.PHONY : $(call gb_Pyuno_get_clean_target,%)
$(call gb_Pyuno_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),PYU,3)
	rm -f $@

define gb_Pyuno_Pyuno
$(call gb_Package_Package,$(1)_pyuno,$(2))
$$(eval $$(call gb_Module_register_target,$(call gb_Pyuno_get_target,$(1)),$(call gb_Pyuno_get_clean_target,$(1))))
$(call gb_Pyuno_get_target,$(1)) : $(call gb_Package_get_target,$(1)_pyuno)
$(call gb_Pyuno_get_clean_target,$(1)) : $(call gb_Package_get_clean_target,$(1)_pyuno)

endef

define gb_Pyuno_add_file
$(call gb_Package_add_file,$(1)_pyuno,$(call gb_Pyuno__get_outdir_path,$(2)),$(2))

endef

define gb_Pyuno_add_files
$(foreach file,$(2),$(call gb_Pyuno_add_file,$(1),$(file)))

endef

gb_Pyuno__COMPONENTPREFIX := vnd.openoffice.pymodule:

define gb_Pyuno_set_componentfile
$(call gb_ComponentTarget_ComponentTarget,$(2),$(gb_Pyuno__COMPONENTPREFIX),$(1))
$(call gb_Pyuno_get_target,$(1)) : $(call gb_ComponentTarget_get_outdir_target,$(2))
$(call gb_Pyuno_get_clean_target,$(1)) : $(call gb_ComponentTarget_get_clean_target,$(2))

endef

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
