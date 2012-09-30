# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,saxon_inc,$(call gb_UnpackedTarball_get_dir,saxon)))

$(eval $(call gb_Package_use_external_project,saxon_inc,saxon))

$(eval $(call gb_Package_add_file,saxon_inc,bin/saxon9.jar,saxon-build/9.0.0.7/bj/saxon9.jar))

# vim: set noet sw=4 ts=4:
