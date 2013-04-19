# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,unoil_climaker,$(WORKDIR)/CustomTarget/unoil/climaker))

$(eval $(call gb_Package_add_file,unoil_climaker,bin/cli_oootypes.config,cli_oootypes.config))
$(eval $(call gb_Package_add_file,unoil_climaker,bin/cli_oootypes.dll,cli_oootypes.dll))
$(eval $(call gb_Package_add_file,unoil_climaker,bin/policy.1.0.cli_oootypes.dll,policy.1.0.cli_oootypes.dll))

# vim:set noet sw=4 ts=4:
