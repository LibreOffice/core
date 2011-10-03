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

all : OpenOffice_dat.c data/dict_ja.cxx data/dict_zh.cxx

include $(GBUILDDIR)/gbuild_simple.mk

data:
	mkdir data

data/dict_%.cxx : data/dict_%_invis.cxx
	sed 's/\tconst/\tSAL_DLLPUBLIC_EXPORT const/' $< > $@

data/dict_%_invis.cxx : $(realpath $(SRC_ROOT)/i18npool/source/breakiterator/data)/%.dic data
ifeq ($(OS_FOR_BUILD),WNT)
	$(gb_Helper_execute)gendict `cygpath -m $<` $@
else
	$(gb_Helper_execute)gendict $< $@
endif

ifeq ($(SYSTEM_GENBRK),)
GENBRK := $(gb_Helper_execute)genbrk
else
GENBRK := $(SYSTEM_GENBRK)
endif

ifeq ($(SYSTEM_GENCODE),)
GENCCODE := $(gb_Helper_execute)genccode
else
GENCCODE := $(SYSTEM_GENCCODE)
endif

ifeq ($(SYSTEM_GENCMN),)
GENCMN := $(gb_Helper_execute)gencmn
else
GENCMN := $(SYSTEM_GENCMN)
endif

TEMPFILE := $(shell $(gb_MKTEMP))
BRKFILES := $(notdir $(subst .txt,.brk,$(wildcard $(SRC_ROOT)/i18npool/source/breakiterator/data/*.txt)))

# 'gencmn', 'genbrk' and 'genccode' are tools generated and delivered by icu project to process icu breakiterator rules.
# The output of gencmn generates warnings under Windows. We want to minimize the patches to external tools,
# so the output (OpenOffice_dat.c) is changed here to include a pragma to disable the warnings.
# Output of gencmn is redirected to OpenOffice_icu_tmp.c with the -t switch.
OpenOffice_dat.c : $(subst .brk,_brk.c,$(BRKFILES))
	$(foreach brkfile,$(BRKFILES),$(shell echo $(brkfile) >> $(TEMPFILE)))
	$(GENCMN) -n OpenOffice -t tmp -S -d ./ 0 $(TEMPFILE)
	echo '#ifdef _MSC_VER' > $@
	echo '#pragma warning( disable : 4229 4668 )' >> $@
	echo '#endif' >> $@
	cat $(subst _dat,_tmp,$@) >> $@

%_brk.c : %.brk
	$(GENCCODE) -n OpenOffice -d ./ $<

%.brk : %.txt.p
	$(GENBRK) -r $< -o $@

# fdo#31271 ")" reclassified in more recent ICU/Unicode Standards
%.txt.p : $(realpath $(SRC_ROOT)/i18npool/source/breakiterator/data)/%.txt
ifeq ($(ICU_RECLASSIFIED_CLOSE_PARENTHESIS),YES)
	sed "s#\[:LineBreak =  Close_Punctuation:\]#\[\[:LineBreak =  Close_Punctuation:\] \[:LineBreak = Close_Parenthesis:\]\]#" $< > $@
else
	cp $< $@
endif

.PHONY: all
# vim: set noet sw=4 ts=4:
