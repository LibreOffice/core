# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# This gbuild module handles the l10n of help modules. The main
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
# HelpTarget				l10n of help module				HelpTranslateTarget
#															HelpTreeTarget

# class HelpTranslatePartTarget

# Translates .xhp files in one directory.

gb_HelpTranslatePartTarget_DEPS := $(call gb_Executable_get_runtime_dependencies,helpex)
gb_HelpTranslatePartTarget_COMMAND := $(call gb_Executable_get_command,helpex)

define gb_HelpTranslatePartTarget__command
HELPFILES=$(call gb_var2file,$(call gb_TmpFile,$(1).help),$(sort $(filter %.xhp,$(3)))) && \
$(call gb_Helper_abbreviate_dirs, \
	$(if $(filter-out qtz,$(HELP_LANG)), \
		POFILES=$(call gb_var2file,$(call gb_TmpFile,$(1).po),$(sort $(POFILES))) && \
		$(gb_HelpTranslatePartTarget_COMMAND) \
			-l $(HELP_LANG) \
			-mi $${HELPFILES} \
			-m $${POFILES} \
			-o $(gb_HelpTranslatePartTarget_workdir)/$(2) && \
		rm -f $${POFILES} \
		, \
		$(gb_HelpTranslatePartTarget_COMMAND) \
			-l $(HELP_LANG) \
			-mi $${HELPFILES} \
			-m \
			-o $(gb_HelpTranslatePartTarget_workdir)/$(2) \
	) \
) && \
touch $@ && \
rm -f $${HELPFILES}

endef

$(dir $(call gb_HelpTranslatePartTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_HelpTranslatePartTarget_get_target,%) : $(gb_HelpTranslatePartTarget_DEPS)
	$(call gb_Output_announce,$*,$(true),HPX,1)
	$(call gb_Trace_StartRange,$*,HPX)
	$(call gb_HelpTranslatePartTarget__command,$@,$*,$^)
	$(call gb_Trace_EndRange,$*,HPX)

clear_HelpTranslatePartTarget:
	$(call gb_Output_announce,clear HelpTranslatePartTarget,$(false),HPX,1)
	rm -rf $(gb_HelpTranslatePartTarget_workdir)

# Translate a set of .xhp files from one directory.
#
# gb_HelpTranslatePartTarget_HelpTranslatePartTarget target lang dir
define gb_HelpTranslatePartTarget_HelpTranslatePartTarget
$(call gb_HelpTranslatePartTarget_get_target,$(1)) : HELP_LANG := $(2)
$(call gb_HelpTranslatePartTarget_get_target,$(1)) : POFILES := $(3)

$(call gb_HelpTranslatePartTarget_get_target,$(1)) : $(gb_Module_CURRENTMAKEFILE)
$(call gb_HelpTranslatePartTarget_get_target,$(1)) : $(3)
$(call gb_HelpTranslatePartTarget_get_target,$(1)) :| $(dir $(call gb_HelpTranslatePartTarget_get_target,$(1))).dir
$(call gb_HelpTranslatePartTarget_get_target,$(1)) :| $(gb_HelpTranslatePartTarget_workdir)/$(1)/.dir

endef

define gb_HelpTranslatePartTarget_add_file
$(call gb_HelpTranslatePartTarget_get_target,$(1)) : $(2)

endef

# class HelpTranslateTarget

# Translates a set of .xhp files.

gb_HelpTranslateTarget__get_lang = $(lastword $(subst /, ,$(1)))

gb_HelpTranslateTarget__get_partname = $(call gb_HelpTranslateTarget__get_lang,$(1))/$(patsubst %/,%,$(2))
gb_HelpTranslateTarget__get_part_workdir = $(gb_HelpTranslatePartTarget_workdir)/$(call gb_HelpTranslateTarget__get_partname,$(1),$(2))

gb_HelpTranslateTarget_get_translated_target = $(call gb_HelpTranslatePartTarget_get_translated_target,$(call gb_HelpTranslateTarget__get_partname,$(1),$(dir $(2))),$(notdir $(2)))
gb_HelpTranslateTarget__get_any_translated_target = $(abspath $(call gb_HelpTranslatePartTarget_get_translated_target,,$(1)))
gb_HelpTranslateTarget_get_workdir = $(call gb_HelpTranslateTarget__get_part_workdir,$(1),$(2))

$(dir $(call gb_HelpTranslateTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_HelpTranslateTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

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

# use wildcard to avoid spurious rebuilds if translation is missing
# gb_HelpTranslateTarget__make_part module part lang dir
define gb_HelpTranslateTarget__make_part
$(call gb_HelpTranslatePartTarget_HelpTranslatePartTarget,$(2),$(3),$(wildcard $(gb_POLOCATION)/$(3)/$(patsubst %/,%,$(4)).po))

$(call gb_HelpTranslateTarget_get_target,$(1)) : $(call gb_HelpTranslatePartTarget_get_target,$(2))
$(call gb_HelpTranslateTarget_get_clean_target,$(1)) : clear_HelpTranslatePartTarget

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
$(call gb_Helper_abbreviate_dirs,\
	$(if $(filter-out qtz,$(HELP_LANG)), \
		POFILES=$(call gb_var2file,$(call gb_TmpFile,$(1)),$(POFILES)) && \
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
	$(call gb_Output_announce,$*,$(true),TRE,1)
	$(call gb_Trace_StartRange,$*,TRE)
	$(call gb_HelpTreeTarget__command,$@,$*)
	$(call gb_Trace_EndRange,$*,TRE)

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

endef

# use wildcard to avoid spurious rebuilds if translation is missing
# gb_HelpTreeTarget_set_treefile target treefile
define gb_HelpTreeTarget_set_treefile
$(call gb_HelpTreeTarget__set_pofiles,$(1),$(wildcard $(gb_POLOCATION)/$(lastword $(subst /, ,$(1)))/$(patsubst %/,%,$(dir $(2))).po))

$(call gb_HelpTreeTarget_get_target,$(1)) : HELP_TREE := $(SRCDIR)/$(2).tree
$(call gb_HelpTreeTarget_get_target,$(1)) : $(SRCDIR)/$(2).tree

endef

# gb_HelpTreeTarget_set_helptextdir target dir
define gb_HelpTreeTarget_set_helptextdir
$(call gb_HelpTreeTarget_get_target,$(1)) : HELP_TEXTDIR := $(2)

endef

# class HelpTarget

# Creates one language version of a help module.
#
# Provides a filelist called HelpTarget/<name>, that is not built by
# default.

gb_HelpTarget_DEFAULT_LANG := en-US

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

$(dir $(call gb_HelpTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_HelpTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

# Translation of the module's .xhp files and all used modules is done.
# Also creates the list for gb_HelpTarget_get_filelist.
$(call gb_HelpTarget_get_translation_target,%) :
	$(file >$@,$(strip $(HELP_FILES)))

# Translation of the module's .xhp files and all used modules is done.
$(call gb_HelpTarget_get_linked_target,%) :
	touch $@

$(call gb_HelpTarget_get_target,%) :
	touch $@

.PHONY : $(call gb_HelpTarget_get_clean_target,%)
$(call gb_HelpTarget_get_clean_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf \
			$(call gb_HelpTarget_get_linked_target,$*) \
			$(call gb_HelpTarget_get_target,$*) \
			$(call gb_HelpTarget_get_translation_target,$*) \
	)

# Create a help target.
#
# depend on makefile to re-build filelist when files are removed
#
# gb_HelpTarget_HelpTarget target lang
define gb_HelpTarget_HelpTarget
$(call gb_HelpTarget_get_translation_target,$(1)) : HELP_FILES :=
$(call gb_HelpTarget_get_translation_target,$(1)) : $(gb_Module_CURRENTMAKEFILE)

$(call gb_HelpTarget__HelpTarget_impl,$(1),$(2))

endef

# gb_HelpTarget__HelpTarget_impl target lang
define gb_HelpTarget__HelpTarget_impl
$(if $(call gb_HelpTarget__test_default_lang,$(2)),,$(call gb_HelpTarget__HelpTarget_impl_lang,$(1),$(2)))

$(call gb_HelpTarget_get_linked_target,$(1)) : $(call gb_HelpTarget_get_translation_target,$(1))

$(call gb_HelpTarget_get_linked_target,$(1)) :| $(dir $(call gb_HelpTarget_get_linked_target,$(1))).dir
$(call gb_HelpTarget_get_target,$(1)) :| $(dir $(call gb_HelpTarget_get_target,$(1))).dir
$(call gb_HelpTarget_get_translation_target,$(1)) :| $(dir $(call gb_HelpTarget_get_translation_target,$(1))).dir

endef

# gb_HelpTarget__HelpTarget_impl_lang target lang
define gb_HelpTarget__HelpTarget_impl_lang
$(call gb_HelpTranslateTarget_HelpTranslateTarget,$(1),$(2))
$(call gb_HelpTreeTarget_HelpTreeTarget,$(1),$(2))

$(call gb_HelpTarget_get_translation_target,$(1)) : $(call gb_HelpTranslateTarget_get_target,$(1))
$(call gb_HelpTreeTarget_get_target,$(1)) : $(call gb_HelpTarget_get_linked_target,$(1))

$(call gb_HelpTarget_get_clean_target,$(1)) : $(call gb_HelpTranslateTarget_get_clean_target,$(1))
$(call gb_HelpTarget_get_clean_target,$(1)) : $(call gb_HelpTreeTarget_get_clean_target,$(1))

endef

# gb_HelpTarget_set_treefile target treefile textdir
define gb_HelpTarget_set_treefile
$(if $(call gb_HelpTarget__is_default_lang,$(1)),,\
	$(call gb_HelpTreeTarget_set_treefile,$(1),$(2)) \
	$(call gb_HelpTreeTarget_set_helptextdir,$(1),$(call gb_HelpTarget__get_helpdir,$(1),$(3))) \
)

endef

define gb_HelpTarget__add_helpfile_impl
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

endef

# gb_HelpTarget_add_helpfiles target helpfile(s)
define gb_HelpTarget_add_helpfiles
$(call gb_HelpTranslateTarget_add_files,$(1),$(2))
$(foreach helpfile,$(2),$(call gb_HelpTarget__add_helpfile,$(1),$(helpfile)))

endef

# gb_HelpTarget_use_module target module
define gb_HelpTarget_use_module
$(call gb_HelpTarget_get_translation_target,$(1)) : $(call gb_HelpTarget_get_translation_target,$(2))

endef

# gb_HelpTarget_use_modules target module(s)
define gb_HelpTarget_use_modules
$(foreach module,$(2),$(call gb_HelpTarget_use_module,$(1),$(module)))

endef

.SECONDEXPANSION:
$(call gb_HelpTranslateTarget_get_target,%) :| $$(dir $(call gb_HelpTranslateTarget_get_target,%)).dir
	$(call gb_Output_announce,$*,$(true),XHP,2)
	$(call gb_Trace_MakeMark,$*,XHP)
	touch $@

# vim: set noet sw=4 ts=4:
