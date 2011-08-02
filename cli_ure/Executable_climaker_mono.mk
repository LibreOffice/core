# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CliExecutable_CliExecutable,climaker))

$(eval $(call gb_CliExecutable_add_csfiles,climaker,\
    cli_ure/source/climaker/climaker_csharp \
))

$(eval $(call gb_CliExecutable_use_assemblies,climaker,\
	cli_basetypes \
))

# vim: set noet sw=4 ts=4:
