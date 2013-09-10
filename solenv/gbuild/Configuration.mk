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

# Configuration files: a bit of an overview of the targets:
#
# Configuration
# => XcsTarget: schemas
# 	 => buildtools
#	 => Xcs source
# => XcuDataTarget: data
# 	 => buildtools
#    => XcsTarget (schema)
#	 => Xcu data source
# => XcuModuleTaret: modules
# 	 => buildtools
#	 => Xcu data source
# => XcuLangpackTarget: langpack (per lang)
# 	 => buildtools
#	 => Xcu data source
# => XcuResTarget: resources (per lang)
#    => buildtools
#    => XcuMergeTarget: merge
#       => buildtools (cfgex)
#       => Xcu data source
#       => *.po
#    => XcsTarget (schema)

# The main LibreOffice registry (cf. officecfg/Configuration_officecfg.mk):
gb_Configuration_PRIMARY_REGISTRY_NAME := registry
gb_Configuration_PRIMARY_REGISTRY_SCHEMA_ROOT = \
    $(SRCDIR)/officecfg/registry/schema

gb_Configuration__stringparam_schemaRoot = --stringparam schemaRoot \
    $(if $(PRIMARY_REGISTRY), \
        $(gb_Configuration_PRIMARY_REGISTRY_SCHEMA_ROOT), \
        $(call gb_XcsTarget_get_outdir_target,))

gb_Configuration_XSLTCOMMAND = $(call gb_ExternalExecutable_get_command,xsltproc)
gb_Configuration_XSLTCOMMAND_DEPS = $(call gb_ExternalExecutable_get_dependencies,xsltproc)

# XcsTarget class

# need to locate a schema file corresponding to some XCU file in the outdir
define gb_XcsTarget_for_XcuTarget
$(call gb_XcsTarget_get_outdir_target,$(basename $(1)).xcs)
endef

gb_Configuration_LANGS := en-US $(filter-out en-US,$(gb_WITH_LANG))

gb_XcsTarget_XSLT_SchemaVal := $(OUTDIR)/xml/processing/schema_val.xsl
gb_XcsTarget_XSLT_Sanity := $(OUTDIR)/xml/processing/sanity.xsl
gb_XcsTarget_XSLT_SchemaTrim := $(OUTDIR)/xml/processing/schema_trim.xsl
gb_XcsTarget_DTD_Schema := $(OUTDIR)/xml/registry/component-schema.dtd

define gb_XcsTarget__command
$(call gb_Output_announce,$(2),$(true),XCS,1)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(gb_Configuration_XSLTCOMMAND) --nonet \
		--noout \
		--stringparam componentName $(subst /,.,$(basename $(XCSFILE))) \
		--stringparam root $(subst $(XCSFILE),,$(3)) \
		$(gb_Configuration__stringparam_schemaRoot) \
		$(gb_XcsTarget_XSLT_SchemaVal) \
		$(3) && \
	$(gb_Configuration_XSLTCOMMAND) --nonet \
		--noout \
		$(gb_XcsTarget_XSLT_Sanity) \
		$(3) && \
	$(gb_Configuration_XSLTCOMMAND) --nonet \
		-o $(1) \
		$(gb_XcsTarget_XSLT_SchemaTrim) \
		$(3))
endef

$(call gb_XcsTarget_get_target,%) : \
	    $(gb_XcsTarget_XSLT_SchemaVal) $(gb_XcsTarget_XSLT_Sanity) \
		$(gb_XcsTarget_XSLT_SchemaTrim) $(gb_XcsTarget_DTD_Schema) \
		| $(gb_Configuration_XSLTCOMMAND_DEPS)
	$(call gb_XcsTarget__command,$@,$*,$(filter %.xcs,$^))

$(call gb_XcsTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),XCS,1)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_XcsTarget_get_target,$*) \
			  $(call gb_XcsTarget_get_outdir_target,$(XCSFILE)))


# XcuDataTarget class

gb_XcuTarget_XSLT_AllLang := $(OUTDIR)/xml/processing/alllang.xsl
gb_XcuDataTarget_XSLT_DataVal := $(OUTDIR)/xml/processing/data_val.xsl
gb_XcuDataTarget_DTD_ComponentUpdate := $(OUTDIR)/xml/registry/component-update.dtd

define gb_XcuDataTarget__command
$(call gb_Output_announce,$(2),$(true),XCU,2)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(gb_Configuration_XSLTCOMMAND) --nonet \
		--noout \
		--stringparam xcs $(call gb_XcsTarget_for_XcuTarget,$(XCUFILE)) \
		$(gb_Configuration__stringparam_schemaRoot) \
		--path $(gb_Configuration_registry) \
		$(gb_XcuDataTarget_XSLT_DataVal) \
		$(3) && \
	$(gb_Configuration_XSLTCOMMAND) --nonet \
		-o $(1) \
		--stringparam xcs $(call gb_XcsTarget_for_XcuTarget,$(XCUFILE)) \
		$(gb_Configuration__stringparam_schemaRoot) \
		--path $(gb_Configuration_registry) \
		$(gb_XcuTarget_XSLT_AllLang) \
		$(3))
endef

$(call gb_XcuDataTarget_get_target,%) : $(gb_XcuDataTarget_XSLT_DataVal) \
		$(gb_XcuTarget_XSLT_AllLang) $(gb_XcuDataTarget_DTD_ComponentUpdate) \
		| $(gb_Configuration_XSLTCOMMAND_DEPS)
	$(call gb_XcuDataTarget__command,$@,$*,$(filter %.xcu,$^))

$(call gb_XcuDataTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),XCU,2)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_XcuDataTarget_get_target,$*) \
			  $(call gb_XcuDataTarget_get_outdir_target,$(XCUFILE)))


# XcuModuleTarget class

define gb_XcuDataSource_for_XcuModuleTarget
$(SRCDIR)/$(basename $(subst -,.,$(basename $(1)))).xcu
endef

define gb_XcsTarget_for_XcuModuleTarget
$(call gb_XcsTarget_get_outdir_target,$(basename $(subst -,.,$(basename $(1)))).xcs)
endef

define gb_XcuModuleTarget__command
$(call gb_Output_announce,$(2),$(true),XCM,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(gb_Configuration_XSLTCOMMAND) --nonet \
		-o $(1) \
		--stringparam xcs $(4) \
		$(gb_Configuration__stringparam_schemaRoot) \
		--stringparam module $(notdir $(subst -,/,$(basename $(notdir $(2))))) \
		$(gb_XcuTarget_XSLT_AllLang) \
		$(3))
endef

$(call gb_XcuModuleTarget_get_target,%) : $(gb_XcuTarget_XSLT_AllLang) \
		| $(gb_Configuration_XSLTCOMMAND_DEPS)
	$(call gb_XcuModuleTarget__command,$@,$*,$(filter %.xcu,$^),$(filter %.xcs,$^))

$(call gb_XcuModuleTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),XCM,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_XcuModuleTarget_get_target,$*) \
			  $(call gb_XcuModuleTarget_get_outdir_target,$(XCUFILE)))


# XcuLangpackTarget class

gb_XcuLangpackTarget__get_name_with_lang = $(basename $(1))-$(2)$(suffix $(1))

gb_XcuLangpackTarget__get_outdir_target_with_lang = \
 $(call gb_XcuLangpackTarget_get_outdir_target,$(call gb_XcuLangpackTarget__get_name_with_lang,$(1),$(2)))
gb_XcuLangpackTarget__get_target_with_lang = \
 $(call gb_XcuLangpackTarget_get_target,$(call gb_XcuLangpackTarget__get_name_with_lang,$(1),$(2)))

gb_XcuLangpackTarget_SED_delcomment := $(OUTDIR)/xml/processing/delcomment.sed

define gb_XcuLangpackTarget__command
$(call gb_Output_announce,$(2),$(true),XCL,1)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	sed -e "s/__LANGUAGE__/$(LANG)/" -f $(gb_XcuLangpackTarget_SED_delcomment)\
	    $(3) > $(1))
endef

$(call gb_XcuLangpackTarget_get_target,%) : \
		$(gb_XcuLangpackTarget_SED_delcomment)
	$(call gb_XcuLangpackTarget__command,$@,$*,$(filter %.tmpl,$^))

$(call gb_XcuLangpackTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),XCL,1)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(foreach lang,$(gb_Configuration_LANGS),\
			  $(call gb_XcuLangpackTarget__get_target_with_lang,$*,$(lang)) \
			  $(call gb_XcuLangpackTarget__get_outdir_target_with_lang,$(XCUFILE),$(lang))))


# XcuMergeTarget class

gb_XcuMergeTarget_CFGEXDEPS := $(call gb_Executable_get_runtime_dependencies,cfgex)
gb_XcuMergeTarget_CFGEXCOMMAND := $(call gb_Executable_get_command,cfgex)

define gb_XcuMergeTarget__command
$(call gb_Output_announce,$(2),$(true),XCX,1)
MERGEINPUT=`$(gb_MKTEMP)` && \
echo $(POFILES) > $${MERGEINPUT} && \
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(gb_XcuMergeTarget_CFGEXCOMMAND) \
		-i $(3) \
		-o $(1) \
		-m $${MERGEINPUT} \
		-l all) && \
rm -rf $${MERGEINPUT}

endef

$(call gb_XcuMergeTarget_get_target,%) : $(gb_XcuMergeTarget_CFGEXDEPS)
	$(if $(filter $(words $(POFILES)),$(words $(wildcard $(POFILES)))),\
		$(call gb_XcuMergeTarget__command,$@,$*,$(filter %.xcu,$^)),\
		mkdir -p $(dir $@) && cp $(filter %.xcu,$^) $@)

$(call gb_XcuMergeTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),XCX,1)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_XcuMergeTarget_get_target,$*))

# $(call gb_XcuMergeTarget_XcuMergeTarget,target,configuration,prefix,xcufile)
define gb_XcuMergeTarget_XcuMergeTarget
$(call gb_XcuMergeTarget_get_target,$(1)) : \
	$(SRCDIR)/$(3)/$(4) \
	$(wildcard $(foreach lang,$(gb_TRANS_LANGS),$(gb_POLOCATION)/$(lang)/$(patsubst %/,%,$(dir $(1))).po))
$(call gb_XcuMergeTarget_get_target,$(1)) : \
	POFILES := $(foreach lang,$(gb_TRANS_LANGS),$(gb_POLOCATION)/$(lang)/$(patsubst %/,%,$(dir $(1))).po)
$(foreach lang,$(gb_TRANS_LANGS),$(gb_POLOCATION)/$(lang)/$(patsubst %/,%,$(dir $(1))).po) :
endef


# XcuResTarget class

# locale is extracted from the stem (parameter $(2))
define gb_XcuResTarget__command
$(call gb_Output_announce,$(2),$(true),XCR,2)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(gb_Configuration_XSLTCOMMAND) --nonet \
		-o $(1) \
		--stringparam xcs $(call gb_XcsTarget_for_XcuTarget,$(XCUFILE)) \
		$(gb_Configuration__stringparam_schemaRoot) \
		--stringparam locale $(word 2,$(subst /, ,$(2))) \
		--path $(gb_Configuration_registry) \
		$(gb_XcuTarget_XSLT_AllLang) \
		$(3))
endef

$(call gb_XcuResTarget_get_target,%) : $(gb_XcuTarget_XSLT_AllLang) \
		| $(gb_Configuration_XSLTCOMMAND_DEPS)
	$(call gb_XcuResTarget__command,$@,$*,$(filter %.xcu,$^))

$(call gb_XcuResTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),XCR,2)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_XcuResTarget_get_target,$*))

# $(call gb_XcuResTarget_XcuResTarget,target,configuration,prefix,xcufile,lang)
# this depends either on the source or on the merge target (if WITH_LANG)
define gb_XcuResTarget_XcuResTarget
ifeq ($(strip $(gb_WITH_LANG)),)
$(call gb_XcuResTarget_get_target,$(1)) : $(SRCDIR)/$(3)/$(4)
else
$(call gb_XcuResTarget_get_target,$(1)) : \
	$(call gb_XcuMergeTarget_get_target,$(3)/$(4))
endif
$(call gb_XcuResTarget_get_target,$(1)) : \
	$(call gb_XcsTarget_for_XcuTarget,$(4))
$(call gb_XcuResTarget_get_target,$(1)) : PRIMARY_REGISTRY := $(filter $(2),$(gb_Configuration_PRIMARY_REGISTRY_NAME))
$(call gb_XcuResTarget_get_target,$(1)) : XCUFILE := $(4)
endef


# Configuration class

$(call gb_Configuration_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),CFG,4)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_Configuration_get_target,$*) $(call gb_Configuration_get_preparation_target,$*))

$(call gb_Configuration_get_target,%) :
	$(call gb_Output_announce,$*,$(true),CFG,4)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && touch $@)

$(call gb_Configuration_get_preparation_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && touch $@)

# $(call gb_Configuration_Configuration,configuration,repo,nodeliver)
# cannot use target local variable for REPO because it's needed in prereq
# last parameter may be used to turn off delivering of files
define gb_Configuration_Configuration
$(eval gb_Configuration_NODELIVER_$(1) := $(2))

$$(eval $$(call gb_Module_register_target,$(call gb_Configuration_get_target,$(1)),$(call gb_Configuration_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),Configuration)

endef

# $(call gb_Configuration_add_schema,configuration,prefix,xcsfile)
# FIXME this is always delivered because commands depend on it...
# hopefully extensions do not need to add schemas with same name as officecfg
define gb_Configuration_add_schema
$(call gb_Configuration_get_clean_target,$(1)) : \
	$(call gb_XcsTarget_get_clean_target,$(2)/$(3))
$(call gb_XcsTarget_get_target,$(2)/$(3)) : \
	$(SRCDIR)/$(2)/$(3) \
	$(call gb_Configuration_get_preparation_target,$(1))
$(call gb_XcsTarget_get_target,$(2)/$(3)) : PRIMARY_REGISTRY := $(filter $(1),$(gb_Configuration_PRIMARY_REGISTRY_NAME))
$(call gb_XcsTarget_get_target,$(2)/$(3)) : XCSFILE := $(3)
$(call gb_XcsTarget_get_clean_target,$(2)/$(3)) : XCSFILE := $(3)
$(call gb_Configuration_get_target,$(1)) : \
	$(call gb_XcsTarget_get_outdir_target,$(3))
$(call gb_XcsTarget_get_outdir_target,$(3)) : \
	$(call gb_XcsTarget_get_target,$(2)/$(3)) \
	| $(dir $(call gb_XcsTarget_get_outdir_target,$(3))).dir
$(call gb_Deliver_add_deliverable,$(call gb_XcsTarget_get_outdir_target,$(3)),\
	$(call gb_XcsTarget_get_target,$(2)/$(3)),$(2)/$(3))

endef

#$(call gb_Configuration_add_schemas,configuration,prefix,xcsfiles)
define gb_Configuration_add_schemas
$(foreach xcs,$(3),$(call gb_Configuration_add_schema,$(1),$(2),$(xcs)))

endef

# $(call gb_Configuration_add_data,configuration,prefix,xcufile)
define gb_Configuration_add_data
$(call gb_Configuration_get_clean_target,$(1)) : \
	$(call gb_XcuDataTarget_get_clean_target,$(2)/$(3))
$(call gb_XcuDataTarget_get_target,$(2)/$(3)) : \
	$(SRCDIR)/$(2)/$(3) \
	$(call gb_Configuration_get_preparation_target,$(1)) \
	$(call gb_XcsTarget_for_XcuTarget,$(3))
$(call gb_XcuDataTarget_get_target,$(2)/$(3)) : PRIMARY_REGISTRY := $(filter $(1),$(gb_Configuration_PRIMARY_REGISTRY_NAME))
$(call gb_XcuDataTarget_get_target,$(2)/$(3)) : XCUFILE := $(3)
$(call gb_XcuDataTarget_get_clean_target,$(2)/$(3)) : XCUFILE := $(3)
ifeq ($(strip $(gb_Configuration_NODELIVER_$(1))),)
$(call gb_Configuration_get_target,$(1)) : \
	$(call gb_XcuDataTarget_get_outdir_target,$(3))
$(call gb_XcuDataTarget_get_outdir_target,$(3)) : \
	$(call gb_XcuDataTarget_get_target,$(2)/$(3)) \
	| $(dir $(call gb_XcuDataTarget_get_outdir_target,$(3))).dir
$(call gb_Deliver_add_deliverable,\
	$(call gb_XcuDataTarget_get_outdir_target,$(3)),\
	$(call gb_XcuDataTarget_get_target,$(2)/$(3)),\
	$(2)/$(3))
else
$(call gb_Configuration_get_target,$(1)) : \
	$(call gb_XcuDataTarget_get_target,$(2)/$(3))
endif

endef

#$(call gb_Configuration_add_datas,configuration,prefix,xcufiles)
define gb_Configuration_add_datas
$(foreach xcu,$(3),$(call gb_Configuration_add_data,$(1),$(2),$(xcu)))

endef

# $(call gb_Configuration_add_spool_module,configuration,prefix,xcufile)
define gb_Configuration_add_spool_module
$(call gb_Configuration_get_clean_target,$(1)) : \
	$(call gb_XcuModuleTarget_get_clean_target,$(2)/$(3))
$(call gb_XcuModuleTarget_get_target,$(2)/$(3)) : \
	$(call gb_XcuDataSource_for_XcuModuleTarget,$(2)/$(3)) \
	$(call gb_Configuration_get_preparation_target,$(1)) \
	$(call gb_XcsTarget_for_XcuModuleTarget,$(3))
$(call gb_XcuModuleTarget_get_target,$(2)/$(3)) : PRIMARY_REGISTRY := $(filter $(1),$(gb_Configuration_PRIMARY_REGISTRY_NAME))
$(call gb_XcuModuleTarget_get_clean_target,$(2)/$(3)) : XCUFILE := $(3)
ifeq ($(strip $(gb_Configuration_NODELIVER_$(1))),)
$(call gb_Configuration_get_target,$(1)) : \
	$(call gb_XcuModuleTarget_get_outdir_target,$(3))
$(call gb_XcuModuleTarget_get_outdir_target,$(3)) : \
	$(call gb_XcuModuleTarget_get_target,$(2)/$(3)) \
	| $(dir $(call gb_XcuModuleTarget_get_outdir_target,$(3))).dir
$(call gb_Deliver_add_deliverable,\
	$(call gb_XcuModuleTarget_get_outdir_target,$(3)),\
	$(call gb_XcuModuleTarget_get_target,$(2)/$(3)),\
	$(2)/$(3))
else
$(call gb_Configuration_get_target,$(1)) : \
	$(call gb_XcuModuleTarget_get_target,$(2)/$(3))
endif

endef

# $(call gb_Configuration_add_spool_modules,configuration,prefix,xcufiles)
define gb_Configuration_add_spool_modules
$(foreach xcu,$(3),$(call gb_Configuration_add_spool_module,$(1),$(2),$(xcu)))

endef

define gb_Configuration__add_langpack
$(if $(gb_Configuration_NODELIVER_$(1)),\
	$(error TODO not needed yet: cannot add langpack if nodeliver))
$(call gb_Configuration_get_clean_target,$(1)) : \
	$(call gb_XcuLangpackTarget_get_clean_target,$(2)/$(3))
$(call gb_Configuration_get_target,$(1)) : \
	$(call gb_XcuLangpackTarget__get_outdir_target_with_lang,$(3),$(4))
$(call gb_XcuLangpackTarget__get_target_with_lang,$(2)/$(3),$(4)) : \
	$(SRCDIR)/$(2)/$(3).tmpl
$(call gb_XcuLangpackTarget_get_clean_target,$(2)/$(3)) : XCUFILE := $(3)
$(call gb_XcuLangpackTarget__get_target_with_lang,$(2)/$(3),$(4)) : LANG := $(4)
$(call gb_XcuLangpackTarget__get_outdir_target_with_lang,$(3),$(4)) : \
	$(call gb_XcuLangpackTarget__get_target_with_lang,$(2)/$(3),$(4)) \
	| $(dir $(call gb_XcuLangpackTarget__get_outdir_target_with_lang,$(3),$(4))).dir
$(call gb_Deliver_add_deliverable,\
	$(call gb_XcuLangpackTarget__get_outdir_target_with_lang,$(3),$(4)),\
	$(call gb_XcuLangpackTarget__get_target_with_lang,$(2)/$(3),$(4)),\
	$(call gb_XcuLangpackTarget__get_name_with_lang,$(2)/$(3),$(4)))

endef

# $(call gb_Configuration_add_spool_langpack,configuration,prefix,xcufile)
define gb_Configuration_add_spool_langpack
$(foreach lang,$(gb_Configuration_LANGS),$(eval \
	$(call gb_Configuration__add_langpack,$(1),$(2),$(strip $(3)),$(lang))))

endef

# $(call gb_Configuration_add_localized_data,configuration,prefix,xcufile)
define gb_Configuration_add_localized_data
$(eval $(call gb_Configuration_add_data,$(1),$(2),$(3)))
ifneq ($(strip $(gb_WITH_LANG)),)
$(eval $(call gb_XcuMergeTarget_XcuMergeTarget,$(2)/$(3),$(1),$(2),$(3)))
$(eval $(call gb_Configuration_get_clean_target,$(1)) : \
	$(call gb_XcuMergeTarget_get_clean_target,$(2)/$(3)))
endif
$(foreach lang,$(gb_Configuration_LANGS),$(eval \
 $(call gb_XcuResTarget_XcuResTarget,$(1)/$(lang)/$(3),$(1),$(2),$(3),$(lang))))
$(foreach lang,$(gb_Configuration_LANGS),$(eval \
  $(call gb_Configuration_get_target,$(1)) : \
	$(call gb_XcuResTarget_get_target,$(1)/$(lang)/$(3))))
$(foreach lang,$(gb_Configuration_LANGS),$(eval \
  $(call gb_Configuration_get_clean_target,$(1)) : \
	$(call gb_XcuResTarget_get_clean_target,$(1)/$(lang)/$(3))))

endef

# $(call gb_Configuration_add_localized_datas,configuration,prefix,xcufile)
define gb_Configuration_add_localized_datas
$(foreach xcu,$(3),$(call gb_Configuration_add_localized_data,$(1),$(2),$(xcu)))

endef

# Set extra registry this configuration can use schemas from.
#
# Example:
# # foo needs schemas from the main configuration
# $(eval $(call gb_Configuration_use_configuration,foo,registry))
define gb_Configuration_use_configuration
$(call gb_Configuration_get_preparation_target,$(1)) : $(call gb_Configuration_get_target,$(2))

endef

# apparently extensions package the XcuMergeTarget directly...
# trivial convenience function to get the right file:
ifeq ($(gb_WITH_LANG),)
gb_XcuFile_for_extension = $(SRCDIR)/$(1)
else
gb_XcuFile_for_extension = $(call gb_XcuMergeTarget_get_target,$(1))
endif

# vim: set noet sw=4 ts=4:
