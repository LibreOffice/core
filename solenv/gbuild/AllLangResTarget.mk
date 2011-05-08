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
# SrsPartTarget         preprocessing       SrsPartMergeTarget (with l10n)
#                                           source file (without l10n)
# SrsPartMergeTarget    merging/transex     source file (only needed with l10n)

# Overview of dependencies and tasks of AllLangResTarget
#
# target                task                depends on
# AllLangResTarget      nothing             ResTarget for all active langs
# ResTarget             running rsc         SrsTarget, resources


# SrsPartMergeTarget class

gb_SrsPartMergeTarget_TRANSEXTARGET := $(call gb_Executable_get_target,transex3)
# gb_SrsPartMergeTarget_TRANSEXPRECOMMAND is set by the platforms

gb_SrsPartMergeTarget_TRANSEXCOMMAND := $(gb_SrsPartMergeTarget_TRANSEXPRECOMMAND) $(gb_SrsPartMergeTarget_TRANSEXTARGET)
gb_SrsPartMergeTarget_SDFLOCATION := $(SRCDIR)/translations/$(INPATH)/misc/sdf/
gb_SrsPartMergeTarget_REPOS := $(gb_REPOS)

define gb_SrsPartMergeTarget__command
$(call gb_Output_announce,$(3),$(true),srs,1)
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	$(gb_SrsPartMergeTarget_TRANSEXCOMMAND) \
		-p $(firstword $(subst /, ,$(2))) \
		-i $(realpath $(3)) \
		-o $(1) \
		-m $(SDF) \
		-l all)

endef

define gb_SrsPartMergeTarget__rules
$$(call gb_SrsPartMergeTarget_get_target,%) : $(1)/% $$(gb_Helper_MISCDUMMY) | $$(gb_SrsPartMergeTarget_TRANSEXTARGET)
	$$(if $$(SDF),$$(call gb_SrsPartMergeTarget__command,$$@,$$*,$$<),mkdir -p $$(dir $$@) && cp $$< $$@)

endef

$(foreach repo,$(gb_SrsPartMergeTarget_REPOS),$(eval $(call gb_SrsPartMergeTarget__rules,$(repo))))

# SrsPartTarget class

gb_SrsPartTarget_REPOS := $(gb_REPOS)
# defined by platform
#  gb_SrsPartTarget_RSCTARGET
#  gb_SrsPartTarget_RSCCOMMAND
#  gb_SrsPartTarget__command_dep

define gb_SrsPartTarget__command
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	RESPONSEFILE=`$(gb_MKTEMP)` && \
	echo "-s \
		$(INCLUDE) \
		-I$(realpath $(dir $(3))) \
		$(DEFS) \
		-fp=$(1) \
		$(realpath $(if $(MERGEDFILE),$(MERGEDFILE),$<))" > $${RESPONSEFILE} && \
	$(gb_SrsPartTarget_RSCCOMMAND) -presponse @$${RESPONSEFILE} && \
	rm -rf $${RESPONSEFILE})

endef

define gb_SrsPartTarget__rules
$$(call gb_SrsPartTarget_get_target,%) : $(1)/% $$(gb_Helper_MISCDUMMY) | $$(gb_SrsPartTarget_RSCTARGET)
	$$(call gb_SrsPartTarget__command_dep,$$*,$$<)
	$$(call gb_SrsPartTarget__command,$$@,$$*,$$<)

ifeq ($(gb_FULLDEPS),$(true))
$$(call gb_SrsPartTarget_get_dep_target,%) : $(1)/% $$(gb_Helper_MISCDUMMY)
	$$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $$(dir $$@) && \
		echo '$$(call gb_SrsPartTarget_get_target,$$*) : $$(gb_Helper_PHONY)' > $$@)
endif

endef

$(foreach repo,$(gb_SrsPartTarget_REPOS),$(eval $(call gb_SrsPartTarget__rules,$(repo))))

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_SrsPartTarget_get_dep_target,%) :
	$(eval $(call gb_Output_error,Unable to find resource definition file $* in repositories: $(gb_SrsPartTarget_REPOS)))
endif


define gb_SrsPartTarget_SrsPartTarget
ifeq ($(strip $(gb_WITH_LANG)),)
$(call gb_SrsPartTarget_get_target,$(1)) : MERGEDFILE := 
else
$(call gb_SrsPartTarget_get_target,$(1)) : MERGEDFILE := $(call gb_SrsPartMergeTarget_get_target,$(1))
$(call gb_SrsPartTarget_get_target,$(1)) : $(call gb_SrsPartMergeTarget_get_target,$(1))
$(call gb_SrsPartMergeTarget_get_target,$(1)) : SDF := $(realpath $(gb_SrsPartMergeTarget_SDFLOCATION)$(dir $(1))localize.sdf)
endif

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

$(call gb_SrsTarget_get_target,%) :
	$(call gb_SrsTarget__command_dep,$(call gb_SrsTarget_get_dep_target,$*),$*,$(foreach part,$(PARTS),$(call gb_SrsPartTarget_get_dep_target,$(part))))
	$(call gb_Output_announce,$*,$(true),SRS,1)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		cat $^ > $@)

ifeq ($(gb_FULLDEPS),$(true))
$(call gb_SrsTarget_get_dep_target,%) :
	$(call gb_SrsTarget__command_dep,$@,$*,$^)
endif

define gb_SrsTarget_SrsTarget
$(call gb_SrsTarget_get_target,$(1)) : DEFS := $(gb_SrsTarget_DEFAULTDEFS)
$(call gb_SrsTarget_get_target,$(1)) : INCLUDE := $(SOLARINC)
$(call gb_SrsTarget_get_clean_target,$(1)) : PARTS :=
$(call gb_SrsTarget_get_target,$(1)) : PARTS :=
ifeq ($(gb_FULLDEPS),$(true))
ifneq ($(wildcard $(call gb_SrsTarget_get_dep_target,$(1))),)
include $(call gb_SrsTarget_get_dep_target,$(1))
else
$(firstword $(MAKEFILE_LIST)) : $(call gb_SrsTarget_get_dep_target,$(1))
endif
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
$(call gb_SrsTarget_get_clean_target,$(1)) : PARTS += $(2)
$(call gb_SrsTarget_get_target,$(1)) : PARTS += $(2)

endef

define gb_SrsTarget_add_files
$(foreach file,$(2),$(call gb_SrsTarget_add_file,$(1),$(file)))

endef


# ResTarget

gb_ResTarget_RSCTARGET := $(gb_SrsPartTarget_RSCTARGET)
gb_ResTarget_RSCCOMMAND := $(gb_SrsPartTarget_RSCCOMMAND)
gb_ResTarget_DEFIMAGESLOCATION := $(realpath $(SRCDIR)/default_images)/

$(call gb_ResTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),RES,2)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_ResTarget_get_target,$*) $(call gb_ResTarget_get_imagelist_target,$*) $(call gb_ResTarget_get_outdir_target,$*) $(call gb_ResTarget_get_outdir_imagelist_target,$*))

$(call gb_ResTarget_get_target,%) : $(gb_Helper_MISCDUMMY) | $(gb_ResTarget_RSCTARGET)
	$(call gb_Output_announce,$*,$(true),RES,2)
	$(call gb_Helper_abbreviate_dirs_native,\
		mkdir -p $(dir $@) $(OUTDIR)/bin \
			$(dir $(call gb_ResTarget_get_imagelist_target,$*)) && \
		RESPONSEFILE=`$(gb_MKTEMP)` && \
		echo "-r -p \
			-lg$(LANGUAGE) \
			-fs=$@ \
			-lip=$(realpath $(gb_ResTarget_DEFIMAGESLOCATION)$(RESLOCATION)/$(LIBRARY)) \
			-lip=$(realpath $(gb_ResTarget_DEFIMAGESLOCATION)$(RESLOCATION)/imglst/$(LANGUAGE)) \
			-lip=$(realpath $(gb_ResTarget_DEFIMAGESLOCATION)$(RESLOCATION)/imglst) \
			-lip=$(realpath $(gb_ResTarget_DEFIMAGESLOCATION)$(RESLOCATION)/res/$(LANGUAGE)) \
			-lip=$(realpath $(gb_ResTarget_DEFIMAGESLOCATION)$(RESLOCATION)/res) \
			-lip=$(realpath $(gb_ResTarget_DEFIMAGESLOCATION)$(RESLOCATION)) \
			-lip=$(gb_ResTarget_DEFIMAGESLOCATION)res/$(LANGUAGE) \
			-lip=$(gb_ResTarget_DEFIMAGESLOCATION)res \
			-subMODULE=$(dir $(realpath $(gb_ResTarget_DEFIMAGESLOCATION)$(RESLOCATION))) \
			-subGLOBALRES=$(gb_ResTarget_DEFIMAGESLOCATION)res \
			-oil=$(dir $(call gb_ResTarget_get_imagelist_target,$*)) \
			$(filter-out $(gb_Helper_MISCDUMMY),$^)" > $${RESPONSEFILE} && \
		$(gb_ResTarget_RSCCOMMAND) @$${RESPONSEFILE} && \
		rm -f $${RESPONSEFILE})

$(call gb_ResTarget_get_outdir_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		$(call gb_Deliver_deliver,$<,$@) && \
		$(call gb_Deliver_deliver,$(dir $<)/$(notdir $(ILSTTARGET)),$(ILSTTARGET)))

define gb_ResTarget_ResTarget
$(call gb_ResTarget_get_target,$(1)) : LIBRARY = $(2)
$(call gb_ResTarget_get_target,$(1)) : LANGUAGE = $(3)
$(call gb_ResTarget_get_target,$(1)) : RESLOCATION = $(2)
$(call gb_AllLangResTarget_get_target,$(2)) : $(call gb_ResTarget_get_outdir_target,$(1))
$(call gb_AllLangResTarget_get_clean_target,$(2)) : $(call gb_ResTarget_get_clean_target,$(1))
$(call gb_ResTarget_get_imagelist_target,$(1)) : $(call gb_ResTarget_get_target,$(1))

$(call gb_ResTarget_get_outdir_target,$(1)) : $(call gb_ResTarget_get_target,$(1)) 
$(call gb_ResTarget_get_outdir_target,$(1)) : ILSTTARGET = $(call gb_ResTarget_get_outdir_imagelist_target,$(1))
$(call gb_Deliver_add_deliverable,$(call gb_ResTarget_get_outdir_target,$(1)),$(call gb_ResTarget_get_target,$(1)))
$(call gb_Deliver_add_deliverable,$(call gb_ResTarget_get_outdir_imagelist_target,$(1)),$(call gb_ResTarget_get_imagelist_target,$(1)))

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
