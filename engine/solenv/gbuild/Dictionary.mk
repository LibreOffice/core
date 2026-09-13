# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# ThesaurusIndexTarget class

gb_ThesaurusIndexTarget_CHECK_TARGET := $(SRCDIR)/dictionaries/util/th_check.pl
gb_ThesaurusIndexTarget_CHECK_COMMAND := PERL_UNICODE=0 $(PERL) -w $(gb_ThesaurusIndexTarget_CHECK_TARGET)
gb_ThesaurusIndexTarget_INDEX_DEPS := $(call gb_Executable_get_runtime_dependencies,idxdict)
gb_ThesaurusIndexTarget_INDEX_COMMAND := $(call gb_Executable_get_command,idxdict)

define gb_ThesaurusIndexTarget__command
$(call gb_Helper_abbreviate_dirs,\
	$(gb_ThesaurusIndexTarget_INDEX_COMMAND) -o $(1) < $(THESAURUS_FILE) \
)
endef
	#$(gb_ThesaurusIndexTarget_CHECK_COMMAND) $(THESAURUS_FILE) && \

$(dir $(call gb_ThesaurusIndexTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_ThesaurusIndexTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_ThesaurusIndexTarget_get_target,%) : $(gb_ThesaurusIndexTarget_INDEX_DEPS) | $(gb_ThesaurusIndexTarget_CHECK_TARGET)
	$(call gb_Output_announce,$*,$(true),THI,1)
	$(call gb_Trace_StartRange,$*,THI)
	$(call gb_ThesaurusIndexTarget__command,$@,$*)
	$(call gb_Trace_EndRange,$*,THI)

.PHONY : $(call gb_ThesaurusIndexTarget_get_clean_target,%)
$(call gb_ThesaurusIndexTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),THI,1)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_ThesaurusIndexTarget_get_target,$*) \
	)

# Creates an index for a given thesaurus
#
# The thesaurus' file path is relative to $(SRCDIR).
#
# gb_ThesaurusIndexTarget_ThesaurusIndexTarget thesaurus
define gb_ThesaurusIndexTarget_ThesaurusIndexTarget
$(call gb_ThesaurusIndexTarget_get_target,$(1)) : THESAURUS_FILE := $(SRCDIR)/$(1)
$(call gb_ThesaurusIndexTarget_get_target,$(1)) : $(SRCDIR)/$(1)
$(call gb_ThesaurusIndexTarget_get_target,$(1)) :| $(dir $(call gb_ThesaurusIndexTarget_get_target,$(1))).dir

endef

# Dictionary class

# Handles creation and delivery of dictionaries.
#
# This class provides a filelist called Dictionary/<name> that contains
# all installed files of the dictionary. What it installs is the data
# itself: what describes it to the linguistic services is generated into
# a configuration layer by CustomTarget_registry.

gb_Dictionary_INSTDIR := $(LIBO_SHARE_FOLDER)/extensions

$(dir $(call gb_Dictionary_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_Dictionary_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_Dictionary_get_target,%) :
	$(call gb_Output_announce,$*,$(true),DIC,3)
	$(call gb_Trace_MakeMark,$*,DIC)
	touch $@

.PHONY : $(call gb_Dictionary_get_clean_target,%)
$(call gb_Dictionary_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),DIC,3)
	rm -f $(call gb_Dictionary_get_target,$*)

gb_Dictionary_get_packagename = Dictionary/$(1)
gb_Dictionary_get_packagesetname = Dictionary/$(1)

# Every dictionary that is actually built, as "installed name=source dir".
# The registry layer is generated from this, so it lists what a given
# configuration ships and nothing else. Recipes expand late, so a rule may
# read it even though the Dictionary makefiles are included after the one
# that builds the registry.
gb_Dictionary_ALL :=

# Creates a dictionary
#
# gb_Dictionary_Dictionary dictionary srcdir
define gb_Dictionary_Dictionary
gb_Dictionary_ALL += $(1)=$(2)
$(call gb_Package_Package_internal,$(call gb_Dictionary_get_packagename,$(1)),$(SRCDIR))
$(call gb_Package_Package_internal,$(call gb_Dictionary_get_packagename,$(1)_generated),$(WORKDIR))
$(call gb_PackageSet_PackageSet_internal,$(call gb_Dictionary_get_packagesetname,$(1)))

$(call gb_PackageSet_add_package,$(call gb_Dictionary_get_packagesetname,$(1)),$(call gb_Dictionary_get_packagename,$(1)))

$(call gb_Dictionary_get_target,$(1)) : $(call gb_PackageSet_get_target,$(call gb_Dictionary_get_packagesetname,$(1)))
$(call gb_Dictionary_get_target,$(1)) :| $(dir $(call gb_Dictionary_get_target,$(1))).dir
$(call gb_Dictionary_get_clean_target,$(1)) : $(call gb_PackageSet_get_clean_target,$(call gb_Dictionary_get_packagesetname,$(1)))

$$(eval $$(call gb_Module_register_target,$(call gb_Dictionary_get_target,$(1)),$(call gb_Dictionary_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),Dictionary)

endef

# gb_Dictionary__add_file dictionary package-base destfile sourcefile
define gb_Dictionary__add_file
$(call gb_Package_add_file,$(call gb_Dictionary_get_packagename,$(2)),$(gb_Dictionary_INSTDIR)/$(1)/$(3),$(4))

endef

# Adds a file from $(SRCDIR) to the dictionary under chosen name
#
# gb_Dictionary_add_file dictionary destfile sourcefile
define gb_Dictionary_add_file
$(call gb_Dictionary__add_file,$(1),$(1),$(2),$(3))

endef

# Adds several files from $(SRCDIR) to the dictionary at once
#
# The files are put into the chosen directory.
#
# gb_Dictionary_add_files dictionary destdir file(s)
define gb_Dictionary_add_files
$(foreach file,$(3),$(call gb_Dictionary_add_file,$(1),$(2)/$(notdir $(file)),$(file)))

endef

# Adds an arbitrary file to the dictionary under chosen name
#
# gb_Dictionary_add_file dictionary destfile sourcefile
define gb_Dictionary_add_generated_file
$(call gb_Dictionary__add_file,$(1),$(1)_generated,$(2),$(subst $(WORKDIR)/,,$(3)))
$(call gb_PackageSet_add_package,$(call gb_Dictionary_get_packagesetname,$(1)),$(call gb_Dictionary_get_packagename,$(1)_generated))

endef

# Adds several arbitrary files to the dictionary at once
#
# The files are put into the chosen directory.
#
# gb_Dictionary_add_files dictionary destdir file(s)
define gb_Dictionary_add_generated_files
$(foreach file,$(3),$(call gb_Dictionary_add_generated_file,$(1),$(2)/$(notdir $(file)),$(file)))

endef

# Adds a localized xcu file, which needs special handling because it may be
# in $(WORKDIR) or $(SRCDIR) depending on whether translations are built.
#
# gb_Dictionary_add_localized_xcu_file dictionary destdir file
define gb_Dictionary_add_localized_xcu_file
ifeq ($(gb_WITH_LANG),)
$(call gb_Dictionary__add_file,$(1),$(1),$(2)/$(notdir $(3)),$(strip $(3)))
else
$(call gb_Dictionary__add_file,$(1),$(1)_generated,$(2)/$(notdir $(3)),$(subst $(WORKDIR)/,,$(call gb_XcuFile_for_extension,$(strip $(3)))))
$(call gb_PackageSet_add_package,$(call gb_Dictionary_get_packagesetname,$(1)),$(call gb_Dictionary_get_packagename,$(1)_generated))
endif

endef

# Adds a file to the root dir of the dictionary
#
# gb_Dictionary_add_root_file dictionary file
define gb_Dictionary_add_root_file
$(call gb_Dictionary_add_file,$(1),$(notdir $(2)),$(2))

endef

# Adds several files to the root dir of the dictionary
#
# gb_Dictionary_add_root_files dictionary file(s)
define gb_Dictionary_add_root_files
$(foreach file,$(2),$(call gb_Dictionary_add_root_file,$(1),$(file)))

endef

define gb_Dictionary__add_thesaurus
$(call gb_Dictionary__add_file,$(1),$(1)_generated,$(notdir $(3)),$(subst $(WORKDIR)/,,$(3)))
$(call gb_PackageSet_add_package,$(call gb_Dictionary_get_packagesetname,$(1)),$(call gb_Dictionary_get_packagename,$(1)_generated))

endef

# Adds a thesaurus to the dictionary
#
# An index for the thesaurus is generated and added to the dictionary as
# well.
#
# gb_Dictionary_add_thesaurus dictionary thesaurus
define gb_Dictionary_add_thesaurus
$(call gb_ThesaurusIndexTarget_ThesaurusIndexTarget,$(2))
$(call gb_Dictionary_add_root_file,$(1),$(2))
$(call gb_Dictionary__add_thesaurus,$(1),$(2),$(call gb_ThesaurusIndexTarget_get_target,$(2)))

endef

# Adds several thesauri to the dictionary at once
#
# Indexes for the thesauri are generated and added to the dictionary as
# well.
#
# gb_Dictionary_add_thesauri dictionary thesauri
define gb_Dictionary_add_thesauri
$(foreach thesaurus,$(2),$(call gb_Dictionary_add_thesaurus,$(1),$(thesaurus)))

endef

# vim: set noet sw=4 ts=4:
