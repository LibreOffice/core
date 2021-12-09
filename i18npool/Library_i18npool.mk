# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,i18npool))

ifeq ($(WITH_LOCALES),en)
$(eval $(call gb_Library_set_componentfile,i18npool,i18npool/util/i18npool.en,services))
else
$(eval $(call gb_Library_set_componentfile,i18npool,i18npool/util/i18npool,services))
endif

$(eval $(call gb_Library_set_include,i18npool,\
	$$(INCLUDE) \
	-I$(SRCDIR)/i18npool/inc \
))

$(eval $(call gb_Library_use_sdk_api,i18npool))

$(eval $(call gb_Library_use_libraries,i18npool,\
	comphelper \
	cppu \
	cppuhelper \
	i18nlangtag \
	i18nutil \
	sal \
))

$(eval $(call gb_Library_use_externals,i18npool,\
	boost_headers \
	icui18n \
	icuuc \
	icu_headers \
))

ifeq ($(DISABLE_DYNLOADING),TRUE)
$(eval $(call gb_Library_add_cxxflags,i18npool,\
	-DDICT_JA_ZH_IN_DATAFILE \
))
endif

$(eval $(call gb_Library_add_exception_objects,i18npool,\
	i18npool/source/breakiterator/breakiterator_cjk \
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
	i18npool/source/characterclassification/unoscripttypedetector \
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
	i18npool/source/transliteration/ignoreDiacritics_CTL \
	i18npool/source/transliteration/ignoreKashida_CTL \
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

ifeq ($(DISABLE_DYNLOADING),TRUE)
$(call gb_CxxObject_get_target,i18npool/source/localedata/localedata): $(call gb_CustomTarget_get_workdir,i18npool/localedata)/localedata_static.hxx

ifeq ($(WITH_LOCALES),)
i18npool_locale_pattern=%
else
i18npool_locale_pattern=$(WITH_LOCALES) $(addsuffix _%,$(WITH_LOCALES))
endif

$(call gb_CustomTarget_get_workdir,i18npool/localedata)/localedata_static.hxx : $(SRCDIR)/i18npool/source/localedata/genstaticheader.pl
	mkdir -p $(call gb_CustomTarget_get_workdir,i18npool/localedata) && $(PERL) $(SRCDIR)/i18npool/source/localedata/genstaticheader.pl $(filter $(i18npool_locale_pattern),$(patsubst $(SRCDIR)/i18npool/source/localedata/data/%.xml,%,$(shell echo $(SRCDIR)/i18npool/source/localedata/data/*.xml))) >$@

$(call gb_CxxObject_get_target,i18npool/source/localedata/localedata) : \
	INCLUDE += -I$(call gb_CustomTarget_get_workdir,i18npool/localedata)

endif # DISABLE_DYNLOADING

# collator_unicode.cxx includes generated lrl_include.hxx
$(call gb_CxxObject_get_target,i18npool/source/collator/collator_unicode) : \
	INCLUDE += -I$(call gb_CustomTarget_get_workdir,i18npool/collator)
$(call gb_CxxObject_get_target,i18npool/source/collator/collator_unicode) :| \
	$(call gb_CustomTarget_get_workdir,i18npool/collator)/lrl_include.hxx

$(eval $(call gb_Library_add_generated_cobjects,i18npool,\
	CustomTarget/i18npool/breakiterator/OpenOffice_dat \
	$(foreach txt,$(wildcard $(SRCDIR)/i18npool/source/breakiterator/data/*.txt),\
		CustomTarget/i18npool/breakiterator/$(notdir $(basename $(txt)))_brk),\
	$(if $(filter GCC,$(COM)),-Wno-unused-macros) \
))

# vim: set noet sw=4 ts=4:
