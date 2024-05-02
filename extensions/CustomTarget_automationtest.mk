# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extensions/automationtest))

extensions_AUTOMATIONTESTDIR := $(gb_CustomTarget_workdir)/extensions/automationtest

extensions_AUTOMATIONTESTLOG1 := $(extensions_AUTOMATIONTESTDIR)/automationtest.1.log
extensions_AUTOMATIONTESTLOG2 := $(extensions_AUTOMATIONTESTDIR)/automationtest.2.log

$(call gb_CustomTarget_get_target,extensions/automationtest) : \
		$(SRCDIR)/extensions/qa/ole/automationtest.vbs \
		| $(extensions_AUTOMATIONTESTDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),VBS,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),VBS)
	SAL_LOG=+INFO.extensions.olebridge+WARN $(INSTDIR)/program/soffice 2>$(extensions_AUTOMATIONTESTLOG1) &
	sleep 10
	$(call gb_Helper_abbreviate_dirs, \
	cscript -nologo $(SRCDIR)/extensions/qa/ole/automationtest.vbs $(SRCDIR)) >$(extensions_AUTOMATIONTESTLOG2) || \
            (cat $(extensions_AUTOMATIONTESTLOG1) $(extensions_AUTOMATIONTESTLOG2) && exit 1)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),VBS)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
