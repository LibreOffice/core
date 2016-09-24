# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# class PackageSet

# Allows to bundle a set of packages under one name.
#
# This is intended to be used by gbuild classes that want to provide a
# filelist for installer, but for implementation reasons have to use
# several Packages internally (e.g., because of different source dirs).

$(dir $(call gb_PackageSet_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_PackageSet_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_PackageSet_get_target,%) :
	$(call gb_Output_announce,$*,$(true),PKS,2)
	cat $(sort $(FILELISTS)) > $@

.PHONY : $(call gb_PackageSet_get_clean_target,%)
$(call gb_PackageSet_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),PKS,2)
	rm -f $(call gb_PackageSet_get_target,$*)

# Create and register a package set.
#
# gb_PackageSet_PackageSet set
define gb_PackageSet_PackageSet
$(call gb_PackageSet_PackageSet_internal,$(1))

$$(eval $$(call gb_Module_register_target,$(call gb_PackageSet_get_target,$(1)),$(call gb_PackageSet_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),PackageSet)

endef

# Create a package set.
#
# gb_PackageSet_PackageSet_internal set
define gb_PackageSet_PackageSet_internal
$(call gb_PackageSet_get_target,$(1)) : FILELISTS :=

$(call gb_PackageSet_get_target,$(1)) :| $(dir $(call gb_PackageSet_get_target,$(1))).dir

endef

# Add a package to the set.
#
# A package can be added more than once.
#
# gb_PackageSet_add_package set package
define gb_PackageSet_add_package
$(call gb_PackageSet_get_target,$(1)) : FILELISTS += $(call gb_Package_get_target,$(2))

$(call gb_PackageSet_get_target,$(1)) : $(call gb_Package_get_target,$(2))
$(call gb_PackageSet_get_clean_target,$(1)) : $(call gb_Package_get_clean_target,$(2))

endef

# Add several packages to the set at once.
#
# gb_PackageSet_add_packages set package(s)
define gb_PackageSet_add_packages
$(foreach package,$(2),$(call gb_PackageSet_add_package,$(1),$(package)))

endef

# vim: set noet sw=4 ts=4:
