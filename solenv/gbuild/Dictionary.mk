# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
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
$(call gb_Output_announce,$(2),$(true),THI,1)
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
	$(call gb_ThesaurusIndexTarget__command,$@,$*)

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

# PropertiesTranslateTarget class

# Handles translation of .properties files in dictionaries.

gb_PropertiesTranslateTarget_COMMAND := $(call gb_Executable_get_command,propex)
gb_PropertiesTranslateTarget_DEPS := $(call gb_Executable_get_runtime_dependencies,propex)

define gb_PropertiesTranslateTarget__command
$(call gb_Output_announce,$(2),$(true),PRP,1)
$(call gb_Helper_abbreviate_dirs, \
	$(if $(filter-out qtz,$(LANG)), \
		MERGEINPUT=$(call var2file,$(shell $(gb_MKTEMP)),100,$(POFILE)) && \
		$(gb_PropertiesTranslateTarget_COMMAND) \
			-i $(PROPERTIES_FILE) \
			-l $(LANG) \
			-m $${MERGEINPUT} \
			-o $(1) && \
		rm -f $${MERGEINPUT} \
		, \
		$(gb_PropertiesTranslateTarget_COMMAND) \
			-i $(PROPERTIES_FILE) \
			-l $(LANG) \
			-m \
			-o $(1) \
	) \
)
endef

$(dir $(call gb_PropertiesTranslateTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_PropertiesTranslateTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_PropertiesTranslateTarget_get_target,%) : $(gb_PropertiesTranslateTarget_DEPS)
	$(call gb_PropertiesTranslateTarget__command,$@,$*)

.PHONY : $(call gb_PropertiesTranslateTarget_get_clean_target,%)
$(call gb_PropertiesTranslateTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),PRP,1)
	rm -f $(call gb_PropertiesTranslateTarget_get_target,$*)

# gb_PropertiesTranslateTarget_PropertiesTranslateTarget target source lang
define gb_PropertiesTranslateTarget_PropertiesTranslateTarget
$(call gb_PropertiesTranslateTarget_get_target,$(1)) : LANG := $(3)
$(call gb_PropertiesTranslateTarget_get_target,$(1)) : POFILE := $(gb_POLOCATION)/$(3)/$(patsubst %/,%,$(dir $(2))).po
$(call gb_PropertiesTranslateTarget_get_target,$(1)) : PROPERTIES_FILE := $(SRCDIR)/$(2)

$(call gb_PropertiesTranslateTarget_get_target,$(1)) : $(SRCDIR)/$(2)
$(call gb_PropertiesTranslateTarget_get_target,$(1)) :| $(dir $(call gb_PropertiesTranslateTarget_get_target,$(1))).dir

$(if $(filter-out qtz,$(3)),\
	$(call gb_PropertiesTranslateTarget__PropertiesTranslateTarget_onelang,$(1),$(gb_POLOCATION)/$(3)/$(patsubst %/,%,$(dir $(2))).po) \
)

endef

# gb_PropertiesTranslateTarget__PropertiesTranslateTarget_onelang target pofile
define gb_PropertiesTranslateTarget__PropertiesTranslateTarget_onelang
$(call gb_PropertiesTranslateTarget_get_target,$(1)) : $(2)
$(2) :

endef

# DescriptionTranslateTarget class

# Handles translation of description.xml files in dictionaries.

gb_DescriptionTranslateTarget_COMMAND := $(call gb_Executable_get_command,xrmex)
gb_DescriptionTranslateTarget_DEPS := $(call gb_Executable_get_runtime_dependencies,xrmex)

define gb_DescriptionTranslateTarget__command
$(call gb_Output_announce,$(2),$(true),XRM,1)
$(call gb_Helper_abbreviate_dirs,\
	MERGEINPUT=$(call var2file,$(shell $(gb_MKTEMP)),100,$(POFILES)) && \
	$(gb_DescriptionTranslateTarget_COMMAND) \
		-i $(DESCRIPTION_XML) \
		-l all \
		-m $${MERGEINPUT} \
		-o $(1) && \
	rm -f $${MERGEINPUT} \
)
endef

$(dir $(call gb_DescriptionTranslateTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_DescriptionTranslateTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_DescriptionTranslateTarget_get_target,%) : $(gb_DescriptionTranslateTarget_DEPS)
	$(call gb_DescriptionTranslateTarget__command,$@,$*)

.PHONY : $(call gb_DescriptionTranslateTarget_get_clean_target,%)
$(call gb_DescriptionTranslateTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),XRM,1)
	rm -f $(call gb_DescriptionTranslateTarget_get_target,$*)

# gb_DescriptionTranslateTarget_DescriptionTranslateTarget target source langs
define gb_DescriptionTranslateTarget_DescriptionTranslateTarget
$(call gb_DescriptionTranslateTarget_get_target,$(1)) : DESCRIPTION_XML := $(SRCDIR)/$(2)
$(call gb_DescriptionTranslateTarget_get_target,$(1)) : POFILES :=

$(call gb_DescriptionTranslateTarget_get_target,$(1)) : $(SRCDIR)/$(2)
$(call gb_DescriptionTranslateTarget_get_target,$(1)) :| $(dir $(call gb_DescriptionTranslateTarget_get_target,$(1))).dir

$(foreach lang,$(3),\
	$(call gb_DescriptionTranslateTarget__DescriptionTranslateTarget_onelang,$(1),$(patsubst %/,%,$(dir $(2))),$(lang)) \
)

endef

# gb_DescriptionTranslateTarget__DescriptionTranslateTarget_onelang target pobase lang
define gb_DescriptionTranslateTarget__DescriptionTranslateTarget_onelang
$(call gb_DescriptionTranslateTarget_get_target,$(1)) : POFILES += $(if $(filter-out qtz,$(3)),$(gb_POLOCATION)/$(3)/$(2).po)
$(if $(filter-out qtz,$(3)),\
	$(call gb_DescriptionTranslateTarget__DescriptionTranslateTarget_onelang_podeps,$(1),$(gb_POLOCATION)/$(3)/$(2).po))

endef

# gb_DescriptionTranslateTarget__DescriptionTranslateTarget_onelang_podeps target pofile
define gb_DescriptionTranslateTarget__DescriptionTranslateTarget_onelang_podeps
$(call gb_DescriptionTranslateTarget_get_target,$(1)) : $(2)
$(2) :

endef

# Dictionary class

# Handles creation and delivery of dictionary extensions.
#
# This class provides a filelist called Dictionary/<name> that contains
# all installed files of the dictionary.

gb_Dictionary_ALL_LANGS := $(filter-out en-US,$(gb_WITH_LANG))
gb_Dictionary_INSTDIR := share/extensions

$(dir $(call gb_Dictionary_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_Dictionary_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_Dictionary_get_target,%) :
	$(call gb_Output_announce,$*,$(true),DIC,3)
	touch $@

.PHONY : $(call gb_Dictionary_get_clean_target,%)
$(call gb_Dictionary_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),DIC,3)
	rm -f $(call gb_Dictionary_get_target,$*)

gb_Dictionary_get_packagename = Dictionary/$(1)
gb_Dictionary_get_packagesetname = Dictionary/$(1)

# Creates a dictionary extension
#
# gb_Dictionary_Dictionary dictionary srcdir
define gb_Dictionary_Dictionary
$(call gb_Package_Package_internal,$(call gb_Dictionary_get_packagename,$(1)),$(SRCDIR))
$(call gb_Package_Package_internal,$(call gb_Dictionary_get_packagename,$(1)_generated),$(WORKDIR))
$(call gb_PackageSet_PackageSet_internal,$(call gb_Dictionary_get_packagesetname,$(1)))

$(call gb_Package_set_outdir,$(call gb_Dictionary_get_packagename,$(1)),$(gb_INSTROOT))
$(call gb_Package_set_outdir,$(call gb_Dictionary_get_packagename,$(1)_generated),$(gb_INSTROOT))
$(call gb_PackageSet_add_package,$(call gb_Dictionary_get_packagesetname,$(1)),$(call gb_Dictionary_get_packagename,$(1)))

ifeq (,$(gb_Dictionary_ALL_LANGS))
$(call gb_Dictionary_add_root_file,$(1),$(2)/description.xml)
else
$(call gb_Dictionary__add_description_translations,$(1),$(2)/description.xml)
endif
$(call gb_Dictionary_add_root_file,$(1),$(2)/dictionaries.xcu)
$(call gb_Dictionary_add_file,$(1),META-INF/manifest.xml,$(2)/META-INF/manifest.xml)

$(call gb_Dictionary_get_target,$(1)) : $(call gb_PackageSet_get_target,$(call gb_Dictionary_get_packagesetname,$(1)))
$(call gb_Dictionary_get_target,$(1)) :| $(dir $(call gb_Dictionary_get_target,$(1))).dir
$(call gb_Dictionary_get_clean_target,$(1)) : $(call gb_PackageSet_get_clean_target,$(call gb_Dictionary_get_packagesetname,$(1)))

$$(eval $$(call gb_Module_register_target,$(call gb_Dictionary_get_target,$(1)),$(call gb_Dictionary_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),Dictionary)

endef

# gb_Dictionary__add_description_translation_impl dictionary desctarget descxml
define gb_Dictionary__add_description_translation_impl
$(call gb_DescriptionTranslateTarget_DescriptionTranslateTarget,$(2),$(3),$(gb_Dictionary_ALL_LANGS))
$(call gb_Dictionary_add_generated_file,$(1),description.xml,$(call gb_DescriptionTranslateTarget_get_target,$(2)))

$(call gb_Dictionary_get_target,$(1)) : $(call gb_DescriptionTranslateTarget_get_target,$(2))
$(call gb_Dictionary_get_clean_target,$(1)) : $(call gb_DescriptionTranslateTarget_get_clean_target,$(2))

endef

# gb_Dictionary__add_description_translations dictionary descxml
define gb_Dictionary__add_description_translations
$(call gb_Dictionary__add_description_translation_impl,$(1),$(basename $(2)),$(2))

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

# gb_Dictionary__translate_propertyfile_impl dictionary destfile propertyfile propertyname lang
define gb_Dictionary__translate_propertyfile_impl
$(call gb_PropertiesTranslateTarget_PropertiesTranslateTarget,$(4),$(3),$(5))
$(call gb_Dictionary_add_generated_file,$(1),$(2),$(call gb_PropertiesTranslateTarget_get_target,$(4)))

endef

# gb_Dictionary__translate_propertyfile dictionary destdir propertyfile propertyname lang
define gb_Dictionary__translate_propertyfile
$(call gb_Dictionary__translate_propertyfile_impl,$(1),$(2)$(notdir $(4)).properties,$(3),$(4),$(5))

endef

# gb_Dictionary__add_propertyfile_translations dictionary destfile propertyfile
define gb_Dictionary__add_propertyfile_translations
$(foreach lang,$(gb_Dictionary_ALL_LANGS),$(call gb_Dictionary__translate_propertyfile,$(1),$(dir $(2)),$(3),$(subst en_US,$(subst -,_,$(lang)),$(basename $(3))),$(lang)))

endef

# Adds a .property file to the dictionary under chosen name
#
# The file is localized automatically.
#
# gb_Dictionary_add_propertyfile dictionary destfile propertyfile
define gb_Dictionary_add_propertyfile
$(call gb_Dictionary__add_file,$(1),$(1),$(2),$(3))
$(call gb_Dictionary__add_propertyfile_translations,$(1),$(2),$(3))

endef

# Adds several .property files to the dictionary at once
#
# The files are put into chosen directory. They are localized automatically.
#
# gb_Dictionary_add_propertyfiles dictionary destdir propertyfile(s)
define gb_Dictionary_add_propertyfiles
$(foreach propertyfile,$(3),$(call gb_Dictionary_add_propertyfile,$(1),$(2)/$(notdir $(propertyfile)),$(propertyfile)))

endef

# vim: set noet sw=4 ts=4:
