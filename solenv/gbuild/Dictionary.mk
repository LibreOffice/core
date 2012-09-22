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
gb_ThesaurusIndexTarget_INDEX_TARGET := $(call gb_Executable_get_target_for_build,idxdict)
gb_ThesaurusIndexTarget_INDEX_COMMAND := $(gb_Helper_set_ld_path) $(gb_ThesaurusIndexTarget_INDEX_TARGET)

define gb_ThesaurusIndexTarget__command
$(call gb_Output_announce,$(2),$(true),THI,1)
$(call gb_Helper_abbreviate_dirs,\
	$(gb_ThesaurusIndexTarget_INDEX_COMMAND) -o $(1) < $(THESAURUS_FILE) \
)
endef
	#$(gb_ThesaurusIndexTarget_CHECK_COMMAND) $(THESAURUS_FILE) && \

$(call gb_ThesaurusIndexTarget_get_target,%) : $(gb_ThesaurusIndexTarget_INDEX_TARGET) | $(gb_ThesaurusIndexTarget_CHECK_TARGET)
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

# Dictionary class

# Handles creation and delivery of dictionary extensions.

gb_Dictionary_CONFIGURATION_FILE := dictionaries.xcu

gb_Dictionary_extensionname = Dictionary/$(1)

# Creates a dictionary extension
#
# gb_Dictionary_Dictionary dictionary srcdir
define gb_Dictionary_Dictionary
$(call gb_ExtensionTarget_ExtensionTarget,$(call gb_Dictionary_extensionname,$(1)),$(2))
$(call gb_Dictionary_add_root_file,$(1),$(2)/$(gb_Dictionary_CONFIGURATION_FILE))

$(call gb_Dictionary_get_target,$(1)) : $(call gb_ExtensionTarget_get_target,$(call gb_Dictionary_extensionname,$(1)))
$(call gb_Dictionary_get_clean_target,$(1)) : $(call gb_ExtensionTarget_get_clean_target,$(call gb_Dictionary_extensionname,$(1)))

$(call gb_Deliver_add_deliverable,$(call gb_Dictionary_get_target,$(1)),$(call gb_ExtensionTarget_get_target,$(call gb_Dictionary_extensionname,$(1))),$(1))

$$(eval $$(call gb_Module_register_target,$(call gb_Dictionary_get_target,$(1)),$(call gb_Dictionary_get_clean_target,$(1))))

endef

# Adds a file from $(SRCDIR) to the dictionary under chosen name
#
# gb_Dictionary_add_file dictionary destfile sourcefile
define gb_Dictionary_add_file
$(call gb_ExtensionTarget_add_file,$(call gb_Dictionary_extensionname,$(1)),$(2),$(SRCDIR)/$(3))

endef

# Adds several files from $(SRCDIR) to the dictionary at once
#
# The files are put into the chosen directory.
#
# gb_Dictionary_add_files dictionary destdir file(s)
define gb_Dictionary_add_files
$(call gb_ExtensionTarget_add_files,$(call gb_Dictionary_extensionname,$(1)),$(2),$(addprefix $(SRCDIR)/,$(3)))

endef

# Adds an arbitrary file to the dictionary under chosen name
#
# gb_Dictionary_add_file dictionary destfile sourcefile
define gb_Dictionary_add_generated_file
$(call gb_ExtensionTarget_add_file,$(call gb_Dictionary_extensionname,$(1)),$(2),$(3))

endef

# Adds several arbitrary files to the dictionary at once
#
# The files are put into the chosen directory.
#
# gb_Dictionary_add_files dictionary destdir file(s)
define gb_Dictionary_add_generated_files
$(call gb_ExtensionTarget_add_files,$(call gb_Dictionary_extensionname,$(1)),$(2),$(3))

endef

define gb_Dictionary__add_root_file
$(call gb_ExtensionTarget_add_file,$(call gb_Dictionary_extensionname,$(1)),$(notdir $(2)),$(2))

endef

# Adds a file to the root dir of the dictionary
#
# gb_Dictionary_add_root_file dictionary file
define gb_Dictionary_add_root_file
$(call gb_Dictionary__add_root_file,$(1),$(SRCDIR)/$(2))

endef

# Adds several files to the root dir of the dictionary
#
# gb_Dictionary_add_root_files dictionary file(s)
define gb_Dictionary_add_root_files
$(foreach file,$(2),$(call gb_Dictionary_add_root_file,$(1),$(file)))

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
$(call gb_Dictionary__add_root_file,$(1),$(call gb_ThesaurusIndexTarget_get_target,$(2)))

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

# Adds a .property file to the dictionary under chosen name
#
# The file is localized automatically.
#
# gb_Dictionary_add_propertyfile dictionary destfile propertyfile
define gb_Dictionary_add_propertyfile
$(call gb_ExtensionTarget_localize_properties,$(call gb_Dictionary_extensionname,$(1)),$(2),$(SRCDIR)/$(3))

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
