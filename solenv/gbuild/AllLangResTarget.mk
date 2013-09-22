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

gb_SrsPartMergeTarget_TRANSEXDEPS := $(call gb_Executable_get_runtime_dependencies,transex3)
gb_SrsPartMergeTarget_TRANSEXCOMMAND = $(call gb_Executable_get_command,transex3)

define gb_SrsPartMergeTarget__command
MERGEINPUT=`$(gb_MKTEMP)` && \
echo $(POFILES) > $${MERGEINPUT} && \
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(1)) && \
	$(gb_SrsPartMergeTarget_TRANSEXCOMMAND) \
		-i $(3) \
		-o $(1) \
		-m $${MERGEINPUT} \
		-l all \
		$(if $(filter MSC,$(COM)),$(if $(strip $(gb_WITH_LANG)),-b))) && \
rm -rf $${MERGEINPUT}

endef

$(call gb_SrsPartMergeTarget_get_target,%) : $(SRCDIR)/% $(gb_Helper_MISCDUMMY)  $(gb_SrsPartMergeTarget_TRANSEXDEPS)
	$(call gb_Output_announce,$*,$(true),SRS,1)
	$(if $(filter $(words $(POFILES)),$(words $(wildcard $(POFILES)))),\
		$(call gb_SrsPartMergeTarget__command,$@,$*,$<),\
		mkdir -p $(dir $@) && cp $< $@)

define gb_SrsPartMergeTarget_SrsPartMergeTarget
$(call gb_SrsPartMergeTarget__SrsPartMergeTarget_impl,$(1),$(if $(2),$(foreach lang,$(gb_TRANS_LANGS),$(gb_POLOCATION)/$(lang)/$(patsubst %/,%,$(dir $(1))).po)))
endef

define gb_SrsPartMergeTarget__SrsPartMergeTarget_impl
$(call gb_SrsPartMergeTarget_get_target,$(1)) : POFILES := $(2)
$(call gb_SrsPartMergeTarget_get_target,$(1)) : $(2)
$(2) :
endef

# SrsPartTarget class

# defined by platform
#  gb_SrsPartTarget__command_dep

gb_ResTarget_RSCDEPS := $(call gb_Executable_get_runtime_dependencies,rsc)
gb_ResTarget_RSCCOMMAND = SOLARBINDIR=$(gb_Executable_BINDIR_FOR_BUILD) $(call gb_Executable_get_command,rsc)

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
		$(gb_ResTarget_RSCDEPS) $(gb_SrsPartTarget__command_target)
	$(call gb_SrsPartTarget__command_dep,$*,$<)
	$(call gb_SrsPartTarget__command,$@,$*,$<)

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_SrsPartTarget_get_dep_target,%) : $(SRCDIR)/% $(gb_Helper_MISCDUMMY)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@))
endif


define gb_SrsPartTarget_SrsPartTarget
ifeq ($(strip $(gb_WITH_LANG)),)
$(call gb_SrsPartTarget_get_target,$(1)) : MERGEDFILE := 
else
$(call gb_SrsPartTarget_get_target,$(1)) : MERGEDFILE := $(call gb_SrsPartMergeTarget_get_target,$(1))
$(call gb_SrsPartTarget_get_target,$(1)) : $(call gb_SrsPartMergeTarget_get_target,$(1))
$(call gb_SrsPartMergeTarget_SrsPartMergeTarget,$(1),$(2))
endif

endef

# SrsTemplatePartTarget class

define gb_SrsTemplatePartTarget_SrsTemplatePartTarget
$(call gb_SrsTemplatePartTarget_get_target,$(1)) : $(call gb_SrsPartMergeTarget_get_target,$(1))
	$$(call gb_Helper_abbreviate_dirs,\
	    mkdir -p $$(dir $$@) && \
	    cp $$< $$@)
ifneq ($(strip $(gb_WITH_LANG)),)
$(call gb_SrsPartMergeTarget_SrsPartMergeTarget,$(1),$(true))
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
# gb_WITH_LANG changes (from nonempty to empty or the other way around),
# _get_target and _get_update_target are switched, therefore _get_target
# is now _older_ than _get_update_target and must be regenerated :-)
gb_SrsTemplateTarget__get_merged_target = $(call gb_SrsTemplateTarget_get_target,$(1))_merged_last
gb_SrsTemplateTarget__get_unmerged_target = $(call gb_SrsTemplateTarget_get_target,$(1))_unmerged_last

ifeq ($(strip $(gb_WITH_LANG)),)
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
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),200,\
		$(foreach part,$(PARTS),$(call gb_SrsPartTarget_get_dep_target,$(part)))) && \
	$(call gb_Executable_get_command,concat-deps) $${RESPONSEFILE} > $(1)) && \
	rm -f $${RESPONSEFILE}
endef
endif

$(call gb_SrsTarget_get_headers_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
	    mkdir -p $(dir $@) && touch $@)

# This recipe actually also builds the dep-target as a side-effect, which
# is an optimization to reduce incremental build time.
$(call gb_SrsTarget_get_target,%) :
ifeq ($(gb_FULLDEPS),$(true))
	$(call gb_SrsTarget__command_dep,$(call gb_SrsTarget_get_dep_target,$*),$*)
endif
	$(call gb_Output_announce,$*,$(true),SRS,1)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		cat $^ > $@)

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_SrsTarget_get_dep_target,%) : \
		$(call gb_Executable_get_runtime_dependencies,concat-deps)
	$(call gb_SrsTarget__command_dep,$@,$*)
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
$(call gb_SrsTarget_get_dep_target,$(1)) : PARTS :=
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

define gb_SrsTarget__add_file
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_SrsTarget_get_dep_target,$(1)) : $(call gb_SrsPartTarget_get_dep_target,$(2))
endif
$(call gb_SrsPartTarget_SrsPartTarget,$(2),$(3))
$(call gb_SrsTarget_get_target,$(1)) : $(call gb_SrsPartTarget_get_target,$(2))
$(call gb_SrsPartTarget_get_target,$(2)) :| $(call gb_SrsTarget_get_headers_target,$(1))
$(call gb_SrsPartTarget_get_target,$(2)) :| $(call gb_SrsTemplateTarget_get_target,$(1))
$(call gb_SrsTarget_get_clean_target,$(1)) : PARTS += $(2)
$(call gb_SrsTarget_get_target,$(1)) : PARTS += $(2)
ifeq ($(gb_FULLDEPS),$(true))
$(call gb_SrsTarget_get_dep_target,$(1)) : PARTS += $(2)
endif

endef

define gb_SrsTarget_add_file
$(call gb_SrsTarget__add_file,$(1),$(2),$(true))

endef

define gb_SrsTarget_add_files
$(foreach file,$(2),$(call gb_SrsTarget_add_file,$(1),$(file)))

endef

# Add a srs file that does not have any localizable content.
#
# This only exists to allow dependencies on SDF files. It must be used
# if neither of the srs files in a directory have any localizable
# content, because in that case there is going to be no SDF generated
# for the directory. Therefore we must avoid depending on the SDF.
#
# gb_SrsTarget_add_nonlocalized_file srs file
define gb_SrsTarget_add_nonlocalized_file
$(call gb_SrsTarget__add_file,$(1),$(2),$(false))

endef

# Add srs files that do not have any localizable content.
#
# gb_SrsTarget_add_nonlocalized_files srs file(s)
define gb_SrsTarget_add_nonlocalized_files
$(foreach file,$(2),$(call gb_SrsTarget_add_nonlocalized_file,$(1),$(file)))

endef

define gb_SrsTarget_add_template
$(call gb_SrsTemplateTarget_add_file,$(1),$(2))

endef

define gb_SrsTarget_add_templates
$(foreach template,$(2),$(call gb_SrsTarget_add_template,$(1),$(template)))

endef

# Use templates built by another SrsTarget.
#
# gb_SrsTarget_use_srstarget srs other-srs
define gb_SrsTarget_use_srstarget
$(call gb_SrsTarget_get_headers_target,$(1)) : $(call gb_SrsTemplateTarget_get_target,$(2))

endef

# Use templates built by other SrsTargets.
#
# gb_SrsTarget_use_srstargets srs other-srs(s)
define gb_SrsTarget_use_srstargets
$(foreach srstarget,$(2),$(call gb_SrsTarget_use_srstarget,$(1),$(srstarget)))

endef


# ResTarget

gb_ResTarget_DEFIMAGESLOCATION := $(SRCDIR)/icon-themes/galaxy/

$(call gb_ResTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),RES,2)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f \
			$(call gb_ResTarget_get_target,$*) \
			$(call gb_ResTarget_get_imagelist_target,$*) \
			$(call gb_ResTarget_get_unittest_target,$*))

$(call gb_ResTarget_get_target,%) : $(gb_Helper_MISCDUMMY) \
		$(gb_ResTarget_RSCDEPS)
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
			$(filter-out $(gb_Helper_MISCDUMMY) $(gb_ResTarget_RSCDEPS),$^)" \
			> $${RESPONSEFILE} && \
		$(gb_ResTarget_RSCCOMMAND) @$${RESPONSEFILE} && \
		rm -f $${RESPONSEFILE})

define gb_ResTarget_ResTarget
$(call gb_ResTarget_get_target,$(1)) : LIBRARY = $(2)
$(call gb_ResTarget_get_target,$(1)) : LANGUAGE = $(3)
$(call gb_ResTarget_get_target,$(1)) : RESLOCATION = $(2)
$(call gb_AllLangResTarget_get_target,$(2)) : $(call gb_ResTarget_get_unittest_target,$(1))
$(call gb_AllLangResTarget_get_clean_target,$(2)) : $(call gb_ResTarget_get_clean_target,$(1))
$(call gb_ResTarget_get_imagelist_target,$(1)) : $(call gb_ResTarget_get_target,$(1))

$(call gb_ResTarget_get_unittest_target,$(1)) : $(call gb_ResTarget_get_target,$(1))
$(call gb_ResTarget_get_unittest_target,$(1)) :| $(dir $(call gb_ResTarget_get_unittest_target,$(1))).dir
$(call gb_Deliver_add_deliverable,$(call gb_ResTarget_get_unittest_target,$(1)),$(call gb_ResTarget_get_target,$(1)),$(1))

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

gb_ResTarget_get_install_target = $(INSTROOT)/$(gb_PROGRAMDIRNAME)/resource/$(1).res

define gb_AllLangResTarget_AllLangResTarget
$(foreach lang,$(gb_AllLangResTarget_LANGS),\
	$(call gb_ResTarget_ResTarget,$(1)$(lang),$(1),$(lang)))

$(foreach lang,$(gb_AllLangResTarget_LANGS),\
$(call gb_Helper_install,$(call gb_AllLangResTarget_get_target,$(1)), \
	$(call gb_ResTarget_get_install_target,$(1)$(lang)), \
	$(call gb_ResTarget_get_target,$(1)$(lang))))

$$(eval $$(call gb_Module_register_target,$(call gb_AllLangResTarget_get_target,$(1)),$(call gb_AllLangResTarget_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),AllLangResTarget)
$(call gb_Postprocess_register_target,AllResources,AllLangResTarget,$(1))

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
