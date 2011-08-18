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

$(eval $(call gb_Library_Library,i18npool))

$(eval $(call gb_Library_add_package_headers,i18npool,i18npool_generated))

$(eval $(call gb_Library_add_precompiled_header,i18npool,$(SRCDIR)/i18npool/inc/pch/precompiled_i18npool))

$(eval $(call gb_Library_set_componentfile,i18npool,i18npool/util/i18npool))

$(eval $(call gb_Library_set_include,i18npool,\
	$$(INCLUDE) \
	-I$(realpath $(SRCDIR)/i18npool/inc) \
	-I$(realpath $(SRCDIR)/i18npool/inc/pch) \
	-I$(WORKDIR)/CustomTarget/i18npool/source/collator \
))

$(eval $(call gb_Library_add_api,i18npool,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_add_linked_libs,i18npool,\
	comphelper \
	cppu \
	cppuhelper \
	i18nisolang1 \
	i18nutil \
	sal \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_use_externals,i18npool,\
	icui18n \
	icuuc \
))

$(eval $(call gb_Library_add_exception_objects,i18npool,\
	i18npool/source/breakiterator/breakiterator_cjk \
	i18npool/source/breakiterator/breakiterator_ctl \
	i18npool/source/breakiterator/breakiteratorImpl \
	i18npool/source/breakiterator/breakiterator_th \
	i18npool/source/breakiterator/breakiterator_unicode \
	i18npool/source/breakiterator/xdictionary \
	i18npool/source/calendar/calendarImpl \
	i18npool/source/calendar/calendar_gregorian \
	i18npool/source/calendar/calendar_hijri \
	i18npool/source/calendar/calendar_jewish \
	i18npool/source/characterclassification/cclass_unicode \
	i18npool/source/characterclassification/cclass_unicode_parser \
	i18npool/source/characterclassification/characterclassificationImpl \
	i18npool/source/characterclassification/scripttypedetector \
	i18npool/source/collator/chaptercollator \
	i18npool/source/collator/collatorImpl \
	i18npool/source/collator/collator_unicode \
	i18npool/source/defaultnumberingprovider/defaultnumberingprovider \
	i18npool/source/indexentry/indexentrysupplier \
	i18npool/source/indexentry/indexentrysupplier_asian \
	i18npool/source/indexentry/indexentrysupplier_common \
	i18npool/source/indexentry/indexentrysupplier_default \
	i18npool/source/indexentry/indexentrysupplier_ja_phonetic \
	i18npool/source/inputchecker/inputsequencechecker \
	i18npool/source/inputchecker/inputsequencechecker_hi \
	i18npool/source/inputchecker/inputsequencechecker_th \
	i18npool/source/localedata/localedata \
	i18npool/source/nativenumber/nativenumbersupplier \
	i18npool/source/numberformatcode/numberformatcode \
	i18npool/source/ordinalsuffix/ordinalsuffix \
	i18npool/source/registerservices/registerservices \
	i18npool/source/textconversion/textconversion \
	i18npool/source/textconversion/textconversionImpl \
	i18npool/source/textconversion/textconversion_ko \
	i18npool/source/textconversion/textconversion_zh \
	i18npool/source/transliteration/chartonum \
	i18npool/source/transliteration/fullwidthToHalfwidth \
	i18npool/source/transliteration/halfwidthToFullwidth \
	i18npool/source/transliteration/hiraganaToKatakana \
	i18npool/source/transliteration/ignoreBaFa_ja_JP \
	i18npool/source/transliteration/ignoreHyuByu_ja_JP \
	i18npool/source/transliteration/ignoreIandEfollowedByYa_ja_JP \
	i18npool/source/transliteration/ignoreIterationMark_ja_JP \
	i18npool/source/transliteration/ignoreKana \
	i18npool/source/transliteration/ignoreKiKuFollowedBySa_ja_JP \
	i18npool/source/transliteration/ignoreMiddleDot_ja_JP \
	i18npool/source/transliteration/ignoreMinusSign_ja_JP \
	i18npool/source/transliteration/ignoreProlongedSoundMark_ja_JP \
	i18npool/source/transliteration/ignoreSeparator_ja_JP \
	i18npool/source/transliteration/ignoreSeZe_ja_JP \
	i18npool/source/transliteration/ignoreSize_ja_JP \
	i18npool/source/transliteration/ignoreSpace_ja_JP \
	i18npool/source/transliteration/ignoreTiJi_ja_JP \
	i18npool/source/transliteration/ignoreTraditionalKana_ja_JP \
	i18npool/source/transliteration/ignoreTraditionalKanji_ja_JP \
	i18npool/source/transliteration/ignoreWidth \
	i18npool/source/transliteration/ignoreZiZu_ja_JP \
	i18npool/source/transliteration/katakanaToHiragana \
	i18npool/source/transliteration/largeToSmall_ja_JP \
	i18npool/source/transliteration/numtochar \
	i18npool/source/transliteration/numtotext_cjk \
	i18npool/source/transliteration/smallToLarge_ja_JP \
	i18npool/source/transliteration/texttonum \
	i18npool/source/transliteration/textToPronounce_zh \
	i18npool/source/transliteration/transliteration_body \
	i18npool/source/transliteration/transliteration_caseignore \
	i18npool/source/transliteration/transliteration_commonclass \
	i18npool/source/transliteration/transliteration_Ignore \
	i18npool/source/transliteration/transliterationImpl \
	i18npool/source/transliteration/transliteration_Numeric \
	i18npool/source/transliteration/transliteration_OneToOne \
))

BRKFILES := $(subst $(WORKDIR)/,,$(basename $(wildcard $(WORKDIR)/CustomTarget/i18npool/source/breakiterator/*_brk.c))) \

$(eval $(call gb_Library_add_generated_cobjects,i18npool,\
	CustomTarget/i18npool/source/breakiterator/OpenOffice_dat \
	$(BRKFILES) \
))

# vim: set noet sw=4 ts=4:
