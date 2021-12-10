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

postprocess_WORKDIR := $(call gb_CustomTarget_get_workdir,postprocess)

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
	TEMPFILE=$(call gb_var2file,$(shell $(gb_MKTEMP)),1, \
            $(foreach comp,$(sort $(gb_ComponentTarget__ALLCOMPONENTS)),$(call gb_ComponentTarget_get_target,$(comp))$(gb_NEWLINE))) && \
            mv $$TEMPFILE $@

$(postprocess_WORKDIR)/services_constructors.list: \
    $(SRCDIR)/solenv/bin/constructors.py \
    $(postprocess_WORKDIR)/services_componentfiles.list \
    | $(postprocess_WORKDIR)/.dir
	$(call gb_Output_announce,$(subst $(BUILDDIR)/,,$@),$(true),GEN,2)
	TEMPFILE=$(shell $(gb_MKTEMP)) && \
	    $(call gb_Helper_abbreviate_dirs,$(call gb_ExternalExecutable_get_command,python) $^) > $$TEMPFILE && \
	    $(call gb_Helper_replace_if_different_and_touch,$${TEMPFILE},$@)

.PHONY: $(postprocess_WORKDIR)/services_componentfiles.list

endif # gb_PARTIAL_BUILD

# vim: set noet sw=4:
