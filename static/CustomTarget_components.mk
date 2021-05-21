# vim: set noet sw=4 ts=4:
# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,static/components))

static_WORKDIR := $(call gb_CustomTarget_get_workdir,static)
postprocess_WORKDIR := $(call gb_CustomTarget_get_workdir,postprocess)

$(call gb_CustomTarget_get_target,static/components): \
    $(static_WORKDIR)/component_maps.cxx \

define gb_static_components_create_component_map
TEMPFILE=`$(gb_MKTEMP)` && \
$(call gb_Helper_abbreviate_dirs, \
    $(call gb_ExternalExecutable_get_command,python) $(1) \
         -c $(postprocess_WORKDIR)/services_constructors.list \
) > $$TEMPFILE && \
$(call gb_Helper_replace_if_different_and_touch,$${TEMPFILE},$(2))

endef

$(static_WORKDIR)/component_maps.cxx: \
    $(postprocess_WORKDIR)/services_constructors.list \
    $(SRCDIR)/solenv/bin/native-code.py \
    | $(static_WORKDIR)/.dir
	$(call gb_Output_announce,$(subst $(BUILDDIR)/,,$@),$(true),GEN,2)
	$(call gb_static_components_create_component_map,$<,$@)

# vim: set noet sw=4:
