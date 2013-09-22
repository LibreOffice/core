# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# This gbuild module handles building of localized help packs. The main
# entry point is class HelpTarget; the other classes should be
# considered private implementation details and not used directly from
# outside of this file.
#
# All defined objects must be named <help-module>/<lang>. If this naming
# scheme is not followed, bad things will happen!

# Overview of classes and dependencies

# class						task							depends on
# HelpTranslatePartTarget	l10n of xhp files in one dir	xhp file(s)
# HelpTranslateTarget		l10n of xhp files				HelpTranslatePartTarget
# HelpTreeTarget			l10n of tree file				tree file
# HelpLinkTarget			linking help module				HelpTranslateTarget
#															HelpTreeTarget
# HelpJarTarget				creating jar of xhp files		HelpLinkTarget
# HelpIndexTarget			indexing help module			HelpLinkTarget
# HelpTarget				creating help pack				HelpIndexTarget
# 															HelpJarTarget
# 															extra added files

# class HelpTranslatePartTarget

# Translates .xhp files in one directory.

gb_HelpTranslatePartTarget_DEPS := $(call gb_Executable_get_runtime_dependencies,helpex)
gb_HelpTranslatePartTarget_COMMAND := $(call gb_Executable_get_command,helpex)

define gb_HelpTranslatePartTarget__command
$(call gb_Output_announce,$(2),$(true),HPX,1)
HELPFILES=$(call var2file,$(shell $(gb_MKTEMP)),100,$(sort $(filter %.xhp,$(3)))) && \
$(call gb_Helper_abbreviate_dirs, \
	$(if $(filter-out qtz,$(HELP_LANG)), \
		POFILES=$(call var2file,$(shell $(gb_MKTEMP)),100,$(sort $(POFILES))) && \
		$(gb_HelpTranslatePartTarget_COMMAND) \
			-l $(HELP_LANG) \
			-mi $${HELPFILES} \
			-m $${POFILES} \
			-o $(call gb_HelpTranslatePartTarget_get_workdir,$(2)) && \
		rm -f $${POFILES} \
		, \
		$(gb_HelpTranslatePartTarget_COMMAND) \
			-l $(HELP_LANG) \
			-mi $${HELPFILES} \
			-m \
			-o $(call gb_HelpTranslatePartTarget_get_workdir,$(2)) \
	) \
) && \
touch $@ && \
rm -f $${HELPFILES}

endef

$(dir $(call gb_HelpTranslatePartTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_HelpTranslatePartTarget_get_target,%) : $(gb_HelpTranslatePartTarget_DEPS)
	$(call gb_HelpTranslatePartTarget__command,$@,$*,$^)

.PHONY : $(call gb_HelpTranslatePartTarget_get_clean_target,%)
$(call gb_HelpTranslatePartTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),HPX,1)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf \
			$(call gb_HelpTranslatePartTarget_get_target,$*) \
			$(call gb_HelpTranslatePartTarget_get_workdir,$*) \
	)

# Translate a set of .xhp files from one directory.
#
# gb_HelpTranslatePartTarget_HelpTranslatePartTarget target lang dir
define gb_HelpTranslatePartTarget_HelpTranslatePartTarget
$(call gb_HelpTranslatePartTarget_get_target,$(1)) : HELP_LANG := $(2)
$(call gb_HelpTranslatePartTarget_get_target,$(1)) : POFILES := $(gb_POLOCATION)/$(2)/$(3).po

$(call gb_HelpTranslatePartTarget_get_target,$(1)) : $(gb_Module_CURRENTMAKEFILE)
$(call gb_HelpTranslatePartTarget_get_target,$(1)) : $(gb_POLOCATION)/$(2)/$(3).po
$(gb_POLOCATION)/$(2)/$(3).po :
$(call gb_HelpTranslatePartTarget_get_target,$(1)) :| $(dir $(call gb_HelpTranslatePartTarget_get_target,$(1))).dir
$(call gb_HelpTranslatePartTarget_get_target,$(1)) :| $(call gb_HelpTranslatePartTarget_get_workdir,$(1))/.dir

endef

define gb_HelpTranslatePartTarget_add_file
$(call gb_HelpTranslatePartTarget_get_target,$(1)) : $(2)

endef

# class HelpTranslateTarget

# Translates a set of .xhp files.

gb_HelpTranslateTarget__get_lang = $(lastword $(subst /, ,$(1)))

gb_HelpTranslateTarget__get_partname = $(call gb_HelpTranslateTarget__get_lang,$(1))/$(patsubst %/,%,$(2))
gb_HelpTranslateTarget__get_part_workdir = $(call gb_HelpTranslatePartTarget_get_workdir,$(call gb_HelpTranslateTarget__get_partname,$(1),$(2)))

gb_HelpTranslateTarget_get_translated_target = $(call gb_HelpTranslatePartTarget_get_translated_target,$(call gb_HelpTranslateTarget__get_partname,$(1),$(dir $(2))),$(notdir $(2)))
gb_HelpTranslateTarget__get_any_translated_target = $(abspath $(call gb_HelpTranslatePartTarget_get_translated_target,,$(1)))
gb_HelpTranslateTarget_get_workdir = $(call gb_HelpTranslateTarget__get_part_workdir,$(1),$(2))

$(dir $(call gb_HelpTranslateTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_HelpTranslateTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_HelpTranslateTarget_get_target,%) :
	$(call gb_Output_announce,$*,$(true),XHP,2)
	touch $@

$(call gb_HelpTranslateTarget__get_any_translated_target,%) :
	touch $@

.PHONY : $(call gb_HelpTranslateTarget_get_clean_target,%)
$(call gb_HelpTranslateTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),XHP,2)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_HelpTranslateTarget_get_target,$*) \
	)

# Localizes a set of .xhp files to one language.
#
# gb_HelpTranslateTarget_HelpTranslateTarget module
define gb_HelpTranslateTarget_HelpTranslateTarget
$(call gb_HelpTranslateTarget_get_target,$(1)) :| $(dir $(call gb_HelpTranslateTarget_get_target,$(1))).dir

endef

# gb_HelpTranslateTarget__make_part module part lang dir
define gb_HelpTranslateTarget__make_part
$(call gb_HelpTranslatePartTarget_HelpTranslatePartTarget,$(2),$(3),$(patsubst %/,%,$(4)))

$(call gb_HelpTranslateTarget_get_target,$(1)) : $(call gb_HelpTranslatePartTarget_get_target,$(2))
$(call gb_HelpTranslateTarget_get_clean_target,$(1)) : $(call gb_HelpTranslatePartTarget_get_clean_target,$(2))

endef

# gb_HelpTranslateTarget__add_file module dir file
define gb_HelpTranslateTarget__add_file
$(call gb_HelpTranslatePartTarget_add_file,$(call gb_HelpTranslateTarget__get_partname,$(1),$(2)),$(SRCDIR)/$(3).xhp)
$(call gb_HelpTranslateTarget_get_translated_target,$(1),$(3)) : $(call gb_HelpTranslateTarget_get_target,$(1))

endef

# gb_HelpTranslateTarget__add_files_impl module lang dir(s) file(s)
define gb_HelpTranslateTarget__add_files_impl
$(foreach part,$(3),$(call gb_HelpTranslateTarget__make_part,$(1),$(call gb_HelpTranslateTarget__get_partname,$(1),$(part)),$(2),$(part)))
$(foreach file,$(4),$(call gb_HelpTranslateTarget__add_file,$(1),$(dir $(file)),$(file)))

endef

# gb_HelpTranslateTarget__add_files module file(s)
define gb_HelpTranslateTarget__add_files
$(call gb_HelpTranslateTarget__add_files_impl,$(1),$(call gb_HelpTranslateTarget__get_lang,$(1)),$(sort $(dir $(2))),$(2))

endef

# gb_HelpTranslateTarget_add_file module file
define gb_HelpTranslateTarget_add_file
$(call gb_HelpTranslateTarget__add_files,$(1),$(2))

endef

# gb_HelpTranslateTarget_add_files module file(s)
define gb_HelpTranslateTarget_add_files
$(call gb_HelpTranslateTarget__add_files,$(1),$(2))

endef

# class HelpTree

# Translates a .tree file.

gb_HelpTreeTarget_DEPS := $(call gb_Executable_get_runtime_dependencies,treex)
gb_HelpTreeTarget_COMMAND := $(call gb_Executable_get_command,treex)

define gb_HelpTreeTarget__command
$(call gb_Output_announce,$(2),$(true),TRE,1)
$(call gb_Helper_abbreviate_dirs,\
	$(if $(filter-out qtz,$(HELP_LANG)), \
		POFILES=$(call var2file,$(shell $(gb_MKTEMP)),100,$(POFILES)) && \
		$(gb_HelpTreeTarget_COMMAND) \
			-i $(HELP_TREE) \
			-l $(HELP_LANG) \
			-m $${POFILES} \
			-o $@ \
			-r $(HELP_TEXTDIR) && \
		rm -f $${POFILES} \
		, \
		$(gb_HelpTreeTarget_COMMAND) \
			-i $(HELP_TREE) \
			-l $(HELP_LANG) \
			-m \
			-o $@ \
			-r $(HELP_TEXTDIR) \
	) \
)

endef

$(dir $(call gb_HelpTreeTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_HelpTreeTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_HelpTreeTarget_get_target,%) : $(gb_HelpTreeTarget_DEPS)
	$(if $(HELP_TEXTDIR),,$(call gb_Output_error,HelpTreeTarget: no help text dir for .tree was set))
	$(if $(HELP_TREE),,$(call gb_Output_error,HelpTreeTarget: no .tree file to translate was set))
	$(call gb_HelpTreeTarget__command,$@,$*)

.PHONY : $(call gb_HelpTreeTarget_get_clean_target,%)
$(call gb_HelpTreeTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),TRE,1)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_HelpTreeTarget_get_target,$*) \
	)

# Translate a .tree file.
#
# gb_HelpTreeTarget_HelpTreeTarget target lang
define gb_HelpTreeTarget_HelpTreeTarget
$(call gb_HelpTreeTarget_get_target,$(1)) : HELP_LANG := $(2)
$(call gb_HelpTreeTarget_get_target,$(1)) : HELP_TEXTDIR :=
$(call gb_HelpTreeTarget_get_target,$(1)) : HELP_TREE :=
$(call gb_HelpTreeTarget_get_target,$(1)) : POFILES :=

$(call gb_HelpTreeTarget_get_target,$(1)) :| $(dir $(call gb_HelpTreeTarget_get_target,$(1))).dir

endef

# gb_HelpTreeTarget__set_pofiles target pofiles
define gb_HelpTreeTarget__set_pofiles
$(call gb_HelpTreeTarget_get_target,$(1)) : POFILES := $(2)
$(call gb_HelpTreeTarget_get_target,$(1)) : $(2)
$(2) :

endef

# gb_HelpTreeTarget_set_treefile target treefile
define gb_HelpTreeTarget_set_treefile
$(call gb_HelpTreeTarget__set_pofiles,$(1),$(gb_POLOCATION)/$(lastword $(subst /, ,$(1)))/$(patsubst %/,%,$(dir $(2))).po)

$(call gb_HelpTreeTarget_get_target,$(1)) : HELP_TREE := $(SRCDIR)/$(2).tree
$(call gb_HelpTreeTarget_get_target,$(1)) : $(SRCDIR)/$(2).tree

endef

# gb_HelpTreeTarget_set_helptextdir target dir
define gb_HelpTreeTarget_set_helptextdir
$(call gb_HelpTreeTarget_get_target,$(1)) : HELP_TEXTDIR := $(2)

endef

# class HelpLinkTarget

# Links a help module.

gb_HelpLinkTarget_HELPLINKERDEPS := $(call gb_Executable_get_runtime_dependencies,HelpLinker)
gb_HelpLinkTarget_HELPLINKERCOMMAND := $(call gb_Executable_get_command,HelpLinker)
gb_HelpLinkTarget_COMPACTTARGET := $(OUTDIR)/bin/compact.xsl
gb_HelpLinkTarget_EMBEDTARGET := $(OUTDIR)/bin/embed.xsl
gb_HelpLinkTarget_IDXCAPTIONTARGET := $(OUTDIR)/bin/idxcaption.xsl
gb_HelpLinkTarget_IDXCONTENTTARGET := $(OUTDIR)/bin/idxcontent.xsl
gb_HelpLinkTarget_DEPS := \
	$(gb_HelpLinkTarget_HELPLINKERDEPS) \
	$(gb_HelpLinkTarget_COMPACTTARGET) \
	$(gb_HelpLinkTarget_EMBEDTARGET) \
	$(gb_HelpLinkTarget_IDXCAPTIONTARGET) \
	$(gb_HelpLinkTarget_IDXCONTENTTARGET)

# delete index files here too just to be on the safe side...
# the index files in the .idxl dir are ceated by HelpIndexer,
# the ones outside the dir by HelpLinker
define gb_HelpLinkTarget__command
$(call gb_Output_announce,$(2),$(true),HLK,3)
	$(if $(HELP_INDEXED),rm -rf $(addprefix $(HELP_WORKDIR)/,$(HELP_INDEXED)) && \)
RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),100,\
	-lang $(HELP_LANG) \
	-mod $(HELP_MODULE) \
	$(if $(HELP_INDEXED),,-noindex) \
	-nolangroot \
	-o $(WORKDIR)/dummy.zip \
	-src $(HELP_SRCDIR) \
	-zipdir $(HELP_WORKDIR) \
	-compact $(gb_HelpLinkTarget_COMPACTTARGET) \
	-idxcaption $(gb_HelpLinkTarget_IDXCAPTIONTARGET) \
	-idxcontent $(gb_HelpLinkTarget_IDXCONTENTTARGET) \
	-sty $(gb_HelpLinkTarget_EMBEDTARGET) \
	$(if $(HELP_CONFIGFILE),-add $(HELP_MODULE).cfg $(HELP_CONFIGFILE)) \
	$(if $(HELP_TREE),-add $(HELP_MODULE).tree $(HELP_TREE)) \
	$(foreach file,$(HELP_ADD_FILES),-add $(notdir $(file)) $(file)) \
	$(foreach extra,$(HELP_EXTRA_ADD_FILES),-add $(subst $(COMMA), ,$(extra))) \
	$(HELP_FILES) \
	$(if $(HELP_LINKED_MODULES),\
		$(shell cat $(foreach module,$(HELP_LINKED_MODULES),$(call gb_HelpTarget_get_filelist,$(module)))) \
	) \
) && \
$(gb_HelpLinkTarget_HELPLINKERCOMMAND) @$${RESPONSEFILE} && \
touch $@ && \
rm -f $${RESPONSEFILE}
endef


$(dir $(call gb_HelpLinkTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_HelpLinkTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_HelpLinkTarget_get_target,%) : $(gb_HelpLinkTarget_DEPS)
	$(call gb_HelpLinkTarget__command,$@,$*)

.PHONY : $(call gb_HelpLinkTarget_get_clean_target,%)
$(call gb_HelpLinkTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),HLK,3)
	rm -f $(call gb_HelpLinkTarget_get_target,$*)

# Create a help linking target.
#
# depend on makefile to re-build when files are removed
#
# gb_HelpLinkTarget_HelpLinkTarget name module lang workdir
define gb_HelpLinkTarget_HelpLinkTarget
$(call gb_HelpLinkTarget_get_target,$(1)) : HELP_ADD_FILES :=
$(call gb_HelpLinkTarget_get_target,$(1)) : HELP_CONFIGFILE :=
$(call gb_HelpLinkTarget_get_target,$(1)) : HELP_EXTRA_ADD_FILES :=
$(call gb_HelpLinkTarget_get_target,$(1)) : HELP_FILES :=
$(call gb_HelpLinkTarget_get_target,$(1)) : HELP_INDEXED :=
$(call gb_HelpLinkTarget_get_target,$(1)) : HELP_MODULE := $(2)
$(call gb_HelpLinkTarget_get_target,$(1)) : HELP_LANG := $(3)
$(call gb_HelpLinkTarget_get_target,$(1)) : HELP_LINKED_MODULES :=
$(call gb_HelpLinkTarget_get_target,$(1)) : HELP_SRCDIR :=
$(call gb_HelpLinkTarget_get_target,$(1)) : HELP_TREE :=
$(call gb_HelpLinkTarget_get_target,$(1)) : HELP_WORKDIR := $(4)

$(call gb_HelpLinkTarget_get_target,$(1)) : $(gb_Module_CURRENTMAKEFILE)
$(call gb_HelpLinkTarget_get_target,$(1)) :| $(dir $(call gb_HelpLinkTarget_get_target,$(1))).dir

endef

# gb_HelpLinkTarget_set_configfile target configfile
define gb_HelpLinkTarget_set_configfile
$(call gb_HelpLinkTarget_get_target,$(1)) : HELP_CONFIGFILE := $(2)
$(call gb_HelpLinkTarget_get_target,$(1)) : $(2)

endef

# gb_HelpLinkTarget_set_sourcedir target source
define gb_HelpLinkTarget_set_sourcedir
$(call gb_HelpLinkTarget_get_target,$(1)) : HELP_SRCDIR := $(2)

endef

# gb_HelpLinkTarget_set_treefile target treefile
define gb_HelpLinkTarget_set_treefile
$(call gb_HelpLinkTarget_get_target,$(1)) : HELP_TREE := $(2)
$(call gb_HelpLinkTarget_get_target,$(1)) : $(2)

endef

# gb_HelpLinkTarget_set_indexed target indexfiles
define gb_HelpLinkTarget_set_indexed
$(call gb_HelpLinkTarget_get_target,$(1)) : HELP_INDEXED := $(2)

endef

# gb_HelpLinkTarget_add_helpfile target helpfile
define gb_HelpLinkTarget_add_helpfile
$(call gb_HelpLinkTarget_get_target,$(1)) : HELP_FILES += $(2)
$(call gb_HelpLinkTarget_get_target,$(1)) : $(2)

endef

# Add an arbitrary file to the help pack.
#
# The file will be added to the root directory of the pack.
#
# gb_HelpLinkTarget_add_file target file
define gb_HelpLinkTarget_add_file
$(call gb_HelpLinkTarget_get_target,$(1)) : HELP_ADD_FILES += $(2)
$(call gb_HelpLinkTarget_get_target,$(1)) : $(2)

endef

# Add an arbitrary file to the help pack under a new name.
#
# The file will be added to the root directory of the pack.
#
# gb_HelpLinkTarget_add_renamed_file target filename file
define gb_HelpLinkTarget_add_renamed_file
$(call gb_HelpLinkTarget_get_target,$(1)) : HELP_EXTRA_ADD_FILES += $(strip $(2)),$(strip $(3))
$(call gb_HelpLinkTarget_get_target,$(1)) : $(3)

endef

# Link with help files from another help module.
#
# gb_HelpLinkTarget_use_linked_module target help
define gb_HelpLinkTarget_use_linked_module
$(call gb_HelpLinkTarget_get_target,$(1)) : HELP_LINKED_MODULES += $(2)

endef

# class HelpIndexTarget

# Creates a full-text search index for a help module.

gb_HelpIndexTarget_DEPS := $(call gb_Executable_get_runtime_dependencies,HelpIndexer)
gb_HelpIndexTarget_COMMAND := $(call gb_Executable_get_command,HelpIndexer)

# first delete the index stuff since when it is generated an existing _0.cfs
# will not be overwritten; instead a new _1.cfs etc. created until disk is full
define gb_HelpIndexTarget__command
$(call gb_Output_announce,$*,$(true),HIX,3)
$(call gb_Helper_abbreviate_dirs,\
	(\
		rm -rf $(HELP_WORKDIR)/$(HELP_MODULE).idxl \
		&& $(gb_HelpIndexTarget_COMMAND) \
			-dir $(HELP_WORKDIR) \
			-lang $(HELP_LANG) \
			-mod $(HELP_MODULE) \
		&& touch $@ \
	) \
	|| ( rm -rf $(HELP_MODULE).* ; false )
)
endef

$(dir $(call gb_HelpIndexTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_HelpIndexTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_HelpIndexTarget_get_target,%) : $(gb_HelpIndexTarget_DEPS)
	$(call gb_HelpIndexTarget__command,$@,$*)

.PHONY : $(call gb_HelpIndexTarget_get_clean_target,%)
$(call gb_HelpIndexTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),HIX,3)
	rm -f $(call gb_HelpIndexTarget_get_target,$*)

# Create a help indexing target.
#
# gb_HelpIndexTarget_HelpIndexTarget target module lang workdir
define gb_HelpIndexTarget_HelpIndexTarget
$(call gb_HelpIndexTarget_get_target,$(1)) : HELP_LANG := $(3)
$(call gb_HelpIndexTarget_get_target,$(1)) : HELP_MODULE := $(2)
$(call gb_HelpIndexTarget_get_target,$(1)) : HELP_WORKDIR := $(4)

$(call gb_HelpIndexTarget_get_target,$(1)) :| $(dir $(call gb_HelpIndexTarget_get_target,$(1))).dir

endef

# class HelpJarTarget

# Packs help files processed by HelpLinker into a jar in the workdir.

gb_HelpJarTarget_COMMAND := zip

define gb_HelpJarTarget__get_command
$(call gb_Output_announce,$*,$(true),HEJ,3)
cd $(HELP_WORKDIR) && \
$(gb_HelpJarTarget_COMMAND) -q -0 -rX --filesync --must-match $(HELP_MODULE).jar text/$(HELP_MODULE) && \
touch $@
endef

$(dir $(call gb_HelpJarTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_HelpJarTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_HelpJarTarget_get_target,%) :
	$(call gb_HelpJarTarget__get_command,$@,$*)

$(call gb_HelpJarTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),HEJ,3)
	rm -f $(call gb_HelpJarTarget_get_target,$*)

# gb_HelpJarTarget_HelpJarTarget target module workdir
define gb_HelpJarTarget_HelpJarTarget
$(call gb_HelpJarTarget_get_target,$(1)) : HELP_MODULE := $(2)
$(call gb_HelpJarTarget_get_target,$(1)) : HELP_WORKDIR := $(3)

$(call gb_HelpJarTarget_get_target,$(1)) :| $(dir $(call gb_HelpJarTarget_get_target,$(1))).dir

endef

# class HelpTarget

# Creates one language version of a help module.
#
# Provides a filelist called HelpTarget/<name>, that is not built by
# default (i.e., the user of HelpTarget has to explicitly depend on the
# Package).

gb_HelpTarget_DEFAULT_LANG := en-US

gb_HelpTarget__get_module = $(patsubst %/$(call gb_HelpTarget__get_lang,$(1)),%,$(1))
gb_HelpTarget__get_lang = $(lastword $(subst /, ,$(1)))
gb_HelpTarget__test_default_lang = $(filter $(gb_HelpTarget_DEFAULT_LANG),$(1))
gb_HelpTarget__is_default_lang = $(call gb_HelpTarget__test_default_lang,$(call gb_HelpTarget__get_lang,$(1)))

define gb_HelpTarget__get_helpdir
$(if $(call gb_HelpTarget__is_default_lang,$(1)) \
	,$(SRCDIR)/$(2) \
	,$(call gb_HelpTranslateTarget_get_workdir,$(1),$(2)) \
)
endef

define gb_HelpTarget__get_helpfile
$(if $(call gb_HelpTarget__is_default_lang,$(1)) \
	,$(SRCDIR)/$(2).xhp \
	,$(call gb_HelpTranslateTarget_get_translated_target,$(1),$(2)) \
)
endef

define gb_HelpTarget__get_treefile
$(if $(call gb_HelpTarget__is_default_lang,$(1)) \
	,$(SRCDIR)/$(2).tree \
	,$(call gb_HelpTreeTarget_get_target,$(1)) \
)
endef

define gb_HelpTarget__command
$(call gb_Output_announce,$(2),$(true),HLP,4)
touch $@
endef

$(dir $(call gb_HelpTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_HelpTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

# Translation of the module's .xhp files and all used modules is done.
# Also creates the list for gb_HelpTarget_get_filelist.
$(call gb_HelpTarget_get_translation_target,%) :
	rm -f $@ && mv $(call var2file,$@.tmp,100,$(HELP_FILES)) $@

# Translation of the module's .xhp files and all used and linked modules
# is done.
$(call gb_HelpTarget_get_linked_target,%) :
	touch $@

$(call gb_HelpTarget_get_target,%) :
	$(call gb_HelpTarget__command,$@,$*)

.PHONY : $(call gb_HelpTarget_get_clean_target,%)
$(call gb_HelpTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),HLP,4)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf \
			$(call gb_HelpTarget_get_linked_target,$*) \
			$(call gb_HelpTarget_get_target,$*) \
			$(call gb_HelpTarget_get_translation_target,$*) \
			$(call gb_HelpTarget_get_workdir,$*) \
	)

gb_HelpTarget_get_packagename = HelpTarget/$(1)

# Create a help target.
#
# depend on makefile to re-build filelist when files are removed
#
# gb_HelpTarget_HelpTarget target module lang
define gb_HelpTarget_HelpTarget
$(call gb_HelpTarget_get_target,$(1)) : HELP_MODULE := $(2)
$(call gb_HelpTarget_get_target,$(1)) : HELP_LANG := $(3)

$(call gb_HelpTarget_get_translation_target,$(1)) : HELP_FILES :=
$(call gb_HelpTarget_get_translation_target,$(1)) : $(gb_Module_CURRENTMAKEFILE)

$(call gb_HelpTarget__HelpTarget_impl,$(1),$(2),$(3),$(call gb_HelpTarget_get_workdir,$(1)),$(call gb_HelpTarget_get_packagename,$(1)))

endef

# gb_HelpTarget__HelpTarget_impl target module lang workdir package
define gb_HelpTarget__HelpTarget_impl
$(if $(call gb_HelpTarget__test_default_lang,$(3)),,$(call gb_HelpTarget__HelpTarget_impl_lang,$(1),$(2),$(3),$(4)))
$(call gb_HelpLinkTarget_HelpLinkTarget,$(1),$(2),$(3),$(4))
$(call gb_HelpIndexTarget_HelpIndexTarget,$(1),$(2),$(3),$(4))
$(call gb_HelpJarTarget_HelpJarTarget,$(1),$(2),$(4))
$(call gb_Package_Package_internal,$(5),$(4))
$(call gb_Package_set_outdir,$(5),$(INSTROOT))

$(call gb_HelpTarget_get_linked_target,$(1)) : $(call gb_HelpTarget_get_translation_target,$(1))
$(call gb_HelpLinkTarget_get_target,$(1)) : $(call gb_HelpTarget_get_linked_target,$(1))
$(call gb_HelpLinkTarget_get_target,$(1)) :| $(call gb_HelpTarget_get_workdir,$(1))/.dir
$(call gb_HelpTarget_get_target,$(1)) : $(call gb_HelpLinkTarget_get_target,$(1))
$(call gb_Package_get_preparation_target,$(5)) : $(call gb_HelpTarget_get_target,$(1))

$(call gb_HelpTarget_get_linked_target,$(1)) :| $(dir $(call gb_HelpTarget_get_linked_target,$(1))).dir
$(call gb_HelpTarget_get_target,$(1)) :| $(dir $(call gb_HelpTarget_get_target,$(1))).dir
$(call gb_HelpTarget_get_translation_target,$(1)) :| $(dir $(call gb_HelpTarget_get_translation_target,$(1))).dir

$(call gb_HelpTarget_get_clean_target,$(1)) : $(call gb_HelpLinkTarget_get_clean_target,$(1))

endef

# gb_HelpTarget__HelpTarget_impl_lang target module lang workdir
define gb_HelpTarget__HelpTarget_impl_lang
$(call gb_HelpTranslateTarget_HelpTranslateTarget,$(1),$(3))
$(call gb_HelpTreeTarget_HelpTreeTarget,$(1),$(3))

$(call gb_HelpTarget_get_translation_target,$(1)) : $(call gb_HelpTranslateTarget_get_target,$(1))
$(call gb_HelpTreeTarget_get_target,$(1)) : $(call gb_HelpTarget_get_linked_target,$(1))

$(call gb_HelpTarget_get_clean_target,$(1)) : $(call gb_HelpTranslateTarget_get_clean_target,$(1))
$(call gb_HelpTarget_get_clean_target,$(1)) : $(call gb_HelpTreeTarget_get_clean_target,$(1))

endef

# Get list of the various index files.
#
# gb_HelpTarget__add_index_files target module
define gb_HelpTarget__get_index_files
$(foreach suffix,.db .ht .idxl/_0.cfs .idxl/segments_3 .idxl/segments.gen .key,$(addsuffix $(suffix),$(call gb_HelpTarget__get_module,$(1))))
endef

# gb_HelpTarget__add_file target file
define gb_HelpTarget__add_file
$(call gb_Package_add_file,$(call gb_HelpTarget_get_packagename,$(1)),help/$(call gb_HelpTarget__get_lang,$(1))/$(2),$(2))

endef

# Set config. file used for the help module.
#
# The configfile is relative to $(SRCDIR) and without extension.
#
# gb_HelpTarget_set_configfile target configfile
define gb_HelpTarget_set_configfile
$(call gb_HelpLinkTarget_set_configfile,$(1),$(SRCDIR)/$(2).cfg)
$(call gb_HelpTarget__add_file,$(1),$(call gb_HelpTarget__get_module,$(1)).cfg)

endef

# gb_HelpTarget_set_helpdir target helpdir
define gb_HelpTarget_set_helpdir
$(call gb_HelpLinkTarget_set_sourcedir,$(1),$(call gb_HelpTarget__get_helpdir,$(1),$(2)))

endef

# gb_HelpTarget_set_treefile target treefile textdir
define gb_HelpTarget_set_treefile
$(if $(call gb_HelpTarget__is_default_lang,$(1)),,\
	$(call gb_HelpTreeTarget_set_treefile,$(1),$(2)) \
	$(call gb_HelpTreeTarget_set_helptextdir,$(1),$(call gb_HelpTarget__get_helpdir,$(1),$(3))) \
)
$(call gb_HelpLinkTarget_set_treefile,$(1),$(call gb_HelpTarget__get_treefile,$(1),$(2)))
$(call gb_HelpTarget__add_file,$(1),$(call gb_HelpTarget__get_module,$(1)).tree)

endef

# Produce full text search index, bookmark list, etc.
#
# gb_HelpTarget_set_indexed target
define gb_HelpTarget_set_indexed
$(call gb_HelpLinkTarget_set_indexed,$(1),$(call gb_HelpTarget__get_index_files,$(1)))
$(foreach file,$(call gb_HelpTarget__get_index_files,$(1)),$(call gb_HelpTarget__add_file,$(1),$(file)))

$(call gb_HelpIndexTarget_get_target,$(1)) : $(call gb_HelpLinkTarget_get_target,$(1))
$(call gb_HelpTarget_get_target,$(1)) : $(call gb_HelpIndexTarget_get_target,$(1))
$(call gb_HelpTarget_get_clean_target,$(1)) : $(call gb_HelpIndexTarget_get_clean_target,$(1))

endef

# gb_HelpTarget__add_jar target
define gb_HelpTarget__add_jar
$(call gb_HelpTarget__add_file,$(1),$(call gb_HelpTarget__get_module,$(1)).jar)
$(call gb_HelpJarTarget_get_target,$(1)) : $(call gb_HelpLinkTarget_get_target,$(1))
$(call gb_HelpTarget_get_target,$(1)) : $(call gb_HelpJarTarget_get_target,$(1))
$(call gb_HelpTarget_get_clean_target,$(1)) : $(call gb_HelpJarTarget_get_clean_target,$(1))

endef

define gb_HelpTarget__add_helpfile_impl
$(call gb_HelpLinkTarget_add_helpfile,$(1),$(2))

$(call gb_HelpTarget_get_translation_target,$(1)) : HELP_FILES += $(2)

endef

# gb_HelpTarget__add_helpfile target helpfile
define gb_HelpTarget__add_helpfile
$(call gb_HelpTarget__add_helpfile_impl,$(1),$(call gb_HelpTarget__get_helpfile,$(1),$(2)))

endef

# gb_HelpTarget_add_helpfile target helpfile
define gb_HelpTarget_add_helpfile
$(call gb_HelpTranslateTarget_add_file,$(1),$(2))
$(call gb_HelpTarget__add_helpfile,$(1),$(2))
$(call gb_HelpTarget__add_jar,$(1))

endef

# gb_HelpTarget_add_helpfiles target helpfile(s)
define gb_HelpTarget_add_helpfiles
$(call gb_HelpTranslateTarget_add_files,$(1),$(2))
$(foreach helpfile,$(2),$(call gb_HelpTarget__add_helpfile,$(1),$(helpfile)))
$(call gb_HelpTarget__add_jar,$(1))

endef

# gb_HelpTarget_add_file target file
define gb_HelpTarget_add_file
$(call gb_HelpLinkTarget_add_file,$(1),$(SRCDIR)/$(2))
$(call gb_HelpTarget__add_file,$(1),$(notdir $(2)))

endef

# gb_HelpTarget_add_files target file(s)
define gb_HelpTarget_add_files
$(foreach file,$(2),$(call gb_HelpTarget_add_file,$(1),$(file)))

endef

# Add a localized file from helpdir under a new name.
#
# This is a hack needed for err.html in shared help module.
#
# gb_HelpTarget_add_helpdir_file target filename file
define gb_HelpTarget_add_helpdir_file
$(call gb_HelpLinkTarget_add_renamed_file,$(1),$(2),$(call gb_HelpTarget__get_helpfile,$(1),$(3)))
$(call gb_HelpTarget__add_file,$(1),$(2))

endef

# gb_HelpTarget_use_module target module
define gb_HelpTarget_use_module
$(call gb_HelpTarget_get_translation_target,$(1)) : $(call gb_HelpTarget_get_translation_target,$(2))

endef

# gb_HelpTarget_use_modules target module(s)
define gb_HelpTarget_use_modules
$(foreach module,$(2),$(call gb_HelpTarget_use_module,$(1),$(module)))

endef

# gb_HelpTarget_use_linked_module target module
define gb_HelpTarget_use_linked_module
$(call gb_HelpLinkTarget_use_linked_module,$(1),$(2))
$(call gb_HelpTarget_get_linked_target,$(1)) : $(call gb_HelpTarget_get_translation_target,$(2))

endef

# gb_HelpTarget_use_linked_modules target module(s)
define gb_HelpTarget_use_linked_modules
$(foreach module,$(2),$(call gb_HelpTarget_use_linked_module,$(1),$(module)))

endef

# vim: set noet sw=4 ts=4:
