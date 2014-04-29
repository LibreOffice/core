# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,i18npool/collator))

i18npool_ICULT53 := $(filter 1, $(shell expr $(ICU_MAJOR) \< 53))
i18npool_CODIR := $(call gb_CustomTarget_get_workdir,i18npool/collator)
i18npool_COTXTS := \
    ca_charset.txt \
    dz_charset.txt \
    hu_charset.txt \
    ja_charset.txt \
    ja_phonetic_alphanumeric_first.txt \
    ja_phonetic_alphanumeric_last.txt \
    $(if $(i18npool_ICULT53), \
        ko_charset.txt \
        ) \
    ku_alphanumeric.txt \
    ln_charset.txt \
    my_dictionary.txt \
    ne_charset.txt \
    sid_charset.txt \
    zh_charset.txt \
    zh_pinyin.txt \
    zh_radical.txt \
    zh_stroke.txt \
    zh_TW_charset.txt \
    zh_TW_radical.txt \
    zh_TW_stroke.txt \
    zh_zhuyin.txt

$(call gb_CustomTarget_get_target,i18npool/collator) : $(SRCDIR)/i18npool/CustomTarget_collator.mk \
	$(i18npool_CODIR)/lrl_include.hxx $(foreach txt,$(i18npool_COTXTS), \
		$(patsubst %.txt,$(i18npool_CODIR)/collator_%.cxx,$(txt)))

$(i18npool_CODIR)/collator_%.cxx : \
		$(SRCDIR)/i18npool/source/collator/data/%.txt \
		$(call gb_Executable_get_runtime_dependencies,gencoll_rule) \
		| $(i18npool_CODIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CLR,1)
	$(call gb_Helper_abbreviate_dirs, \
		$(call gb_Helper_execute,gencoll_rule) $< $@ $*)

$(i18npool_CODIR)/lrl_include.hxx : $(SRCDIR)/i18npool/CustomTarget_collator.mk \
		$(SRCDIR)/i18npool/source/collator/data | $(i18npool_CODIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	echo '#define LOCAL_RULE_LANGS "$(sort $(foreach txt,$(i18npool_COTXTS), \
		$(firstword $(subst _, ,$(txt)))))"' > $@

# vim: set noet sw=4 ts=4:
