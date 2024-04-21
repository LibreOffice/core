# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,editeng/generated))

editeng_SRC := $(SRCDIR)/editeng/source/misc
editeng_PY := $(SRCDIR)/solenv/bin/gentoken.py
editeng_INC := $(call gb_CustomTarget_get_workdir,editeng/generated)

$(editeng_INC)/tokens.hxx $(editeng_INC)/tokens.gperf : $(editeng_SRC)/tokens.txt $(editeng_PY) \
        $(call gb_ExternalExecutable_get_dependencies,python)
	mkdir -p $(editeng_INC)
	$(call gb_ExternalExecutable_get_command,python) $(editeng_PY) $(editeng_SRC)/tokens.txt $(editeng_INC)/tokens.gperf

$(editeng_INC)/tokens.cxx : $(editeng_INC)/tokens.gperf
	$(GPERF) --compare-strncmp --readonly-tables $(editeng_INC)/tokens.gperf \
	| sed -e '/^#line/d' -e 's/(char\*)0/(char\*)0, XML_TOKEN_INVALID/g' > $@

$(call gb_CustomTarget_get_target,editeng/generated) : $(editeng_INC)/tokens.cxx

# vim: set noet sw=4 ts=4:
