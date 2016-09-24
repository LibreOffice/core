# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,postprocess/signing))


$(eval $(call gb_CustomTarget_register_targets,postprocess/signing,\
	signing.done \
))

# PFXFILE and PFXPASSWORD should be set in environment
TIMESTAMPURL ?= "http://timestamp.globalsign.com/scripts/timestamp.dll"

$(call gb_CustomTarget_get_workdir,postprocess/signing)/signing.done: \
	$(SRCDIR)/postprocess/signing/signing.pl \
	$(SRCDIR)/postprocess/signing/no_signing.txt \
	$(call gb_Postprocess_get_target,AllLibraries) \
	$(call gb_Postprocess_get_target,AllExecutables) \
	$(call gb_Postprocess_get_target,AllModuleTests) \
	$(call gb_Postprocess_get_target,AllModuleSlowtests) \
	$(call gb_Postprocess_get_target,AllModuleScreenshots)

$(call gb_CustomTarget_get_workdir,postprocess/signing)/signing.done:
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,2)
ifeq ($(COM),MSC)
ifneq ($(ENABLE_DBGUTIL),TRUE)
	EXCLUDELIST=$(shell $(gb_MKTEMP)) && \
	cat $(SRCDIR)/postprocess/signing/no_signing.txt > $$EXCLUDELIST && \
	echo "$(foreach lib,$(gb_MERGEDLIBS),$(call gb_Library_get_filename,$(lib)))" | tr ' ' '\n' >> $$EXCLUDELIST && \
	$(PERL) $(SRCDIR)/postprocess/signing/signing.pl \
			-e $$EXCLUDELIST \
			-l $(subst .done,_log.txt,$@) \
			$(if $(PFXFILE),-f $(PFXFILE)) \
			$(if $(PFXPASSWORD),-p $(PFXPASSWORD)) \
			$(if $(TIMESTAMPURL),-t $(TIMESTAMPURL)) \
			$(INSTDIR)/URE/bin/*.dll \
			$(INSTDIR)/URE/bin/*.exe \
			$(INSTDIR)/program/*.dll \
			$(INSTDIR)/program/*.exe \
			$(INSTDIR)/program/shlxthdl/*.dll \
			$(INSTDIR)/sdk/cli/*.dll \
			$(INSTDIR)/sdk/bin/*.exe \
			$(INSTDIR)/share/extensions/mysql-connector-ooo/*.dll\
	&& rm $$EXCLUDELIST && touch $@
else
	@echo "Doing nothing on non product builds ..."
endif
else
	@echo "Nothing to do, signing is Windows (MSC) only."
endif

# vim: set noet sw=4 ts=4:
