# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# The Initial Developer of the Original Code is
#       Matúš Kukan <matus.kukan@gmail.com>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_CustomTarget_CustomTarget,i18npool/breakiterator))

i18npool_BIDIR := $(call gb_CustomTarget_get_workdir,i18npool/breakiterator)

$(call gb_CustomTarget_get_target,i18npool/breakiterator) : \
	$(i18npool_BIDIR)/dict_ja.cxx $(i18npool_BIDIR)/dict_zh.cxx $(i18npool_BIDIR)/OpenOffice_dat.c

$(i18npool_BIDIR)/dict_%.cxx : \
		$(SRCDIR)/i18npool/source/breakiterator/data/%.dic \
		$(call gb_Executable_get_target_for_build,gendict) \
		| $(i18npool_BIDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),DIC,1)
	$(call gb_Helper_abbreviate_dirs,\
		$(call gb_Helper_execute,gendict) $< $@ $(patsubst $(i18npool_BIDIR)/dict_%.cxx,%,$@))

ifeq ($(SYSTEM_GENBRK),)
i18npool_GENBRKTARGET := $(call gb_Executable_get_target_for_build,genbrk)
i18npool_GENBRK := $(call gb_Helper_execute,genbrk)
else
i18npool_GENBRKTARGET :=
i18npool_GENBRK := $(SYSTEM_GENBRK)
endif

ifeq ($(SYSTEM_GENCCODE),)
i18npool_GENCCODETARGET := $(call gb_Executable_get_target_for_build,genccode)
i18npool_GENCCODE := $(call gb_Helper_execute,genccode)
else
i18npool_GENCCODETARGET :=
i18npool_GENCCODE := $(SYSTEM_GENCCODE)
endif

ifeq ($(SYSTEM_GENCMN),)
i18npool_GENCMNTARGET := $(call gb_Executable_get_target_for_build,gencmn)
i18npool_GENCMN := $(call gb_Helper_execute,gencmn)
else
i18npool_GENCMNTARGET :=
i18npool_GENCMN := $(SYSTEM_GENCMN)
endif

i18npool_BRKTXTS := \
    char_in.brk \
    char.brk \
    count_word_fi.brk \
    count_word.brk \
    dict_word_fi.brk \
    dict_word_he.brk \
    dict_word_hu.brk \
    dict_word_nodash.brk \
    dict_word_prepostdash.brk \
    dict_word.brk \
    edit_word_he.brk \
    edit_word_hu.brk \
    edit_word.brk \
    line.brk \
    sent.brk

# 'gencmn', 'genbrk' and 'genccode' are tools generated and delivered by icu project to process icu breakiterator rules.
# The output of gencmn generates warnings under Windows. We want to minimize the patches to external tools,
# so the output (OpenOffice_dat.c) is changed here to include a pragma to disable the warnings.
# Output of gencmn is redirected to OpenOffice_tmp.c with the -t switch.
$(i18npool_BIDIR)/OpenOffice_dat.c : $(SRCDIR)/i18npool/CustomTarget_breakiterator.mk \
		$(patsubst %.brk,$(i18npool_BIDIR)/%_brk.c,$(i18npool_BRKTXTS)) \
		$(i18npool_GENCMNTARGET)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CMN,1)
	$(call gb_Helper_abbreviate_dirs,\
		RESPONSEFILE=$(shell $(gb_MKTEMP)) && \
		$(foreach brk,$(i18npool_BRKTXTS),echo '$(brk)' >> $${RESPONSEFILE} && ) \
		$(i18npool_GENCMN) -n OpenOffice -t tmp -S -d $(i18npool_BIDIR)/ 0 $${RESPONSEFILE} && \
		rm -f $${RESPONSEFILE} && \
		echo '#ifdef _MSC_VER' > $@ && \
		echo '#pragma warning( disable : 4229 4668 )' >> $@ && \
		echo '#endif' >> $@ && \
		cat $(subst _dat,_tmp,$@) >> $@)

$(i18npool_BIDIR)/%_brk.c : $(i18npool_BIDIR)/%.brk $(i18npool_GENCCODETARGET)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CCD,1)
	$(call gb_Helper_abbreviate_dirs,\
		$(i18npool_GENCCODE) -n OpenOffice -d $(i18npool_BIDIR)/ $< \
			$(if $(findstring s,$(MAKEFLAGS)),> /dev/null))

$(i18npool_BIDIR)/%.brk : $(i18npool_BIDIR)/%.txt $(i18npool_GENBRKTARGET)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),BRK,1)
	$(call gb_Helper_abbreviate_dirs,\
		$(i18npool_GENBRK) -r $< -o $@ $(if $(findstring s,$(MAKEFLAGS)),> /dev/null))

# fdo#31271 ")" reclassified in more recent Unicode Standards / ICU 4.4
# * Prepend set empty as of Unicode Version 6.1 / ICU 4.9, which bails out if used.
#   NOTE: strips every line with _word_ 'Prepend', including $Prepend
# * Conditional_Japanese_Starter does not exist in ICU 4.6, which bails out if used.
# * Hebrew_Letter does not exist in ICU 4.6, which bails out if used.
#   NOTE: I sincerely hope there is a better way to avoid problems than this abominable
#   sed substitution...
$(i18npool_BIDIR)/%.txt : \
	$(SRCDIR)/i18npool/source/breakiterator/data/%.txt | $(i18npool_BIDIR)/.dir
	sed -e ': dummy' \
		-e "s#\[:LineBreak =  Close_Punctuation:\]#\[& \[:LineBreak = Close_Parenthesis:\]\]#" \
		$(if $(filter-out YES,$(ICU_RECLASSIFIED_CONDITIONAL_JAPANESE_STARTER)),\
			-e '/\[:LineBreak =  Conditional_Japanese_Starter:\]/d' \
			-e 's# $$CJ##' \
		) \
		$(if $(filter-out YES,$(ICU_RECLASSIFIED_HEBREW_LETTER)),\
			-e '/\[:LineBreak =  Hebrew_Letter:\]/d' \
			-e '/^$$HLcm =/d' \
			-e '/^$$HLcm  $$NUcm;/d' \
			-e '/^$$NUcm  $$HLcm;/d' \
			-e '/^$$HL $$CM+;/d' \
			-e 's# | $$HL\(cm\)\?##g' \
			-e 's#$$HLcm ##g' \
			-e 's# $$HL##g' \
		) \
		$(if $(filter YES,$(ICU_RECLASSIFIED_PREPEND_SET_EMPTY)),-e "/Prepend/d") \
		$< > $@

# vim: set noet sw=4 ts=4:
