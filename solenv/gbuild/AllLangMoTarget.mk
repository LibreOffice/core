# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# Overview of dependencies and tasks of AllLangMoTarget
#
# target                task                depends on
# AllLangMoTarget      nothing              MoTarget for all active langs
# MoTarget             running msgfmt

gb_MoTarget_LOCALESTRDEPS := $(call gb_Executable_get_runtime_dependencies,localestr)
gb_MoTarget_LOCALESTRCOMMAND := $(call gb_Executable_get_command,localestr)

# MoTarget

$(call gb_MoTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),RES,2)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f \
			$(call gb_MoTarget_get_target,$*) \
			$(call gb_MoTarget_get_install_target,$*))

#for the moment merge existing source and ui translations into a single .po,
#remove msgctxt and unique the translations and allow fuzzy
#eventually instead can do something like
#msgfmt $(gb_POLOCATION)/$(LANGUAGE)/$(LIBRARY)/$(LIBRARY).po -o $@)
$(call gb_MoTarget_get_target,%) : $(gb_Helper_MISCDUMMY) $(gb_MoTarget_LOCALESTRDEPS)
	$(call gb_Output_announce,$*,$(true),MO,2)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		mkdir -p $(WORKDIR)/MoTarget/$(LIBRARY)/$(LANGUAGE) && \
		msgcat `find $(gb_POLOCATION)/$(LANGUAGE)/$(POLOCATION) -name "*.po" | grep -v registry` | $(SRCDIR)/solenv/bin/strip-msgctxt | msguniq > $(WORKDIR)/MoTarget/$(LIBRARY)/$(LANGUAGE)/combined.po && \
		cat $(WORKDIR)/MoTarget/$(LIBRARY)/$(LANGUAGE)/combined.po | msgfmt - -f -o $@ && \
		touch $@)

#$(info $(call gb_MoTarget_get_target,$(1)))
define gb_MoTarget_MoTarget
$(call gb_MoTarget_get_target,$(1)) : LIBRARY = $(2)
$(call gb_MoTarget_get_target,$(1)) : LANGUAGE = $(3)
$(call gb_MoTarget_get_target,$(1)) : POLOCATION = $(2)
$(call gb_AllLangMoTarget_get_clean_target,$(2)) : $(call gb_MoTarget_get_clean_target,$(1))

endef

define gb_MoTarget_set_polocation
$(call gb_MoTarget_get_target,$(1)) : POLOCATION = $(2)

endef

# AllLangMoTarget

gb_AllLangMoTarget_LANGS := $(filter-out qtz,$(filter-out en-US,$(gb_WITH_LANG)))

define gb_AllLangMoTarget_set_langs
gb_AllLangMoTarget_LANGS := $(1)
endef

$(call gb_AllLangMoTarget_get_clean_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_AllLangMoTarget_get_target,$*))

$(call gb_AllLangMoTarget_get_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && touch $@)

gb_MoTarget_get_install_target = $(INSTROOT)/$(LIBO_SHARE_RESOURCE_FOLDER)/$(1).mo

define gb_AllLangMoTarget_AllLangMoTarget
ifeq (,$$(filter $(1),$$(gb_AllLangMoTarget_REGISTERED)))
$$(eval $$(call gb_Output_info,Currently known AllLangMoTargets are: $(sort $(gb_AllLangMoTarget_REGISTERED)),ALL))
$$(eval $$(call gb_Output_error,AllLangMoTarget $(1) must be registered in Repository.mk))
endif
$(foreach lang,$(gb_AllLangMoTarget_LANGS),\
	$(call gb_MoTarget_MoTarget,$(1)$(lang),$(1),$(lang)))

$(foreach lang,$(gb_AllLangMoTarget_LANGS),\
$(call gb_Helper_install,$(call gb_AllLangMoTarget_get_target,$(1)), \
	$(call gb_MoTarget_get_install_target,$(shell $(SRCDIR)/bin/run localestr $(lang))/LC_MESSAGES/$(1)), \
	$(call gb_MoTarget_get_target,$(1)$(lang))))

$$(eval $$(call gb_Module_register_target,$(call gb_AllLangMoTarget_get_target,$(1)),$(call gb_AllLangMoTarget_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),AllLangMoTarget)
$(call gb_Postprocess_register_target,AllResources,AllLangMoTarget,$(1))

endef

define gb_AllLangMoTarget_set_polocation
$(foreach lang,$(gb_AllLangMoTarget_LANGS),\
        $(call gb_MoTarget_set_polocation,$(1)$(lang),$(2)))

endef

# vim: set noet sw=4: 
