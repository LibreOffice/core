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

packimages_DIR := $(gb_CustomTarget_workdir)/postprocess/images

$(eval $(call gb_CustomTarget_register_targets,postprocess/images,\
	$(foreach theme,$(WITH_THEMES),images_$(theme).zip) \
	$(foreach theme,$(WITH_THEMES),$(theme)_links.txt) \
	commandimagelist.ilst \
	sourceimagelist.ilst \
	sorted.lst \
))

$(packimages_DIR)/images.zip : DEFAULT_THEME := $(true)
$(packimages_DIR)/images_%.zip : DEFAULT_THEME :=

# tdf#124023 make links.txt depend on phony sorted.lst depend
# so this is evaluated every time
$(packimages_DIR)/%_links.txt : $(packimages_DIR)/sorted.lst
	@if test -f "$(SRCDIR)/icon-themes/$*/links.txt"; then \
		cp "$(SRCDIR)/icon-themes/$*/links.txt" $@.tmp ; \
	else \
		THEME_FILE="$(SRCDIR)/icon-themes/$(subst _svg,,$*)/links.txt" ; \
		if test -f "$${THEME_FILE}"; then \
			sed 's/\.png/\.svg/g' "$${THEME_FILE}" > $@.tmp ; \
		fi ; \
		THEME_FILE="$(SRCDIR)/icon-themes/$(subst _dark,,$*)/links.txt" ; \
		if test -f "$${THEME_FILE}"; then \
			cp "$${THEME_FILE}" $@.tmp ; \
		fi ; \
	fi
	$(call gb_Helper_replace_if_different_and_touch,$@.tmp,$@)

$(packimages_DIR)/images_%.zip : \
		$(packimages_DIR)/sorted.lst \
		$(packimages_DIR)/commandimagelist.ilst \
		$(packimages_DIR)/sourceimagelist.ilst \
		$(SRCDIR)/wizards/source/imagelists/imagelists.ilst \
		$(packimages_DIR)/%_links.txt \
		$(call gb_Helper_get_imagelists) \
		| $(call gb_ExternalExecutable_get_dependencies,python)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,2)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),PRL)
	$(call gb_Helper_abbreviate_dirs, \
		ILSTFILE=$(call gb_var2file,$(shell $(gb_MKTEMP)),$(filter %.ilst,$^)) && \
		$(call gb_ExternalExecutable_get_command,python) \
			$(SRCDIR)/solenv/bin/pack_images.py \
			$(if $(DEFAULT_THEME),\
				-g $(packimages_DIR) -m $(packimages_DIR) -c $(packimages_DIR),\
				-g $(SRCDIR)/icon-themes/$* -m $(SRCDIR)/icon-themes/$* -c $(SRCDIR)/icon-themes/$* \
			) \
			-l $${ILSTFILE} \
			-L $(packimages_DIR)/$*_links.txt \
			-s $< -o $@ \
			$(if $(findstring s,$(MAKEFLAGS)),> /dev/null) && \
		rm -rf $${ILSTFILE})
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),PRL)

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
	grep res $^ | cut -d'"' -f2 | sed "s/^/%MODULE%\//" | sed "s/%MODULE%.res/%GLOBALRES%/g" > $@.png
	sed 's/\.png/\.svg/g' $@.png > $@.svg
	cat $@.png $@.svg > $@

packimages_everything := $(shell $(FIND) $(SRCDIR)/icon-themes)
packimages_icon_themes := $(filter %/,$(wildcard $(SRCDIR)/icon-themes/*/))
# TODO: awkward workaround for windows - the filter call chokes for some reason
# on the full list, and similarly the recursive filtering call from the initial
# version of the patch also fails on windows for some reason
# so don't try to be clever and do it in this awkward/tedious way.
# all this does is filtering everything for png/svg files that have /cmd/ as
# part of their path and then replacing the path to the theme-directory with
# %MODULE% - the surrounding sort not only sorts the lists, but gets rid of
# duplicate filenames that are left after the %MODULE% replacement
packimages_cmd_images := $(sort $(foreach file,$(packimages_everything),\
        $(if $(findstring /cmd/,$(filter %.png %.svg,$(file))),\
            $(foreach theme,$(packimages_icon_themes),\
                $(filter-out $(SRCDIR)/%,$(subst $(theme),%MODULE%/,$(file)))))))

# adding everything as dependencies here, so that file deletion (when only the directory timestamp
# gets updated) will also trigger regeneration of the list
$(packimages_DIR)/commandimagelist.ilst : $(packimages_everything)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),LST,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),LST)
	$(file >$@,$(subst $(WHITESPACE),$(NEWLINE),$(packimages_cmd_images)))
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),LST)

$(packimages_DIR)/sorted.lst : $(packimages_DIR)/commandimagelist.ilst \
		$(SRCDIR)/postprocess/packimages/image-sort.lst \
		$(call gb_Postprocess_get_target,AllUIConfigs) \
		$(call gb_ExternalExecutable_get_dependencies,python)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),PRL)
	$(call gb_Helper_abbreviate_dirs, \
		$(call gb_ExternalExecutable_get_command,python) \
			$(SRCDIR)/solenv/bin/image-sort.py \
			$(if $(filter build,$(gb_Side)),--quiet) \
			$< $(INSTROOT)/$(gb_UIConfig_INSTDIR) $@)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),PRL)

# vim: set noet sw=4 ts=4:
