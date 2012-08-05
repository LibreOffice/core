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

# ExtensionTarget class

gb_ExtensionTarget_ZIPCOMMAND := zip $(if $(findstring s,$(MAKEFLAGS)),-q)
ifeq ($(GUI),WNT)
gb_ExtensionTarget_LICENSEFILE_DEFAULT := $(OUTDIR)/bin/osl/license.txt
else
gb_ExtensionTarget_LICENSEFILE_DEFAULT := $(OUTDIR)/bin/osl/LICENSE
endif
gb_ExtensionTarget_XRMEXTARGET := $(call gb_Executable_get_target_for_build,xrmex)
gb_ExtensionTarget_XRMEXCOMMAND := \
	$(gb_Helper_set_ld_path) $(gb_ExtensionTarget_XRMEXTARGET)
# propmerge is a perl script
gb_ExtensionTarget_PROPMERGETARGET := $(OUTDIR_FOR_BUILD)/bin/propmerge
gb_ExtensionTarget_PROPMERGECOMMAND := \
	$(PERL) $(gb_ExtensionTarget_PROPMERGETARGET)
gb_ExtensionTarget_HELPEXTARGET := $(call gb_Executable_get_target_for_build,helpex)
gb_ExtensionTarget_HELPEXCOMMAND := \
	$(gb_Helper_set_ld_path) $(gb_ExtensionTarget_HELPEXTARGET)
# does not contain en-US because it is special cased in gb_ExtensionTarget_ExtensionTarget
gb_ExtensionTarget_LANGS := $(filter-out en-US,$(gb_WITH_LANG))

# Substitute platform or copy if no platform has been set
define gb_ExtensionTarget__subst_platform
$(if $(PLATFORM),\
	sed \
		-e 's/@PLATFORM@/$(PLATFORM)/' \
		-e 's/@EXEC_EXTENSION@/$(gb_Executable_EXT)/' \
		-e 's/@SHARED_EXTENSION@/$(gb_Library_DLLEXT)/' \
		$(1) > $(2),\
	cp -f $(1) $(2))
endef

# remove extension directory in workdir and oxt file in workdir
$(call gb_ExtensionTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),OXT,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f -r $(call gb_ExtensionTarget_get_workdir,$*) && \
		rm -f $(call gb_ExtensionTarget_get_target,$*) \
	)

ifeq ($(strip $(gb_WITH_LANG)),)
$(call gb_ExtensionTarget_get_workdir,%)/description.xml :
	$(call gb_Output_announce,$*/description.xml,$(true),CPY,3)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(call gb_ExtensionTarget_get_workdir,$*) && \
		cp -f $(LOCATION)/description.xml $@)
else
$(call gb_ExtensionTarget_get_workdir,%)/description.xml : $(gb_ExtensionTarget_XRMEXTARGET)
	$(call gb_Output_announce,$*/description.xml,$(true),XRM,3)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(call gb_ExtensionTarget_get_workdir,$*) && \
		$(gb_ExtensionTarget_XRMEXCOMMAND) \
			-p $(PRJNAME) \
			-i $(filter %.xml,$^) \
			-o $@ \
			-m $(SDF) \
			-l all)
endif

# rule to create oxt package in workdir
# --filesync makes sure that all files in the oxt package will be removed that no longer are in $(FILES)
$(call gb_ExtensionTarget_get_target,%) : \
		$(call gb_ExtensionTarget_get_workdir,%)/description.xml
	$(call gb_Output_announce,$*,$(true),OXT,3)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(call gb_ExtensionTarget_get_rootdir,$*)/META-INF \
			$(if $(LICENSE),$(call gb_ExtensionTarget_get_rootdir,$*)/registration) && \
		$(call gb_ExtensionTarget__subst_platform,$(call gb_ExtensionTarget_get_workdir,$*)/description.xml,$(call gb_ExtensionTarget_get_rootdir,$*)/description.xml) && \
		$(call gb_ExtensionTarget__subst_platform,$(LOCATION)/manifest.xml,$(call gb_ExtensionTarget_get_rootdir,$*)/META-INF/manifest.xml) && \
		$(if $(LICENSE),cp -f $(LICENSE) $(call gb_ExtensionTarget_get_rootdir,$*)/registration &&) \
		$(if $(gb_WITH_LANG),cp $(foreach lang,$(gb_ExtensionTarget_LANGS),$(call gb_ExtensionTarget_get_workdir,$*)/description-$(lang).txt) $(call gb_ExtensionTarget_get_rootdir,$*) &&) \
		cd $(call gb_ExtensionTarget_get_rootdir,$*) && \
		$(gb_ExtensionTarget_ZIPCOMMAND) -rX --filesync \
			$(call gb_ExtensionTarget_get_target,$*) \
			$(FILES))

# set file list and location of manifest and description files
# register target and clean target
# add deliverable
# add dependency for outdir target to workdir target (pattern rule for delivery is in Package.mk)
define gb_ExtensionTarget_ExtensionTarget
$(call gb_ExtensionTarget_get_target,$(1)) : FILES := META-INF description.xml
$(call gb_ExtensionTarget_get_target,$(1)) : LICENSE :=
$(call gb_ExtensionTarget_get_target,$(1)) : LOCATION := $(SRCDIR)/$(2)
$(call gb_ExtensionTarget_get_target,$(1)) : PLATFORM :=
$(call gb_ExtensionTarget_get_target,$(1)) : PRJNAME := $(firstword $(subst /, ,$(2)))
$(call gb_ExtensionTarget_get_workdir,$(1))/description.xml : $(SRCDIR)/$(2)/description.xml
ifneq ($(strip $(gb_WITH_LANG)),)
$(call gb_ExtensionTarget_get_target,$(1)) : FILES += $(foreach lang,$(gb_ExtensionTarget_LANGS),description-$(lang).txt)
$(call gb_ExtensionTarget_get_target,$(1)) : SDF := $(gb_SDFLOCATION)/$(2)/localize.sdf
$(call gb_ExtensionTarget_get_workdir,$(1))/description.xml : $$(SDF)
endif
$(call gb_ExtensionTarget_add_file,$(1),description-en-US.txt,$(SRCDIR)/$(2)/description-en-US.txt)

endef

# Set platform.
#
# Only use this if the extension is platform-dependent.
define gb_ExtensionTarget_set_platform
$(call gb_ExtensionTarget_get_target,$(1)) : PLATFORM := $(2)

endef

# Use the default license file
define gb_ExtensionTarget_use_default_license
$(call gb_ExtensionTarget_get_target,$(1)) : FILES += registration
$(call gb_ExtensionTarget_get_target,$(1)) : LICENSE := $(gb_ExtensionTarget_LICENSEFILE_DEFAULT)
$(call gb_ExtensionTarget_get_target,$(1)) : $(gb_ExtensionTarget_LICENSEFILE_DEFAULT)

endef

# adding a file creates a dependency to it
# file is copied to $(WORKDIR)
# $(3) is the target of the copied file, and $(4) can be used to override that
# with a different actual file, which is needed in gb_ExtensionTarget_add_library
# to make it work on Windows where the DLL doesn't have a gbuild target...
define gb_ExtensionTarget_add_file
$(call gb_ExtensionTarget_get_target,$(1)) : FILES += $(2)
$(call gb_ExtensionTarget_get_target,$(1)) : $(call gb_ExtensionTarget_get_rootdir,$(1))/$(2)
$(call gb_ExtensionTarget_get_rootdir,$(1))/$(2) : $(3)
	mkdir -p $$(dir $$@) && \
	cp -f $(if $(4),$(4),$(3)) $$@

endef

# Add several files at once
#
# This function avoids the need to specify each file's name twice. The
# files are added directly under specified path in the extension,
# without any subpath. If no path is specified, they are added directly
# to the root dir of the extension.
define gb_ExtensionTarget_add_files
$(foreach file,$(3),$(call gb_ExtensionTarget_add_file,$(1),$(if $(strip $(2)),$(strip $(2))/)$(notdir $(file)),$(file)))

endef

# add a library from the solver; DO NOT use gb_Library_get_target
define gb_ExtensionTarget_add_library
$(call gb_ExtensionTarget_add_file,$(1),$(call gb_Library_get_runtime_filename,$(2)),\
	$(call gb_Library_get_target,$(2)),\
	$(gb_Helper_OUTDIRLIBDIR)/$(call gb_Library_get_runtime_filename,$(2)))
endef

define gb_ExtensionTarget_add_libraries
$(foreach lib,$(2),$(call gb_ExtensionTarget_add_library,$(1),$(lib)))
endef

# add an executable from the solver
define gb_ExtensionTarget_add_executable
$(call gb_ExtensionTarget_add_file,$(1),$(notdir $(call gb_Executable_get_target,$(2))),\
	$(call gb_Executable_get_target,$(2)),\
	$(call gb_Executable_get_target,$(2)))
endef

define gb_ExtensionTarget_add_executables
$(foreach exe,$(2),$(call gb_ExtensionTarget_add_executable,$(1),$(exe)))
endef

# localize .properties file
# source file is copied to $(WORKDIR)
define gb_ExtensionTarget_localize_properties
$(call gb_ExtensionTarget_get_target,$(1)) : FILES += $(2)
ifneq ($(strip $(gb_WITH_LANG)),)
$(call gb_ExtensionTarget_get_target,$(1)) : FILES += $(foreach lang,$(subst -,_,$(gb_ExtensionTarget_LANGS)),$(subst en_US,$(lang),$(2)))
$(call gb_ExtensionTarget_get_rootdir,$(1))/$(2) : SDF := $(gb_SDFLOCATION)$(subst $(SRCDIR),,$(dir $(3)))localize.sdf
$(call gb_ExtensionTarget_get_rootdir,$(1))/$(2) : $$(SDF)
endif
$(call gb_ExtensionTarget_get_target,$(1)) : $(call gb_ExtensionTarget_get_rootdir,$(1))/$(2)
$(call gb_ExtensionTarget_get_rootdir,$(1))/$(2) : $(3) \
		$(gb_ExtensionTarget_PROPMERGETARGET)
	$$(call gb_Output_announce,$(2),$(true),PRP,3)
	mkdir -p $$(dir $$@) && \
	cp -f $$< $$@ \
	$(if $(strip $(gb_WITH_LANG)),&& $(gb_ExtensionTarget_PROPMERGECOMMAND) -i $$@ -m $$(SDF))

endef

# localize extension help
define gb_ExtensionTarget_localize_help
ifneq ($(strip $(gb_WITH_LANG)),)
$(call gb_ExtensionTarget_get_target,$(1)) : FILES += $(foreach lang,$(gb_ExtensionTarget_LANGS),$(subst lang,$(lang),$(2)))
$(foreach lang,$(gb_ExtensionTarget_LANGS),$(call gb_ExtensionTarget_localize_help_onelang,$(1),$(subst lang,$(lang),$(2)),$(3),$(lang)))
endif

endef

define gb_ExtensionTarget_localize_help_onelang
$(call gb_ExtensionTarget_get_target,$(1)) : $(call gb_ExtensionTarget_get_rootdir,$(1))/$(2)
$(call gb_ExtensionTarget_get_rootdir,$(1))/$(2) : SDF := $(gb_SDFLOCATION)$(subst $(SRCDIR),,$(subst $(WORKDIR)/CustomTarget,,$(dir $(3))))localize.sdf
$(call gb_ExtensionTarget_get_rootdir,$(1))/$(2) : $$(SDF)
$(call gb_ExtensionTarget_get_rootdir,$(1))/$(2) : $(gb_ExtensionTarget_HELPEXTARGET)
$(call gb_ExtensionTarget_get_rootdir,$(1))/$(2) : $(3)
	$$(call gb_Output_announce,$(2),$(true),XHP,3)
	mkdir -p $$(dir $$@) && \
	$(gb_ExtensionTarget_HELPEXCOMMAND) -i $$< -o $$@ -l $(4) -m $$(SDF)

endef

define gb_ExtensionTarget_use_package
$(call gb_ExtensionTarget_get_target,$(1)) : $(call gb_Package_get_target,$(2))

endef

define gb_ExtensionTarget_use_packages
$(foreach package,$(2),$(call gb_ExtensionTarget_use_package,$(1),$(package)))

endef

# vim: set noet sw=4 ts=4:
