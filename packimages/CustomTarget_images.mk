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

$(eval $(call gb_CustomTarget_CustomTarget,packimages/images,new_style))

PIIM := $(call gb_CustomTarget_get_workdir,packimages/images)

# Custom sets, at 24x24 & 16x16 fall-back to Tango preferentially
# (Tango fallbacks to Industrial for the missing icons)
CUSTOM_PREFERRED_FALLBACK_1 := -c $(SRCDIR)/icon-themes/tango
CUSTOM_PREFERRED_FALLBACK_2 := -c $(SRCDIR)/icon-themes/industrial

$(call gb_CustomTarget_get_target,packimages/images) : \
	$(PIIM)/images.zip $(PIIM)/images_brand.zip $(foreach theme,$(WITH_THEMES),\
		$(PIIM)/images_$(theme).zip)

$(PIIM)/images.zip : $(PIIM)/sorted.lst $(PIIM)/commandimagelist.ilst
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,2)
	$(call gb_Helper_abbreviate_dirs_native, \
		$(PERL) $(SOLARENV)/bin/packimages.pl -g $(SRCDIR)/icon-themes/galaxy \
			-m $(SRCDIR)/icon-themes/galaxy -c $(PIIM) \
			-l $(PIIM) -l $(OUTDIR)/res/img -s $< -o $@ \
			$(if $(findstring s,$(MAKEFLAGS)),> /dev/null))

$(PIIM)/images_%.zip : $(PIIM)/sorted.lst $(PIIM)/commandimagelist.ilst
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,2)
	$(call gb_Helper_abbreviate_dirs_native, \
		$(PERL) $(SOLARENV)/bin/packimages.pl -g $(SRCDIR)/icon-themes/galaxy \
			-m $(SRCDIR)/icon-themes/galaxy -c $(SRCDIR)/icon-themes/$* \
			$(CUSTOM_PREFERRED_FALLBACK_1) $(CUSTOM_PREFERRED_FALLBACK_2) \
			-l $(PIIM) -l $(OUTDIR)/res/img -s $< -o $@ \
			$(if $(findstring s,$(MAKEFLAGS)),> /dev/null))

# make sure to have one to keep packing happy
$(PIIM)/images_brand.zip :| $(PIIM)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),TCH,2)
	touch $@

# commandimagelist.ilst and sorted.lst are phony to rebuild everything each time
.PHONY : $(PIIM)/commandimagelist.ilst $(PIIM)/sorted.lst

$(PIIM)/commandimagelist.ilst :| $(PIIM)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,1)
	$(call gb_Helper_abbreviate_dirs_native, \
		find $(SRCDIR)/icon-themes/galaxy/cmd -name "*.png" | \
			sed "s#$(SRCDIR)/icon-themes/galaxy#%MODULE%#" | \
			$(PERL) $(SOLARENV)/bin/sort.pl > $@.$(INPATH) && \
		$(PERL) $(SOLARENV)/bin/diffmv.pl $@.$(INPATH) $@ \
			$(if $(findstring s,$(MAKEFLAGS)),2> /dev/null))

$(PIIM)/sorted.lst : $(SRCDIR)/packimages/pack/image-sort.lst | $(PIIM)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,1)
	$(call gb_Helper_abbreviate_dirs_native, \
		$(PERL) $(SOLARENV)/bin/image-sort.pl $< $(OUTDIR)/xml $@)

# vim: set noet sw=4 ts=4:
