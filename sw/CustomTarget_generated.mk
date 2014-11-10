# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,sw/generated))

SRC := $(SRCDIR)/sw/source/core/swg
PL := $(SRCDIR)/solenv/bin/gentoken.pl
INC := $(call gb_CustomTarget_get_workdir,sw/generated)

$(INC)/TextBlockTokens.gperf : $(SRC)/TextBlockTokens.txt $(PL)
	mkdir -p $(INC)
	$(PERL) $(PL) $(SRC)/TextBlockTokens.txt $(INC)/TextBlockTokens.gperf
	
$(INC)/BlockListTokens.gperf : $(SRC)/BlockListTokens.txt $(PL)
	mkdir -p $(INC)s
	$(PERL) $(PL) $(SRC)/BlockListTokens.txt $(INC)/BlockListTokens.gperf

$(INC)/tokens.cxx : $(INC)/TextBlockTokens.gperf $(INC)/BlockListTokens.gperf
	$(GPERF) --compare-strncmp --readonly-tables --output-file=$(INC)/tokens.cxx \
	--class-name=TextBlockTokens --word-array-name=textBlockList --enum $(INC)/TextBlockTokens.gperf
	$(GPERF) --compare-strncmp --readonly-tables -T $(INC)/BlockListTokens.gperf \
	--class-name=BlockListTokens --word-array-name=blockListList >> $(INC)/tokens.cxx
	sed -i -e "s/(char\*)0/(char\*)0, XML_TOKEN_INVALID/g" $(INC)/tokens.cxx
	sed -i -e "/^#line/d" $(INC)/tokens.cxx

$(call gb_CustomTarget_get_target,sw/generated) : $(INC)/tokens.cxx

# vim: set noet sw=4 ts=4:
