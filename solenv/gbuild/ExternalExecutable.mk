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
# The configuration for an external executable is done in
# RepositoryExternal.mk by defining function
# gb_ExternalExecutable__register_EXECUTABLE, which should set up to 4
# variables. The variables are (presuming the executable is named foo):

# * gb_ExternalExecutable__foo_COMMAND
#   Defines the complete command for the executable. This must include
#   shell variables, if required. Defaults to
#   $(gb_ExternalExecutable__foo_PRECOMMAND) $(ICECREAM_RUN) $(gb_ExternalExecutable__foo_TARGET)
#   (if gb_ExternalExecutable__foo_TARGET is not defined, plain "foo" is substituted).
#
# * gb_ExternalExecutable__foo_DEPS
#   The complete dependencies needed for running the executable. Only
#   useful for internally built executables to make sure all data,
#   config. files etc. are present. Defaults to
#   $(gb_ExternalExecutable__foo_TARGET).
#
# * gb_ExternalExecutable__foo_PRECOMMAND
#   Any variables that need to be set for running the executable.
#   Typical example is use of $(gb_Helper_set_ld_path) for internally
#   built executables.
#
# * gb_ExternalExecutable__foo_TARGET
#   A full path to the executable. Typical definition for internally built
#   executable is $(call gb_Executable_get_target_for_build,foo).
#
# Because of the particular choice of the defaults, there is typically
# nothing that needs to be set for a system executable. 
#
# All external executables must be registered (by listing the executable
# name in gb_ExternalExecutable_register_executables call). This is done in
# Repository.mk .

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
$(foreach executable,$(gb_ExternalExecutable_REGISTERED_EXECUTABLES),$(call gb_ExternalExecutable__process_registration,$(executable)))

endef

define gb_ExternalExecutale__check_registration
$(if $(filter $(1),$(gb_ExternalExecutable_REGISTERED_EXECUTABLES)),,\
	$(call gb_Output_error,external executable $(1) has not been registered) \
)

endef

define gb_ExternalExecutable__get_command
$(call gb_ExternalExecutale__check_registration,$(1))
$(if $(filter undefined,$(origin gb_ExternalExecutable__$(1)_COMMAND)) \
    ,$(gb_ExternalExecutable__$(1)_PRECOMMAND) $(ICECREAM_RUN) \
    	$(if $(gb_ExternalExecutable__$(1)_TARGET),$(gb_ExternalExecutable__$(1)_TARGET),$(1)) \
    ,$(gb_ExternalExecutable__$(1)_COMMAND) \
)
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

define gb_ExternalExecutable__get_deps
$(call gb_ExternalExecutale__check_registration,$(1))
$(if $(filter undefined,$(origin gb_ExternalExecutable__$(1)_DEPS)) \
    ,$(gb_ExternalExecutable__$(1)_TARGET) \
    ,$(gb_ExternalExecutable__$(1)_DEPS) \
)
endef

# Return the deps needed for running an external executable.
#
# gb_ExternalExecutable_get_deps executable
define gb_ExternalExecutable_get_deps
$(strip $(call gb_ExternalExecutable__get_deps,$(1)))
endef

# vim: set noet sw=4 ts=4:
