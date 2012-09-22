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

# platform
#  gb_ExtensionTarget_LICENSEFILE_DEFAULT

gb_ExtensionTarget_ZIPCOMMAND := zip $(if $(findstring s,$(MAKEFLAGS)),-q)
gb_ExtensionTarget_XRMEXTARGET := $(call gb_Executable_get_target_for_build,xrmex)
gb_ExtensionTarget_XRMEXCOMMAND := \
	$(gb_Helper_set_ld_path) $(gb_ExtensionTarget_XRMEXTARGET)
# propmerge is a perl script
gb_ExtensionTarget_PROPMERGETARGET := $(OUTDIR_FOR_BUILD)/bin/propmerge
gb_ExtensionTarget_PROPMERGECOMMAND := \
	$(PERL) $(gb_ExtensionTarget_PROPMERGETARGET)

gb_ExtensionTarget_UPDATETREETARGET := $(SRCDIR)/l10ntools/scripts/update_tree.pl
gb_ExtensionTarget_UPDATETREECOMMAND := \
    $(gb_Helper_set_ld_path) $(PERL) $(gb_ExtensionTarget_UPDATETREETARGET)
    # update_tree.pl calls xmllint, which needs $(gb_Helper_set_ld_path) if it
    # is the internal one

gb_ExtensionTarget_HELPEXTARGET := $(call gb_Executable_get_target_for_build,helpex)
gb_ExtensionTarget_HELPEXCOMMAND := \
	$(gb_Helper_set_ld_path) $(gb_ExtensionTarget_HELPEXTARGET)
gb_ExtensionTarget_HELPINDEXERTARGET := \
    $(call gb_Executable_get_target_for_build,HelpIndexer)
gb_ExtensionTarget_HELPINDEXERCOMMAND := \
	$(gb_Helper_set_ld_path) $(gb_ExtensionTarget_HELPINDEXERTARGET)
gb_ExtensionTarget_HELPLINKERTARGET := \
    $(call gb_Executable_get_target_for_build,HelpLinker)
gb_ExtensionTarget_HELPLINKERCOMMAND := \
	$(gb_Helper_set_ld_path) $(gb_ExtensionTarget_HELPLINKERTARGET)
# does not contain en-US because it is special cased in gb_ExtensionTarget_ExtensionTarget
gb_ExtensionTarget_TRANS_LANGS := $(filter-out en-US,$(gb_WITH_LANG))
gb_ExtensionTarget_ALL_LANGS := en-US $(gb_ExtensionTarget_TRANS_LANGS)

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
			-i $(call gb_Helper_symlinked_native,$(filter %.xml,$^)) \
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
		$(if $(and $(gb_WITH_LANG),$(DESCRIPTION)),cp $(foreach lang,$(gb_ExtensionTarget_TRANS_LANGS),$(call gb_ExtensionTarget_get_workdir,$*)/description-$(lang).txt) $(call gb_ExtensionTarget_get_rootdir,$*) &&) \
		cd $(call gb_ExtensionTarget_get_rootdir,$*) && \
		$(gb_ExtensionTarget_ZIPCOMMAND) -rX --filesync \
			$(call gb_ExtensionTarget_get_target,$*) \
			$(sort $(FILES)))

# set file list and location of manifest and description files
# register target and clean target
# add deliverable
# add dependency for outdir target to workdir target (pattern rule for delivery is in Package.mk)
define gb_ExtensionTarget_ExtensionTarget
$(call gb_ExtensionTarget_get_target,$(1)) : DESCRIPTION :=
$(call gb_ExtensionTarget_get_target,$(1)) : FILES := META-INF description.xml
$(call gb_ExtensionTarget_get_target,$(1)) : LICENSE :=
$(call gb_ExtensionTarget_get_target,$(1)) : LOCATION := $(SRCDIR)/$(2)
$(call gb_ExtensionTarget_get_target,$(1)) : PLATFORM :=
$(call gb_ExtensionTarget_get_target,$(1)) : PRJNAME := $(firstword $(subst /, ,$(2)))
$(call gb_ExtensionTarget_get_workdir,$(1))/description.xml : $(SRCDIR)/$(2)/description.xml
ifneq ($(strip $(gb_WITH_LANG)),)
$(call gb_ExtensionTarget_get_target,$(1)) : SDF := $(gb_SDFLOCATION)/$(2)/localize.sdf
$(call gb_ExtensionTarget_get_workdir,$(1))/description.xml : $(gb_SDFLOCATION)/$(2)/localize.sdf
endif

$(foreach lang,$(gb_ExtensionTarget_ALL_LANGS), \
    $(call gb_ExtensionTarget__compile_help_onelang,$(1),$(lang)))
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

# Use the default description file
define gb_ExtensionTarget_use_default_description
$(call gb_ExtensionTarget_add_file,$(1),description-en-US.txt,$(SRCDIR)/$(2)/description-en-US.txt)
$(call gb_ExtensionTarget_get_target,$(1)) : DESCRIPTION := $(true)
ifneq ($(strip $(gb_WITH_LANG)),)
$(call gb_ExtensionTarget_get_target,$(1)) : FILES += $(foreach lang,$(gb_ExtensionTarget_TRANS_LANGS),description-$(lang).txt)
endif

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
$(call gb_ExtensionTarget_get_target,$(1)) : FILES += $(foreach lang,$(subst -,_,$(gb_ExtensionTarget_TRANS_LANGS)),$(subst en_US,$(lang),$(2)))
$(call gb_ExtensionTarget_get_rootdir,$(1))/$(2) : SDF := $(gb_SDFLOCATION)$(subst $(SRCDIR),,$(dir $(3)))localize.sdf
$(call gb_ExtensionTarget_get_rootdir,$(1))/$(2) : $(gb_SDFLOCATION)$(subst $(SRCDIR),,$(dir $(3)))localize.sdf
endif
$(call gb_ExtensionTarget_get_target,$(1)) : $(call gb_ExtensionTarget_get_rootdir,$(1))/$(2)
$(call gb_ExtensionTarget_get_rootdir,$(1))/$(2) : $(3) \
		$(gb_ExtensionTarget_PROPMERGETARGET)
	$$(call gb_Output_announce,$(2),$(true),PRP,3)
	mkdir -p $$(dir $$@) && \
	cp -f $$< $$@ \
	$(if $(strip $(gb_WITH_LANG)),&& $(gb_ExtensionTarget_PROPMERGECOMMAND) -i $$@ -m $$(SDF))

endef

# add an .xhp help file, to be localized and compiled
# $(1): extension identifier
# $(2): absolute path prefix of en-US source file without $(3) (resp. $(4))
#     suffix
# $(3): relative path of (target) .xhp file (e.g.,
#     com.sun.wiki-publisher/wiki.xhp)
# $(4): optional relative path of source .xhp file, when it differs from $(3)
#     (i.e., if $(4) is empty the en-US source file is $(2)/$(3), otherwise it
#     is $(2)/$(4))
define gb_ExtensionTarget_add_helpfile
$(foreach lang,$(gb_ExtensionTarget_ALL_LANGS), \
    $(call gb_ExtensionTarget__localize_helpfile_onelang,$(1),$(2),$(3),$(4),$(lang)) \
    $(call gb_ExtensionTarget__add_compiled_help_dependency_onelang,$(1),$(lang)))
endef

# add a help.tree file, to be localized and compiled
# $(1): extension identifier
# $(2): absolute path prefix of en-US source file without $(3) (resp. $(4))
#     suffix
# $(3): relative path of (target) help.tree file (e.g.,
#     com.sun.wiki-publisher/help.tree)
# $(4): optional relative path of source help.tree file, when it differs from $(3)
#     (i.e., if $(4) is empty the en-US source file is $(2)/$(3), otherwise it
#     is $(2)/$(4))
define gb_ExtensionTarget_add_helptreefile
$(foreach lang,$(gb_ExtensionTarget_ALL_LANGS), \
    $(call gb_ExtensionTarget__localize_helptreefile_onelang,$(1),$(2),$(3),$(4),$(lang),$(5)) \
    $(call gb_ExtensionTarget__add_compiled_help_dependency_onelang,$(1),$(lang)))
endef

# add a list of .xhp help files, to be localized and compiled
# $(1): extension identifier
# $(2): absolute path prefix of en-US source files without $(3) suffixes
# $(3): list of relative paths of .xhp files (see
#     gb_ExtensionTarget_add_helpfile)
define gb_ExtensionTarget_add_helpfiles
$(foreach helpfile,$(3), \
    $(call gb_ExtensionTarget_add_helpfile,$(1),$(2),$(helpfile),))
endef

# localize one .xhp help file for one language; the result is stored as
# help/$(4)/$(3) in the extension's workdir; as a special case, if $(4) is
# "en-US", the source file is just copied, not passed through helpex
# $(1): extension identifier
# $(2): absolute path prefix of en-US source file without $(3) (resp. $(4))
#     suffix
# $(3): relative path of (target) .xhp file (see
#     gb_ExtensionTarget_add_helpfile)
# $(4): optional relative path of source .xhp file (see
#     gb_ExtensionTarget_add_helpfile)
# $(5): language
define gb_ExtensionTarget__localize_helpfile_onelang
$(call gb_ExtensionTarget_get_rootdir,$(1))/help/$(5).done : HELPFILES += $(3)
$(call gb_ExtensionTarget_get_rootdir,$(1))/help/$(5).done : \
        $(call gb_ExtensionTarget_get_workdir,$(1))/help/$(5)/$(3)
ifneq ($(strip $(gb_WITH_LANG)),)
$(call gb_ExtensionTarget_get_workdir,$(1))/help/$(5)/$(3) : \
        SDF := $(gb_SDFLOCATION)$(subst $(SRCDIR),,$(subst $(WORKDIR)/CustomTarget,,$(2)/$(dir $(or $(4),$(3)))))localize.sdf
$(call gb_ExtensionTarget_get_workdir,$(1))/help/$(5)/$(3) : \
        $(gb_SDFLOCATION)$(subst $(SRCDIR),,$(subst $(WORKDIR)/CustomTarget,,$(2)/$(dir $(or $(4),$(3)))))localize.sdf
endif
$(call gb_ExtensionTarget_get_workdir,$(1))/help/$(5)/$(3) : \
        $(if $(filter-out en-US,$(5)),$(gb_ExtensionTarget_HELPEXTARGET)) | \
        $(call gb_ExtensionTarget_get_workdir,$(1))/help/.dir
$(call gb_ExtensionTarget_get_workdir,$(1))/help/$(5)/$(3) : \
        $(2)/$(or $(4),$(3))
	$$(call gb_Output_announce,$(1) $(3) $(5),$(true),XHP,3)
	$$(call gb_Helper_abbreviate_dirs, \
        mkdir -p $$(dir $$@) && \
        $(if $(filter-out en-US,$(5)), \
            $(gb_ExtensionTarget_HELPEXCOMMAND) -i $$< -o $$@ -l $(5) \
                -m $$(SDF), \
            cp $$< $$@))

endef

# localize one help.tree for one language; the result is stored as
# help/$(4)/$(3) in the extension's workdir; as a special case, if $(4) is
# "en-US", the source file is just copied, not passed through update_tree.pl
# $(1): extension identifier
# $(2): absolute path prefix of en-US source file without $(3) (resp. $(4))
#     suffix
# $(3): relative path of (target) help.tree file (see
#     gb_ExtensionTarget_add_helptreefile)
# $(4): optional relative path of source help.tree file (see
#     gb_ExtensionTarget_add_helptreefile)
# $(5): language
define gb_ExtensionTarget__localize_helptreefile_onelang
$(call gb_ExtensionTarget_get_rootdir,$(1))/help/$(5).done : \
        $(call gb_ExtensionTarget_get_rootdir,$(1))/help/$(5)/$(3)
ifneq ($(strip $(gb_WITH_LANG)),)
$(call gb_ExtensionTarget_get_rootdir,$(1))/help/$(5)/$(3) : \
        SDF := $(gb_SDFLOCATION)$(subst $(SRCDIR),,$(subst $(WORKDIR)/CustomTarget,,$(2)/$(dir $(or $(4),$(3)))))localize.sdf
$(call gb_ExtensionTarget_get_rootdir,$(1))/help/$(5)/$(3) : \
        $(gb_SDFLOCATION)$(subst $(SRCDIR),,$(subst $(WORKDIR)/CustomTarget,,$(2)/$(dir $(or $(4),$(3)))))localize.sdf
endif
$(call gb_ExtensionTarget_get_rootdir,$(1))/help/$(5)/$(3) : \
        $(if $(filter-out en-US,$(5)),$(gb_ExtensionTarget_UPDATETREETARGET)) | \
        $(2)/$(4)
$(call gb_ExtensionTarget_get_rootdir,$(1))/help/$(5)/$(3) : \
        $(2)/$(or $(4),$(3))
	$$(call gb_Output_announce,$(1) $(3) $(5),$(true),TRE,3)
	$$(call gb_Helper_abbreviate_dirs, \
        mkdir -p $$(dir $$@) && \
		$(if $(filter-out en-US,$(5)), \
			$(gb_ExtensionTarget_UPDATETREECOMMAND) $$< $(5) $$(SDF) $$@ $(6),\
			cp $$< $$@))

endef

# compile help for one language; the result is stored as help/$(3)/ in the
# extension's rootdir and marked for zipping into the .oxt
# $(1): extension identifier
# $(2): language
# Target-specific HELPFILES: list of relative paths of .xhp files (see
#     gb_ExtensionTarget_add_helpfile)
define gb_ExtensionTarget__compile_help_onelang
$(call gb_ExtensionTarget_get_rootdir,$(1))/help/$(2).done : \
        $(gb_ExtensionTarget_HELPINDEXERTARGET) \
        $(gb_ExtensionTarget_HELPLINKERTARGET) \
        $(OUTDIR_FOR_BUILD)/bin/embed.xsl \
        $(OUTDIR_FOR_BUILD)/bin/idxcaption.xsl \
        $(OUTDIR_FOR_BUILD)/bin/idxcontent.xsl | \
        $(call gb_ExtensionTarget_get_rootdir,$(1))/help/.dir
	$$(call gb_Output_announce,$(1) $(2),$(true),XHC,3)
	$$(call gb_Helper_abbreviate_dirs, \
        mkdir -p $$(basename $$@) && \
        $(gb_ExtensionTarget_HELPLINKERCOMMAND) -mod help \
            -extlangsrc $(call gb_ExtensionTarget_get_workdir,$(1))/help/$(2) \
            -sty $(OUTDIR_FOR_BUILD)/bin/embed.xsl \
            -extlangdest $$(basename $$@) \
            -idxcaption $(OUTDIR_FOR_BUILD)/bin/idxcaption.xsl \
            -idxcontent $(OUTDIR_FOR_BUILD)/bin/idxcontent.xsl \
            $$(HELPFILES) && \
        (cd $(call gb_ExtensionTarget_get_workdir,$(1))/help/$(2) && \
            $(gb_ExtensionTarget_ZIPCOMMAND) -r $$(basename $$@)/help.jar \
            $$(HELPFILES)) && \
        $(gb_ExtensionTarget_HELPINDEXERCOMMAND) -lang $(2) -mod help \
            -dir $$(basename $$@) && \
        touch $$@)

endef

# establish the dependency that actually causes inclusion of the compiled help
# into the .oxt, for one language; in principle, this would only need to be done
# once per language iff the extension uses any help -- currently it is done from
# each individual gb_ExtensionTarget_add_helpfile call (and thus requires $strip
# to remove duplicates from FILES)
# $(1): extension identifier
# $(2): language
define gb_ExtensionTarget__add_compiled_help_dependency_onelang
$(call gb_ExtensionTarget_get_target,$(1)) : FILES += help/$(2)
$(call gb_ExtensionTarget_get_target,$(1)) : \
        $(call gb_ExtensionTarget_get_rootdir,$(1))/help/$(2).done

endef


define gb_ExtensionTarget_use_package
$(call gb_ExtensionTarget_get_target,$(1)) : $(call gb_Package_get_target,$(2))

endef

define gb_ExtensionTarget_use_packages
$(foreach package,$(2),$(call gb_ExtensionTarget_use_package,$(1),$(package)))

endef

# vim: set noet sw=4 ts=4:
