# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,boostdatetime,$(gb_StaticLibrary_OUTDIRLOCATION)))

$(eval $(call gb_Package_add_file,boostdatetime,lib/libboost_date_time-vc90-mt-1_44.lib,$(notdir $(call gb_StaticLibrary_get_target,boostdatetime))))

# vim: set noet sw=4 ts=4:
