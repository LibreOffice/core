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
sw_PY := $(SRCDIR)/solenv/bin/gentoken.py
sw_INC := $(gb_CustomTarget_workdir)/sw/generated

# static pattern rule
$(sw_INC)/TextBlockTokens.gperf \
$(sw_INC)/BlockListTokens.gperf : $(sw_INC)/%.gperf : $(sw_SRC)/%.txt $(sw_PY) \
            | $(call gb_ExternalExecutable_get_dependencies,python)
	mkdir -p $(@D)
	$(call gb_ExternalExecutable_get_command,python) $(sw_PY) $< $@

# appending when wrapping with wsl.exe is broken, will always replace from start instead of add
# https://github.com/microsoft/WSL/issues/4400 so create them as separate intermediates
$(sw_INC)/TextBlockTokens.processed : $(sw_INC)/TextBlockTokens.gperf
	$(GPERF) --compare-strncmp --readonly-tables --class-name=TextBlockTokens \
	--word-array-name=textBlockList --enum $(sw_INC)/TextBlockTokens.gperf > $@

$(sw_INC)/BlockListTokens.processed : $(sw_INC)/BlockListTokens.gperf
	$(GPERF) --compare-strncmp --readonly-tables -T $(sw_INC)/BlockListTokens.gperf \
	--class-name=BlockListTokens --word-array-name=blockListList > $@

$(sw_INC)/tokens.cxx : sw_INC:=$(call gb_Helper_wsl_path,$(sw_INC))
$(sw_INC)/tokens.cxx : $(sw_INC)/TextBlockTokens.processed $(sw_INC)/BlockListTokens.processed
	sed -e '/^#line/d' -e 's/(char\*)0/(char\*)0, XML_TOKEN_INVALID/g' $^ > $@

$(call gb_CustomTarget_get_target,sw/generated) : $(sw_INC)/tokens.cxx

# vim: set noet sw=4 ts=4:
