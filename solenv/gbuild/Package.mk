# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#


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

# Dummy rule to force rebuild of files that depend on headers in
# $(OUTDIR)/inc (thus the dep files will be rebuild too). Otherwise
# incremental build will fail with 'No rule to make target ...'.
#
# This should be removed again after some time.
$(OUTDIR)/inc/% :
	@true

# Deliver one file to the output dir.
#
# gb_PackagePart_PackagePart destfile source prep-target outdir
define gb_PackagePart_PackagePart
$(4)/$(1) : $(2) | $(dir $(4)/$(1)).dir
$(2) :| $(3)

$(if $(gb_Package_PRESTAGEDIR),\
	$(if $(wildcard $(gb_Package_PRESTAGEDIR)/$(1)),\
		$(call gb_Deliver_add_deliverable,$(4)/$(1),$(gb_Package_PRESTAGEDIR)/$(1),$(3)),\
		$(call gb_Deliver_add_deliverable,$(4)/$(1),$(2),$(3)) \
	),\
	$(call gb_Deliver_add_deliverable,$(4)/$(1),$(2),$(3)) \
)

endef


# Package class

gb_Package_PROGRAMDIRNAME := $(LIBO_BIN_FOLDER)

$(dir $(call gb_Package_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_Package_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

.PHONY : $(call gb_Package_get_clean_target,%)
$(call gb_Package_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),PKG,2)
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),500,$(FILES)) \
	&& cat $${RESPONSEFILE} | xargs $(if $(filter MACOSX,$(OS_FOR_BUILD)),-n 1000) rm -f \
	&& rm -f $${RESPONSEFILE}

$(call gb_Package_get_preparation_target,%) :
	mkdir -p $(dir $@) && touch $@

# NOTE: It is possible that a file has been added to the package more
# than once, so we must drop the duplicates, or Windows installer will
# be unhappy.
# TODO: this is only for convenience for impl. of gbuild classes. There
# should be check that it does not happen in "normal" use, i.e., in
# Package_foo makefiles.
$(call gb_Package_get_target,%) :
	$(call gb_Output_announce,$*,$(true),PKG,2)
	rm -f $@ && \
	mv $(call var2file,$@.tmp,100,$(sort $(FILES))) $@

# for other targets that want to create Packages, does not register at Module
define gb_Package_Package_internal
gb_Package_SOURCEDIR_$(1) := $(2)
gb_Package_OUTDIR_$(1) := $(OUTDIR)
$(call gb_Package_get_target,$(1)) : FILES :=
$(call gb_Package_get_clean_target,$(1)) : FILES := $(call gb_Package_get_target,$(1)) $(call gb_Package_get_preparation_target,$(1))
$(call gb_Package_get_target,$(1)) : $(call gb_Package_get_preparation_target,$(1))
$(call gb_Package_get_target,$(1)) : $(gb_Module_CURRENTMAKEFILE)
$(call gb_Package_get_target,$(1)) :| $(dir $(call gb_Package_get_target,$(1))).dir

endef

define gb_Package_Package
$(if $(filter postprocess% instsetoo_native%,$(1)),,\
    $(call gb_Postprocess_register_target,AllPackages,Package,$(1)))
$(call gb_Package_Package_internal,$(1),$(2))
$$(eval $$(call gb_Module_register_target,$(call gb_Package_get_target,$(1)),$(call gb_Package_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),Package)

endef

# Ensure that the package is defined.
#
# gb_Package__check package
define gb_Package__check
$$(if $$(gb_Package_SOURCEDIR_$(1)),,$$(call gb_Output_error,gb_Package__check: Package $(1) has not been defined))

endef

# Set output dir for the package files.
#
# Default is $(OUTDIR).
#
# gb_Package_set_outdir package outdir
define gb_Package_set_outdir
$(call gb_Package__check,$(1))
gb_Package_OUTDIR_$(1) := $(2)

endef

define gb_Package_add_symbolic_link
$(call gb_Package__check,$(1))
$(if $(strip $(3)),,$(call gb_Output_error,gb_Package_add_symbolic_link requires 3 arguments))
$(call gb_Package_get_target,$(1)) : $$(gb_Package_OUTDIR_$(1))/$(2)
$$(gb_Package_OUTDIR_$(1))/$(2) : $$(dir $$(gb_Package_OUTDIR_$(1))/$(2)).dir
	rm -f $$@ && ln -s $(3) $$@

$(call gb_Package_get_target,$(1)) : FILES += $$(gb_Package_OUTDIR_$(1))/$(2)
$(call gb_Package_get_clean_target,$(1)) : FILES += $$(gb_Package_OUTDIR_$(1))/$(2)

endef

define gb_Package_add_file
$(call gb_Package__check,$(1))
$(if $(strip $(3)),,$(call gb_Output_error,gb_Package_add_file requires 3 arguments))
$(call gb_Package_get_target,$(1)) : $$(gb_Package_OUTDIR_$(1))/$(2)
$(call gb_Package_get_target,$(1)) : FILES += $$(gb_Package_OUTDIR_$(1))/$(2)
$(call gb_Package_get_clean_target,$(1)) : FILES += $$(gb_Package_OUTDIR_$(1))/$(2)
$(call gb_PackagePart_PackagePart,$(2),$$(gb_Package_SOURCEDIR_$(1))/$(3),$(call gb_Package_get_preparation_target,$(1)),$$(gb_Package_OUTDIR_$(1)))

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
$(call gb_Package__check,$(1))
$(if $(strip $(3)),,$(if $(filter 1,$(words $(2))),,$(call gb_Output_error,gb_Package_add_files: it looks like either pkg name or dest. dir is missing)))
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
$(call gb_Package__check,$(1))
$(if $(strip $(3)),,$(if $(filter 1,$(words $(2))),,$(call gb_Output_error,gb_Package_add_files: it looks like either pkg name or dest. dir is missing)))
$(foreach file,$(3),$(call gb_Package_add_file,$(1),$(2)/$(file),$(file)))

endef

# Package files from custom target
define gb_Package_use_custom_target
$(call gb_Package__check,$(1))
$(call gb_Package_get_preparation_target,$(1)) :| $(call gb_CustomTarget_get_target,$(2))

endef

# Package files from unpacked tarball of an external project
define gb_Package_use_unpacked
$(call gb_Package__check,$(1))
$(call gb_Package_get_preparation_target,$(1)) :| $(call gb_UnpackedTarball_get_target,$(2))

endef

# Package files from build of an external project
define gb_Package_use_external_project
$(call gb_Package__check,$(1))
$(call gb_Package_get_preparation_target,$(1)) :| $(call gb_ExternalProject_get_target,$(2))

endef

# vim: set noet sw=4:
