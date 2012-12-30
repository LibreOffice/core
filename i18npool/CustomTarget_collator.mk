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

$(eval $(call gb_CustomTarget_CustomTarget,i18npool/collator))

i18npool_CODIR := $(call gb_CustomTarget_get_workdir,i18npool/collator)
i18npool_COTXTS := \
    ca_charset.txt \
    dz_charset.txt \
    hu_charset.txt \
    ja_charset.txt \
    ja_phonetic_alphanumeric_first.txt \
    ja_phonetic_alphanumeric_last.txt \
    ko_charset.txt \
    ku_alphanumeric.txt \
    ln_charset.txt \
    my_dictionary.txt \
    ne_charset.txt \
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

$(i18npool_CODIR)/lrl_include.hxx : \
		$(SRCDIR)/i18npool/source/collator/data | $(i18npool_CODIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	echo '#define LOCAL_RULE_LANGS "$(sort $(foreach txt,$(i18npool_COTXTS), \
		$(firstword $(subst _, ,$(txt)))))"' > $@

# vim: set noet sw=4 ts=4:
