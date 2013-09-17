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
#
# Note: An ExternalPackage object can be used in functions that expect a
# Package (e.g., gb_LinkTarget_use_package(s)).

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
$(call gb_Helper_make_userfriendly_targets,$(1),ExternalPackage)

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

# Set output dir for the package's files.
#
# Default is $(OUTDIR).
#
# gb_ExternalPackage_set_outdir package outdir
define gb_ExternalPackage_set_outdir
$(call gb_Package_set_outdir,$(1),$(2))

endef

# Mark a source file to be used outside of this module
#
# This results in the timestamp of the file being updated, so a possible
# change is recognized properly by other files depending on it.
#
# gb_ExternalPackage_mark_generated_file package file
define gb_ExternalPackage_mark_generated_file
$(call gb_UnpackedTarball_get_dir,$(gb_ExternalPackage_UNPACKED_$(1)))/$(2) : \
		$(call gb_ExternalProject_get_target,$(gb_ExternalPackage_PROJECT_$(1)))
$(if $(suffix $(2)),\
	$(call gb_UnpackedTarbal__ensure_pattern_rule,$(gb_ExternalPackage_UNPACKED_$(1)),$(suffix $(2))),\
	$(call gb_UnpackedTarbal__make_file_rule,$(gb_ExternalPackage_UNPACKED_$(1)),$(2)) \
)

endef

# Mark several source files to be used outside of this module
#
# gb_ExternalProject_mark_generated_files package file(s)
define gb_ExternalPackage_mark_generated_files
$(foreach file,$(2),$(call gb_ExternalPackage_mark_generated_file,$(1),$(file)))

endef

define gb_ExternalPackage_add_symbolic_link
$(call gb_Package_add_symbolic_link,$(1),$(2),$(3))

endef

# Add a file
#
# See gb_Package_add_file for details.
#
# gb_ExternalPackage_add_file package dest src
define gb_ExternalPackage_add_file
$(call gb_ExternalPackage_mark_generated_file,$(1),$(3))
$(call gb_Package_add_file,$(1),$(2),$(3))

endef

# Add several files at once
#
# See gb_Package_add_files for details.
#
# gb_ExternalPackage_add_files package destdir file(s)
define gb_ExternalPackage_add_files
$(call gb_ExternalPackage_mark_generated_files,$(1),$(3))
$(call gb_Package_add_files,$(1),$(2),$(3))

endef

# Add several files at once
#
# See gb_Package_add_files_with_dir for details.
#
# gb_ExternalPackage_add_files_with_dir package destdir file(s)
define gb_ExternalPackage_add_files_with_dir
$(call gb_ExternalPackage_mark_generated_files,$(1),$(3))
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

$(if $(gb_ExternalPackage_PROJECT_$(1)),$(call gb_Output_error,gb_ExternalPackage_use_external_project: only one project allowed))
gb_ExternalPackage_PROJECT_$(1) := $(2)

endef

# gb_ExternalPackage__add_file_for_install package dest dest-inst src
define gb_ExternalPackage__add_file_for_install
$(call gb_ExternalPackage_add_file,$(1),$(2),$(4))

$(call gb_Helper_install,$(call gb_ExternalPackage_get_target,$(1)), \
	$(gb_INSTROOT)/$(3), \
	$(call gb_UnpackedTarball_get_dir,$(gb_ExternalPackage_UNPACKED_$(1)))/$(4))

$(call gb_UnpackedTarball_get_dir,$(gb_ExternalPackage_UNPACKED_$(1)))/$(4) :| \
	$(call gb_Package_get_preparation_target,$(1))

endef

# Add a (dynamic) library that is a part of the installation.
#
# This function is very similar to gb_ExternalPackage_add_file, except
# that it also allows to deliver the library to its proper place in
# $(gb_INSTROOT).
#
# The last argument is the name under which the library was registered.
# It is used to determine layer, if the library is not in layer OOO.
#
# gb_ExternalPackage_add_library_for_install package dest src library?
define gb_ExternalPackage_add_library_for_install
$(call gb_ExternalPackage__add_file_for_install,$(1),$(2),$(if $(4),$(call gb_Library_get_instdir,$(4)),$(gb_Package_PROGRAMDIRNAME))/$(notdir $(2)),$(3))

endef

# Add several libraries for install at once.
#
# gb_ExternalPackage_add_libraries_for_install package destdir file(s)
define gb_ExternalPackage_add_libraries_for_install
$(foreach file,$(3),$(call gb_ExternalPackage_add_library_for_install,$(1),$(2)/$(notdir $(file)),$(file)))

endef

# Add a jar that is a part of the installation.
#
# This function works just like to gb_ExternalPackage_add_file, except
# that it also allows to deliver the jar to its proper place in
# $(gb_INSTROOT).
#
# gb_ExternalPackage_add_jar_for_install package dest src
define gb_ExternalPackage_add_jar_for_install
$(call gb_ExternalPackage__add_file_for_install,$(1),$(2),$(gb_Package_PROGRAMDIRNAME)/classes/$(notdir $(2)),$(3))

endef

# Add several jars for install at once.
#
# gb_ExternalPackage_add_jars_for_install package destdir file(s)
define gb_ExternalPackage_add_jars_for_install
$(foreach file,$(3),$(call gb_ExternalPackage_add_jar_for_install,$(1),$(2)/$(notdir $(file)),$(file)))

endef

# vim: set noet sw=4 ts=4:
