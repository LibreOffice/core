# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# ExternalPackage class

# This class extends Package to reliably deliver header files (and
# possibly other kinds of files) from unpacked tarballs. The problem
# with using Package is that the unpacked files' timestamps do not
# depend on the extraction time; when the project's tarball is updated,
# some header files might have been changed, but it is likely their
# timestamps will be older than these of the headers delivered from the
# previous version, so the delivered headers will not be updated.
#
# Uff, I hope this is at least partially understandable :-)

$(dir $(call gb_ExternalPackage_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_ExternalPackage_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_ExternalPackage_get_target,%) :
	$(call gb_Output_announce,$*,$(true),EPK,2)
	touch $@

$(call gb_ExternalPackage_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),EPK,2)
	rm -f $(call gb_ExternalPackage_get_target,$*)

# Create and register a new ExternalPackage
#
# The base directory of the package is the directory of the unpacked
# tarball.
#
# gb_ExternalPackage_ExternalPackage name unpacked
define gb_ExternalPackage_ExternalPackage
$(call gb_ExternalPackage_ExternalPackage_internal,$(1),$(2))

$$(eval $$(call gb_Module_register_target,$(call gb_ExternalPackage_get_target,$(1)),$(call gb_ExternalPackage_get_clean_target,$(1))))

endef

# Create a new ExternalPackage
#
# This function should only be used in implementations of other gbuild
# classes.
#
# gb_ExternalPackage_ExternalPackage_internal name unpacked
define gb_ExternalPackage_ExternalPackage_internal
$(call gb_Package_Package_internal,$(1),$(call gb_UnpackedTarball_get_dir,$(2)))
$(call gb_Package_use_unpacked,$(1),$(2))

$(call gb_ExternalPackage_get_target,$(1)) : $(call gb_Package_get_target,$(1))
$(call gb_ExternalPackage_get_target,$(1)) :| $(dir $(call gb_ExternalPackage_get_target,$(1))).dir
$(call gb_ExternalPackage_get_clean_target,$(1)) : $(call gb_Package_get_clean_target,$(1))

gb_ExternalPackage_UNPACKED_$(1) := $(2)

endef

# Add a file
#
# See gb_Package_add_file for details.
#
# gb_ExternalPackage_add_file package dest src
define gb_ExternalPackage_add_file
$(call gb_Package_add_file,$(1),$(2),$(3))

endef

# Add several files at once
#
# See gb_Package_add_files for details.
#
# gb_ExternalPackage_add_files package destdir file(s)
define gb_ExternalPackage_add_files
$(call gb_Package_add_files,$(1),$(2),$(3))

endef

# Add several files at once
#
# See gb_Package_add_files_with_dir for details.
#
# gb_ExternalPackage_add_files_with_dir package destdir file(s)
define gb_ExternalPackage_add_files_with_dir
$(call gb_Package_add_files_with_dir,$(1),$(2),$(3))

endef

define gb_ExternalPackage__add_file
$(call gb_UnpackedTarball_mark_output_file,$(gb_ExternalPackage_UNPACKED_$(1)),$(2))

endef

# Add an unpacked file
#
# See gb_Package_add_file for details.
#
# gb_ExternalPackage_add_unpacked_file package dest src
define gb_ExternalPackage_add_unpacked_file
$(call gb_Package_add_file,$(1),$(2),$(3))
$(call gb_ExternalPackage__add_file,$(1),$(3))

endef

define gb_ExternalPackage__add_files
$(foreach file,$(2),$(call gb_ExternalPackage__add_file,$(1),$(file)))

endef

# Add several unpacked files at once
#
# See gb_Package_add_files for details.
#
# gb_ExternalPackage_add_unpacked_files package destdir file(s)
define gb_ExternalPackage_add_unpacked_files
$(call gb_Package_add_files,$(1),$(2),$(3))
$(call gb_ExternalPackage__add_files,$(1),$(3))

endef

# Add several unpacked files at once
#
# See gb_Package_add_files_with_dir for details.
#
# gb_ExternalPackage_add_unpacked_files_with_dir package destdir file(s)
define gb_ExternalPackage_add_unpacked_files_with_dir
$(call gb_Package_add_files_with_dir,$(1),$(2),$(3))
$(call gb_ExternalPackage__add_files,$(1),$(3))

endef

# Package files from build of an external project
#
# gb_ExternalPackage_use_external_project package external
define gb_ExternalPackage_use_external_project
$(call gb_Package_use_external_project,$(1),$(2))

endef

# vim: set noet sw=4 ts=4:
