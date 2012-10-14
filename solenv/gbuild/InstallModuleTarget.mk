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
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
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

# Overview of dependencies and tasks of InstallModuleTarget
# target						task						depends on

# ScpTemplateTarget class

gb_ScpTemplateTarget_TARGET := $(SRCDIR)/scp2/source/templates/modules.pl
gb_ScpTemplateTarget_COMMAND := $(PERL) -w $(gb_ScpTemplateTarget_TARGET)

gb_ScpTemplateTarget_LANGS := $(sort $(ALL_LANGS))

# Pass first arg if make is running in silent mode, second arg otherwise
define gb_ScpTemplateTarget__if_silent
$(if $(findstring s,$(filter-out --%,$(MAKEFLAGS))),$(1),$(2))
endef

gb_ScpTemplateTarget_get_source = $(SRCDIR)/$(1).sct

define gb_ScpTemplateTarget__command
$(call gb_Output_announce,$(2),$(true),SCT,1)
$(call gb_Helper_abbreviate_dirs,\
	export COMPLETELANGISO_VAR='$(gb_ScpTemplateTarget_LANGS)' && \
	$(gb_ScpTemplateTarget_COMMAND) \
		$(call gb_ScpTemplateTarget__if_silent,,-verbose) \
		-i $(SCP_TEMPLATE) \
		-o $(1) \
)
endef

$(dir $(call gb_ScpTemplateTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_ScpTemplateTarget_get_target,%) : $(gb_ScpTemplateTarget_TARGET)
	$(call gb_ScpTemplateTarget__command,$@,$*)

.PHONY : $(call gb_ScpTemplateTarget_get_clean_target,%)
$(call gb_ScpTemplateTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),SCT,1)
	rm -f $(call gb_ScpTemplateTarget_get_target,$*)

# gb_ScpTemplateTarget_ScpTemplateTarget(<target>)
define gb_ScpTemplateTarget_ScpTemplateTarget
$(call gb_ScpTemplateTarget_get_target,$(1)) : $(call gb_ScpTemplateTarget_get_source,$(1))
$(call gb_ScpTemplateTarget_get_target,$(1)) :| $(dir $(call gb_ScpTemplateTarget_get_target,$(1))).dir
$(call gb_ScpTemplateTarget_get_target,$(1)) : SCP_TEMPLATE := $(call gb_ScpTemplateTarget_get_source,$(1))

endef

# ScpPreprocessTarget class

gb_ScpPreprocessTarget_TARGET := $(call gb_Executable_get_target_for_build,cpp)
gb_ScpPreprocessTarget_COMMAND := $(gb_Helper_set_ld_path) $(gb_ScpPreprocessTarget_TARGET)

gb_ScpPreprocessTarget_get_source = $(SRCDIR)/$(1).scp

define gb_ScpPreprocessTarget__command
$(call gb_Output_announce,$(2),$(true),SPP,2)
$(call gb_Helper_abbreviate_dirs,\
	$(gb_ScpPreprocessTarget_COMMAND) \
		-+ -P \
		$(SCPDEFS) $(SCP_DEFS) -DDLLPOSTFIX=$(gb_Library_DLLPOSTFIX) \
		$(SCP_INCLUDE) $(SCP_TEMPLATE_INCLUDE) \
		$(SCP_SOURCE) > $(1) \
)
endef

$(dir $(call gb_ScpPreprocessTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_ScpPreprocessTarget_get_target,%) : $(gb_ScpPreprocessTarget_TARGET)
	$(call gb_ScpPreprocessTarget__command,$@,$*)

.PHONY : $(call gb_ScpPreprocessTarget_get_clean_target,%)
$(call gb_ScpPreprocessTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),SPP,2)
	rm -f $(call gb_ScpPreprocessTarget_get_target,$*)

# gb_ScpPreprocessTarget_ScpPreprocessTarget(<target>)
define gb_ScpPreprocessTarget_ScpPreprocessTarget
$(call gb_ScpPreprocessTarget_get_target,$(1)) : SCP_SOURCE := $(call gb_ScpPreprocessTarget_get_source,$(1))
$(call gb_ScpPreprocessTarget_get_target,$(1)) : $(call gb_ScpPreprocessTarget_get_source,$(1))
$(call gb_ScpPreprocessTarget_get_target,$(1)) :| $(dir $(call gb_ScpPreprocessTarget_get_target,$(1))).dir

endef

# ScpMergeTarget class

gb_ScpMergeTarget_TARGET := $(call gb_Executable_get_target_for_build,ulfex)
gb_ScpMergeTarget_COMMAND := $(gb_Helper_set_ld_path) $(gb_ScpMergeTarget_TARGET)

gb_ScpMergeTarget_get_source = $(SRCDIR)/$(1).ulf

define gb_ScpMergeTarget__command
$(call gb_Output_announce,$(2),$(true),SUM,1)
MERGEINPUT=`$(gb_MKTEMP)` && \
echo $(SCP_POFILES) > $${MERGEINPUT} && \
$(call gb_Helper_abbreviate_dirs,\
	$(gb_ScpMergeTarget_COMMAND) -p scp2 -i $(SCP_SOURCE) -o $(1) -m $${MERGEINPUT} -l all ) && \
rm -rf $${MERGEINPUT}

endef

$(dir $(call gb_ScpMergeTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_ScpMergeTarget_get_target,%) : $(gb_ScpMergeTarget_TARGET)
	$(call gb_ScpMergeTarget__command,$@,$*)

.PHONY : $(call gb_ScpMergeTarget_get_clean_target,%)
$(call gb_ScpMergeTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),SUM,1)
	rm -f $(call gb_ScpMergeTarget_get_target,$*)

# gb_ScpMergeTarget_ScpMergeTarget(<target>)
define gb_ScpMergeTarget_ScpMergeTarget
$(call gb_ScpMergeTarget_get_target,$(1)) : SCP_SOURCE := $(call gb_ScpMergeTarget_get_source,$(1))
$(call gb_ScpMergeTarget_get_target,$(1)) : $(call gb_ScpMergeTarget_get_source,$(1))
$(call gb_ScpMergeTarget_get_target,$(1)) :| $(dir $(call gb_ScpMergeTarget_get_target,$(1))).dir
$(call gb_ScpMergeTarget_get_target,$(1)) : \
	SCP_POFILES := $(foreach lang,$(filter-out en-US,$(gb_WITH_LANG)),$(gb_POLOCATION)/$(lang)/$(patsubst %/,%,$(dir $(1))).po)
$(call gb_ScpMergeTarget_get_target,$(1)) : \
	$(foreach lang,$(filter-out en-US,$(gb_WITH_LANG)),$(gb_POLOCATION)/$(lang)/$(patsubst %/,%,$(dir $(1))).po)

endef

# ScpConvertTarget class

# platform:
#  gb_ScpConvertTarget_ScpConvertTarget_platform

gb_ScpConvertTarget_TARGET := $(call gb_Executable_get_target_for_build,ulfconv)
gb_ScpConvertTarget_COMMAND := $(gb_Helper_set_ld_path) $(gb_ScpConvertTarget_TARGET)

define gb_ScpConvertTarget__command
$(call gb_Output_announce,$(2),$(true),SCC,1)
$(call gb_Helper_abbreviate_dirs,\
	$(gb_ScpConvertTarget_COMMAND) $(SCP_FLAGS) -o $(1) $(SCP_CONVERT_ULF) \
)
endef

$(dir $(call gb_ScpConvertTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_ScpConvertTarget_get_target,%) : $(gb_ScpConvertTarget_TARGET)
	$(call gb_ScpConvertTarget__command,$@,$*)

.PHONY : $(call gb_ScpConvertTarget_get_clean_target,%)
$(call gb_ScpConvertTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),SCC,1)
	rm -f $(call gb_ScpConvertTarget_get_target,$*)

# gb_ScpConvertTarget_ScpConvertTarget(<target>,<ulffile>)
define gb_ScpConvertTarget_ScpConvertTarget
$(call gb_ScpConvertTarget_get_target,$(1)) : $(2)
$(call gb_ScpConvertTarget_get_target,$(1)) :| $(dir $(call gb_ScpConvertTarget_get_target,$(1))).dir
$(call gb_ScpConvertTarget_get_target,$(1)) : SCP_CONVERT_ULF := $(2)
$(call gb_ScpConvertTarget_get_target,$(1)) : SCP_FLAGS :=

$(call gb_ScpConvertTarget_ScpConvertTarget_platform,$(1))

endef

# ScpTarget class

gb_ScpTarget_TARGET := $(SOLARENV)/bin/pre2par.pl
gb_ScpTarget_COMMAND := $(PERL) $(gb_ScpTarget_TARGET)

define gb_ScpTarget__command
$(call gb_Output_announce,$(2),$(true),SCP,2)
$(call gb_Helper_abbreviate_dirs,\
	$(gb_ScpTarget_COMMAND) -l $(SCP_ULF) -s $(SCP_SOURCE) -o $(1) \
)
endef

$(dir $(call gb_ScpTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_ScpTarget_get_target,%) : $(gb_ScpTarget_TARGET)
	$(call gb_ScpTarget__command,$@,$*)

$(call gb_ScpTarget_get_external_target,%) :
	touch $@

.PHONY : $(call gb_ScpTarget_get_clean_target,%)
$(call gb_ScpTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),SCP,2)
	rm -f \
		$(call gb_ScpTarget_get_target,$*) \
		$(call gb_ScpTarget_get_external_target,$*)

# gb_ScpTarget_ScpTarget(<target>)
define gb_ScpTarget_ScpTarget
$(call gb_ScpPreprocessTarget_ScpPreprocessTarget,$(1))

$(call gb_ScpTarget_get_target,$(1)) : $(call gb_ScpPreprocessTarget_get_target,$(1))
$(call gb_ScpTarget_get_target,$(1)) :| $(call gb_ScpTarget_get_external_target,$(1))
$(call gb_ScpTarget_get_external_target,$(1)) :| $(dir $(call gb_ScpTarget_get_target,$(1))).dir
$(call gb_ScpPreprocessTarget_get_target,$(1)) :| $(call gb_ScpTarget_get_external_target,$(1))
$(call gb_ScpTarget_get_clean_target,$(1)) : $(call gb_ScpPreprocessTarget_get_clean_target,$(1))
$(call gb_ScpTarget_get_target,$(1)) : SCP_SOURCE := $(call gb_ScpPreprocessTarget_get_target,$(1))
$(call gb_ScpTarget_get_target,$(1)) : SCP_ULF := $(gb_Helper_PHONY)

endef

define gb_ScpTarget_set_localized
ifneq ($(gb_WITH_LANG),)
$(call gb_ScpMergeTarget_ScpMergeTarget,$(1))
$(call gb_ScpConvertTarget_ScpConvertTarget,$(1),$(call gb_ScpMergeTarget_get_target,$(1)))
$(call gb_ScpTarget_get_clean_target,$(1)) : $(call gb_ScpMergeTarget_get_clean_target,$(1))
else
$(call gb_ScpConvertTarget_ScpConvertTarget,$(1),$(call gb_ScpMergeTarget_get_source,$(1)))
endif

$(call gb_ScpTarget_get_target,$(1)) : $(call gb_ScpConvertTarget_get_target,$(1))
$(call gb_ScpTarget_get_clean_target,$(1)) : $(call gb_ScpConvertTarget_get_clean_target,$(1))
$(call gb_ScpTarget_get_target,$(1)) : SCP_ULF := $(call gb_ScpConvertTarget_get_target,$(1))

endef

# InstallModuleTarget class

# platform:
#  gb_InstallModuleTarget_InstallModuleTarget_platform

define gb_InstallModuleTarget__command
$(call gb_Output_announce,$(2),$(true),INM,3)
$(call gb_Helper_abbreviate_dirs,\
	rm -f $(1) \
	$(foreach scpfile,$(SCP_FILES),&& echo $(scpfile) >> $(1)) \
)
endef

$(dir $(call gb_InstallModuleTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_InstallModuleTarget_get_target,%) :
	$(call gb_InstallModuleTarget__command,$@,$*)

$(call gb_InstallModuleTarget_get_external_target,%) :
	touch $@

.PHONY : $(call gb_InstallModuleTarget_get_clean_target,%)
$(call gb_InstallModuleTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),INM,3)
	rm -rf \
		$(call gb_InstallModuleTarget_get_target,$*) \
		$(call gb_InstallModuleTarget_get_external_target,$*)

define gb_InstallModuleTarget_InstallModuleTarget
$(call gb_InstallModuleTarget_get_target,$(1)) :| \
	$(call gb_InstallModuleTarget_get_external_target,$(1))
$(call gb_InstallModuleTarget_get_external_target,$(1)) :| \
	$(dir $(call gb_InstallModuleTarget_get_target,$(1))).dir

$(call gb_InstallModuleTarget_get_target,$(1)) : SCP_FILES :=
$(call gb_InstallModuleTarget_get_target,$(1)) : SCP_DEFS :=
$(call gb_InstallModuleTarget_get_target,$(1)) : SCP_INCLUDE := -I$(SRCDIR)/scp2/inc
$(call gb_InstallModuleTarget_get_target,$(1)) : SCP_TEMPLATE_INCLUDE :=
$(call gb_InstallModuleTarget_use_custom_headers,$(1),scp2/macros)

$(call gb_InstallModuleTarget_InstallModuleTarget_platform,$(1))

endef

define gb_InstallModuleTarget_add_defs
$(call gb_InstallModuleTarget_get_target,$(1)) : SCP_DEFS += $(2)

endef

define gb_InstallModuleTarget_define_if_set
$(call gb_InstallModuleTarget_add_defs,$(1),\
	$(foreach def,$(2),$(if $(filter TRUE YES,$($(def))),-D$(def))) \
)

endef

define gb_InstallModuleTarget_define_value_if_set
$(call gb_InstallModuleTarget_add_defs,$(1),\
	$(foreach def,$(2),$(if $($(def)),-D$(def)=\""$($(def))"\")) \
)

endef

define gb_InstallModuleTarget_define_mingw_dll_if_set
$(call gb_InstallModuleTarget_add_defs,$(1),\
	$(foreach def,$(2),$(if $($(def)),-DNEEDS_$(def) -D$(def)=\""$($(def))"\")) \
)

endef

define gb_InstallModuleTarget_use_custom_header
$(call gb_InstallModuleTarget_get_external_target,$(1)) :| $(call gb_CustomTarget_get_target,$(2))
$(call gb_InstallModuleTarget_get_target,$(1)) : SCP_INCLUDE += -I$(call gb_CustomTarget_get_workdir,$(2)) \

endef

define gb_InstallModuleTarget_use_custom_headers
$(foreach customtarget,$(2),$(call gb_InstallModuleTarget_use_custom_header,$(1),$(customtarget)))

endef

define gb_InstallModuleTarget_add_scpfile
$(call gb_ScpTarget_ScpTarget,$(2))
$(call gb_InstallModuleTarget_get_target,$(1)) : $(call gb_ScpTarget_get_target,$(2))
$(call gb_InstallModuleTarget_get_clean_target,$(1)) : $(call gb_ScpTarget_get_clean_target,$(2))
$(call gb_InstallModuleTarget_get_target,$(1)) : SCP_FILES += $(call gb_ScpTarget_get_target,$(2))
$(call gb_ScpTarget_get_external_target,$(2)) :| $(call gb_InstallModuleTarget_get_external_target,$(1))

endef

define gb_InstallModuleTarget_add_scpfiles
$(foreach scpfile,$(2),$(call gb_InstallModuleTarget_add_scpfile,$(1),$(scpfile)))

endef

define gb_InstallModuleTarget_add_localized_scpfile
$(call gb_InstallModuleTarget_add_scpfile,$(1),$(2))
$(call gb_ScpTarget_set_localized,$(2))

endef

define gb_InstallModuleTarget_add_localized_scpfiles
$(foreach scpfile,$(2),$(call gb_InstallModuleTarget_add_localized_scpfile,$(1),$(scpfile)))

endef

define gb_InstallModuleTarget_add_template
$(call gb_ScpTemplateTarget_ScpTemplateTarget,$(2))
$(call gb_InstallModuleTarget_get_external_target,$(1)) : $(call gb_ScpTemplateTarget_get_target,$(2))
$(call gb_InstallModuleTarget_get_clean_target,$(1)) : $(call gb_ScpTemplateTarget_get_clean_target,$(2))
$(call gb_InstallModuleTarget_get_target,$(1)) : \
	SCP_TEMPLATE_INCLUDE := $$(sort $$(SCP_TEMPLATE_INCLUDE) -I$(call gb_ScpTemplateTarget_get_dir,$(2)))

endef

define gb_InstallModuleTarget_add_templates
$(foreach template,$(2),$(call gb_InstallModuleTarget_add_template,$(1),$(template)))

endef

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
