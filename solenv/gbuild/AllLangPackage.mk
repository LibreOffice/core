# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# AllLangPackage class

# Handles creation of a bunch of packages with content dependent on
# language. The package files are placed into $(gb_INSTROOT).

gb_AllLangPackage_LANGS := $(if $(strip $(gb_WITH_LANG)),$(gb_WITH_LANG),en-US)

$(dir $(call gb_AllLangPackage_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_AllLangPackage_get_target,%) :
	$(call gb_Output_announce,$*,$(true),ALP,3)
	touch $@

.PHONY : $(call gb_AllLangPackage_get_clean_target,%)
$(call gb_AllLangPackage_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(true),ALP,3)
	rm -f $(call gb_AllLangPackage_get_target,$*)

# Define a new package group.
#
# gb_AllLangPackage_AllLangPackage group srcdir
define gb_AllLangPackage_AllLangPackage
$(foreach lang,$(gb_AllLangPackage_LANGS),$(call gb_AllLangPackage__AllLangPackage_onelang,$(1),$(1)_$(lang),$(2)))

$(call gb_AllLangPackage_get_target,$(1)) :| $(dir $(call gb_AllLangPackage_get_target,$(1))).dir

$$(eval $$(call gb_Module_register_target,$(call gb_AllLangPackage_get_target,$(1)),$(call gb_AllLangPackage_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),AllLangPackage)

endef

# Define a package for one lang.
#
# gb_AllLangPackage__AllLangPackage_lang group package srcdir
define gb_AllLangPackage__AllLangPackage_onelang
$(call gb_Package_Package_internal,$(2),$(3))
$(call gb_Package_set_outdir,$(2),$(gb_INSTROOT))
$(call gb_AllLangPackage_get_target,$(1)) : $(call gb_Package_get_target,$(2))
$(call gb_AllLangPackage_get_clean_target,$(1)) : $(call gb_Package_get_clean_target,$(2))

endef

# gb_AllLangPackage__add_to_package target package destination source
define gb_AllLangPackage__add_to_package
$(call gb_Package_add_file,$(2),$(3),$(4))

endef

# gb_AllLangPackage__add_file target destination source lang
define gb_AllLangPackage__add_file
$(if $(filter $(4),$(gb_AllLangPackage_LANGS)),$(call gb_AllLangPackage__add_to_package,$(1),$(1)_$(4),$(2),$(3)))

endef

# Add a file to one of the child packages.
#
# The language is taken from the first component of the file name. The
# file is only added if there is a package defined for the language
# (i.e., if we are building with the language).
#
# gb_AllLangPackage_add_file target destination source
define gb_AllLangPackage_add_file
$(call gb_AllLangPackage__add_file,$(1),$(2),$(3),$(firstword $(subst /, ,$(3))))

endef

# Add several files to the child packages at once.
#
# gb_AllLangPackage_add_files target destination-dir file(s)
define gb_AllLangPackage_add_files
$(if $(strip $(2)),,$(call gb_Output_error,gb_AllLangPackage_add_files: destination dir cannot be empty))
$(foreach file,$(3),$(call gb_AllLangPackage_add_file,$(1),$(2)/$(file),$(file)))

endef

# Add several files to the child packages at once.
#
# The files are placed into subdir under the language-dependent path.
#
# Example:
# $(eval $(call # gb_AllLangPackage_add_files_with_subdir,foo,destdir,subdir,cs/file.ext))
# # -> destdir/cs/subdir/file.ext
#
# gb_AllLangPackage_add_files_with_subdir target destination-dir subdir file(s)
define gb_AllLangPackage_add_files_with_subdir
$(if $(strip $(2)),,$(call gb_Output_error,gb_AllLangPackage_add_files_with_subdir: destination dir cannot be empty))
$(if $(strip $(3)),,$(call gb_Output_error,gb_AllLangPackage_add_files_with_subdir: there is no subdir, just use gb_AllLangPackage_add_files))
$(foreach file,$(4),$(call gb_AllLangPackage_add_file,$(1),$(2)/$(dir $(file))$(3)/$(notdir $(file)),$(file)))

endef

# Use unpacked tarball.
#
# gb_AllLangPackage_add_dependency target unpacked
define gb_AllLangPackage_use_unpacked
$(foreach lang,$(gb_AllLangPackage_LANGS),$(call gb_Package_use_unpacked,$(1)_$(lang),$(2)))

endef

# vim: set noet sw=4 ts=4:
