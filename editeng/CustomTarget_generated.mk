# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,editeng/generated))

SRC := $(SRCDIR)/editeng/source/misc
PL := $(SRCDIR)/solenv/bin/gentoken.pl
INC := $(call gb_CustomTarget_get_workdir,editeng/generated)

$(INC)/tokens.hxx $(INC)/tokens.gperf : $(SRC)/tokens.txt $(PL)
	mkdir -p $(INC)
	$(PERL) $(PL) $(SRC)/tokens.txt $(INC)/tokens.gperf

$(INC)/tokens.cxx : $(INC)/tokens.gperf
	$(GPERF) --compare-strncmp --readonly-tables --output-file=$(INC)/tokens.cxx $(INC)/tokens.gperf
	sed -i -e "s/(char\*)0/(char\*)0, XML_TOKEN_INVALID/g" $(INC)/tokens.cxx
	sed -i -e "/^#line/d" $(INC)/tokens.cxx

$(call gb_CustomTarget_get_target,editeng/generated) : $(INC)/tokens.cxx

# vim: set noet sw=4 ts=4:
