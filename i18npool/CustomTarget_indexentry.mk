# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,i18npool/indexentry))

i18npool_IDXTXTS := \
    indexdata_ko_dict.txt \
    indexdata_zh_pinyin.txt \
    indexdata_zh_radical.txt \
    indexdata_zh_stroke.txt \
    indexdata_zh_TW_radical.txt \
    indexdata_zh_TW_stroke.txt \
    indexdata_zh_zhuyin.txt \
    ko_phonetic.txt \
    zh_pinyin.txt \
    zh_zhuyin.txt

$(call gb_CustomTarget_get_target,i18npool/indexentry) : $(SRCDIR)/i18npool/CustomTarget_indexentry.mk \
	$(patsubst %.txt,$(gb_CustomTarget_workdir)/i18npool/indexentry/%.cxx,$(i18npool_IDXTXTS))

$(gb_CustomTarget_workdir)/i18npool/indexentry/%.cxx : \
		$(SRCDIR)/i18npool/source/indexentry/data/%.txt \
		$(call gb_Executable_get_runtime_dependencies,genindex_data) \
		| $(gb_CustomTarget_workdir)/i18npool/indexentry/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),IND,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),IND)
	$(call gb_Helper_abbreviate_dirs, \
		$(call gb_Helper_execute,genindex_data) $< $@ $*)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),IND)

# vim: set noet sw=4 ts=4:
