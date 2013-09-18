# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Extension class

# platform
#  gb_Extension_LICENSEFILE_DEFAULT

gb_Extension__get_preparation_target = $(WORKDIR)/Extension/$(1).prepare

gb_Extension_ZIPCOMMAND := zip $(if $(findstring s,$(MAKEFLAGS)),-q)
gb_Extension_XRMEXDEPS := $(call gb_Executable_get_runtime_dependencies,xrmex)
gb_Extension_XRMEXCOMMAND := $(call gb_Executable_get_command,xrmex)

gb_Extension_PROPMERGEDEPS := $(call gb_Executable_get_runtime_dependencies,propex)
gb_Extension_PROPMERGECOMMAND := $(call gb_Executable_get_command,propex)

gb_Extension_TREEXDEPS := $(call gb_Executable_get_runtime_dependencies,treex)
gb_Extension_TREEXCOMMAND := $(call gb_Executable_get_command,treex)

gb_Extension_HELPEXDEPS := $(call gb_Executable_get_runtime_dependencies,helpex)
gb_Extension_HELPEXCOMMAND := $(call gb_Executable_get_command,helpex)
gb_Extension_HELPINDEXERDEPS := $(call gb_Executable_get_runtime_dependencies,HelpIndexer)
gb_Extension_HELPINDEXERCOMMAND := $(call gb_Executable_get_command,HelpIndexer)
gb_Extension_HELPLINKERDEPS := $(call gb_Executable_get_runtime_dependencies,HelpLinker)
gb_Extension_HELPLINKERCOMMAND := $(call gb_Executable_get_command,HelpLinker)
# does not contain en-US because it is special cased in gb_Extension_Extension
gb_Extension_TRANS_LANGS := $(filter-out en-US,$(gb_WITH_LANG))
gb_Extension_ALL_LANGS := en-US $(gb_Extension_TRANS_LANGS)

# Substitute platform or copy if no platform has been set
define gb_Extension__subst_platform
$(if $(PLATFORM),\
	sed \
		-e 's/@PLATFORM@/$(PLATFORM)/' \
		-e 's/@EXEC_EXTENSION@/$(gb_Executable_EXT)/' \
		-e 's/@SHARED_EXTENSION@/$(gb_Library_DLLEXT)/' \
		$(1) > $(2),\
	cp -f $(1) $(2))
endef

$(call gb_Extension_get_workdir,%)/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

# remove extension directory in workdir and oxt file in workdir
$(call gb_Extension_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),OXT,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f -r $(call gb_Extension_get_workdir,$*) && \
		rm -f $(call gb_Extension__get_preparation_target,$*) \
			  $(call gb_Extension_get_target,$*) \
	)

# preparation target to delay adding files produced by e.g. UnpackedTarball
$(call gb_Extension__get_preparation_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && touch $@)

ifeq ($(strip $(gb_WITH_LANG)),)
$(call gb_Extension_get_workdir,%)/description.xml :
	$(call gb_Output_announce,$*/description.xml,$(true),CPY,3)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(call gb_Extension_get_workdir,$*) && \
		cp -f $(LOCATION)/description.xml $@)
else
$(call gb_Extension_get_workdir,%)/description.xml : $(gb_Extension_XRMEXDEPS)
	$(call gb_Output_announce,$*/description.xml,$(true),XRM,3)
	MERGEINPUT=`$(gb_MKTEMP)` && \
	echo $(POFILES) > $${MERGEINPUT} && \
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(call gb_Extension_get_workdir,$*) && \
		$(gb_Extension_XRMEXCOMMAND) \
			-i $(filter %.xml,$^) \
			-o $@ \
			-m $${MERGEINPUT} \
			-l all) && \
	rm -rf $${MERGEINPUT}

endif

# rule to create oxt package in workdir
# --filesync makes sure that all files in the oxt package will be removed that no longer are in $(FILES)
$(call gb_Extension_get_target,%) : \
		$(call gb_Extension_get_workdir,%)/description.xml
	$(call gb_Output_announce,$*,$(true),OXT,3)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(call gb_Extension_get_rootdir,$*)/META-INF \
			$(if $(LICENSE),$(call gb_Extension_get_rootdir,$*)/registration) && \
		$(call gb_Extension__subst_platform,$(call gb_Extension_get_workdir,$*)/description.xml,$(call gb_Extension_get_rootdir,$*)/description.xml) && \
		$(call gb_Extension__subst_platform,$(LOCATION)/META-INF/manifest.xml,$(call gb_Extension_get_rootdir,$*)/META-INF/manifest.xml) && \
		$(if $(LICENSE),cp -f $(LICENSE) $(call gb_Extension_get_rootdir,$*)/registration &&) \
		$(if $(and $(gb_Extension_TRANS_LANGS),$(DESCRIPTION)),cp $(foreach lang,$(gb_Extension_TRANS_LANGS),$(call gb_Extension_get_workdir,$*)/description-$(lang).txt) $(call gb_Extension_get_rootdir,$*) &&) \
		cd $(call gb_Extension_get_rootdir,$*) && \
		$(gb_Extension_ZIPCOMMAND) -rX --filesync --must-match \
			$(call gb_Extension_get_target,$*) \
			$(sort $(FILES)))

# set file list and location of manifest and description files
# register target and clean target
# add deliverable
# add dependency for outdir target to workdir target (pattern rule for delivery is in Package.mk)
#
# gb_Extension_Extension extension srcdir
define gb_Extension_Extension
$(call gb_Extension_get_target,$(1)) : DESCRIPTION :=
$(call gb_Extension_get_target,$(1)) : FILES := META-INF description.xml
$(call gb_Extension_get_target,$(1)) : LICENSE :=
$(call gb_Extension_get_target,$(1)) : LOCATION := $(SRCDIR)/$(2)
$(call gb_Extension_get_target,$(1)) : PLATFORM := $(PLATFORMID)
$(call gb_Extension_get_workdir,$(1))/description.xml : \
	$(SRCDIR)/$(2)/description.xml
$(call gb_Extension_get_workdir,$(1))/description.xml :| \
	$(call gb_Extension__get_preparation_target,$(1))

ifneq ($(strip $(gb_WITH_LANG)),)
$(call gb_Extension_get_target,$(1)) : \
	POFILES := $(foreach lang,$(gb_TRANS_LANGS),$(gb_POLOCATION)/$(lang)/$(2).po)
$(call gb_Extension_get_workdir,$(1))/description.xml : \
	$(foreach lang,$(gb_TRANS_LANGS),$(gb_POLOCATION)/$(lang)/$(2).po)
$(foreach lang,$(gb_TRANS_LANGS),$(gb_POLOCATION)/$(lang)/$(2).po) :
endif

$(foreach lang,$(gb_Extension_ALL_LANGS), \
    $(call gb_Extension__compile_help_onelang,$(1),$(lang)))

$$(eval $$(call gb_Module_register_target,$(call gb_Extension_get_target,$(1)),$(call gb_Extension_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),Extension)

endef

# adding a file creates a dependency to it
# file is copied to $(WORKDIR)
define gb_Extension_add_file
$(call gb_Extension_get_target,$(1)) : FILES += $(2)
$(call gb_Extension_get_target,$(1)) : $(call gb_Extension_get_rootdir,$(1))/$(2)
$(3) :| $(call gb_Extension__get_preparation_target,$(1))
$(call gb_Extension_get_rootdir,$(1))/$(2) : $(3)
	mkdir -p $$(dir $$@) && \
	cp -f $(3) $$@

endef

# Add several files at once
#
# This function avoids the need to specify each file's name twice. The
# files are added directly under specified path in the extension,
# without any subpath. If no path is specified, they are added directly
# to the root dir of the extension.
define gb_Extension_add_files
$(foreach file,$(3),$(call gb_Extension_add_file,$(1),$(if $(strip $(2)),$(strip $(2))/)$(notdir $(file)),$(file)))

endef

# add a library from the solver; DO NOT use gb_Library_get_target
define gb_Extension_add_library
$(call gb_Extension_add_file,$(1),$(call gb_Library_get_runtime_filename,$(2)),\
	$(gb_Helper_OUTDIRLIBDIR)/$(call gb_Library_get_runtime_filename,$(2)))

endef

define gb_Extension_add_libraries
$(foreach lib,$(2),$(call gb_Extension_add_library,$(1),$(lib)))

endef

# add an executable from the solver
define gb_Extension_add_executable
$(call gb_Extension_add_file,$(1),$(notdir $(call gb_Executable_get_target,$(2))),\
	$(call gb_Executable_get_target,$(2)))

endef

define gb_Extension_add_executables
$(foreach exe,$(2),$(call gb_Extension_add_executable,$(1),$(exe)))

endef

# localize .properties file
# source file is copied to $(WORKDIR)
define gb_Extension_localize_properties
$(foreach lang,$(gb_Extension_ALL_LANGS),\
	$(call gb_Extension__localize_properties_onelang,$(1),$(subst en_US,$(subst -,_,$(lang)),$(2)),$(3),$(lang)))

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
define gb_Extension_add_helpfile
$(foreach lang,$(gb_Extension_ALL_LANGS), \
    $(call gb_Extension__localize_helpfile_onelang,$(1),$(2),$(3),$(4),$(lang)) \
    $(call gb_Extension__add_compiled_help_dependency_onelang,$(1),$(lang)))

endef

# add a list of .xhp help files, to be localized and compiled
# $(1): extension identifier
# $(2): absolute path prefix of en-US source files without $(3) suffixes
# $(3): list of relative paths of .xhp files (see gb_Extension_add_helpfile)
define gb_Extension_add_helpfiles
$(foreach helpfile,$(3), \
    $(call gb_Extension_add_helpfile,$(1),$(2),$(helpfile),))

endef

# add a help.tree file, to be localized and compiled
# $(1): extension identifier
# $(2): absolute path prefix of en-US source file without $(3) (resp. $(4))
#     suffix
# $(3): relative path of (target) help.tree file (e.g.,
#     com.sun.wiki-publisher/help.tree)
# $(4): relative path of source help.tree file
# $(5): relative path of localized xhp files (PlatformID included)
define gb_Extension_add_helptreefile
$(foreach lang,$(gb_Extension_ALL_LANGS), \
    $(call gb_Extension__localize_helptreefile_onelang,$(1),$(2),$(3),$(4),$(lang),$(5)) \
    $(call gb_Extension__add_compiled_help_dependency_onelang,$(1),$(lang)))

endef

# Use the default description file
define gb_Extension_use_default_description
$(call gb_Extension_add_file,$(1),description-en-US.txt,$$(LOCATION)/description-en-US.txt)
$(call gb_Extension_get_target,$(1)) : DESCRIPTION := $(true)
ifneq ($(strip $(gb_WITH_LANG)),)
$(call gb_Extension_get_target,$(1)) : FILES += $(foreach lang,$(gb_Extension_TRANS_LANGS),description-$(lang).txt)
endif

endef

# Use the default license file
define gb_Extension_use_default_license
$(call gb_Extension_get_target,$(1)) : FILES += registration
$(call gb_Extension_get_target,$(1)) : LICENSE := $(gb_Extension_LICENSEFILE_DEFAULT)
$(call gb_Extension_get_target,$(1)) : $(gb_Extension_LICENSEFILE_DEFAULT)

endef

define gb_Extension_use_unpacked
$(call gb_Extension__get_preparation_target,$(1)) \
	:| $(call gb_UnpackedTarball_get_final_target,$(2))

endef

define gb_Extension_use_package
$(call gb_Extension__get_preparation_target,$(1)) \
	:| $(call gb_Package_get_target,$(2))

endef

define gb_Extension_use_packages
$(foreach package,$(2),$(call gb_Extension_use_package,$(1),$(package)))

endef


define gb_Extension__localize_properties_onelang
$(call gb_Extension_get_target,$(1)) : FILES += $(2)
ifneq ($(filter-out en-US,$(4)),)
ifneq ($(filter-out qtz,$(4)),)
$(call gb_Extension_get_rootdir,$(1))/$(2) : \
	POFILE := $(gb_POLOCATION)/$(or $(5),$(4))/$(patsubst /%/,%,$(subst $(SRCDIR),,$(dir $(3)))).po
$(call gb_Extension_get_rootdir,$(1))/$(2) : \
	$(gb_POLOCATION)/$(or $(5),$(4))/$(patsubst /%/,%,$(subst $(SRCDIR),,$(dir $(3)))).po
$(gb_POLOCATION)/$(or $(5),$(4))/$(patsubst /%/,%,$(subst $(SRCDIR),,$(dir $(3)))).po :
endif
endif
$(call gb_Extension_get_target,$(1)) : $(call gb_Extension_get_rootdir,$(1))/$(2)
$(call gb_Extension_get_rootdir,$(1))/$(2) \
		:| $(call gb_Extension__get_preparation_target,$(1))
$(call gb_Extension_get_rootdir,$(1))/$(2) : $(3) \
		$(gb_Extension_PROPMERGEDEPS)
	$$(call gb_Output_announce,$(2),$(true),PRP,3)
	$$(call gb_Helper_abbreviate_dirs, \
		mkdir -p $$(dir $$@) && \
		$(if $(filter qtz,$(4)), \
			$(gb_Extension_PROPMERGECOMMAND) -i $$< -o $$@ -m -l $(4) \
			, \
			$(if $(filter-out en-US,$(4)), \
				MERGEINPUT=`$(gb_MKTEMP)` && \
				echo $$(POFILE) > $$$${MERGEINPUT} && \
				$(gb_Extension_PROPMERGECOMMAND) -i $$< -o $$@ -m $$$${MERGEINPUT} -l $(4) && \
				rm -rf $$$${MERGEINPUT} \
				, \
				cp -f $$< $$@ \
			) \
		) \
	)

endef

# localize one .xhp help file for one language; the result is stored as
# help/$(4)/$(3) in the extension's workdir; as a special case, if $(4) is
# "en-US", the source file is just copied, not passed through helpex
# $(1): extension identifier
# $(2): absolute path prefix of en-US source file without $(3) (resp. $(4))
#     suffix
# $(3): relative path of (target) .xhp file (see
#     gb_Extension_add_helpfile)
# $(4): optional relative path of source .xhp file (see
#     gb_Extension_add_helpfile)
# $(5): language
define gb_Extension__localize_helpfile_onelang
$(call gb_Extension_get_rootdir,$(1))/help/$(5).done : HELPFILES += $(3)
$(call gb_Extension_get_rootdir,$(1))/help/$(5).done : \
        $(call gb_Extension_get_workdir,$(1))/help/$(5)/$(3)
$(call gb_Extension_get_rootdir,$(1))/help/$(5)-xhp.done : \
        $(call gb_Extension_get_workdir,$(1))/help/$(5)/$(3)
ifneq ($(filter-out en-US,$(5)),)
ifneq ($(filter-out qtz,$(5)),)
$(call gb_Extension_get_workdir,$(1))/help/$(5)/$(3) : \
	POFILE := $(gb_POLOCATION)/$(5)$(subst $(SRCDIR),,$(2))$(patsubst %/,/%.po,$(patsubst ./,.po,$(dir $(or $(4),$(3)))))
$(call gb_Extension_get_workdir,$(1))/help/$(5)/$(3) : \
        $(gb_POLOCATION)/$(5)$(subst $(SRCDIR),,$(2))$(patsubst %/,/%.po,$(patsubst ./,.po,$(dir $(or $(4),$(3)))))
$(gb_POLOCATION)/$(5)$(subst $(SRCDIR),,$(2))$(patsubst %/,/%.po,$(patsubst ./,.po,$(dir $(or $(4),$(3))))) :
endif
endif
$(call gb_Extension_get_workdir,$(1))/help/$(5)/$(3) : \
        $(if $(filter-out en-US,$(5)),$(gb_Extension_HELPEXDEPS)) | \
        $(call gb_Extension_get_workdir,$(1))/help/.dir
$(call gb_Extension_get_workdir,$(1))/help/$(5)/$(3) : \
        $(2)/$(or $(4),$(3))
	$$(call gb_Output_announce,$(1) $(3) $(5),$(true),XHP,3)
	$$(call gb_Helper_abbreviate_dirs, \
		mkdir -p $$(dir $$@) && \
		$(if $(filter qtz,$(5)), \
			$(gb_Extension_HELPEXCOMMAND) -i $$< -o $$@ -l $(5) -m \
			, \
			$(if $(filter-out en-US,$(5)), \
				MERGEINPUT=`$(gb_MKTEMP)` && \
				echo $$(POFILE) > $$$${MERGEINPUT} && \
				$$(gb_Extension_HELPEXCOMMAND) -i $$< -o $$@ -l $(5) \
					-m $$$${MERGEINPUT} && \
				rm -rf $$$${MERGEINPUT} \
				, \
				cp $$< $$@ \
			) \
		) && \
		touch $(call gb_Extension_get_rootdir,$(1))/help/$(5)-xhp.done \
	)

endef

# localize one help.tree for one language; the result is stored as
# help/$(4)/$(3) in the extension's workdir;
# $(1): extension identifier
# $(2): absolute path prefix of en-US source file without $(3) (resp. $(4))
#     suffix
# $(3): relative path of (target) help.tree file (see
#     gb_Extension_add_helptreefile)
# $(4): relative path of source help.tree file (see
#     gb_Extension_add_helptreefile)
# $(5): language
# $(6): relative path of localized xhp files (PlatformID included)
define gb_Extension__localize_helptreefile_onelang
$(call gb_Extension_get_rootdir,$(1))/help/$(5).done : \
        $(call gb_Extension_get_rootdir,$(1))/help/$(5)/$(3)
ifneq ($(filter-out en-US,$(5)),)
ifneq ($(filter-out qtz,$(5)),)
$(call gb_Extension_get_rootdir,$(1))/help/$(5)/$(3) : \
	POFILE := $(gb_POLOCATION)/$(5)$(subst $(SRCDIR),,$(2))$(patsubst %/,/%.po,$(patsubst ./,.po,$(dir $(4))))
$(call gb_Extension_get_rootdir,$(1))/help/$(5)/$(3) : \
        $(gb_POLOCATION)/$(5)$(subst $(SRCDIR),,$(2))$(patsubst %/,/%.po,$(patsubst ./,.po,$(dir $(4))))
$(gb_POLOCATION)/$(5)$(subst $(SRCDIR),,$(2))$(patsubst %/,/%.po,$(patsubst ./,.po,$(dir $(4)))) :
endif
endif
$(call gb_Extension_get_rootdir,$(1))/help/$(5)/$(3) : \
        $(call gb_Extension_get_rootdir,$(1))/help/$(5)-xhp.done
$(call gb_Extension_get_rootdir,$(1))/help/$(5)/$(3) : \
        $(gb_Extension_TREEXDEPS) | \
        $(2)/$(4)
$(call gb_Extension_get_rootdir,$(1))/help/$(5)/$(3) : \
        $(2)/$(or $(4),$(3))
	$$(call gb_Output_announce,$(1) $(3) $(5),$(true),TRE,3)
	$$(call gb_Helper_abbreviate_dirs, \
		mkdir -p $$(dir $$@) && \
		$(if $(filter qtz,$(5)), \
			$(gb_Extension_TREEXCOMMAND) -i $$< -o $$@ -l $(5) -m \
				-r $$(call gb_Extension_get_workdir,$(1))/help/$(5)/$(6) \
			, \
			$(if $(filter-out en-US,$(5)), \
				MERGEINPUT=`$(gb_MKTEMP)` && \
				echo $$(POFILE) > $$$${MERGEINPUT} && \
				$(gb_Extension_TREEXCOMMAND) -i $$< -o $$@ -l $(5) \
					-m $$$${MERGEINPUT} \
					-r $$(call gb_Extension_get_workdir,$(1))/help/$(5)/$(6) && \
				rm -rf $$$${MERGEINPUT} \
				, \
				$(gb_Extension_TREEXCOMMAND) -i $$< -o $$@ -l $(5) \
					-r $$(call gb_Extension_get_workdir,$(1))/help/$(5)/$(6) \
			) \
		) \
	)

endef

# compile help for one language; the result is stored as help/$(3)/ in the
# extension's rootdir and marked for zipping into the .oxt
# $(1): extension identifier
# $(2): language
# Target-specific HELPFILES: list of relative paths of .xhp files (see
#     gb_Extension_add_helpfile)
define gb_Extension__compile_help_onelang
$(call gb_Extension_get_rootdir,$(1))/help/$(2).done : \
        $(gb_Extension_HELPINDEXERDEPS) \
        $(gb_Extension_HELPLINKERDEPS) \
        $(OUTDIR_FOR_BUILD)/bin/embed.xsl \
        $(OUTDIR_FOR_BUILD)/bin/idxcaption.xsl \
        $(OUTDIR_FOR_BUILD)/bin/idxcontent.xsl | \
        $(call gb_Extension_get_rootdir,$(1))/help/.dir
	$$(call gb_Output_announce,$(1) $(2),$(true),XHC,3)
	$$(call gb_Helper_abbreviate_dirs, \
        mkdir -p $$(basename $$@) && \
        $$(gb_Extension_HELPLINKERCOMMAND) -mod help \
            -extlangsrc $(call gb_Extension_get_workdir,$(1))/help/$(2) \
            -sty $(OUTDIR_FOR_BUILD)/bin/embed.xsl \
            -extlangdest $$(basename $$@) \
            -idxcaption $(OUTDIR_FOR_BUILD)/bin/idxcaption.xsl \
            -idxcontent $(OUTDIR_FOR_BUILD)/bin/idxcontent.xsl \
            $$(HELPFILES) && \
        (cd $(call gb_Extension_get_workdir,$(1))/help/$(2) && \
            $$(gb_Extension_ZIPCOMMAND) -r $$(basename $$@)/help.jar \
            $$(HELPFILES)) && \
        $$(gb_Extension_HELPINDEXERCOMMAND) -lang $(2) -mod help \
            -dir $$(basename $$@) && \
            rm -fr $$(basename $$@)/caption $$(basename $$@)/content && \
        touch $$@)

endef

# establish the dependency that actually causes inclusion of the compiled help
# into the .oxt, for one language; in principle, this would only need to be done
# once per language iff the extension uses any help -- currently it is done from
# each individual gb_Extension_add_helpfile call (and thus requires $strip
# to remove duplicates from FILES)
# $(1): extension identifier
# $(2): language
define gb_Extension__add_compiled_help_dependency_onelang
$(call gb_Extension_get_target,$(1)) : FILES += help/$(2)
$(call gb_Extension_get_target,$(1)) : \
        $(call gb_Extension_get_rootdir,$(1))/help/$(2).done
$(call gb_Extension_get_rootdir,$(1))/help/$(2).done \
	:| $(call gb_Extension__get_preparation_target,$(1))

endef

# vim: set noet sw=4 ts=4:
