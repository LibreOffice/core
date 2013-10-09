# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,setup_native/ulf,$(call gb_CustomTarget_get_workdir,setup_native/mac)))

$(eval $(call gb_Package_add_file,setup_native/ulf,bin/macinstall.ulf,macinstall.ulf))

# vim: set noet sw=4 ts=4:
