# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# AllLangZip class

# Handles creation of a bunch of zip files with content dependent on
# language

gb_AllLangZip_LANGS := $(gb_WITH_LANG)

$(dir $(call gb_AllLangZip_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_AllLangZip_get_target,%) :
	$(call gb_Output_announce,$*,$(true),ALZ,4)
	touch $@

.PHONY : $(call gb_AllLangZip_get_clean_target,%)
$(call gb_AllLangZip_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(true),ALZ,4)
	rm -f $(call gb_AllLangZip_get_target,$*)

# Define a new zip group
#
# NOTE: we cannot add dependencies on the child zips here: Zip fails if
# the source directory does not exist and we cannot test its existence
# because the directory might be generated. So we postpone the
# dependency creation to gb_AllLangZip_add_file.
#
# gb_AllLangZip_AllLangZip group srcdir
define gb_AllLangZip_AllLangZip
$(foreach lang,$(gb_AllLangZip_LANGS),$(call gb_Zip_Zip_internal,$(1)_$(lang),$(2)/$(lang)))

$(call gb_AllLangZip_get_target,$(1)) :| $(dir $(call gb_AllLangZip_get_target,$(1))).dir

$$(eval $$(call gb_Module_register_target,$(call gb_AllLangZip_get_target,$(1)),$(call gb_AllLangZip_get_clean_target,$(1))))

endef

define gb_AllLangZip__add_to_zip
$(call gb_Zip_add_file,$(2),$(3))
$(call gb_AllLangZip_get_target,$(1)) : $(call gb_Zip_get_outdir_target,$(2))
$(call gb_AllLangZip_get_clean_target,$(1)) : $(call gb_Zip_get_clean_target,$(2))

endef

define gb_AllLangZip__add_file_impl
$(if $(filter $(2),$(gb_AllLangZip_LANGS)),$(call gb_AllLangZip__add_to_zip,$(1),$(1)_$(2),$(3)))

endef

define gb_AllLangZip__add_file
$(call gb_AllLangZip__add_file_impl,$(1),$(firstword $(2)),$(subst $() $(),/,$(wordlist 2,$(words $(2)),$(2))))

endef

# Add a file to one of the child zips
#
# The language is taken from the first component of the file name. The
# file is only added if there is a zip defined for the language (i.e.,
# if we are building with the language).
#
# gb_AllLangZip_add_file target file
define gb_AllLangZip_add_file
$(call gb_AllLangZip__add_file,$(1),$(subst /, ,$(2)))

endef

# Add several files to the child zips at once
#
# gb_AllLangZip_add_files target file(s)
define gb_AllLangZip_add_files
$(foreach file,$(2),$(call gb_AllLangZip_add_file,$(1),$(file)))

endef

# Add extra dependency that must exist before the archives can be created
#
# gb_AllLangZip_add_dependency target dep
define gb_AllLangZip_add_dependency
$(foreach lang,$(gb_AllLangZip_LANGS),$(call gb_Zip_add_dependency,$(1)_$(lang),$(2)))

endef

# Add extra dependencies that must exist before the archives can be created
#
# gb_AllLangZip_add_dependencies target dep(s)
define gb_AllLangZip_add_dependencies
$(foreach dependency,$(2),$(call gb_AllLangZip_add_dependency,$(1),$(dependency)))

endef

# vim: set noet sw=4 ts=4:
