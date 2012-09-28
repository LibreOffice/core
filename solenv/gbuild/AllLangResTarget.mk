# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************


# Overview of dependencies and tasks of SrsTarget
#
# target                task                depends on
# SrsTarget             joining the parts   SrsPartTarget
#                                           SrsTemplateTarget
# SrsPartTarget         preprocessing       SrsPartMergeTarget (with l10n)
#                                           source file (without l10n)
# SrsTemplateTarget     handling templates  SrsTemplatePartTarget
# SrsTemplatePartTarget handling template   SrsPartMergeTarget
# SrsPartMergeTarget    merging/transex     source file (only needed with l10n)

# Overview of dependencies and tasks of AllLangResTarget
#
# target                task                depends on
# AllLangResTarget      nothing             ResTarget for all active langs
# ResTarget             running rsc         SrsTarget, resources


# SrsPartMergeTarget class

gb_SrsPartMergeTarget_TRANSEXTARGET := $(call gb_Executable_get_target_for_build,transex3)
gb_SrsPartMergeTarget_TRANSEXCOMMAND := \
	$(gb_Helper_set_ld_path) $(gb_SrsPartMergeTarget_TRANSEXTARGET)

define gb_SrsPartMergeTarget__command
$(call gb_Output_announce,$(3),$(true),srs,1)
MERGEINPUT=`$(gb_MKTEMP)` && \
echo $(POFILES) > $${MERGEINPUT} && \
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(gb_SrsPartMergeTarget_TRANSEXCOMMAND) \
		-p $(firstword $(subst /, ,$(2))) \
		-i $(3) \
		-o $(1) \
		-m $${MERGEINPUT} \
		-l all) && \
rm -rf $${MERGEINPUT}

endef

$(call gb_SrsPartMergeTarget_get_target,%) : $(SRCDIR)/% $(gb_Helper_MISCDUMMY)  $(gb_SrsPartMergeTarget_TRANSEXTARGET)
	$(if $(filter $(words $(POFILES)),$(words $(wildcard $(POFILES)))),\
		$(call gb_SrsPartMergeTarget__command,$@,$*,$<),\
		mkdir -p $(dir $@) && cp $< $@)

# SrsPartTarget class

# defined by platform
#  gb_SrsPartTarget__command_dep

gb_ResTarget_RSCTARGET := $(OUTDIR_FOR_BUILD)/bin/rsc$(gb_Executable_EXT_for_build)
gb_ResTarget_RSCCOMMAND := $(gb_Helper_set_ld_path) SOLARBINDIR=$(OUTDIR_FOR_BUILD)/bin $(gb_ResTarget_RSCTARGET)

define gb_SrsPartTarget__command
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	RESPONSEFILE=`$(gb_MKTEMP)` && \
	echo "-s \
		$(INCLUDE) \
		-I$(dir $(3)) \
		$(DEFS) \
		-fp=$(1) \
		$(if $(MERGEDFILE),$(MERGEDFILE),$<)" > $${RESPONSEFILE} && \
	$(gb_ResTarget_RSCCOMMAND) -presponse @$${RESPONSEFILE} && \
	rm -rf $${RESPONSEFILE})

endef

$(call gb_SrsPartTarget_get_target,%) : $(SRCDIR)/% $(gb_Helper_MISCDUMMY) \
		$(gb_ResTarget_RSCTARGET)
	$(call gb_SrsPartTarget__command_dep,$*,$<)
	$(call gb_SrsPartTarget__command,$@,$*,$<)

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_SrsPartTarget_get_dep_target,%) : $(SRCDIR)/% $(gb_Helper_MISCDUMMY)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		echo '$(call gb_SrsPartTarget_get_target,$*) : $(gb_Helper_PHONY)' > $@)
endif


define gb_SrsPartTarget_SrsPartTarget
ifeq ($(strip $(gb_WITH_LANG)),)
$(call gb_SrsPartTarget_get_target,$(1)) : MERGEDFILE := 
else
$(call gb_SrsPartTarget_get_target,$(1)) : MERGEDFILE := $(call gb_SrsPartMergeTarget_get_target,$(1))
$(call gb_SrsPartTarget_get_target,$(1)) : $(call gb_SrsPartMergeTarget_get_target,$(1))
$(call gb_SrsPartMergeTarget_get_target,$(1)) : \
	POFILES := $(foreach lang,$(filter-out en-US,$(gb_WITH_LANG)),$(gb_POLOCATION)/$(lang)/$(patsubst %/,%,$(dir $(1))).po)
endif

endef

# SrsTemplatePartTarget class

define gb_SrsTemplatePartTarget_SrsTemplatePartTarget
$(call gb_SrsTemplatePartTarget_get_target,$(1)) : $(call gb_SrsPartMergeTarget_get_target,$(1))
	$$(call gb_Helper_abbreviate_dirs,\
	    mkdir -p $$(dir $$@) && \
	    cp $$< $$@)
ifneq ($(strip $(WITH_LANG)),)
$(call gb_SrsPartMergeTarget_get_target,$(1)) : \
	POFILES := $(foreach lang,$(filter-out en-US,$(gb_WITH_LANG)),$(gb_POLOCATION)/$(lang)/$(patsubst %/,%,$(dir $(1))).po)
$(call gb_SrsPartMergeTarget_get_target,$(1)) : \
	$(wildcard $(foreach lang,$(filter-out en-US,$(gb_WITH_LANG)),$(gb_POLOCATION)/$(lang)/$(patsubst %/,%,$(dir $(1))).po))
endif

endef

# SrsTemplateTarget class
#
# This class handles src templates.
#
# Templates are src files that contain only macros that are then used
# from other src files, but these macros contain translatable strings.
# Because the processing of src files is done in two phases: 1/
# localization and 2/ merging, templates must be translated
# independently from the src files that include them. Special care must
# be taken to ensure that the right file (i.e., the localized one) is
# included; in order to do that, the templates in the source tree are
# called foo_tmpl.src, but the localization phase produces foo.src, and
# these names (i.e., without the _tmpl suffix) must be placed in
# #include statements in other src files.

define gb_SrsTemplateTarget__command
	$(call gb_Helper_abbreviate_dirs,\
	    mkdir -p $(dir $(1)) && \
	    touch $(1))
endef

# This machinery ensures that templates are regenerated when switching
# from localized to unlocalized configuration and back.
#
# How it works:
# We let _get_target depend on _get_update_target, so after they are
# done, _get_target is newer than _get_update_target. If
# WITH_LANG changes (from nonempty to empty or the other way around),
# _get_target and _get_update_target are switched, therefore _get_target
# is now _older_ than _get_update_target and must be regenerated :-)
gb_SrsTemplateTarget__get_merged_target = $(call gb_SrsTemplateTarget_get_target,$(1))_merged_last
gb_SrsTemplateTarget__get_unmerged_target = $(call gb_SrsTemplateTarget_get_target,$(1))_unmerged_last

ifeq ($(strip $(WITH_LANG)),)
gb_SrsTemplateTarget__get_target = $(call gb_SrsTemplateTarget__get_unmerged_target,$(1))
gb_SrsTemplateTarget__get_update_target = $(call gb_SrsTemplateTarget__get_merged_target,$(1))
else
gb_SrsTemplateTarget__get_target = $(call gb_SrsTemplateTarget__get_merged_target,$(1))
gb_SrsTemplateTarget__get_update_target = $(call gb_SrsTemplateTarget__get_unmerged_target,$(1))
endif

define gb_SrsTemplateTarget_SrsTemplateTarget
$(call gb_SrsTemplateTarget_get_target,$(1)) : PARTS :=
$(call gb_SrsTemplateTarget_get_clean_target,$(1)) : PARTS :=
$(call gb_SrsTemplateTarget_get_target,$(1)) : $(call gb_SrsTemplateTarget__get_target,$(1))
$(call gb_SrsTemplateTarget__get_target,$(1)) : $(call gb_SrsTemplateTarget__get_update_target,$(1))

endef

.PHONY : $(call gb_SrsTemplateTarget_get_target,%_last)
$(call gb_SrsTemplateTarget_get_target,%_last) :
	$(call gb_SrsTemplateTarget__command,$@)

.PHONY : $(call gb_SrsTemplateTarget_get_target,%)
$(call gb_SrsTemplateTarget_get_target,%) :
	$(if $(strip $(PARTS)),$(call gb_Output_announce,$*,$(true),SRT,4))
	$(call gb_SrsTemplateTarget__command,$@)

.PHONY : $(call gb_SrsTemplateTarget_get_clean_target,%)
$(call gb_SrsTemplateTarget_get_clean_target,%) :
	$(if $(strip $(PARTS)),$(call gb_Output_announce,$*,$(false),SRT,4))
	-$(call gb_Helper_abbreviate_dirs,\
	    rm -f $(call gb_SrsTemplateTarget_get_target,$*) \
		    $(call gb_SrsTemplateTarget__get_merged_target,$*) \
		    $(call gb_SrsTemplateTarget__get_unmerged_target,$*) \
		    $(foreach part,$(PARTS),$(call gb_SrsTemplatePartTarget_get_target,$(part))) \
		    $(foreach part,$(PARTS),$(call gb_SrsPartMergeTarget_get_target,$(part))))

define gb_SrsTemplateTarget_add_file
$(call gb_SrsTemplatePartTarget_SrsTemplatePartTarget,$(2))
$(call gb_SrsTemplateTarget_get_target,$(1)) : PARTS += $(2)
$(call gb_SrsTemplateTarget_get_clean_target,$(1)) : PARTS += $(2)
$(call gb_SrsTemplateTarget__get_target,$(1)) : $(call gb_SrsTemplatePartTarget_get_target,$(2))
$(call gb_SrsTemplatePartTarget_get_target,$(2)) : $(call gb_SrsTemplateTarget__get_update_target,$(1))
$(call gb_SrsPartMergeTarget_get_target,$(2)) : $(call gb_SrsTemplateTarget__get_update_target,$(1))

endef

# SrsTarget class

gb_SrsTarget_DEFAULTDEFS := $(gb_GLOBALDEFS)

.PHONY : $(call gb_SrsTarget_get_clean_target,%)
$(call gb_SrsTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),SRS,1)
	-$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_SrsTarget_get_target,$*) \
			$(call gb_SrsTarget_get_dep_target,$*) \
			$(foreach part,$(PARTS),$(call gb_SrsPartTarget_get_target,$(part))) \
			$(foreach part,$(PARTS),$(call gb_SrsPartTarget_get_dep_target,$(part))) \
			$(foreach part,$(PARTS),$(call gb_SrsPartMergeTarget_get_target,$(part))))

ifeq ($(gb_FULLDEPS),$(true))
define gb_SrsTarget__command_dep
$(call gb_Output_announce,SRS:$(2),$(true),DEP,1)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	cat $(3) > $(1))
endef
endif

$(call gb_SrsTarget_get_external_headers_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
	    mkdir -p $(dir $@) && touch $@)

$(call gb_SrsTarget_get_target,%) :
	$(call gb_Output_announce,$*,$(true),SRS,1)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		cat $^ > $@)

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_SrsTarget_get_dep_target,%) :
	$(call gb_SrsTarget__command_dep,$@,$*,$^)
endif

define gb_SrsTarget_SrsTarget
$(call gb_SrsTemplateTarget_SrsTemplateTarget,$(1))
$(call gb_SrsTarget_get_target,$(1)) : DEFS := $(gb_SrsTarget_DEFAULTDEFS)
$(call gb_SrsTarget_get_target,$(1)) : INCLUDE := $(SOLARINC)
$(call gb_SrsTarget_get_clean_target,$(1)) : PARTS :=
$(call gb_SrsTarget_get_target,$(1)) : PARTS :=
$(call gb_SrsTarget_get_target,$(1)) : $(call gb_SrsTemplateTarget_get_target,$(1))
$(call gb_SrsTarget_get_clean_target,$(1)) : $(call gb_SrsTemplateTarget_get_clean_target,$(1))
ifeq ($(gb_FULLDEPS),$(true))
-include $(call gb_SrsTarget_get_dep_target,$(1))
endif

endef

define gb_SrsTarget_set_defs
$(call gb_SrsTarget_get_target,$(1)) : DEFS := $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_SrsTarget_get_dep_target,$(1)) : DEFS := $(2)
endif

endef

define gb_SrsTarget_set_include
$(call gb_SrsTarget_get_target,$(1)) : INCLUDE := $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_SrsTarget_get_dep_target,$(1)) : INCLUDE := $(2)
endif

endef

define gb_SrsTarget_add_file
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_SrsTarget_get_dep_target,$(1)) : $(call gb_SrsPartTarget_get_dep_target,$(2))
endif
$(call gb_SrsPartTarget_SrsPartTarget,$(2))
$(call gb_SrsTarget_get_target,$(1)) : $(call gb_SrsPartTarget_get_target,$(2))
$(call gb_SrsPartTarget_get_target,$(2)) :| $(call gb_SrsTarget_get_external_headers_target,$(1))
$(call gb_SrsPartTarget_get_target,$(2)) :| $(call gb_SrsTemplateTarget_get_target,$(1))
$(call gb_SrsTarget_get_clean_target,$(1)) : PARTS += $(2)
$(call gb_SrsTarget_get_target,$(1)) : PARTS += $(2)

endef

define gb_SrsTarget_add_files
$(foreach file,$(2),$(call gb_SrsTarget_add_file,$(1),$(file)))

endef

define gb_SrsTarget_add_template
$(call gb_SrsTemplateTarget_add_file,$(1),$(2))

endef

define gb_SrsTarget_add_templates
$(foreach template,$(2),$(call gb_SrsTarget_add_template,$(1),$(template)))

endef

define gb_SrsTarget_use_package
$(call gb_SrsTarget_get_external_headers_target,$(1)) : $(call gb_Package_get_target,$(2))

endef

define gb_SrsTarget_use_packages
$(foreach package,$(2),$(call gb_SrsTarget_use_package,$(1),$(package)))

endef


# ResTarget

gb_ResTarget_DEFIMAGESLOCATION := $(SRCDIR)/icon-themes/galaxy/

$(call gb_ResTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),RES,2)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_ResTarget_get_target,$*) $(call gb_ResTarget_get_imagelist_target,$*) $(call gb_ResTarget_get_outdir_target,$*))

$(call gb_ResTarget_get_target,%) : $(gb_Helper_MISCDUMMY) \
		$(gb_ResTarget_RSCTARGET)
	$(call gb_Output_announce,$*,$(true),RES,2)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) $(OUTDIR)/bin \
			$(dir $(call gb_ResTarget_get_imagelist_target,$*)) && \
		RESPONSEFILE=`$(gb_MKTEMP)` && \
		echo "-r -p \
			-lg$(LANGUAGE) \
			-fs=$@ \
			-lip=$(gb_ResTarget_DEFIMAGESLOCATION)$(RESLOCATION)/$(LIBRARY) \
			-lip=$(gb_ResTarget_DEFIMAGESLOCATION)$(RESLOCATION)/imglst/$(LANGUAGE) \
			-lip=$(gb_ResTarget_DEFIMAGESLOCATION)$(RESLOCATION)/imglst \
			-lip=$(gb_ResTarget_DEFIMAGESLOCATION)$(RESLOCATION)/res/$(LANGUAGE) \
			-lip=$(gb_ResTarget_DEFIMAGESLOCATION)$(RESLOCATION)/res \
			-lip=$(gb_ResTarget_DEFIMAGESLOCATION)$(RESLOCATION) \
			-lip=$(gb_ResTarget_DEFIMAGESLOCATION)res/$(LANGUAGE) \
			-lip=$(gb_ResTarget_DEFIMAGESLOCATION)res \
			-subMODULE=$(gb_ResTarget_DEFIMAGESLOCATION) \
			-subGLOBALRES=$(gb_ResTarget_DEFIMAGESLOCATION)res \
			-oil=$(dir $(call gb_ResTarget_get_imagelist_target,$*)) \
			$(filter-out $(gb_Helper_MISCDUMMY) $(gb_ResTarget_RSCTARGET),$^)" \
			> $${RESPONSEFILE} && \
		$(gb_ResTarget_RSCCOMMAND) @$${RESPONSEFILE} && \
		rm -f $${RESPONSEFILE})

$(call gb_ResTarget_get_outdir_target,%) :
	$(call gb_Deliver_deliver,$<,$@)

define gb_ResTarget_ResTarget
$(call gb_ResTarget_get_target,$(1)) : LIBRARY = $(2)
$(call gb_ResTarget_get_target,$(1)) : LANGUAGE = $(3)
$(call gb_ResTarget_get_target,$(1)) : RESLOCATION = $(2)
$(call gb_AllLangResTarget_get_target,$(2)) : $(call gb_ResTarget_get_outdir_target,$(1))
$(call gb_AllLangResTarget_get_clean_target,$(2)) : $(call gb_ResTarget_get_clean_target,$(1))
$(call gb_ResTarget_get_imagelist_target,$(1)) : $(call gb_ResTarget_get_target,$(1))

$(call gb_ResTarget_get_outdir_target,$(1)) : $(call gb_ResTarget_get_target,$(1)) 
$(call gb_Deliver_add_deliverable,$(call gb_ResTarget_get_outdir_target,$(1)),$(call gb_ResTarget_get_target,$(1)),$(1))

endef

define gb_ResTarget_add_file
$(call gb_ResTarget_get_target,$(1)) : $(2)

endef

define gb_ResTarget_add_one_srs
$(call gb_ResTarget_add_file,$(1),$(call gb_SrsTarget_get_target,$(2)))
$(call gb_ResTarget_get_clean_target,$(1)) : $(call gb_SrsTarget_get_clean_target,$(2))

endef

define gb_ResTarget_add_files
$(foreach file,$(2),\
	$(call gb_ResTarget_add_file,$(1),$(file)))

endef

define gb_ResTarget_add_srs
$(foreach srs,$(2),\
	$(call gb_ResTarget_add_one_srs,$(1),$(srs)))

endef

define gb_ResTarget_set_reslocation
$(call gb_ResTarget_get_target,$(1)) : RESLOCATION = $(2)

endef


# AllLangResTarget

gb_AllLangResTarget_LANGS := en-US $(filter-out en-US,$(gb_WITH_LANG))

define gb_AllLangResTarget_set_langs
gb_AllLangResTarget_LANGS := $(1)
endef

$(call gb_AllLangResTarget_get_clean_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_AllLangResTarget_get_target,$*))

$(call gb_AllLangResTarget_get_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && touch $@)

define gb_AllLangResTarget_AllLangResTarget
$(foreach lang,$(gb_AllLangResTarget_LANGS),\
	$(call gb_ResTarget_ResTarget,$(1)$(lang),$(1),$(lang)))
$$(eval $$(call gb_Module_register_target,$(call gb_AllLangResTarget_get_target,$(1)),$(call gb_AllLangResTarget_get_clean_target,$(1))))

endef

define gb_AllLangResTarget_add_one_srs
$(foreach lang,$(gb_AllLangResTarget_LANGS),\
	$(call gb_ResTarget_add_one_srs,$(1)$(lang),$(2)))

endef

define gb_AllLangResTarget_add_srs
$(foreach srs,$(2),\
	$(call gb_AllLangResTarget_add_one_srs,$(1),$(srs)))

endef

define gb_AllLangResTarget_set_reslocation
$(foreach lang,$(gb_AllLangResTarget_LANGS),\
	$(call gb_ResTarget_set_reslocation,$(1)$(lang),$(2)))

endef

# vim: set noet sw=4:
