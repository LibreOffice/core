#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



$(eval $(call gb_Library_Library,i18npool))

$(eval $(call gb_Library_add_package_headers,i18npool,i18npool_inc))

$(eval $(call gb_Library_add_precompiled_header,i18npool,$(SRCDIR)/i18npool/inc/pch/precompiled_i18npool))

$(eval $(call gb_Library_set_componentfile,i18npool,i18npool/util/i18npool))

$(eval $(call gb_Library_set_include,i18npool,\
	$$(INCLUDE) \
	-I$(SRCDIR)/i18npool/inc \
	-I$(SRCDIR)/i18npool/inc/pch \
	-I$(WORKDIR)/CustomTarget/i18npool/source/collator \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_api,i18npool, \
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_defs,i18npool,\
	-DI18NPOOL_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,i18npool,\
	comphelper \
	cppu \
	cppuhelper \
	i18nisolang1 \
	i18nutil \
	sal \
	stl \
	$(gb_STDLIBS) \
))

$(call gb_Library_use_external,i18npool,icui18n)
$(call gb_Library_use_external,i18npool,icudata)
$(call gb_Library_use_external,i18npool,icuuc)

$(eval $(call gb_Library_add_exception_objects,i18npool,\
	i18npool/source/registerservices/registerservices \
	i18npool/source/breakiterator/breakiteratorImpl \
	i18npool/source/breakiterator/breakiterator_cjk \
	i18npool/source/breakiterator/breakiterator_ctl \
	i18npool/source/breakiterator/breakiterator_th \
	i18npool/source/breakiterator/breakiterator_unicode \
	i18npool/source/breakiterator/xdictionary \
	i18npool/source/characterclassification/characterclassificationImpl \
	i18npool/source/characterclassification/cclass_unicode \
	i18npool/source/characterclassification/cclass_unicode_parser \
	i18npool/source/characterclassification/scripttypedetector \
	i18npool/source/transliteration/transliteration_body \
	i18npool/source/transliteration/transliteration_caseignore \
	i18npool/source/transliteration/transliterationImpl \
	i18npool/source/transliteration/transliteration_commonclass \
	i18npool/source/transliteration/transliteration_OneToOne \
	i18npool/source/transliteration/transliteration_Ignore \
	i18npool/source/transliteration/transliteration_Numeric \
	i18npool/source/transliteration/hiraganaToKatakana \
	i18npool/source/transliteration/katakanaToHiragana \
	i18npool/source/transliteration/ignoreKana \
	i18npool/source/transliteration/halfwidthToFullwidth \
	i18npool/source/transliteration/fullwidthToHalfwidth \
	i18npool/source/transliteration/ignoreWidth \
	i18npool/source/transliteration/smallToLarge_ja_JP \
	i18npool/source/transliteration/largeToSmall_ja_JP \
	i18npool/source/transliteration/ignoreSize_ja_JP \
	i18npool/source/transliteration/ignoreMinusSign_ja_JP \
	i18npool/source/transliteration/ignoreIterationMark_ja_JP \
	i18npool/source/transliteration/ignoreTraditionalKana_ja_JP \
	i18npool/source/transliteration/ignoreTraditionalKanji_ja_JP \
	i18npool/source/transliteration/ignoreProlongedSoundMark_ja_JP \
	i18npool/source/transliteration/ignoreZiZu_ja_JP \
	i18npool/source/transliteration/ignoreBaFa_ja_JP \
	i18npool/source/transliteration/ignoreTiJi_ja_JP \
	i18npool/source/transliteration/ignoreHyuByu_ja_JP \
	i18npool/source/transliteration/ignoreSeZe_ja_JP \
	i18npool/source/transliteration/ignoreIandEfollowedByYa_ja_JP \
	i18npool/source/transliteration/ignoreKiKuFollowedBySa_ja_JP \
	i18npool/source/transliteration/ignoreSeparator_ja_JP \
	i18npool/source/transliteration/ignoreSpace_ja_JP \
	i18npool/source/transliteration/ignoreMiddleDot_ja_JP \
	i18npool/source/transliteration/textToPronounce_zh \
	i18npool/source/transliteration/numtochar \
	i18npool/source/transliteration/numtotext_cjk \
	i18npool/source/transliteration/chartonum \
	i18npool/source/transliteration/texttonum \
	i18npool/source/calendar/calendarImpl \
	i18npool/source/calendar/calendar_gregorian \
	i18npool/source/calendar/calendar_hijri \
	i18npool/source/calendar/calendar_jewish \
	i18npool/source/numberformatcode/numberformatcode \
	i18npool/source/defaultnumberingprovider/defaultnumberingprovider \
	i18npool/source/nativenumber/nativenumbersupplier \
	i18npool/source/inputchecker/inputsequencechecker \
	i18npool/source/inputchecker/inputsequencechecker_th \
	i18npool/source/inputchecker/inputsequencechecker_hi \
	i18npool/source/ordinalsuffix/ordinalsuffix \
	i18npool/source/localedata/localedata \
	i18npool/source/indexentry/indexentrysupplier \
	i18npool/source/indexentry/indexentrysupplier_asian \
	i18npool/source/indexentry/indexentrysupplier_ja_phonetic \
	i18npool/source/indexentry/indexentrysupplier_default \
	i18npool/source/indexentry/indexentrysupplier_common \
	i18npool/source/textconversion/textconversionImpl \
	i18npool/source/textconversion/textconversion \
	i18npool/source/textconversion/textconversion_ko \
	i18npool/source/textconversion/textconversion_zh \
	i18npool/source/collator/collatorImpl \
	i18npool/source/collator/chaptercollator \
	i18npool/source/collator/collator_unicode \
))

$(call gb_Library_get_headers_target,i18npool) : $(WORKDIR)/CustomTarget/i18npool/source/collator/lrl_include.hxx

$(WORKDIR)/CustomTarget/i18npool/source/collator/lrl_include.hxx : $(wildcard $(SRCDIR)/i18npool/source/collator/data/*.txt)
	mkdir -p $(dir $@) \
	&& LOCAL_RULE_LANGS="$(sort $(foreach txt,$(wildcard $(SRCDIR)/i18npool/source/collator/data/*.txt),$(firstword $(subst _, ,$(notdir $(txt))))))" \
	&& echo \#define LOCAL_RULE_LANGS \"$${LOCAL_RULE_LANGS}\" > $@


# fdo#31271 ")" reclassified in more recent ICU/Unicode Standards
-include $(OUTDIR)/inc/icuversion.mk
ICU_RECLASSIFIED_BRACKET := $(shell [ ${ICU_MAJOR} -ge 5 -o \( ${ICU_MAJOR} -eq 4 -a ${ICU_MINOR} -ge 4 \) ] && echo YES)


$(WORKDIR)/CustomTarget/i18npool/source/breakiterator/data/%.txt : $(SRCDIR)/i18npool/source/breakiterator/data/%.txt
ifeq ($(ICU_RECLASSIFIED_BRACKET),YES)
	mkdir -p $(dir $@) && \
	sed "s#\[:LineBreak =  Close_Punctuation:\]#\[\[:LineBreak =  Close_Punctuation:\] \[:LineBreak = Close_Parenthesis:\]\]#" $< > $@
else
	mkdir -p $(dir $@) && \
	cp $< $@
endif


ifeq ($(SYSTEM_ICU),YES)
GENCMN := $(SYSTEM_GENCMN)
GENBRK := $(SYSTEM_GENBRK)
GENCCODE := $(SYSTEM_GENCCODE)
else
GENCMN := $(gb_Augment_Library_Path) $(OUTDIR)/bin/gencmn$(gb_Executable_EXT)
GENBRK := $(gb_Augment_Library_Path) $(OUTDIR)/bin/genbrk$(gb_Executable_EXT)
GENCCODE := $(gb_Augment_Library_Path) $(OUTDIR)/bin/genccode$(gb_Executable_EXT)
$(WORKDIR)/CustomTarget/i18npool/source/breakiterator/data/%.brk : $(OUTDIR)/bin/genbrk$(gb_Executable_EXT)
$(WORKDIR)/CustomTarget/i18npool/source/breakiterator/data/%_brk.c: $(OUTDIR)/bin/genccode$(gb_Executable_EXT)
$(WORKDIR)/CustomTarget/i18npool/source/breakiterator/data/OpenOffice_dat.c : $(OUTDIR)/bin/gencmn$(gb_Executable_EXT)
endif

$(WORKDIR)/CustomTarget/i18npool/source/breakiterator/data/%.brk : $(WORKDIR)/CustomTarget/i18npool/source/breakiterator/data/%.txt
	$(GENBRK) -r $(call gb_Helper_convert_native,$<) -o $(call gb_Helper_convert_native,$(WORKDIR)/CustomTarget/i18npool/source/breakiterator/data/$*.brk)

$(WORKDIR)/CustomTarget/i18npool/source/breakiterator/data/%_brk.c : $(WORKDIR)/CustomTarget/i18npool/source/breakiterator/data/%.brk
	$(GENCCODE) -n OpenOffice -d $(call gb_Helper_convert_native,$(WORKDIR)/CustomTarget/i18npool/source/breakiterator/data) $(call gb_Helper_convert_native,$(WORKDIR)/CustomTarget/i18npool/source/breakiterator/data/$*.brk)

# 'gencmn', 'genbrk' and 'genccode' are tools generated and delivered by icu project to process icu breakiterator rules.
# The output of gencmn generates warnings under Windows. We want to minimize the patches to external tools,
# so the output (OpenOffice_icu_dat.c) is changed here to include a pragma to disable the warnings.
# Output of gencmn is redirected to OpenOffice_icu_tmp.c with the -t switch.
$(WORKDIR)/CustomTarget/i18npool/source/breakiterator/data/OpenOffice_dat.c : $(wildcard $(SRCDIR)/i18npool/source/breakiterator/data/*.txt)
	RESPONSEFILE=$(call var2filecr,$(shell $(gb_MKTEMP)),1,$(foreach txt,$(wildcard $(SRCDIR)/i18npool/source/breakiterator/data/*.txt),$(notdir $(basename $(txt))).brk)) \
	&& cat $${RESPONSEFILE} \
	&& $(GENCMN) -n OpenOffice -t tmp -S -d $(call gb_Helper_convert_native,$(dir $@)) O $(call gb_Helper_convert_native,$${RESPONSEFILE}) \
	&& echo "#ifdef _MSC_VER" > $@ \
	&& echo "#pragma warning( disable : 4229 4668 )" >> $@ \
	&& echo "#endif" >> $@ \
	&& cat $(WORKDIR)/CustomTarget/i18npool/source/breakiterator/data/OpenOffice_tmp.c >> $@ \
	&& rm -f $${RESPONSEFILE}

#$(eval $(call gb_Library_add_generated_cobjects,i18npool,\
#	$(foreach txt,$(wildcard $(SRCDIR)/i18npool/source/breakiterator/data/*.txt),CustomTarget/i18npool/source/breakiterator/data/$(notdir $(basename $(txt)))_brk) \
#))

$(foreach txt,$(wildcard $(SRCDIR)/i18npool/source/breakiterator/data/*.txt),$(eval $(call gb_Library_add_generated_cobject,i18npool,CustomTarget/i18npool/source/breakiterator/data/$(notdir $(basename $(txt)))_brk)))

$(eval $(call gb_Library_add_generated_cobject,i18npool,CustomTarget/i18npool/source/breakiterator/data/OpenOffice_dat))

# vim: set noet sw=4 ts=4:

