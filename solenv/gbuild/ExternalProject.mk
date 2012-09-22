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

# vim: set noet sw=4 ts=4:
