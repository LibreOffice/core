# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

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

# depend on configure output to rebuild everything
$(call gb_ScpTemplateTarget_get_target,%) : \
		$(gb_ScpTemplateTarget_TARGET) $(BUILDDIR)/config_$(gb_Side).mk
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

gb_ScpPreprocessTarget_DEPS := $(call gb_Executable_get_runtime_dependencies,cpp)
gb_ScpPreprocessTarget_COMMAND := $(call gb_Executable_get_command,cpp)

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

# depend on configure output to rebuild everything
$(call gb_ScpPreprocessTarget_get_target,%) : \
		$(gb_ScpPreprocessTarget_DEPS) $(BUILDDIR)/config_$(gb_Side).mk
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

gb_ScpMergeTarget_get_source = $(SRCDIR)/$(1).ulf

$(dir $(call gb_ScpMergeTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(eval $(call gb_CustomTarget_ulfex_rule,\
	$(call gb_ScpMergeTarget_get_target,%),\
	$(call gb_ScpMergeTarget_get_source,%),\
	$$(SCP_POFILES)))

.PHONY : $(call gb_ScpMergeTarget_get_clean_target,%)
$(call gb_ScpMergeTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),SUM,1)
	rm -f $(call gb_ScpMergeTarget_get_target,$*)

# gb_ScpMergeTarget_ScpMergeTarget(<target>)
define gb_ScpMergeTarget_ScpMergeTarget
$(call gb_ScpMergeTarget_get_target,$(1)) :| $(dir $(call gb_ScpMergeTarget_get_target,$(1))).dir
$(call gb_ScpMergeTarget_get_target,$(1)) : \
	SCP_POFILES := $(foreach lang,$(gb_TRANS_LANGS),$(gb_POLOCATION)/$(lang)/$(patsubst %/,%,$(dir $(1))).po)
$(call gb_ScpMergeTarget_get_target,$(1)) : \
	$(foreach lang,$(gb_TRANS_LANGS),$(gb_POLOCATION)/$(lang)/$(patsubst %/,%,$(dir $(1))).po)
$(foreach lang,$(gb_TRANS_LANGS),$(gb_POLOCATION)/$(lang)/$(patsubst %/,%,$(dir $(1))).po) :

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
$(call gb_ScpTarget_get_target,$(1)) : $(call gb_ScpTarget_get_external_target,$(1))
$(call gb_ScpTarget_get_external_target,$(1)) :| $(dir $(call gb_ScpTarget_get_target,$(1))).dir
$(call gb_ScpPreprocessTarget_get_target,$(1)) : $(call gb_ScpTarget_get_external_target,$(1))
$(call gb_ScpTarget_get_clean_target,$(1)) : $(call gb_ScpPreprocessTarget_get_clean_target,$(1))
$(call gb_ScpTarget_get_target,$(1)) : SCP_SOURCE := $(call gb_ScpPreprocessTarget_get_target,$(1))
$(call gb_ScpTarget_get_target,$(1)) : SCP_ULF := $(gb_Helper_PHONY)

endef

define gb_ScpTarget_set_localized
ifneq ($(gb_WITH_LANG),)
$(call gb_ScpMergeTarget_ScpMergeTarget,$(1))
$(call gb_ScpTarget_get_target,$(1)) : SCP_ULF := $(call gb_ScpMergeTarget_get_target,$(1))
$(call gb_ScpTarget_get_target,$(1)) : $(call gb_ScpMergeTarget_get_target,$(1))
$(call gb_ScpTarget_get_clean_target,$(1)) : $(call gb_ScpMergeTarget_get_clean_target,$(1))
else
$(call gb_ScpTarget_get_target,$(1)) : SCP_ULF := $(call gb_ScpMergeTarget_get_source,$(1))
$(call gb_ScpTarget_get_target,$(1)) : $(call gb_ScpMergeTarget_get_source,$(1))
endif

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
$(call gb_InstallModuleTarget_get_target,$(1)) : \
	$(call gb_InstallModuleTarget_get_external_target,$(1))
$(call gb_InstallModuleTarget_get_external_target,$(1)) :| \
	$(dir $(call gb_InstallModuleTarget_get_target,$(1))).dir

$(call gb_InstallModuleTarget_get_target,$(1)) : SCP_FILES :=
$(call gb_InstallModuleTarget_get_target,$(1)) : SCP_DEFS :=
$(call gb_InstallModuleTarget_get_target,$(1)) : SCP_INCLUDE := -I$(SRCDIR)/scp2/inc -I$(WORKDIR) -I$(BUILDDIR)/config_$(gb_Side)
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

define gb_InstallModuleTarget_use_auto_install_libs
$(call gb_InstallModuleTarget_get_external_target,$(1)) : $(call gb_AutoInstall_get_target,$(2))

endef

define gb_InstallModuleTarget_use_custom_header
$(call gb_InstallModuleTarget_get_external_target,$(1)) : $(call gb_CustomTarget_get_target,$(2))
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
$(call gb_ScpTarget_get_external_target,$(2)) : $(call gb_InstallModuleTarget_get_external_target,$(1))

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
