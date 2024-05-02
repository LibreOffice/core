# vim: set noet sw=4 ts=4:
# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,postprocess/components))

ifeq (,$(gb_PARTIAL_BUILD))

postprocess_WORKDIR := $(gb_CustomTarget_workdir)/postprocess

$(call gb_CustomTarget_get_target,postprocess/components): \
    $(postprocess_WORKDIR)/services_constructors.list \

gb_EMPTY :=
define gb_NEWLINE

$(gb_EMPTY)
endef

$(postprocess_WORKDIR)/services_componentfiles.list: \
    $(call gb_Rdb_get_target,services) \
    $(call gb_Rdb_get_target,ure/services) \
    $(foreach comp,$(gb_ComponentTarget__ALLCOMPONENTS),$(call gb_ComponentTarget_get_target,$(comp))) \
    | $(postprocess_WORKDIR)/.dir
	$(call gb_Output_announce,$(subst $(BUILDDIR)/,,$@),$(true),GEN,2)
	$(file >$@,$(foreach comp,$(sort $(gb_ComponentTarget__ALLCOMPONENTS)),$(call gb_ComponentTarget_get_target,$(comp))$(gb_NEWLINE)))

$(postprocess_WORKDIR)/services_constructors.list: \
    $(SRCDIR)/solenv/bin/constructors.py \
    $(postprocess_WORKDIR)/services_componentfiles.list \
    | $(postprocess_WORKDIR)/.dir
	$(call gb_Output_announce,$(subst $(BUILDDIR)/,,$@),$(true),GEN,2)
	$(call gb_Helper_abbreviate_dirs,$(call gb_ExternalExecutable_get_command,python) $^) > $@.tmp && \
	    $(call gb_Helper_replace_if_different_and_touch,$@.tmp,$@)

.PHONY: $(postprocess_WORKDIR)/services_componentfiles.list

endif # gb_PARTIAL_BUILD

# vim: set noet sw=4:
