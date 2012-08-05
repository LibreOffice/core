# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 Matúš Kukan <matus.kukan@gmail.com>
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

# Extension class

define gb_Extension_Extension
$(call gb_ExtensionTarget_ExtensionTarget,$(1),$(2))
$(call gb_ExtensionTarget_use_default_license,$(1))

$(call gb_Extension_get_target,$(1)) : $(call gb_ExtensionTarget_get_target,$(1))
$(call gb_Extension_get_clean_target,$(1)) : $(call gb_ExtensionTarget_get_clean_target,$(1))

$(call gb_Deliver_add_deliverable,$(call gb_Extension_get_target,$(1)),$(call gb_ExtensionTarget_get_target,$(1)),$(1))

$$(eval $$(call gb_Module_register_target,$(call gb_Extension_get_target,$(1)),$(call gb_Extension_get_clean_target,$(1))))

endef

# Set platform.
#
# Only use this if the extension is platform-dependent.
define gb_Extension_set_platform
$(call gb_ExtensionTarget_set_platform,$(1),$(2))

endef

define gb_Extension_add_file
$(call gb_ExtensionTarget_add_file,$(1),$(2),$(3))

endef

# Add several files at once
#
# This function avoids the need to specify each file's name twice. The
# files are added directly under specified path in the extension,
# without any subpath. If no path is specified, they are added directly
# to the root dir of the extension.
define gb_Extension_add_files
$(call gb_ExtensionTarget_add_files,$(1),$(2),$(3))

endef

# add a library from the solver; DO NOT use gb_Library_get_target
define gb_Extension_add_library
$(call gb_ExtensionTarget_add_library,$(1),$(2))

endef

define gb_Extension_add_libraries
$(call gb_ExtensionTarget_add_libraries,$(1),$(2))

endef

# add an executable from the solver
define gb_Extension_add_executable
$(call gb_ExtensionTarget_add_executable,$(1),$(2))

endef

define gb_Extension_add_executables
$(call gb_ExtensionTarget_add_executables,$(1),$(2))

endef

# localize .properties file
define gb_Extension_localize_properties
$(call gb_ExtensionTarget_localize_properties,$(1),$(2),$(3))

endef

# localize extension help
define gb_Extension_localize_help
$(call gb_ExtensionTarget_localize_help,$(1),$(2),$(3))

endef

define gb_Extension_use_package
$(call gb_ExtensionTarget_use_package,$(1),$(2))

endef

define gb_Extension_use_packages
$(call gb_ExtensionTarget_use_packages,$(1),$(2))

endef

define gb_Extension_add_package_dependency
$$(call gb_Output_error,\
 gb_Extension_add_package_dependency: use gb_Extension_use_package instead.)
endef

define gb_Extension_add_package_dependencies
$$(call gb_Output_error,\
 gb_Extension_add_package_dependencies: use gb_Extension_use_packages instead.)
endef

# vim: set noet sw=4 ts=4:
