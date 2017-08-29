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

$(eval $(call gb_CustomTarget_register_targets,postprocess/images,\
	$(foreach theme,$(WITH_THEMES),images_$(theme).zip) \
	commandimagelist.ilst \
	sourceimagelist.ilst \
	sorted.lst \
))

$(packimages_DIR)/images.zip : DEFAULT_THEME := $(true)
$(packimages_DIR)/images_%.zip : DEFAULT_THEME :=
$(packimages_DIR)/images_tango.zip : INDUSTRIAL_FALLBACK := -c $(SRCDIR)/icon-themes/industrial
$(packimages_DIR)/images%.zip : INDUSTRIAL_FALLBACK :=

$(packimages_DIR)/%.zip : \
		$(packimages_DIR)/sorted.lst \
		$(packimages_DIR)/commandimagelist.ilst \
		$(packimages_DIR)/sourceimagelist.ilst \
		$(SRCDIR)/wizards/source/imagelists/imagelists.ilst \
		$(call gb_Helper_get_imagelists)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,2)
	$(call gb_Helper_abbreviate_dirs, \
		ILSTFILE=$(call var2file,$(shell $(gb_MKTEMP)),100,$(filter %.ilst,$^)) && \
		$(PYTHON) $(SRCDIR)/solenv/bin/pack_images.py \
			$(if $(DEFAULT_THEME),\
				-g $(packimages_DIR) -m $(packimages_DIR) -c $(packimages_DIR),\
				-g $(SRCDIR)/icon-themes/$(subst images_,,$*) -m $(SRCDIR)/icon-themes/$(subst images_,,$*) -c $(SRCDIR)/icon-themes/$(subst images_,,$*) \
			) \
			$(INDUSTRIAL_FALLBACK) \
			-l $${ILSTFILE} \
			-s $< -o $@ \
			$(if $(findstring s,$(MAKEFLAGS)),> /dev/null) && \
		rm -rf $${ILSTFILE})

# turn the #defines foo "resource.png" of hlst into the final ilst format
$(packimages_DIR)/sourceimagelist.ilst : \
		$(SRCDIR)/avmedia/inc/bitmaps.hlst \
		$(SRCDIR)/basctl/inc/bitmaps.hlst \
		$(SRCDIR)/connectivity/inc/bitmaps.hlst \
		$(SRCDIR)/cui/inc/bitmaps.hlst \
		$(SRCDIR)/chart2/inc/bitmaps.hlst \
		$(SRCDIR)/dbaccess/inc/bitmaps.hlst \
		$(SRCDIR)/desktop/inc/bitmaps.hlst \
		$(SRCDIR)/extensions/inc/bitmaps.hlst \
		$(SRCDIR)/filter/inc/bitmaps.hlst \
		$(SRCDIR)/formula/inc/bitmaps.hlst \
		$(SRCDIR)/fpicker/inc/bitmaps.hlst \
		$(SRCDIR)/framework/inc/bitmaps.hlst \
		$(SRCDIR)/reportdesign/inc/bitmaps.hlst \
		$(SRCDIR)/sc/inc/bitmaps.hlst \
		$(SRCDIR)/sd/inc/bitmaps.hlst \
		$(SRCDIR)/sdext/inc/bitmaps.hlst \
		$(SRCDIR)/sfx2/inc/bitmaps.hlst \
		$(SRCDIR)/svtools/inc/bitmaps.hlst \
		$(SRCDIR)/svx/inc/bitmaps.hlst \
		$(SRCDIR)/sw/inc/bitmaps.hlst \
		$(SRCDIR)/vcl/inc/bitmaps.hlst \
		$(SRCDIR)/xmlsecurity/inc/bitmaps.hlst
	grep res $^ | cut -d'"' -f2 | sed "s/^/%MODULE%\//" | sed "s/%MODULE%.res/%GLOBALRES%/g" > $@

# commandimagelist.ilst and sorted.lst are phony to rebuild everything each time
.PHONY : $(packimages_DIR)/commandimagelist.ilst $(packimages_DIR)/sorted.lst

$(packimages_DIR)/commandimagelist.ilst :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,1)
	$(call gb_Helper_abbreviate_dirs, \
		$(FIND) $(SRCDIR)/icon-themes -name "*.png" -o -name "*.svg" | \
			grep -e '/cmd/' | sed 's#^.*/icon-themes/[^/]*##' | $(SORT) | uniq | \
			sed "s#^#%MODULE%#" | \
			LC_ALL=C $(SORT) > $@.tmp && \
		$(call gb_Helper_replace_if_different_and_touch,$@.tmp,$@))

$(packimages_DIR)/sorted.lst : \
		$(SRCDIR)/postprocess/packimages/image-sort.lst \
		$(call gb_Postprocess_get_target,AllUIConfigs)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,1)
	$(call gb_Helper_abbreviate_dirs, \
		$(PERL) $(SRCDIR)/solenv/bin/image-sort.pl \
			$< $(INSTROOT)/$(gb_UIConfig_INSTDIR) $@)

# vim: set noet sw=4 ts=4:
