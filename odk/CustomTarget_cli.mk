# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/odkcommon/cli))

odk_CLILIST := cli_basetypes.dll \
		cli_uretypes.dll \
		cli_oootypes.dll \
		cli_ure.dll \
		cli_cppuhelper.dll

define odk_cli
odkcommon_ZIPLIST += cli/$(1)
$(call gb_CustomTarget_get_target,odk/odkcommon/cli): $(odk_WORKDIR)/cli/$(1)
$(odk_WORKDIR)/cli/$(1): $(call gb_CliAssembly_get_target,$(1))
	mkdir -p $$(dir $$@)
	$$(call gb_Output_announce,$$(subst $$(WORKDIR)/,,$$@),build,CPY,1)
	cp $$< $$@
endef

$(foreach cli,$(odk_CLILIST),$(eval $(call odk_cli,$(cli))))

# vim: set noet sw=4 ts=4:
