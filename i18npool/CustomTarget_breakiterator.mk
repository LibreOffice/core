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

$(eval $(call gb_CustomTarget_CustomTarget,i18npool/breakiterator,new_style))

IPBI := $(call gb_CustomTarget_get_workdir,i18npool/breakiterator)

$(call gb_CustomTarget_get_target,i18npool/breakiterator) : \
	$(IPBI)/dict_ja.cxx $(IPBI)/dict_zh.cxx $(IPBI)/OpenOffice_dat.c

$(IPBI)/dict_%.cxx : $(SRCDIR)/i18npool/source/breakiterator/data/%.dic \
		$(call gb_Executable_get_target_for_build,gendict) | $(IPBI)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),DIC,1)
	$(call gb_Helper_abbreviate_dirs_native,\
		$(call gb_Helper_execute,gendict) $< $@.tmp && \
		sed 's/\tconst/\tSAL_DLLPUBLIC_EXPORT const/' $@.tmp > $@  && \
		rm $@.tmp)

ifeq ($(SYSTEM_GENBRK),)
GENBRKTARGET := $(call gb_Executable_get_target_for_build,genbrk)
GENBRK := $(call gb_Helper_execute,genbrk)
else
GENBRKTARGET :=
GENBRK := $(SYSTEM_GENBRK)
endif

ifeq ($(SYSTEM_GENCCODE),)
GENCCODETARGET := $(call gb_Executable_get_target_for_build,genccode)
GENCCODE := $(call gb_Helper_execute,genccode)
else
GENCCODETARGET :=
GENCCODE := $(SYSTEM_GENCCODE)
endif

ifeq ($(SYSTEM_GENCMN),)
GENCMNTARGET := $(call gb_Executable_get_target_for_build,gencmn)
GENCMN := $(call gb_Helper_execute,gencmn)
else
GENCMNTARGET :=
GENCMN := $(SYSTEM_GENCMN)
endif

BRKFILES := $(subst .txt,.brk,$(notdir \
	$(wildcard $(SRCDIR)/i18npool/source/breakiterator/data/*.txt)))

# 'gencmn', 'genbrk' and 'genccode' are tools generated and delivered by icu project to process icu breakiterator rules.
# The output of gencmn generates warnings under Windows. We want to minimize the patches to external tools,
# so the output (OpenOffice_dat.c) is changed here to include a pragma to disable the warnings.
# Output of gencmn is redirected to OpenOffice_tmp.c with the -t switch.
$(IPBI)/OpenOffice_dat.c : $(patsubst %.brk,$(IPBI)/%_brk.c,$(BRKFILES)) $(GENCMNTARGET)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CMN,1)
	$(call gb_Helper_abbreviate_dirs_native,\
		RESPONSEFILE=$(shell $(gb_MKTEMP)) && \
		$(foreach brk,$(BRKFILES),echo '$(brk)' >> $${RESPONSEFILE} && ) \
		$(GENCMN) -n OpenOffice -t tmp -S -d $(IPBI)/ 0 $${RESPONSEFILE} && \
		rm -f $${RESPONSEFILE} && \
		echo '#ifdef _MSC_VER' > $@ && \
		echo '#pragma warning( disable : 4229 4668 )' >> $@ && \
		echo '#endif' >> $@ && \
		cat $(subst _dat,_tmp,$@) >> $@)

$(IPBI)/%_brk.c : $(IPBI)/%.brk $(GENCCODETARGET)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CCD,1)
	$(call gb_Helper_abbreviate_dirs_native,\
		$(GENCCODE) -n OpenOffice -d $(IPBI)/ $< \
			$(if $(findstring s,$(MAKEFLAGS)),> /dev/null))

$(IPBI)/%.brk : $(IPBI)/%.txt $(GENBRKTARGET)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),BRK,1)
	$(call gb_Helper_abbreviate_dirs_native,\
		$(GENBRK) -r $< -o $@ $(if $(findstring s,$(MAKEFLAGS)),> /dev/null))

# fdo#31271 ")" reclassified in more recent ICU/Unicode Standards
$(IPBI)/%.txt : $(SRCDIR)/i18npool/source/breakiterator/data/%.txt | $(IPBI)/.dir
ifeq ($(ICU_RECLASSIFIED_CLOSE_PARENTHESIS),YES)
	sed "s#\[:LineBreak =  Close_Punctuation:\]#\[\[:LineBreak =  Close_Punctuation:\] \[:LineBreak = Close_Parenthesis:\]\]#" $< > $@
else
	cp $< $@
endif

# vim: set noet sw=4 ts=4:
