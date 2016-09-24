# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,i18npool/breakiterator))

i18npool_BIDIR := $(call gb_CustomTarget_get_workdir,i18npool/breakiterator)

ifeq ($(OS),IOS)

$(call gb_CustomTarget_get_target,i18npool/breakiterator) : \
	$(i18npool_BIDIR)/dict_ja.data $(i18npool_BIDIR)/dict_zh.data $(i18npool_BIDIR)/OpenOffice_dat.c

$(i18npool_BIDIR)/dict_%.data : \
		$(SRCDIR)/i18npool/source/breakiterator/data/%.dic \
		$(call gb_Executable_get_runtime_dependencies,gendict) \
		| $(i18npool_BIDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),DIC,1)
	$(call gb_Helper_abbreviate_dirs,\
		$(call gb_Helper_execute,gendict) $< $@ $(patsubst $(i18npool_BIDIR)/dict_%.cxx,%,$@))
else

$(call gb_CustomTarget_get_target,i18npool/breakiterator) : \
	$(i18npool_BIDIR)/dict_ja.cxx $(i18npool_BIDIR)/dict_zh.cxx $(i18npool_BIDIR)/OpenOffice_dat.c

$(i18npool_BIDIR)/dict_%.cxx : \
		$(SRCDIR)/i18npool/source/breakiterator/data/%.dic \
		$(call gb_Executable_get_runtime_dependencies,gendict) \
		| $(i18npool_BIDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),DIC,1)
	$(call gb_Helper_abbreviate_dirs,\
		$(call gb_Helper_execute,gendict) $< $@ $(patsubst $(i18npool_BIDIR)/dict_%.cxx,%,$@))

endif

# Do we want the char_in.brk data? It's for languages in India (and bn_BD)
ifeq ($(WITH_LOCALES),)
i18npool_breakiterator_want_in=TRUE
else ifneq ($(filter bn bn_% gu gu_% hi hi_% kn kn_% ml ml_% or or_% pa pa_% ta ta_% te te_%,$(WITH_LOCALES)),)
i18npool_breakiterator_want_in=TRUE
endif

i18npool_BRKTXTS := \
    $(if $(i18npool_breakiterator_want_in),char_in.brk) \
    char.brk \
    count_word.brk \
    $(call gb_Helper_optional_locale,he,dict_word_he.brk) \
    $(call gb_Helper_optional_locale,hu,dict_word_hu.brk) \
    dict_word_nodash.brk \
    dict_word_prepostdash.brk \
    dict_word.brk \
    $(call gb_Helper_optional_locale,he,edit_word_he.brk) \
    $(call gb_Helper_optional_locale,hu,edit_word_hu.brk) \
    edit_word.brk \
    line.brk \
    sent.brk

# 'gencmn', 'genbrk' and 'genccode' are tools generated and delivered by icu project to process icu breakiterator rules.
# The output of gencmn generates warnings under Windows. We want to minimize the patches to external tools,
# so the output (OpenOffice_dat.c) is changed here to include a pragma to disable the warnings.
# Output of gencmn is redirected to OpenOffice_tmp.c with the -t switch.
$(i18npool_BIDIR)/OpenOffice_dat.c : $(SRCDIR)/i18npool/CustomTarget_breakiterator.mk \
		$(patsubst %.brk,$(i18npool_BIDIR)/%_brk.c,$(i18npool_BRKTXTS)) \
		$(call gb_ExternalExecutable_get_dependencies,gencmn)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CMN,1)
	$(call gb_Helper_abbreviate_dirs,\
		RESPONSEFILE=$(shell $(gb_MKTEMP)) && \
		$(foreach brk,$(i18npool_BRKTXTS),echo '$(brk)' >> $${RESPONSEFILE} && ) \
		$(call gb_ExternalExecutable_get_command,gencmn) -n OpenOffice -t tmp -S -d $(i18npool_BIDIR)/ 0 $${RESPONSEFILE} && \
		rm -f $${RESPONSEFILE} && \
		echo '#ifdef _MSC_VER' > $@ && \
		echo '#pragma warning( disable : 4229 4668 )' >> $@ && \
		echo '#endif' >> $@ && \
		cat $(subst _dat,_tmp,$@) >> $@)

$(i18npool_BIDIR)/%_brk.c : $(i18npool_BIDIR)/%.brk $(call gb_ExternalExecutable_get_dependencies,genccode)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CCD,1)
	$(call gb_Helper_abbreviate_dirs,\
		$(call gb_ExternalExecutable_get_command,genccode) -n OpenOffice -d $(i18npool_BIDIR)/ $< \
			$(if $(findstring s,$(MAKEFLAGS)),> /dev/null))

$(i18npool_BIDIR)/%.brk : $(i18npool_BIDIR)/%.txt $(call gb_ExternalExecutable_get_dependencies,genbrk)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),BRK,1)
	$(call gb_Helper_abbreviate_dirs,\
		$(call gb_ExternalExecutable_get_command,genbrk) -r $< -o $@ $(if $(findstring s,$(MAKEFLAGS)),> /dev/null))

# fdo#31271 ")" reclassified in more recent Unicode Standards / ICU 4.4
# * Prepend set empty as of Unicode Version 6.1 / ICU 49, which bails out if used.
#   NOTE: strips every line with _word_ 'Prepend', including $Prepend
# * Conditional_Japanese_Starter does not exist in ICU < 49, which bail out if used.
# * Hebrew_Letter does not exist in ICU < 49, which bail out if used.
#   NOTE: I sincerely hope there is a better way to avoid problems than this abominable
#   sed substitution...
$(i18npool_BIDIR)/%.txt : \
	$(SRCDIR)/i18npool/source/breakiterator/data/%.txt | $(i18npool_BIDIR)/.dir
	sed -e "s#\[:LineBreak =  Close_Punctuation:\]#\[& \[:LineBreak = Close_Parenthesis:\]\]#" \
		$(if $(ICU_RECLASSIFIED_CONDITIONAL_JAPANESE_STARTER),,\
			-e '/\[:LineBreak =  Conditional_Japanese_Starter:\]/d' \
			-e 's# $$CJ##' \
		) \
		$(if $(ICU_RECLASSIFIED_HEBREW_LETTER),,\
			-e '/\[:LineBreak =  Hebrew_Letter:\]/d' \
			-e '/^$$HLcm =/d' \
			-e '/^$$HLcm $$NUcm;/d' \
			-e '/^$$NUcm $$HLcm;/d' \
			-e '/^$$HL $$CM+;/d' \
			-e 's# | $$HL\(cm\)\?##g' \
			-e 's#$$HLcm ##g' \
			-e 's# $$HL##g' \
		) \
		$(if $(ICU_RECLASSIFIED_PREPEND_SET_EMPTY),-e "/Prepend/d") \
		$< > $@

# vim: set noet sw=4 ts=4:
