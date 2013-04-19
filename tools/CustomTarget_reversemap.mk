#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,tools/reversemap))

$(call gb_CustomTarget_get_target,tools/reversemap) : \
	$(call gb_CustomTarget_get_workdir,tools/reversemap)/reversemap.hxx

$(call gb_CustomTarget_get_workdir,tools/reversemap)/reversemap.hxx : \
		$(call gb_Executable_get_runtime_dependencies,bestreversemap) \
		| $(call gb_CustomTarget_get_workdir,tools/reversemap)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),BRM,1)
	$(call gb_Helper_execute,bestreversemap > $@)

# vim: set noet sw=4 ts=4:
