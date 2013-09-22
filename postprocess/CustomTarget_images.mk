# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,postprocess/images))

packimages_DIR := $(call gb_CustomTarget_get_workdir,postprocess/images)
helpimages_DIR := $(call gb_CustomTarget_get_workdir,helpcontent2/source/auxiliary)

# Custom sets, at 24x24 & 16x16 fall-back to Tango preferentially
# (Tango fallbacks to Industrial for the missing icons)
packimages_CUSTOM_FALLBACKS := -c $(SRCDIR)/icon-themes/tango -c $(SRCDIR)/icon-themes/industrial

$(eval $(call gb_CustomTarget_register_targets,postprocess/images,\
	$(if $(filter default,$(WITH_THEMES)),images.zip) \
	$(foreach theme,$(filter-out default,$(WITH_THEMES)),images_$(theme).zip) \
	commandimagelist.ilst \
	sorted.lst \
))

$(packimages_DIR)/images.zip : DEFAULT_THEME := $(true)
$(packimages_DIR)/images_%.zip : DEFAULT_THEME :=

$(packimages_DIR)/%.zip : \
		$(packimages_DIR)/sorted.lst \
		$(packimages_DIR)/commandimagelist.ilst \
		$(call gb_Helper_optional,HELP,$(helpimages_DIR)/helpimg.ilst) \
		$(call gb_Helper_optional,DBCONNECTIVITY,$(if $(ENABLE_JAVA),$(SRCDIR)/connectivity/source/drivers/hsqldb/hsqlui.ilst)) \
		$(call gb_Postprocess_get_target,AllResources) \
		$(call gb_Postprocess_get_target,AllUIConfigs)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,2)
	$(call gb_Helper_abbreviate_dirs, \
		$(PERL) $(SOLARENV)/bin/packimages.pl \
			-g $(SRCDIR)/icon-themes/galaxy \
			-m $(SRCDIR)/icon-themes/galaxy \
			$(if $(DEFAULT_THEME),\
				-c $(packimages_DIR),\
				-c $(SRCDIR)/icon-themes/$(subst images_,,$*) $(packimages_CUSTOM_FALLBACKS) \
			) \
			$(call gb_Helper_optional,HELP,-l $(helpimages_DIR) ) \
			-l $(packimages_DIR) \
			-l $(dir $(call gb_ResTarget_get_imagelist_target)) \
			-l $(dir $(call gb_UIConfig_get_imagelist_target)) \
			-l $(dir $(call gb_UIConfig_get_imagelist_target,modules/)) \
			$(call gb_Helper_optional,DBCONNECTIVITY,$(if $(ENABLE_JAVA),-l $(SRCDIR)/connectivity/source/drivers/hsqldb)) \
			$(call gb_Helper_optional,DBCONNECTIVITY,$(if $(ENABLE_FIREBIRD_SDBC),-l $(SRCDIR)/connectivity/source/drivers/firebird)) \
			-s $< -o $@ \
			$(if $(findstring s,$(MAKEFLAGS)),> /dev/null))

# commandimagelist.ilst and sorted.lst are phony to rebuild everything each time
.PHONY : $(packimages_DIR)/commandimagelist.ilst $(packimages_DIR)/sorted.lst

$(packimages_DIR)/commandimagelist.ilst :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,1)
	$(call gb_Helper_abbreviate_dirs, \
		find $(SRCDIR)/icon-themes/galaxy/cmd -name "*.png" -o -name "*.svg" | \
			sed "s#$(SRCDIR)/icon-themes/galaxy#%MODULE%#" | \
			LC_ALL=C sort > $@.$(INPATH) && \
		$(PERL) $(SOLARENV)/bin/diffmv.pl $@.$(INPATH) $@ \
			$(if $(findstring s,$(MAKEFLAGS)),2> /dev/null))

$(packimages_DIR)/sorted.lst : \
		$(SRCDIR)/postprocess/packimages/image-sort.lst \
		$(call gb_Postprocess_get_target,AllUIConfigs)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,1)
	$(call gb_Helper_abbreviate_dirs, \
		$(PERL) $(SOLARENV)/bin/image-sort.pl $< $(INSTROOT)/$(gb_UIConfig_INSTDIR) $@)

# vim: set noet sw=4 ts=4:
