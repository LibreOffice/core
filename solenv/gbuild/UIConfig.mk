# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# class UILocalizeTarget

# Produces translations for one .ui file.

gb_UILocalizeTarget_WORKDIR := $(WORKDIR)/UILocalizeTarget

gb_UILocalizeTarget_DEPS := $(call gb_Executable_get_runtime_dependencies,uiex)
gb_UILocalizeTarget_COMMAND := $(call gb_Executable_get_command,uiex)

# If translatable strings from a .ui file are not merged into the
# respective .po file yet, the produced translated files are empty,
# which breaks delivery. This hack avoids the problem by creating a
# dummy translation file.
$(call gb_UILocalizeTarget_get_workdir,%).ui :
	$(if $(wildcard $@) \
		,touch $@ \
		,echo '<?xml version="1.0"?><t></t>' > $@ \
	)

define gb_UILocalizeTarget__command
$(call gb_Output_announce,$(2),$(true),UIX,1)
MERGEINPUT=`$(gb_MKTEMP)` && \
echo $(POFILES) > $${MERGEINPUT} && \
$(call gb_Helper_abbreviate_dirs,\
	$(gb_UILocalizeTarget_COMMAND) \
		-i $(UIConfig_FILE) \
		-o $(call gb_UILocalizeTarget_get_workdir,$(2)) \
		-l all \
		-m $${MERGEINPUT} \
	&& touch $(1) \
) && \
rm -rf $${MERGEINPUT}
endef

$(dir $(call gb_UILocalizeTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_UILocalizeTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_UILocalizeTarget_get_target,%) : $(gb_UILocalizeTarget_DEPS)
	$(call gb_UILocalizeTarget__command,$@,$*)

.PHONY : $(call gb_UILocalizeTarget_get_clean_target,%)
$(call gb_UILocalizeTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),UIX,1)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(call gb_UILocalizeTarget_get_target,$*) $(call gb_UILocalizeTarget_get_workdir,$*) \
	)

# Produce translations for one .ui file
#
# gb_UILocalizeTarget_UILocalizeTarget target
define gb_UILocalizeTarget_UILocalizeTarget
$(call gb_UILocalizeTarget__UILocalizeTarget_impl,$(1),$(wildcard $(foreach lang,$(gb_TRANS_LANGS),$(gb_POLOCATION)/$(lang)/$(patsubst %/,%,$(dir $(1))).po)))

endef

# gb_UILocalizeTarget__UILocalizeTarget_impl target pofiles
define gb_UILocalizeTarget__UILocalizeTarget_impl
$(call gb_UILocalizeTarget_get_target,$(1)) : POFILES := $(2)
$(call gb_UILocalizeTarget_get_target,$(1)) : UIConfig_FILE := $(SRCDIR)/$(1).ui

$(call gb_UILocalizeTarget_get_target,$(1)) : $(2)
$(call gb_UILocalizeTarget_get_target,$(1)) : $(SRCDIR)/$(1).ui
$(call gb_UILocalizeTarget_get_target,$(1)) :| \
	$(dir $(call gb_UILocalizeTarget_get_target,$(1))).dir \
	$(call gb_UILocalizeTarget_get_workdir,$(1))/.dir

endef

# class UIMenubarTarget

# Handles platform-specific processing of menubar config files.

# defined by platform:
#  gb_UIMenubarTarget_UIMenubarTarget_platform
#  gb_UIMenubarTarget__command

$(dir $(call gb_UIMenubarTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_UIMenubarTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_UIMenubarTarget_get_target,%) :
	$(call gb_UIMenubarTarget__command,$@,$*,$<)

.PHONY : $(call gb_UIMenubarTarget_get_clean_target,%)
$(call gb_UIMenubarTarget_get_clean_target,%) :
	$(call gb_Output_announce,$(2),$(false),UIM,1)
	rm -f $(call gb_UIMenubarTarget_get_target,$*)

# Process a menubar configuration file.
#
# gb_UIMenubarTarget_UIMenubarTarget target source
define gb_UIMenubarTarget_UIMenubarTarget
$(call gb_UIMenubarTarget_get_target,$(1)) : $(2)
$(call gb_UIMenubarTarget_get_target,$(1)) :| $(dir $(call gb_UIMenubarTarget_get_target,$(1))).dir

$(call gb_UIMenubarTarget_UIMenubarTarget_platform,$(1),$(2))

endef

# class UIImageListTarget

# Handles creation of image lists for .ui files.

gb_UIImageListTarget_COMMAND = $(call gb_ExternalExecutable_get_command,xsltproc)
gb_UIImageListTarget_DEPS = $(call gb_ExternalExecutable_get_dependencies,xsltproc)
gb_UIImageListTarget_XSLTFILE := $(SRCDIR)/solenv/bin/uiimagelist.xsl

# NOTE: for some reason xsltproc does not produce any file if there is
# no output, so we touch the target to make sure it exists.
define gb_UIImageListTarget__command
$(call gb_Output_announce,$(2),$(true),UIL,1)
$(call gb_Helper_abbreviate_dirs,\
	$(gb_UIImageListTarget_COMMAND) -o $@ $(gb_UIImageListTarget_XSLTFILE) $(UIFILE) && \
	touch $@ \
)
endef

$(dir $(call gb_UIImageListTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_UIImageListTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_UIImageListTarget_get_target,%) : $(gb_UIImageListTarget_DEPS) $(gb_UIImageListTarget_XSLTFILE)
	$(call gb_UIImageListTarget__command,$@,$*)

.PHONY : $(call gb_UIImageListTarget_get_clean_target,%)
$(call gb_UIImageListTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),UIL,1)
	rm -f $(call gb_UIImageListTarget_get_target,$*)

# Extract list of images referenced in a .ui file.
#
# gb_UIImageListTarget_UIImageListTarget uifile
define gb_UIImageListTarget_UIImageListTarget
$(call gb_UIImageListTarget_get_target,$(1)) : UIFILE := $(SRCDIR)/$(1).ui

$(call gb_UIImageListTarget_get_target,$(1)) : $(SRCDIR)/$(1).ui
$(call gb_UIImageListTarget_get_target,$(1)) :| $(dir $(call gb_UIImageListTarget_get_target,$(1))).dir

endef

# class UIConfig

# Handles UI configuration files.
#
# This mostly means UI description files (suffix .ui) for the new layouting
# mechanism.
#
# This class provides the following filelists:
# * UIConfig/<name> containing all nontranslatable files
# * UIConfig/<name>_<lang> for each active lang, containing translations
#   of .ui files. This filelist only exists if the UIConfig contains any
#   .ui files.

gb_UIConfig_INSTDIR := $(LIBO_SHARE_FOLDER)/config/soffice.cfg
# en-US is the default, so there is no translation for it
gb_UIConfig_LANGS := $(filter-out en-US,$(gb_WITH_LANG))

$(dir $(call gb_UIConfig_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_UIConfig_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_UIConfig_get_target,%) : $(call gb_UIConfig_get_imagelist_target,%)
	$(call gb_Output_announce,$*,$(true),UIC,2)
	$(call gb_Helper_abbreviate_dirs,\
		touch $@ \
	)

$(call gb_UIConfig_get_imagelist_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		sort -u $(UI_IMAGELISTS) /dev/null > $@ \
	)

.PHONY : $(call gb_UIConfig_get_clean_target,%)
$(call gb_UIConfig_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),UIC,2)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_UIConfig_get_target,$*) \
	)

gb_UIConfig_get_packagename = UIConfig/$(1)
gb_UIConfig_get_packagename_for_lang = UIConfig/$(1)_$(2)
gb_UIConfig_get_packagesetname = UIConfig/$(1)

# Processes and delivers a set of UI configuration files.
#
# gb_UIConfig_UIConfig modulename
define gb_UIConfig_UIConfig
$(call gb_UIConfig_get_imagelist_target,$(1)) : UI_IMAGELISTS :=

$(call gb_PackageSet_PackageSet_internal,$(call gb_UIConfig_get_packagesetname,$(1)))
$(call gb_Package_Package_internal,$(call gb_UIConfig_get_packagename,$(1)),$(SRCDIR))
$(call gb_Package_Package_internal,$(call gb_UIConfig_get_packagename,$(1)_generated),$(WORKDIR))

$(call gb_Package_set_outdir,$(call gb_UIConfig_get_packagename,$(1)),$(gb_INSTROOT))
$(call gb_Package_set_outdir,$(call gb_UIConfig_get_packagename,$(1)_generated),$(gb_INSTROOT))
$(call gb_PackageSet_add_package,$(call gb_UIConfig_get_packagesetname,$(1)),$(call gb_UIConfig_get_packagename,$(1)))

$(call gb_UIConfig_get_target,$(1)) :| $(dir $(call gb_UIConfig_get_target,$(1))).dir
$(call gb_UIConfig_get_imagelist_target,$(1)) :| $(dir $(call gb_UIConfig_get_imagelist_target,$(1))).dir
$(call gb_UIConfig_get_target,$(1)) : $(call gb_PackageSet_get_target,$(call gb_UIConfig_get_packagesetname,$(1)))
$(call gb_UIConfig_get_clean_target,$(1)) : $(call gb_PackageSet_get_clean_target,$(call gb_UIConfig_get_packagesetname,$(1)))

ifneq ($(gb_UIConfig_LANGS),)
$(foreach lang,$(gb_UIConfig_LANGS),$(call gb_UIConfig__UIConfig_for_lang,$(1),$(lang)))
endif

$$(eval $$(call gb_Module_register_target,$(call gb_UIConfig_get_target,$(1)),$(call gb_UIConfig_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),UIConfig)
$(call gb_Postprocess_register_target,AllUIConfigs,UIConfig,$(1))

endef

define gb_UIConfig__UIConfig_for_lang
$(call gb_Package_Package_internal,$(call gb_UIConfig_get_packagename_for_lang,$(1),$(2)),$(gb_UILocalizeTarget_WORKDIR))
$(call gb_Package_set_outdir,$(call gb_UIConfig_get_packagename_for_lang,$(1),$(2)),$(gb_INSTROOT))
$(call gb_UIConfig_get_target,$(1)) : $(call gb_Package_get_target,$(call gb_UIConfig_get_packagename_for_lang,$(1),$(2)))
$(call gb_UIConfig_get_clean_target,$(1)) : $(call gb_Package_get_clean_target,$(call gb_UIConfig_get_packagename_for_lang,$(1),$(2)))

endef

# gb_UIConfig__package_file target package type destfile srcfile
define gb_UIConfig__package_file
$(call gb_Package_add_file,$(2),$(gb_UIConfig_INSTDIR)/$(1)/$(3)/$(4),$(5))

endef

# gb_UIConfig__package_uifile target package destfile srcfile
define gb_UIConfig__package_uifile
$(call gb_UIConfig__package_file,$(1),$(2),ui,$(3),$(4))

endef

# gb_UIConfig__add_uifile target file
define gb_UIConfig__add_uifile
$(call gb_UIConfig__package_uifile,$(1),$(call gb_UIConfig_get_packagename,$(1)),$(notdir $(2)).ui,$(2).ui)
$(call gb_UIImageListTarget_UIImageListTarget,$(2))

$(call gb_UIConfig_get_imagelist_target,$(1)) : UI_IMAGELISTS += $(call gb_UIImageListTarget_get_target,$(2))
$(call gb_UIConfig_get_imagelist_target,$(1)) : $(call gb_UIImageListTarget_get_target,$(2))
$(call gb_UIConfig_get_clean_target,$(1)) : $(call gb_UIImageListTarget_get_clean_target,$(2))

endef

# Add a l10n for an .ui file to respective lang package.
#
# gb_UIConfig__add_uifile_for_lang target file lang
define gb_UIConfig__add_uifile_for_lang
$(call gb_UIConfig__package_uifile,$(1),$(call gb_UIConfig_get_packagename_for_lang,$(1),$(3)),res/$(3)/$(notdir $(2)),$(2)/$(3).ui)
$(call gb_Package_get_preparation_target,$(call gb_UIConfig_get_packagename_for_lang,$(1),$(3))) : $(call gb_UILocalizeTarget_get_target,$(2))

endef

# Add a l10n for an .ui file to respective lang package.
#
# This is only for "real" languages, i.e., everything except qtz.
#
# gb_UIConfig__add_uifile_for_real_lang target file lang
define gb_UIConfig__add_uifile_for_real_lang
$(if $(filter qtz,$(3)),$(call gb_Output_error,gb_UIConfig__add_uifile_for_real_lang called with qtz))
$(call gb_UIConfig__add_uifile_for_lang,$(1),$(2),$(3))

endef

# gb_UIConfig__add_translations_impl target uifile langs
define gb_UIConfig__add_translations_impl
$(call gb_UILocalizeTarget_UILocalizeTarget,$(2))
$(call gb_UIConfig_get_target,$(1)) : $(call gb_UILocalizeTarget_get_target,$(2))
$(call gb_UIConfig_get_clean_target,$(1)) : $(call gb_UILocalizeTarget_get_clean_target,$(2))
$(foreach lang,$(3),$(call gb_UIConfig__add_uifile_for_real_lang,$(1),$(2),$(lang)))

endef

# gb_UIConfig__add_translations target uifile langs qtz
define gb_UIConfig__add_translations
$(if $(strip $(3) $(4)),$(call gb_UIConfig__add_translations_impl,$(1),$(2),$(3)))
$(if $(strip $(4)),$(call gb_UIConfig__add_uifile_for_lang,$(1),$(2),$(strip $(4))))

endef

# Adds translations for languages that have corresponding .po file
#
# gb_UIConfig__add_uifile_translations target uifile
define gb_UIConfig__add_uifile_translations
$(call gb_UIConfig__add_translations,$(1),$(2),\
	$(foreach lang,$(gb_UIConfig_LANGS),\
		$(if $(wildcard $(gb_POLOCATION)/$(lang)/$(patsubst %/,%,$(dir $(2))).po),$(lang)) \
	),\
	$(filter qtz,$(gb_UIConfig_LANGS)) \
)

endef

# Adds .ui file to the package
#
# The file is relative to $(SRCDIR) and without extension.
#
# gb_UIConfig_add_uifile target uifile
define gb_UIConfig_add_uifile
$(call gb_UIConfig__add_uifile,$(1),$(2))

ifneq ($(gb_UIConfig_LANGS),)
$(call gb_UIConfig__add_uifile_translations,$(1),$(2))
endif

endef

# Adds multiple .ui files to the package
#
# gb_UIConfig_add_uifiles target uifile(s)
define gb_UIConfig_add_uifiles
$(foreach uifile,$(2),$(call gb_UIConfig_add_uifile,$(1),$(uifile)))

endef

# gb_UIConfig__add_xmlfile target package-target type xmlfile
define gb_UIConfig__add_xmlfile
$(call gb_UIConfig__package_file,$(1),$(call gb_UIConfig_get_packagename,$(2)),$(3),$(notdir $(4)).xml,$(4).xml)

endef

define gb_UIConfig__add_menubarfile
$(call gb_UIMenubarTarget_UIMenubarTarget,$(2),$(3))
$(call gb_Package_add_file,$(call gb_UIConfig_get_packagename,$(1)_generated),$(gb_UIConfig_INSTDIR)/$(1)/menubar/$(notdir $(2)).xml,$(subst $(WORKDIR)/,,$(call gb_UIMenubarTarget_get_target,$(2))))
$(call gb_PackageSet_add_package,$(call gb_UIConfig_get_packagesetname,$(1)),$(call gb_UIConfig_get_packagename,$(1)_generated))

$(call gb_Package_get_target,$(call gb_UIConfig_get_packagename,$(1)_generated)) : $(call gb_UIMenubarTarget_get_target,$(2))
$(call gb_Package_get_clean_target,$(call gb_UIConfig_get_packagename,$(1)_generated)) : $(call gb_UIMenubarTarget_get_clean_target,$(2))

endef

# Add menubar config file to the package.
#
# The file is relative to $(SRCDIR) and without extension.
#
# gb_UIConfig_add_menubarfile target file
define gb_UIConfig_add_menubarfile
$(call gb_UIConfig__add_menubarfile,$(1),$(gb_UIConfig_INSTDIR)/$(1)/menubar/$(notdir $(2)),$(SRCDIR)/$(2).xml)

endef

# Adds multiple menubar config files to the package.
#
# gb_UIConfig_add_menubarfiles target file(s)
define gb_UIConfig_add_menubarfiles
$(foreach menubarfile,$(2),$(call gb_UIConfig_add_menubarfile,$(1),$(menubarfile)))

endef

# Add a generated menubar config file to the package.
#
# The file is relative to $(WORKDIR) and without extension.
#
# gb_UIConfig_add_generated_menubarfile target file
define gb_UIConfig_add_generated_menubarfile
$(call gb_UIConfig__add_menubarfile,$(1),$(gb_UIConfig_INSTDIR)/$(1)/menubar/$(notdir $(2)),$(WORKDIR)/$(2).xml)

endef

# Adds multiple menubar config files to the package.
#
# gb_UIConfig_add_generated_menubarfiles target file(s)
define gb_UIConfig_add_generated_menubarfiles
$(foreach menubarfile,$(2),$(call gb_UIConfig_add_generated_menubarfile,$(1),$(menubarfile)))

endef

# Add statusbar config file to the package.
#
# The file is relative to $(SRCDIR) and without extension.
#
# gb_UIConfig_add_statusbarfile target file
define gb_UIConfig_add_statusbarfile
$(call gb_UIConfig__add_xmlfile,$(1),$(1),statusbar,$(2))

endef

# Adds multiple statusbar config files to the package.
#
# gb_UIConfig_add_statusbarfiles target file(s)
define gb_UIConfig_add_statusbarfiles
$(foreach statusbarfile,$(2),$(call gb_UIConfig_add_statusbarfile,$(1),$(statusbarfile)))

endef

# Add toolbar config file to the package.
#
# The file is relative to $(SRCDIR) and without extension.
#
# gb_UIConfig_add_toolbarfile target file
define gb_UIConfig_add_toolbarfile
$(call gb_UIConfig__add_xmlfile,$(1),$(1),toolbar,$(2))

endef

# Adds multiple toolbar config files to the package.
#
# gb_UIConfig_add_toolbarfiles target file(s)
define gb_UIConfig_add_toolbarfiles
$(foreach toolbarfile,$(2),$(call gb_UIConfig_add_toolbarfile,$(1),$(toolbarfile)))

endef

# vim: set noet sw=4 ts=4:
