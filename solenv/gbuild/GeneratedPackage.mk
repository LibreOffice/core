# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# class GeneratedPackage

# Enables to deliver whole directories (of generated files) to $(INSTDIR).
#
# GeneratedPackage shall be used as a substitution for Package when the
# names of the produced files are not known in advance (in older times,
# we used Zip in these places). It shall only be used to deliver files
# for installation.
#
# If you know the filenames in advance, use Package. Laziness is not an
# excuse.

gb_GeneratedPackage__get_srcdir = $(lastword $(subst :, ,$(1)))
gb_GeneratedPackage__get_destdir = $(firstword $(subst :, ,$(1)))

define gb_GeneratedPackage__command_cp
mkdir -p $(dir $(INSTDIR)/$(2)) && \
cp -R $(PACKAGE_SOURCEDIR)/$(1) $(INSTDIR)/$(2)
endef

define gb_GeneratedPackage__command
$(call gb_Output_announce,$(2),$(true),GPK,2)
$(call gb_Helper_abbreviate_dirs,\
	rm -rf $(addprefix $(INSTDIR)/,$(foreach pair,$(PACKAGE_DIRS),$(call gb_GeneratedPackage__get_destdir,$(pair)))) && \
	$(foreach pair,$(PACKAGE_DIRS),\
		$(call gb_GeneratedPackage__command_cp,$(call gb_GeneratedPackage__get_srcdir,$(pair)),$(call gb_GeneratedPackage__get_destdir,$(pair))) &&) \
	find \
		$(addprefix $(INSTDIR)/,$(foreach pair,$(PACKAGE_DIRS),$(call gb_GeneratedPackage__get_destdir,$(pair)))) \
		\( -type f -o -type l \) -print \
		> $(1) \
)
endef

define gb_GeneratedPackage__check_dirs
$(if $(PACKAGE_DIRS),,$(call gb_Ouput_error,no dirs were added))
$(foreach pair,$(PACKAGE_DIRS),\
	$(if $(wildcard $(PACKAGE_SOURCEDIR)/$(call gb_GeneratedPackage__get_srcdir,$(pair))),,\
		$(call gb_Output_error,source dir $(PACKAGE_SOURCEDIR)/$(call gb_GeneratedPackage__get_srcdir,$(pair)) does not exist) \
	) \
)

endef

$(dir $(call gb_GeneratedPackage_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_GeneratedPackage_get_target,%))%.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_GeneratedPackage_get_target,%) :
	$(call gb_GeneratedPackage__check_dirs,$*)
	$(call gb_GeneratedPackage__command,$@,$*)

.PHONY : $(call gb_GeneratedPackage_get_clean_target,%)
$(call gb_GeneratedPackage_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),GPK,2)
	rm -rf $(call gb_GeneratedPackage_get_target,$*) $(addprefix $(INSTDIR)/,$(PACKAGE_DIRS))

# Create a generated package.
#
# gb_GeneratedPackage_GeneratedPackage package srcdir
define gb_GeneratedPackage_GeneratedPackage
$(call gb_GeneratedPackage_get_target,$(1)) : PACKAGE_DIRS :=
$(call gb_GeneratedPackage_get_target,$(1)) : PACKAGE_SOURCEDIR := $(2)
$(call gb_GeneratedPackage_get_clean_target,$(1)) : PACKAGE_DIRS :=

$(call gb_GeneratedPackage_get_target,$(1)) : $(gb_Module_CURRENTMAKEFILE)
$(call gb_GeneratedPackage_get_target,$(1)) :| $(dir $(call gb_GeneratedPackage_get_target,$(1))).dir

$$(eval $$(call gb_Module_register_target,$(call gb_GeneratedPackage_get_target,$(1)),$(call gb_GeneratedPackage_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),GeneratedPackage)

endef

# Depend on a custom target.
#
# gb_GeneratedPackage_use_customtarget package custom-target
define gb_GeneratedPackage_use_customtarget
$(call gb_GeneratedPackage_get_target,$(1)) : $(call gb_CustomTarget_get_target,$(2))

endef

# Depend on an unpacked tarball.
#
# gb_GeneratedPackage_use_unpacked package unpacked
define gb_GeneratedPackage_use_unpacked
$(call gb_GeneratedPackage_get_target,$(1)) : $(call gb_UnpackedTarball_get_target,$(2))

endef

# Depend on an external project.
#
# gb_GeneratedPackage_use_external_project package project
define gb_GeneratedPackage_use_external_project
$(call gb_GeneratedPackage_get_target,$(1)) : $(call gb_ExternalProject_get_target,$(2))

endef

# Add a dir to the package.
#
# The srcdir will be copied to $(INSTDIR) as destdir.
#
# gb_GeneratedPackage_add_dir package destdir srcdir
define gb_GeneratedPackage_add_dir
$(call gb_GeneratedPackage_get_target,$(1)) : PACKAGE_DIRS += $(strip $(2)):$(strip $(3))
$(call gb_GeneratedPackage_get_clean_target,$(1)) : PACKAGE_DIRS += $(2)

endef

# vim: set noet sw=4 ts=4:
