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
# Copyright (C) 2012 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
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

$(eval $(call gb_CustomTarget_CustomTarget,packimages/images))

packimages_DIR := $(call gb_CustomTarget_get_workdir,packimages/images)

# Custom sets, at 24x24 & 16x16 fall-back to Tango preferentially
# (Tango fallbacks to Industrial for the missing icons)
packimages_CUSTOM_FALLBACK_1 := -c $(SRCDIR)/icon-themes/tango
packimages_CUSTOM_FALLBACK_2 := -c $(SRCDIR)/icon-themes/industrial

$(call gb_CustomTarget_get_target,packimages/images) : \
	$(packimages_DIR)/images_brand.zip \
	$(if $(filter default,$(WITH_THEMES)),$(packimages_DIR)/images.zip) \
	$(foreach theme,$(filter-out default,$(WITH_THEMES)),$(packimages_DIR)/images_$(theme).zip)

$(packimages_DIR)/images.zip : \
		$(packimages_DIR)/sorted.lst $(packimages_DIR)/commandimagelist.ilst
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,2)
	$(call gb_Helper_abbreviate_dirs, \
		$(PERL) $(SOLARENV)/bin/packimages.pl -g $(SRCDIR)/icon-themes/galaxy \
			-m $(SRCDIR)/icon-themes/galaxy -c $(packimages_DIR) \
			-l $(packimages_DIR) -l $(dir $(call gb_ResTarget_get_imagelist_target)) -l $(OUTDIR)/res/img -s $< -o $@ \
			$(if $(findstring s,$(MAKEFLAGS)),> /dev/null))

$(packimages_DIR)/images_%.zip : \
		$(packimages_DIR)/sorted.lst $(packimages_DIR)/commandimagelist.ilst
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,2)
	$(call gb_Helper_abbreviate_dirs, \
		$(PERL) $(SOLARENV)/bin/packimages.pl -g $(SRCDIR)/icon-themes/galaxy \
			-m $(SRCDIR)/icon-themes/galaxy -c $(SRCDIR)/icon-themes/$* \
			$(packimages_CUSTOM_FALLBACK_1) $(packimages_CUSTOM_FALLBACK_2) \
			-l $(packimages_DIR) -l $(dir $(call gb_ResTarget_get_imagelist_target)) -l $(OUTDIR)/res/img -s $< -o $@ \
			$(if $(findstring s,$(MAKEFLAGS)),> /dev/null))

# make sure to have one to keep packing happy
$(packimages_DIR)/images_brand.zip :| $(packimages_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),TCH,2)
	touch $@

# commandimagelist.ilst and sorted.lst are phony to rebuild everything each time
.PHONY : $(packimages_DIR)/commandimagelist.ilst $(packimages_DIR)/sorted.lst

$(packimages_DIR)/commandimagelist.ilst :| $(packimages_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,1)
	$(call gb_Helper_abbreviate_dirs, \
		find $(SRCDIR)/icon-themes/galaxy/cmd -name "*.png" -o -name "*.svg" | \
			sed "s#$(SRCDIR)/icon-themes/galaxy#%MODULE%#" | \
			$(PERL) $(SOLARENV)/bin/sort.pl > $@.$(INPATH) && \
		$(PERL) $(SOLARENV)/bin/diffmv.pl $@.$(INPATH) $@ \
			$(if $(findstring s,$(MAKEFLAGS)),2> /dev/null))

$(packimages_DIR)/sorted.lst : \
		$(SRCDIR)/packimages/pack/image-sort.lst | $(packimages_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,1)
	$(call gb_Helper_abbreviate_dirs, \
		$(PERL) $(SOLARENV)/bin/image-sort.pl $< $(OUTDIR)/xml $@)

# vim: set noet sw=4 ts=4:
