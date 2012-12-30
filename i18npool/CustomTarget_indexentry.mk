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
	$(patsubst %.txt,$(call gb_CustomTarget_get_workdir,i18npool/indexentry)/%.cxx,$(i18npool_IDXTXTS))

$(call gb_CustomTarget_get_workdir,i18npool/indexentry)/%.cxx : \
		$(SRCDIR)/i18npool/source/indexentry/data/%.txt \
		$(call gb_Executable_get_runtime_dependencies,genindex_data) \
		| $(call gb_CustomTarget_get_workdir,i18npool/indexentry)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),IND,1)
	$(call gb_Helper_abbreviate_dirs, \
		$(call gb_Helper_execute,genindex_data) $< $@.tmp $* && \
		sed 's/\(^.*get_\)/SAL_DLLPUBLIC_EXPORT \1/' $@.tmp > $@ && \
		rm $@.tmp)

# vim: set noet sw=4 ts=4:
