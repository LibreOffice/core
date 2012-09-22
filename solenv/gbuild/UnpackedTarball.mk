# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# UnpackedTarget class

# Handles unpacking of a tarball

# platform
#  gb_UnpackedTarget_TARFILE_LOCATION

gb_UnpackedTarget_STRIP_COMPONENTS_TAR_DEFAULT := 1
gb_UnpackedTarget_STRIP_COMPONENTS_ZIP_DEFAULT := 0

# gb_UnpackedTarget__get_strip_components target strip-components?
define gb_UnpackedTarget__get_strip_components
$(strip $(if $(2),\
	$(2),\
	$(if $(filter zip,$(suffix $(1))),\
		$(gb_UnpackedTarget_STRIP_COMPONENTS_ZIP_DEFAULT),\
		$(gb_UnpackedTarget_STRIP_COMPONENTS_TAR_DEFAULT) \
	) \
))
endef

define gb_UnpackedTarget__command_untar
$(GNUTAR) \
	-x \
	$(3) \
	-C $(UNPACKED_DIR) \
	$(STRIP_COMPONENTS)=$(UNPACKED_STRIP_COMPONENTS) \
	-f $(UNPACKED_TARBALL)
endef

define gb_UnpackedTarget__command_unzip
unzip \
	-qq \
	-d $(UNPACKED_DIR) $(UNPACKED_TARBALL) && \
$(if $(filter-out 0,$(UNPACKED_STRIP_COMPONENTS)),\
	UNZIP_DIR=`ls $(UNPACKED_DIR)` && \
	mv $(UNPACKED_DIR)/$$UNZIP_DIR/* $(UNPACKED_DIR) && \
	rmdir $(UNPACKED_DIR)/$$UNZIP_DIR \
)
endef

define gb_UnpackedTarget__command
$(call gb_Output_announce,$(notdir $(2)),$(true),UPK,1)
$(call gb_Helper_abbreviate_dirs,\
	$(if $(wildcard $(UNPACKED_DIR)),rm -rf $(UNPACKED_DIR) &&) \
	mkdir -p $(UNPACKED_DIR) && \
	$(call gb_UnpackedTarget__command_$(1),$(2),$(3),$(4)) && \
	touch $(2) \
)
endef

$(dir $(call gb_UnpackedTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_UnpackedTarget_get_target,%).tar.bz2 :
	$(call gb_UnpackedTarget__command,untar,$@,$*,-j)

$(call gb_UnpackedTarget_get_target,%).tar.gz :
	$(call gb_UnpackedTarget__command,untar,$@,$*,-z)

$(call gb_UnpackedTarget_get_target,%).tgz :
	$(call gb_UnpackedTarget__command,untar,$@,$*,-z)

$(call gb_UnpackedTarget_get_target,%).zip :
	$(call gb_UnpackedTarget__command,unzip,$@,$*)

.PHONY : $(call gb_UnpackedTarget_get_clean_target,%)
$(call gb_UnpackedTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),UPK,1)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_UnpackedTarget_get_target,$*) \
	)

# gb_UnpackedTarget_UnpackedTarget target outdir strip-components?
define gb_UnpackedTarget_UnpackedTarget
$(call gb_UnpackedTarget_get_target,$(1)) : UNPACKED_DIR := $(2)
$(call gb_UnpackedTarget_get_target,$(1)) : UNPACKED_TARBALL := $(gb_UnpackedTarget_TARFILE_LOCATION)/$(1)
$(call gb_UnpackedTarget_get_target,$(1)) : UNPACKED_STRIP_COMPONENTS := $(call gb_UnpackedTarget__get_strip_components,$(1),$(3))

$(call gb_UnpackedTarget_get_target,$(1)) : $(gb_UnpackedTarget_TARFILE_LOCATION)/$(1)
$(call gb_UnpackedTarget_get_target,$(1)) :| $(dir $(call gb_UnpackedTarget_get_target,$(1))).dir

endef

# UnpackedTarball class

# Handles unpacking and patching of an external project
#
# The unpacked dir is recreated every time one of the patches, copied
# files or the makefile changes.

# This is what dmake patches use. Once all external modules are
# converted, it is better to be changed to 1.
gb_UnpackedTarball_PATCHLEVEL_DEFAULT := 3

gb_UnpackedTarball_CONVERTTODOS = \
	$(gb_AWK) 'sub("$$","\r")' $(1) > $(1).TEMP && mv $(1).TEMP $(1)
gb_UnpackedTarball_CONVERTTOUNIX = \
	tr -d '\r' < $(1) > $(1).TEMP && mv $(1).TEMP $(1)

define gb_UnpackedTarball__copy_files_impl
$(if $(1),\
	&& cp $(firstword $(1)) $(firstword $(2)) \
	$(call gb_UnpackedTarball__copy_files_impl,$(wordlist 2,$(words $(1)),$(1)),$(wordlist 2,$(words $(2)),$(2))) \
)
endef

# Drop leading &&
define gb_UnpackedTarball__copy_files_fix
$(wordlist 2,$(words $(1)),$(1))
endef

define gb_UnpackedTarball__copy_files
$(call gb_UnpackedTarball__copy_files_fix,$(call gb_UnpackedTarball__copy_files_impl,$(1),$(2)))
endef

define gb_UnpackedTarball__command
$(call gb_Output_announce,$(2),$(true),PAT,2)
$(call gb_Helper_abbreviate_dirs,\
	( \
		cd $(3) && \
		$(foreach file,$(UNPACKED_FIX_EOL),$(call gb_UnpackedTarball_CONVERTTOUNIX,$(file)) && ) \
		$(if $(UNPACKED_PATCHES),\
			for p in $(UNPACKED_PATCHES); do \
				$(GNUPATCH) -s -p$(UNPACKED_PATCHLEVEL) < "$$p" || exit 1;\
			done && \
		) \
		$(foreach file,$(UNPACKED_FIX_EOL),$(call gb_UnpackedTarball_CONVERTTODOS,$(file)) && ) \
		$(if $(UNPACKED_FILES),\
			mkdir -p $(sort $(dir $(UNPACKED_DESTFILES))) && \
			$(call gb_UnpackedTarball__copy_files,$(UNPACKED_FILES),$(UNPACKED_DESTFILES)) && \
		) \
		$(if $(UNPACKED_SUBDIRS),\
			cp -rf $(UNPACKED_SUBDIRS) $(gb_EXTERNAL_HEADERS_DIR) && \
		) \
		$(if $(UNPACKED_POST_ACTION),\
			$(UNPACKED_POST_ACTION) && \
		) \
		touch $(1) \
	) || \
	( \
		touch $(call gb_UnpackedTarball_get_preparation_target,$(2)) && \
		exit 1 \
	)
)
endef

$(dir $(call gb_UnpackedTarball_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_UnpackedTarball_get_preparation_target,%) :
	touch $@

$(call gb_UnpackedTarball_get_target,%) :
	$(call gb_UnpackedTarball__command,$@,$*,$(call gb_UnpackedTarball_get_dir,$*))

$(call gb_UnpackedTarball_get_final_target,%) :
	touch $@

.PHONY : $(call gb_UnpackedTarball_get_clean_target,%)
$(call gb_UnpackedTarball_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),PAT,2)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf \
			$(call gb_UnpackedTarball_get_target,$*) \
			$(call gb_UnpackedTarball_get_preparation_target,$*) \
			$(call gb_UnpackedTarball_get_dir,$*) \
			$(foreach subdir,$(UNPACKED_SUBDIRS),$(gb_EXTERNAL_HEADERS_DIR)/$(subdir)) \
	)

define gb_UnpackedTarball__get_makefile
$(lastword $(foreach makefile,$(MAKEFILE_LIST),$(if $(filter UnpackedTarball_%,$(notdir $(makefile))),$(makefile))))

endef

# Initialize unpacked tarball
define gb_UnpackedTarball_UnpackedTarball_internal
$(call gb_UnpackedTarball_get_target,$(1)) : UNPACKED_DESTFILES :=
$(call gb_UnpackedTarball_get_target,$(1)) : UNPACKED_FILES :=
$(call gb_UnpackedTarball_get_target,$(1)) : UNPACKED_FIX_EOL :=
$(call gb_UnpackedTarball_get_target,$(1)) : UNPACKED_PATCHES :=
$(call gb_UnpackedTarball_get_target,$(1)) : UNPACKED_PATCHLEVEL := $(gb_UnpackedTarball_PATCHLEVEL_DEFAULT)
$(call gb_UnpackedTarball_get_target,$(1)) : UNPACKED_POST_ACTION :=
$(call gb_UnpackedTarball_get_target,$(1)) : UNPACKED_SUBDIRS :=
$(call gb_UnpackedTarball_get_clean_target,$(1)) : UNPACKED_SUBDIRS :=

$(call gb_UnpackedTarball_get_preparation_target,$(1)) : $(call gb_UnpackedTarball__get_makefile)
$(call gb_UnpackedTarball_get_preparation_target,$(1)) :| $(dir $(call gb_UnpackedTarball_get_target,$(1))).dir
$(call gb_UnpackedTarball_get_target,$(1)) : $(call gb_UnpackedTarball_get_preparation_target,$(1))
$(call gb_UnpackedTarball_get_target,$(1)) :| $(dir $(call gb_UnpackedTarball_get_target,$(1))).dir
$(call gb_UnpackedTarball_get_final_target,$(1)) : $(call gb_UnpackedTarball_get_target,$(1))

private gb_UnpackedTarball_PATTERN_RULES_$(1) :=

endef

# Define a new unpacked tarball
define gb_UnpackedTarball_UnpackedTarball
$(call gb_UnpackedTarball_UnpackedTarball_internal,$(1))

$$(eval $$(call gb_Module_register_target,$(call gb_UnpackedTarball_get_final_target,$(1)),$(call gb_UnpackedTarball_get_clean_target,$(1))))

endef

# Convert line ending from dos to unix style for selected files
#
# This is done before applying patches, because patches expect unix
# style line ending, and the files are converted back after that. The
# files are relative to the unpacked path.
#
# gb_UnpackedTarball_fix_end_of_line unpacked file(s)
define gb_UnpackedTarball_fix_end_of_line
$(call gb_UnpackedTarball_get_target,$(1)) : UNPACKED_FIX_EOL += $(addprefix $(call gb_UnpackedTarball_get_dir,$(1))/,$(2))

endef

# Set tarball name
#
# gb_UnpackedTarball_set_tarball unpacked tarball-name
define gb_UnpackedTarball_set_tarball
$(call gb_UnpackedTarget_UnpackedTarget,$(2),$(call gb_UnpackedTarball_get_dir,$(1)),$(3))
$(call gb_UnpackedTarball_get_target,$(1)) : $(call gb_UnpackedTarget_get_target,$(2))
$(call gb_UnpackedTarball_get_clean_target,$(1)) : $(call gb_UnpackedTarget_get_clean_target,$(2))
$(call gb_UnpackedTarget_get_target,$(2)) : $(call gb_UnpackedTarball_get_preparation_target,$(1))

endef

# Set patch level to be used for all patches
#
# The default value is 3 to be able to work with current dmake patches.
#
# gb_UnpackedTarball_set_patchlevel unpacked level
define gb_UnpackedTarball_set_patchlevel
$(call gb_UnpackedTarball_get_target,$(1)) : UNPACKED_PATCHLEVEL := $(2)

endef

# Add a patch to be applied on the unpacked files
#
# gb_UnpackedTarball_add_patch unpacked patch
define gb_UnpackedTarball_add_patch
$(call gb_UnpackedTarball_get_target,$(1)) : UNPACKED_PATCHES += $(SRCDIR)/$(2)
$(call gb_UnpackedTarball_get_preparation_target,$(1)) : $(SRCDIR)/$(2)

endef

# Add several patches at once
#
# gb_UnpackedTarball_add_patches unpacked patch(es)
define gb_UnpackedTarball_add_patches
$(foreach patch,$(2),$(call gb_UnpackedTarball_add_patch,$(1),$(patch)))

endef

# Add a file from source dir to the unpacked dir
#
# This function should not be used for overwriting existing files--use a
# patch for that purpose.
#
# gb_UnpackedTarball_add_file unpacked destfile file
define gb_UnpackedTarball_add_file
$(call gb_UnpackedTarball_get_target,$(1)) : UNPACKED_FILES += $(SRCDIR)/$(3)
$(call gb_UnpackedTarball_get_target,$(1)) : UNPACKED_DESTFILES += $(call gb_UnpackedTarball_get_dir,$(1))/$(2)
$(call gb_UnpackedTarball_get_preparation_target,$(1)) : $(SRCDIR)/$(3)

endef

# Add several files(s) from source dir to the unpacked dir
#
# The files are added into the specified subdir.
#
# gb_UnpackedTarball_add_files unpacked subdir file(s)
define gb_UnpackedTarball_add_files
$(foreach file,$(3),$(call gb_UnpackedTarball_add_file,$(1),$(2)/$(notdir $(file)),$(file)))

endef

# Copy header files from the unpacked subdir(s) to solver
#
# Used for boost.
# For other external headers, include path is added in RepositoryExternal.mk
#
# gb_UnpackedTarball_copy_header_files unpacked subdir(s)
define gb_UnpackedTarball_copy_header_files
$(call gb_UnpackedTarball_get_target,$(1)) :| $(gb_EXTERNAL_HEADERS_DIR)/.dir
$(call gb_UnpackedTarball_get_target,$(1)) : UNPACKED_SUBDIRS := $(2)
$(call gb_UnpackedTarball_get_clean_target,$(1)) : UNPACKED_SUBDIRS := $(2)

endef

# Set arbitrary shell command to be run during unpack
#
# The command is run at the very end: after patching, C++ extension
# mangling and copying additional files in. The command is run in the
# unpacked directory. If more than one command is used, care should be
# taken that the whole command fails if either of the sub-commands
# fails.
#
# NOTE: This is a bit hackish, but it is the easiest way to move files
# around or delete files (typically because the file causes build
# problems in the original location, c.f. clucene). This is doable by
# using -E with patch (we use GNU patch anyway), but it would mean an
# additional patch to maintain....
#
# gb_UnpackedTarball_set_post_action unpacked shell-command
define gb_UnpackedTarball_set_post_action
$(call gb_UnpackedTarball_get_target,$(1)) : UNPACKED_POST_ACTION := $(strip $(2))

endef

define gb_UnpackedTarbal__make_pattern_rule
$(call gb_UnpackedTarball_get_dir,$(1))/%$(2) :
	$$(if $$(wildcard $$@),,$$(call gb_Output_error,file $$@ does not exist in the tarball))
	touch $$@

$(eval gb_UnpackedTarball_PATTERN_RULES_$(1) += $(2))

endef

define gb_UnpackedTarbal__ensure_pattern_rule
$(if $(filter $(2),$(gb_UnpackedTarball_PATTERN_RULES_$(1))),,$(call gb_UnpackedTarbal__make_pattern_rule,$(1),$(2)))

endef

define gb_UnpackedTarbal__make_file_rule
$(call gb_UnpackedTarball_get_dir,$(1))/$(2) :
	$$(if $$(wildcard $$@),,$$(call gb_Output_error,file $$@ does not exist in the tarball))
	touch $$@

endef

# Mark a source file to be used outside of this module
#
# This results in the timestamp of the file being updated, so a possible
# change is recognized properly by other files depending on it. The
# update is run after possible post action.
#
# See description of class ExternalPackage for more information.
#
# gb_UnpackedTarball_mark_output_file unpacked file
define gb_UnpackedTarball_mark_output_file
$(call gb_UnpackedTarball_get_final_target,$(1)) : $(call gb_UnpackedTarball_get_dir,$(1))/$(2)
$(call gb_UnpackedTarball_get_dir,$(1))/$(2) : $(call gb_UnpackedTarball_get_target,$(1))
$(if $(suffix $(2)),\
	$(call gb_UnpackedTarbal__ensure_pattern_rule,$(1),$(suffix $(2))),\
	$(call gb_UnpackedTarbal__make_file_rule,$(1),$(2)) \
)

endef

# Mark several source files to be used outside of this module
#
# gb_UnpackedTarball_mark_output_files unpacked file(s)
define gb_UnpackedTarball_mark_output_files
$(foreach file,$(2),$(call gb_UnpackedTarball_mark_output_file,$(1),$(file)))

endef

# vim: set noet sw=4 ts=4:
