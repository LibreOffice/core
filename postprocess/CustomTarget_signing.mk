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

# postprocess_PFXFILE has to be set elsewhere ??
# postprocess_PFXPASSWORD has to be set elsewhere ??

postprocess_EXCLUDELIST := $(SRCDIR)/postprocess/signing/no_signing.txt
postprocess_IMAGENAMES := $(OUTDIR)/bin/*.dll $(OUTDIR)/bin/so/*.dll $(OUTDIR)/bin/*.exe $(OUTDIR)/bin/so/*.exe

$(call gb_CustomTarget_get_workdir,postprocess/signing)/signing.done: \
		$(SRCDIR)/postprocess/signing/signing.pl \
		$(postprocess_EXCLUDELIST) \

$(call gb_CustomTarget_get_workdir,postprocess/signing)/signing.done:
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,2)
ifeq ($(COM),MSC)
ifeq ($(PRODUCT),full)
	$(PERL) $< -e $(postprocess_EXCLUDELIST) \
			-l $(subst .done,_log.txt,$@) \
			-f $(postprocess_PFXFILE) \
			-p $(postprocess_PFXPASSWORD) \
			-t "http://timestamp.verisign.com/scripts/timstamp.dll" \
			$(postprocess_IMAGENAMES) \
	&& touch $@
else
	@echo "Doing nothing on non product builds ..."
endif
else
	@echo "Nothing to do, signing is Windows (MSC) only."
endif

# vim: set noet sw=4 ts=4:
