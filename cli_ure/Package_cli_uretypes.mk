# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(SRCDIR)/cli_ure/version/version.txt

$(eval $(call gb_Package_Package,cli_ure_unotypes,$(WORKDIR)/CustomTarget/cli_ure/unotypes))

$(eval $(call gb_Package_add_file,cli_ure_unotypes,bin/cli_uretypes.config,cli_uretypes.config))
$(eval $(call gb_Package_add_file,cli_ure_unotypes,bin/cli_uretypes.dll,cli_uretypes.dll))
$(eval $(call gb_Package_add_file,cli_ure_unotypes,bin/$(CLI_URETYPES_POLICY_ASSEMBLY).dll,$(CLI_URETYPES_POLICY_ASSEMBLY).dll))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
