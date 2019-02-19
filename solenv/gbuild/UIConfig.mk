# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

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
	$(gb_UIImageListTarget_COMMAND) --nonet -o $@ $(gb_UIImageListTarget_XSLTFILE) $(UIFILE) && \
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

gb_UIConfig_INSTDIR := $(LIBO_SHARE_FOLDER)/config/soffice.cfg

ifneq ($(filter LXML,$(BUILD_TYPE)),)
gb_UIConfig_LXML_PATH := PYPATH=$${PYPATH:+$$PYPATH:}$(call gb_UnpackedTarball_get_dir,lxml)/install ;
gb_UIConfig_LXML_TARGET := $(call gb_ExternalProject_get_target,lxml)
endif
gb_UIConfig_gla11y_SCRIPT := $(SRCDIR)/bin/gla11y

$(dir $(call gb_UIConfig_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_UIConfig_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_UIConfig_get_target,%) : $(call gb_UIConfig_get_imagelist_target,%) $(call gb_UIConfig_get_a11yerrors_target,%)
	$(call gb_Output_announce,$*,$(true),UIC,2)
	$(call gb_Helper_abbreviate_dirs,\
		touch $@ \
	)

$(call gb_UIConfig_get_imagelist_target,%) :
	$(call gb_UIConfig__command)

.PHONY : $(call gb_UIConfig_get_clean_target,%)
$(call gb_UIConfig_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),UIC,2)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_UIConfig_get_target,$*) $(call gb_UIConfig_get_imagelist_target,$*) \
	)
	$(call gb_Output_announce,$*,$(false),UIA,2)
	rm -f $(call gb_UIConfig_get_a11yerrors_target,$*)

gb_UIConfig_gla11y_PARAMETERS = -P $(SRCDIR)/ -f $(UI_A11YFALSE)

# Disable this to see suppressed warnings
ifeq (1,1)
gb_UIConfig_gla11y_PARAMETERS += -s $(UI_A11YSUPPRS)
endif
# Enable this to regenerate suppression files
ifeq (1,0)
gb_UIConfig_gla11y_PARAMETERS += -g $(UI_A11YSUPPRS)
endif

# Tell gla11y about LO-specific widgets
# These are already automatically labelled Shrink/Expand
gb_UIConfig_gla11y_PARAMETERS += --widgets-ignored +foruilo-RefButton
# These, however, do need a label like a GtkEntry
gb_UIConfig_gla11y_PARAMETERS += --widgets-needlabel +foruilo-RefEdit
# These are storage, containers, or preview
gb_UIConfig_gla11y_PARAMETERS += --widgets-suffixignored +ValueSet,HBox,VBox,ToolBox,Preview,PreviewWin,PreviewWindow,PrevWindow
# These are buttons, thus already contain their label (but an image is not enough)
gb_UIConfig_gla11y_PARAMETERS += --widgets-button +vcllo-SmallButton,chartcontrollerlo-LightButton,svtlo-ManagedMenuButton

# All new warnings should be fatal except a few kinds which could be only doubtful
gb_UIConfig_gla11y_PARAMETERS += --fatal-all --not-fatal-type duplicate-mnemonic --not-fatal-type labelled-by-and-mnemonic --not-fatal-type orphan-label

define gb_UIConfig_a11yerrors__command
$(call gb_Output_announce,$(2),$(true),UIA,1)
$(call gb_UIConfig__gla11y_command)
endef

$(call gb_UIConfig_get_a11yerrors_target,%) : $(gb_UIConfig_LXML_TARGET) $(call gb_ExternalExecutable_get_dependencies,python) $(gb_UIConfig_gla11y_SCRIPT)
	$(call gb_UIConfig_a11yerrors__command,$@,$*)

gb_UIConfig_get_packagename = UIConfig/$(1)
gb_UIConfig_get_packagesetname = UIConfig/$(1)

# Processes and delivers a set of UI configuration files.
#
# gb_UIConfig_UIConfig modulename
define gb_UIConfig_UIConfig
ifeq (,$$(filter $(1),$$(gb_UIConfig_REGISTERED)))
$$(eval $$(call gb_Output_info,Currently known UI configs are: $(sort $(gb_UIConfig_REGISTERED)),ALL))
$$(eval $$(call gb_Output_error,UIConfig $(1) must be registered in Repository.mk))
endif
$(call gb_UIConfig_get_imagelist_target,$(1)) : UI_IMAGELISTS :=

$(call gb_PackageSet_PackageSet_internal,$(call gb_UIConfig_get_packagesetname,$(1)))
$(call gb_Package_Package_internal,$(call gb_UIConfig_get_packagename,$(1)),$(SRCDIR))
$(call gb_Package_Package_internal,$(call gb_UIConfig_get_packagename,$(1)_generated),$(WORKDIR))

$(call gb_PackageSet_add_package,$(call gb_UIConfig_get_packagesetname,$(1)),$(call gb_UIConfig_get_packagename,$(1)))

$(call gb_UIConfig_get_target,$(1)) :| $(dir $(call gb_UIConfig_get_target,$(1))).dir
$(call gb_UIConfig_get_imagelist_target,$(1)) :| $(dir $(call gb_UIConfig_get_imagelist_target,$(1))).dir
$(call gb_UIConfig_get_a11yerrors_target,$(1)) :| $(dir $(call gb_UIConfig_get_a11yerrors_target,$(1))).dir
$(call gb_UIConfig_get_a11yerrors_target,$(1)) : UI_A11YSUPPRS := $(SRCDIR)/solenv/sanitizers/ui/$(1).suppr
$(call gb_UIConfig_get_a11yerrors_target,$(1)) : UI_A11YFALSE := $(SRCDIR)/solenv/sanitizers/ui/$(1).false
$(call gb_UIConfig_get_target,$(1)) : $(call gb_PackageSet_get_target,$(call gb_UIConfig_get_packagesetname,$(1)))
$(call gb_UIConfig_get_clean_target,$(1)) : $(call gb_PackageSet_get_clean_target,$(call gb_UIConfig_get_packagesetname,$(1)))

$$(eval $$(call gb_Module_register_target,$(call gb_UIConfig_get_target,$(1)),$(call gb_UIConfig_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),UIConfig)
$(call gb_Postprocess_register_target,AllUIConfigs,UIConfig,$(1))

endef

# gb_UIConfig__package_file target package type destfile srcfile
define gb_UIConfig__package_file
$(call gb_Package_add_file,$(2),$(gb_UIConfig_INSTDIR)/$(1)/$(3)/$(4),$(5))

endef

# gb_UIConfig__package_uifile target package destfile srcfile
define gb_UIConfig__package_uifile
$(call gb_UIConfig__package_file,$(1),$(2),ui,$(3),$(4))

endef

define gb_UIConfig_add_a11yerrors_uifile
$(call gb_UIConfig_get_a11yerrors_target,$(1)) : UIFILES += $(SRCDIR)/$(2).ui
$(call gb_UIConfig_get_a11yerrors_target,$(1)) : $(SRCDIR)/$(2).ui

endef

# gb_UIConfig__add_uifile target file
define gb_UIConfig__add_uifile
$(call gb_UIConfig__package_uifile,$(1),$(call gb_UIConfig_get_packagename,$(1)),$(notdir $(2)).ui,$(2).ui)
$(call gb_UIImageListTarget_UIImageListTarget,$(2))

$(call gb_UIConfig_get_imagelist_target,$(1)) : UI_IMAGELISTS += $(call gb_UIImageListTarget_get_target,$(2))
$(call gb_UIConfig_get_imagelist_target,$(1)) : $(call gb_UIImageListTarget_get_target,$(2))
$(call gb_UIConfig_get_clean_target,$(1)) : $(call gb_UIImageListTarget_get_clean_target,$(2))

$(call gb_UIConfig_add_a11yerrors_uifile,$(1),$(2))

endef

gb_UIConfig_ALLFILES:=
# Adds .ui file to the package
#
# The file is relative to $(SRCDIR) and without extension.
#
# gb_UIConfig_add_uifile target uifile
define gb_UIConfig_add_uifile
gb_UIConfig_ALLFILES+=$(1):$(notdir $(2))
$(call gb_UIConfig__add_uifile,$(1),$(2))

endef

# Adds multiple .ui files to the package
#
# gb_UIConfig_add_uifiles target uifile(s)
define gb_UIConfig_add_uifiles
$(foreach uifile,$(2),$(call gb_UIConfig_add_uifile,$(1),$(uifile)))

endef

# gb_UIConfig_add_uifiles target uifile(s) but only for running gla11y
define gb_UIConfig_add_a11yerrors_uifiles
$(foreach uifile,$(2),$(call gb_UIConfig_add_a11yerrors_uifile,$(1),$(uifile)))

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

# Add popupmenu config file to the package.
#
# The file is relative to $(SRCDIR) and without extension.
#
# gb_UIConfig_add_popupmenufile target file
define gb_UIConfig_add_popupmenufile
$(call gb_UIConfig__add_xmlfile,$(1),$(1),popupmenu,$(2))

endef

# Adds multiple popupmenu config files to the package.
#
# gb_UIConfig_add_popupmenufiles target file(s)
define gb_UIConfig_add_popupmenufiles
$(foreach popupmenufile,$(2),$(call gb_UIConfig_add_popupmenufile,$(1),$(popupmenufile)))

endef

# vim: set noet sw=4 ts=4:
