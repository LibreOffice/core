# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(SRCDIR)/cli_ure/version/version.txt

$(eval $(call gb_CustomTarget_CustomTarget,cli_ure/unotypes))

cli_uretypes_CLIDIR := $(call gb_CustomTarget_get_workdir,cli_ure/unotypes)

$(call gb_CustomTarget_get_target,cli_ure/unotypes) : \
	$(cli_uretypes_CLIDIR)/cli_uretypes.dll \
	$(cli_uretypes_CLIDIR)/cli_uretypes.config \
	$(cli_uretypes_CLIDIR)/$(CLI_URETYPES_POLICY_ASSEMBLY).dll


$(cli_uretypes_CLIDIR)/cli_uretypes.dll : $(OUTDIR)/inc/external/cli/cli_uretypes.dll
	mkdir -p $(@D)
	$(GNUCOPY) $? $@

$(cli_uretypes_CLIDIR)/cli_uretypes.config : $(OUTDIR)/inc/external/cli/cli_uretypes.config
	mkdir -p $(@D)
	$(GNUCOPY) $? $@

$(cli_uretypes_CLIDIR)/$(CLI_URETYPES_POLICY_ASSEMBLY).dll : $(OUTDIR)/inc/external/cli/$(CLI_URETYPES_POLICY_ASSEMBLY).dll
	mkdir -p $(@D)
	$(GNUCOPY) $< $@

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
