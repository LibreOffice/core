# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,sw/generated))

sw_SRC := $(SRCDIR)/sw/source/core/swg
sw_PL := $(SRCDIR)/solenv/bin/gentoken.pl
sw_INC := $(call gb_CustomTarget_get_workdir,sw/generated)

$(sw_INC)/TextBlockTokens.gperf : $(sw_SRC)/TextBlockTokens.txt $(sw_PL)
	mkdir -p $(sw_INC)
	$(PERL) $(sw_PL) $(sw_SRC)/TextBlockTokens.txt $(sw_INC)/TextBlockTokens.gperf

$(sw_INC)/BlockListTokens.gperf : $(sw_SRC)/BlockListTokens.txt $(sw_PL)
	mkdir -p $(sw_INC)
	$(PERL) $(sw_PL) $(sw_SRC)/BlockListTokens.txt $(sw_INC)/BlockListTokens.gperf

$(sw_INC)/tokens.cxx : $(sw_INC)/TextBlockTokens.gperf $(sw_INC)/BlockListTokens.gperf
	$(GPERF) --compare-strncmp --readonly-tables --output-file=$(sw_INC)/tokens.cxx \
	--class-name=TextBlockTokens --word-array-name=textBlockList --enum $(sw_INC)/TextBlockTokens.gperf
	$(GPERF) --compare-strncmp --readonly-tables -T $(sw_INC)/BlockListTokens.gperf \
	--class-name=BlockListTokens --word-array-name=blockListList >> $(sw_INC)/tokens.cxx
	sed -i $(if $(filter MACOSX,$(OS_FOR_BUILD)),'') -e "s/(char\*)0/(char\*)0, XML_TOKEN_INVALID/g" $(sw_INC)/tokens.cxx
	sed -i $(if $(filter MACOSX,$(OS_FOR_BUILD)),'') -e "/^#line/d" $(sw_INC)/tokens.cxx

$(call gb_CustomTarget_get_target,sw/generated) : $(sw_INC)/tokens.cxx

# vim: set noet sw=4 ts=4:
