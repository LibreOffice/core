# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,i18npool/breakiterator))

i18npool_BIDIR := $(gb_CustomTarget_workdir)/i18npool/breakiterator

$(call gb_CustomTarget_get_target,i18npool/breakiterator) : \
	$(i18npool_BIDIR)/OpenOffice_dat.c

i18npool_BRKTXTS := \
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
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),CMN)
	$(call gb_Helper_abbreviate_dirs,\
		RESPONSEFILE=$(shell $(gb_MKTEMP)) && \
		$(foreach brk,$(i18npool_BRKTXTS),echo '$(brk)' >> $${RESPONSEFILE} && ) \
		$(call gb_ExternalExecutable_get_command,gencmn) -n OpenOffice -t tmp -S -d $(i18npool_BIDIR)/ 0 $${RESPONSEFILE} && \
		rm -f $${RESPONSEFILE} && \
		echo '#ifdef _MSC_VER' > $@ && \
		echo '#pragma warning( disable : 4229 )' >> $@ && \
		echo '#endif' >> $@ && \
		cat $(subst _dat,_tmp,$@) >> $@)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),CMN)

$(i18npool_BIDIR)/%_brk.c : $(i18npool_BIDIR)/%.brk $(call gb_ExternalExecutable_get_dependencies,genccode)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CCD,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),CCD)
	$(call gb_Helper_abbreviate_dirs,\
		$(call gb_ExternalExecutable_get_command,genccode) -n OpenOffice -d $(i18npool_BIDIR)/ $< \
			$(if $(findstring s,$(MAKEFLAGS)),> /dev/null))
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),CCD)

$(i18npool_BIDIR)/%.brk : $(SRCDIR)/i18npool/source/breakiterator/data/%.txt \
		$(call gb_ExternalExecutable_get_dependencies,genbrk) \
		| $(i18npool_BIDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),BRK,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),BRK)
	$(call gb_Helper_abbreviate_dirs,\
		$(call gb_ExternalExecutable_get_command,genbrk) \
		$(if $(SYSTEM_ICU),,-i $(gb_UnpackedTarball_workdir)/icu/source/data/out/tmp) \
		-r $< -o $@ $(if $(findstring s,$(MAKEFLAGS)),> /dev/null))
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),BRK)

# vim: set noet sw=4 ts=4:
