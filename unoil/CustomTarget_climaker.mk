# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(SRCDIR)/unoil/climaker/version.txt

$(eval $(call gb_CustomTarget_CustomTarget,unoil/climaker))

unoil_CLIDIR := $(call gb_CustomTarget_get_workdir,unoil/climaker)

$(call gb_CustomTarget_get_target,unoil/climaker) : \
	$(unoil_CLIDIR)/cli_oootypes.dll \
	$(unoil_CLIDIR)/cli_oootypes.config \
	$(unoil_CLIDIR)/$(CLI_OOOTYPES_POLICY_ASSEMBLY).dll

$(unoil_CLIDIR)/cli_oootypes.dll : $(SRCDIR)/unoil/climaker/version.txt \
		$(call gb_UnoApiTarget_get_target,offapi) \
		$(call gb_UnoApiTarget_get_target,udkapi) \
		$(OUTDIR)/bin/cliuno.snk $(OUTDIR)/bin/cli_uretypes.dll \
		$(call gb_Executable_get_runtime_dependencies,climaker) \
		| $(unoil_CLIDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CLM,1)
	$(call gb_Helper_abbreviate_dirs, \
	$(call gb_Helper_execute,climaker \
		$(if $(filter -s,$(MAKEFLAGS)),,--verbose) \
		--out $@ \
		--assembly-version $(CLI_OOOTYPES_NEW_VERSION) \
		--assembly-company "LibreOffice" \
		--assembly-description "This assembly contains metadata for the LibreOffice API." \
		-X $(call gb_UnoApiTarget_get_target,udkapi).oldformat \
		-r $(OUTDIR)/bin/cli_uretypes.dll \
		--keyfile $(OUTDIR)/bin/cliuno.snk \
		$(call gb_UnoApiTarget_get_target,offapi).oldformat) > /dev/null)

$(unoil_CLIDIR)/cli_oootypes.config : \
		$(SRCDIR)/unoil/climaker/cli_oootypes_config \
		$(SRCDIR)/unoil/climaker/version.txt | $(unoil_CLIDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,1)
	$(call gb_Helper_abbreviate_dirs, \
	perl $(SRCDIR)/solenv/bin/clipatchconfig.pl $^ $@)

$(unoil_CLIDIR)/$(CLI_OOOTYPES_POLICY_ASSEMBLY).dll : \
		$(unoil_CLIDIR)/cli_oootypes.config \
		$(unoil_CLIDIR)/cli_oootypes.dll $(OUTDIR)/bin/cliuno.snk
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),AL ,1)
	$(call gb_Helper_abbreviate_dirs, \
	al -out:$@ \
		-version:$(CLI_OOOTYPES_POLICY_VERSION) \
		-keyfile:$(OUTDIR)/bin/cliuno.snk \
		-link:$<)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
