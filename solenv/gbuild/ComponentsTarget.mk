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
# Copyright (C) 2011 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
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

# Create a .components file bundling multiple .component files:

# $(1): ID of this components entity (a relative pathname without ".components"
#  extension)
define gb_ComponentsTarget_ComponentsTarget
$(call gb_ComponentsTarget_get_target,$(1)): \
    $(call gb_ComponentsTarget_get_target,$(1)).input
$(call gb_ComponentsTarget_get_target,$(1)).input: COMPONENTFILES :=
$(eval $(call gb_Module_register_target,$(call gb_ComponentsTarget_get_target,$(1)),$(call gb_ComponentsTarget_get_clean_target,$(1))))

endef

# $(1): ID of this components entity
# $(2): absolute .component pathname (must be underneath $(WORKDIR) due to the
#  --stringparam prefix approach of $(SOLARENV)/bin/packcomponents.xslt)
define gb_ComponentsTarget_add_componentfile
$(call gb_ComponentsTarget_get_target,$(1)): $(2)
$(call gb_ComponentsTarget_get_target,$(1)).input: COMPONENTFILES += $(2)

endef

$(call gb_ComponentsTarget_get_target,%): \
            $(call gb_ComponentsTarget_get_target,%).input
	$(call gb_Output_announce,$*,$(true),CPS,1)
	$(call gb_Helper_abbreviate_dirs, \
            mkdir -p $(dir $@))
	$(call gb_Helper_abbreviate_dirs, \
            $(gb_XSLTPROC) --nonet --stringparam prefix $(WORKDIR) -o $@ \
            $(SOLARENV)/bin/packcomponents.xslt $@.input)

$(call gb_ComponentsTarget_get_clean_target,%): \
            $(call gb_ComponentsTarget_get_clean_target,%).input
	$(call gb_Output_announce,$*,$(false),CPS,1)
	$(call gb_Helper_abbreviate_dirs, \
            rm -f $(call gb_ComponentsTarget_get_target,$*))

$(call gb_ComponentsTarget_get_target,%).input:
	$(call gb_Output_announce,$*,$(true),CPI,1)
	$(call gb_Helper_abbreviate_dirs, \
            mkdir -p $(dir $@))
	$(call gb_Helper_abbreviate_dirs, \
            printf '<list>%s</list>' \
            "$(foreach i,$(COMPONENTFILES),<filename>$(subst $(WORKDIR),,$i)</filename>)" \
            > $@)

$(call gb_ComponentsTarget_get_clean_target,%).input:
	$(call gb_Output_announce,$*,$(false),CPI,1)
	$(call gb_Helper_abbreviate_dirs, \
            rm -f $(call gb_ComponentsTarget_get_target,$*).input)

# vim: set noet sw=4:
