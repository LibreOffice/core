# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# class ExternalProject

# Handles build of an external project

# Build of an external typically uses three gbuild classes:
# ExternalProject, ExternalPackage or Package, and UnpackedTarball. The
# first step is to prepare sources using UnpackedTarball. The tarball is
# passed to an ExternalProject, which handles the build proper and the
# results are delivered by an ExternalPackage (or Package, again;
# Package is sufficient if no files--e.g., headers--from the unpacked
# tarball need to be delivered.)
#
# ExternalProject has no gbuild abstraction for actually building the
# external code, so it is necessary to define rule(s) and recipe(s) to
# handle it. It does not matter if there are several rules handling
# separate phases of the build (e.g., configure, build, install) or if
# the whole build is handled by one rule.
#
# ExternalProject uses two directories during the build: state dir
# serves to keep file targets that mark state of the build progress
# (e.g., "configure done", "build done") and the targets are accessible
# via gb_ExternalProject_get_state_target. It is highly advised to
# register them using gb_ExternalProject_register_targets. The second
# directory is work dir, accessible only from recipes via variable
# $(EXTERNAL_WORKDIR).

$(dir $(call gb_ExternalProject_get_statedir,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_ExternalProject_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_ExternalProject_get_preparation_target,%) :
	touch $@

$(call gb_ExternalProject_get_target,%) :
	$(call gb_Output_announce,$*,$(true),PRJ,3)
	touch $@

.PHONY : $(call gb_ExternalProject_get_clean_target,%)
$(call gb_ExternalProject_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),PRJ,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf \
			$(call gb_ExternalProject_get_target,$*) \
			$(call gb_ExternalProject_get_statedir,$*) \
	)

# Define a new external project
#
# gb_ExternalProject_ExternalProject project
define gb_ExternalProject_ExternalProject
$(call gb_ExternalProject_get_target,$(1)) : EXTERNAL_WORKDIR :=

$(call gb_ExternalProject_get_preparation_target,$(1)) :| $(dir $(call gb_ExternalProject_get_target,$(1))).dir
$(call gb_ExternalProject_get_target,$(1)) : $(call gb_ExternalProject_get_preparation_target,$(1))
$(call gb_ExternalProject_get_target,$(1)) :| $(dir $(call gb_ExternalProject_get_target,$(1))).dir

$$(eval $$(call gb_Module_register_target,$(call gb_ExternalProject_get_target,$(1)),$(call gb_ExternalProject_get_clean_target,$(1))))

endef

# Use unpacked tarball as source for build
#
# gb_ExternalProject_use_unpacked project unpacked
define gb_ExternalProject_use_unpacked
$(call gb_ExternalProject_get_target,$(1)) : EXTERNAL_WORKDIR := $(call gb_UnpackedTarball_get_dir,$(2))

$(call gb_ExternalProject_get_preparation_target,$(1)) : $(call gb_UnpackedTarball_get_target,$(2))
$(call gb_ExternalProject_get_clean_target,$(1)) : $(call gb_UnpackedTarball_get_clean_target,$(2))

endef

# Register a target in state directory
#
# This function defines proper dependencies for the target to ensure
# that:
# * the main target is updated if this target is updated
# * this target is updated if the unpacked tarball has changed.
#
# gb_ExternalProject_register_target project target
define gb_ExternalProject_register_target
$(call gb_ExternalProject_get_target,$(1)) : $(call gb_ExternalProject_get_state_target,$(1),$(2))
$(call gb_ExternalProject_get_state_target,$(1),$(2)) : $(call gb_ExternalProject_get_preparation_target,$(1))
$(call gb_ExternalProject_get_state_target,$(1),$(2)) :| $(dir $(call gb_ExternalProject_get_state_target,$(1),$(2))).dir

endef

# Register several targets at once
#
# gb_ExternalProject_register_targets project target(s)
define gb_ExternalProject_register_targets
$(foreach target,$(2),$(call gb_ExternalProject_register_target,$(1),$(target)))

endef

# Make an external Project depend on another ExternalProject
define gb_ExternalProject_use_external_project
$(call gb_ExternalProject_get_preparation_target,$(1)) : $(call gb_ExternalProject_get_target,$(2))

endef

# Make an ExternalProject depend on an external
#
# this forwards to functions that must be defined in RepositoryExternal.mk.
# $(eval $(call gb_ExternalProject_use_external,library,external))
define gb_ExternalProject_use_external
$(if $(filter undefined,$(origin gb_ExternalProject__use_$(2))),\
  $(error gb_ExternalProject_use_external: unknown external: $(2)),\
  $(call gb_ExternalProject__use_$(2),$(1)))
endef

define gb_ExternalProject_use_externals
$(foreach external,$(2),$(call gb_ExternalProject_use_external,$(1),$(external)))
endef

# Make an external project depend on a package
#
# This is most useful for depending on output files created by another
# ExternalProject.
#
# gb_ExternalProject_use_package external package
define gb_ExternalProject_use_package
$(call gb_ExternalProject_get_preparation_target,$(1)) : $(call gb_Package_get_target,$(2))

endef

# Make an external project depend on several packages at once
#
# gb_ExternalProject_use_packages external package(s)
define gb_ExternalProject_use_packages
$(foreach package,$(2),$(call gb_ExternalProject_use_package,$(1),$(package)))

endef

# Make an external project depend on a StaticLibrary
#
# Realistically there are some externals that do not have a usable build
# system, and other externals that do may depend on those.
#
# gb_ExternalProject_use_static_libraries external staticlibraries
define gb_ExternalProject_use_static_libraries
ifneq (,$$(filter-out $(gb_StaticLibrary_KNOWNLIBS),$(2)))
$$(eval $$(call gb_Output_info, currently known static libraries are: $(sort $(gb_StaticLibrary_KNOWNLIBS)),ALL))
$$(eval $$(call gb_Output_error,Cannot link against static library/libraries $$(filter-out $(gb_StaticLibrary_KNOWNLIBS),$(2)). Static libraries must be registered in Repository.mk))
endif

$(call gb_ExternalProject_get_preparation_target,$(1)) : \
	$(foreach lib,$(2),$(call gb_StaticLibrary_get_target,$(lib)))

endef

# Make an external project depend on a Library
#
# Realistically there are some externals that do not have a usable build
# system, and other externals that do may depend on those.
#
# gb_ExternalProject_use_libraries external libraries
define gb_ExternalProject_use_libraries
ifneq (,$$(filter-out $(gb_Library_KNOWNLIBS),$(2)))
$$(eval $$(call gb_Output_info,currently known libraries are: $(sort $(gb_Library_KNOWNLIBS)),ALL))
$$(eval $$(call gb_Output_error,Cannot link against library/libraries $$(filter-out $(gb_Library_KNOWNLIBS),$(2)). Libraries must be registered in Repository.mk))
endif
ifneq (,$$(filter $$(gb_MERGEDLIBS),$(2)))
$$(eval $$(call gb_Output_error,Cannot link against library/libraries $$(filter $$(gb_MERGEDLIBS),$(2)) because they are merged.))
endif

$(call gb_ExternalProject_get_preparation_target,$(1)) : \
	$(foreach lib,$(2),$(call gb_Library_get_target,$(lib)))

endef

# vim: set noet sw=4 ts=4:
