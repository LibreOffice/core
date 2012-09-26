# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************


# PackagePart class

# a pattern rule with multiple targets is actually executed only once for each
# match, so define only pattern rules with one target here
# the .dir is for make 3.81, which ignores trailing /
define gb_PackagePart__rule
$(1)/.dir :
	$$(if $$(wildcard $$(dir $$@)),,mkdir -p $$(dir $$@))
$(1)/%/.dir :
	$$(if $$(wildcard $$(dir $$@)),,mkdir -p $$(dir $$@))
$(1)/% :
	$$(call gb_Deliver_deliver,$$<,$$@)
endef

$(foreach destination,$(call gb_PackagePart_get_destinations),$(eval \
  $(call gb_PackagePart__rule,$(destination))))

define gb_PackagePart_PackagePart
$(OUTDIR)/$(1) : $(2) | $(dir $(OUTDIR)/$(1)).dir
$(2) :| $(3)
$(call gb_Deliver_add_deliverable,$(OUTDIR)/$(1),$(2),$(3))
endef


# Package class

.PHONY : $(call gb_Package_get_clean_target,%)
$(call gb_Package_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),PKG,2)
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),500,$(FILES)) \
	&& cat $${RESPONSEFILE} | xargs $(if $(filter MACOSX,$(OS_FOR_BUILD)),-n 1000) rm -f \
 	&& rm -f $${RESPONSEFILE}

$(call gb_Package_get_preparation_target,%) :
	mkdir -p $(dir $@) && touch $@

$(call gb_Package_get_target,%) :
	$(call gb_Output_announce,$*,$(true),PKG,2)
	mkdir -p $(dir $@) && touch $@

# for other targets that want to create Packages, does not register at Module
define gb_Package_Package_internal
gb_Package_SOURCEDIR_$(1) := $(2)
$(call gb_Package_get_clean_target,$(1)) : FILES := $(call gb_Package_get_target,$(1)) $(call gb_Package_get_preparation_target,$(1))
$(call gb_Package_get_target,$(1)) : $(call gb_Package_get_preparation_target,$(1))

endef

define gb_Package_Package
$(call gb_Package_Package_internal,$(1),$(2))
$$(eval $$(call gb_Module_register_target,$(call gb_Package_get_target,$(1)),$(call gb_Package_get_clean_target,$(1))))

endef

define gb_Package_add_file
$(call gb_Package_get_target,$(1)) : $(OUTDIR)/$(2)
$(call gb_Package_get_clean_target,$(1)) : FILES += $(OUTDIR)/$(2)
$(call gb_PackagePart_PackagePart,$(2),$$(gb_Package_SOURCEDIR_$(1))/$(3),$(call gb_Package_get_preparation_target,$(1)))

endef

# Adds several files at once.
#
# Files are copied directly into the specified directory.
#
# Example:
# $(eval $(call gb_Package_Package,foo_inc,$(SRCDIR)/foo/inc))
# $(eval $(call gb_Package_add_files,foo_inc,inc/foo,foo/bar/foo.hxx))
# # -> inc/foo/foo.hxx
define gb_Package_add_files
$(foreach file,$(3),$(call gb_Package_add_file,$(1),$(2)/$(notdir $(file)),$(file)))

endef

# Adds several files at once.
#
# Files are copied including subdirectories.
#
# Example:
# $(eval $(call gb_Package_Package,foo_inc,$(SRCDIR)/foo/inc))
# $(eval $(call gb_Package_add_files,foo_inc,inc,foo/bar/foo.hxx))
# # -> inc/foo/bar/foo.hxx
define gb_Package_add_files_with_dir
$(foreach file,$(3),$(call gb_Package_add_file,$(1),$(2)/$(file),$(file)))

endef

# Package files from unpacked tarball of an external project
define gb_Package_use_unpacked
$(call gb_Package_get_preparation_target,$(1)) :| $(call gb_UnpackedTarball_get_target,$(2))

endef

# Package files from build of an external project
define gb_Package_use_external_project
$(call gb_Package_get_preparation_target,$(1)) :| $(call gb_ExternalProject_get_target,$(2))

endef

# vim: set noet sw=4:
