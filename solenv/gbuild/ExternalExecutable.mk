# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# class ExternalExecutable

# ExternalExecutable is a little helper for using executables that might
# either come from system or be built internally.
#
# === Setup ===
#
# An ExternalExecutable command consists of 4 parts:
# * precommand: any command line variables that need to be set
# * internal: unspecified command(s), possibly including calls of gdb,
#   valgrind or icerun
# * executable: the executable, with or without path
# * arguments: command line arguments that are specific for either
#   external or internal call, or that are common for _all_ uses of the
#   executable
#
# The configuration is done in RepositoryExternal.mk by defining function
# gb_ExternalExecutable__register_EXECUTABLE, which can call up to 4
# functions:
# * gb_ExternalExecutable_set_external / gb_ExternalExecutable_set_internal
# * gb_ExternalExecutable_set_precommand
# * gb_ExternalExecutable_add_dependencies
# * gb_ExternalExecutable_add_arguments.
# If neither gb_ExternalExecutable_set_external nor
# gb_ExternalExecutable_set_internal is used, the executable defaults to
# the ExternalExecutable's name. Due to that, nothing needs to be set
# for an external executable in the typical case.
#
# All external executables must be registered (by listing the executable
# name in gb_ExternalExecutable_register_executables call). This is done in
# Repository.mk .
#
# === Usage ===
#
# The call site(s) should always use both of the following functions:
# * gb_ExternalExecutable_get_command: the complete command for the
#   executable
# * gb_ExternalExecutable_get_dependencies: all run-time dependencies
#   needed by the command.

## Infrastructure functions

# The list of registered executables.
gb_ExternalExecutable_REGISTERED_EXECUTABLES :=

define gb_ExternalExecutable__add_executable
$(if $(filter $(executable),$(gb_ExternalExecutable_REGISTERED_EXECUTABLES)),\
    $(call gb_Output_error,external executable $(executable) has already been registered) \
)
gb_ExternalExecutable_REGISTERED_EXECUTABLES += $(1)

endef

# Register one or more external executables.
#
# gb_ExternalExecutable_register_executables executable(s)
define gb_ExternalExecutable_register_executables
$(foreach executable,$(1),$(call gb_ExternalExecutable__add_executable,$(executable)))

endef

define gb_ExternalExecutable__process_registration
$(if $(filter undefined,$(origin gb_ExternalExecutable__register_$(executable))),\
    $(call gb_Output_error,there is no definition for external executable $(executable)) \
)
$(call gb_ExternalExecutable__register_$(executable))

endef

# Collect definitions for registered executables.
#
# The registration functions will be run.
#
# gb_ExternalExecutable_collect_registrations
define gb_ExternalExecutable_collect_registrations
$(eval $(foreach executable,$(gb_ExternalExecutable_REGISTERED_EXECUTABLES),\
	$(call gb_ExternalExecutable__process_registration,$(executable)))
)

endef

define gb_ExternalExecutale__check_registration
$(if $(filter $(1),$(gb_ExternalExecutable_REGISTERED_EXECUTABLES)),,\
	$(call gb_Output_error,external executable $(1) has not been registered) \
)

endef

## Setup functions

# Set the executable as external
#
# Optionally set a specific executable call to use.
# Example:
# 	$(call gb_ExternalExecutable_set_external,genbrk,$(SYSTEM_GENBRK))
#
# gb_ExternalExecutable_set_external executable call?
define gb_ExternalExecutable_set_external
$(if $(2),gb_ExternalExecutable_$(1)_EXECUTABLE := $(2))

endef

# FIXME need to subst in some more $$ in gb_Helper_set_ld_path here - ugly
# but other uses (gb_CppunitTest_CPPTESTPRECOMMAND) require less $$ - ugly
define gb_ExternalExecutable__set_internal
gb_ExternalExecutable_$(1)_EXECUTABLE := $(2)
gb_ExternalExecutable_$(1)_DEPENDENCIES := $(2)
gb_ExternalExecutable_$(1)_PRECOMMAND := $(subst $$,$$$$,$(gb_Helper_set_ld_path))

endef

# Set the executable as internal
#
# Optionally set a specific executable target to use (if the target
# $(gb_Executable_BINDIR_FOR_BUILD)/$(1)$(gb_Executable_EXT_for_build) is
# not suitable).
#
# gb_ExternalExecutable_set_internal executable call?
define gb_ExternalExecutable_set_internal
$(call gb_ExternalExecutable__set_internal,$(1),$(if $(strip $(2)),$(2),$(gb_Executable_BINDIR_FOR_BUILD)/$(1)$(gb_Executable_EXT_for_build)))

endef

# Set pre-command for the executable
#
# This call should set any command line variables needed for the
# executable to run.
#
# gb_ExternalExecutable_set_precommand executable precommand
define gb_ExternalExecutable_set_precommand
gb_ExternalExecutable_$(1)_PRECOMMAND := $(2)

endef

# Add dependencies needed for running the executable
#
# Note that the dependencies should in most (if not all) cases be
# _for_build targets, or there might be problems in cross-compilation
# Specifically, not using _for_build target would mean either:
# * the target is built before the command even if it is not necessary
#   (not really a problem, but might be a nuisance)
# * the build breaks because the target is not known. This might happen
#   if there is a difference in configuration between build and host
#   phases.
#
# gb_ExternalExecutable_add_dependencies executable dependencies
define gb_ExternalExecutable_add_dependencies
gb_ExternalExecutable_$(1)_DEPENDENCIES += $(2)

endef

# Add arguments needed for running the executable
#
# This should only contain arguments that differ between external and
# internal executable call or that are common for all call sites.
#
# gb_ExternalExecutable_add_arguments executable arguments
define gb_ExternalExecutable_add_arguments
gb_ExternalExecutable_$(1)_ARGUMENTS += $(2)

endef

## User functions

gb_ExternalExecutable__get_internal := $(ICECREAM_RUN)

define gb_ExternalExecutable__get_executable
$(if $(gb_ExternalExecutable_$(1)_EXECUTABLE),$(gb_ExternalExecutable_$(1)_EXECUTABLE),$(1))
endef

define gb_ExternalExecutable__get_command
$(call gb_ExternalExecutale__check_registration,$(1))
$(gb_ExternalExecutable_$(1)_PRECOMMAND) \
	$(call gb_ExternalExecutable__get_internal,$(1)) \
	$(call gb_ExternalExecutable__get_executable,$(1)) \
	$(gb_ExternalExecutable_$(1)_ARGUMENTS)
endef

# Return the command for running an external executable.
#
# The command includes the required shell variables, if any (e.g.,
# LD_LIBRARY_PATH for internally built executables), and icerun wrapper
# for limiting the maximum number of processes, if available.
#
# gb_ExternalExecutable_get_command executable
define gb_ExternalExecutable_get_command
$(strip $(call gb_ExternalExecutable__get_command,$(1)))
endef

define gb_ExternalExecutable__get_dependencies
$(call gb_ExternalExecutale__check_registration,$(1))
$(gb_ExternalExecutable_$(1)_DEPENDENCIES)
endef

# Return the dependencies needed for running an external executable.
#
# gb_ExternalExecutable_get_dependencies executable
define gb_ExternalExecutable_get_dependencies
$(strip $(call gb_ExternalExecutable__get_dependencies,$(1)))
endef

# vim: set noet sw=4 ts=4:
