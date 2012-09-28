# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,apache_commons_logging_inc,$(call gb_UnpackedTarball_get_dir,apache_commons_logging)))

$(eval $(call gb_Package_use_external_project,apache_commons_logging_inc,apache_commons_logging))

$(eval $(call gb_Package_add_file,apache_commons_logging_inc,bin/commons-logging-1.1.1.jar,target/commons-logging-1.1.1-SNAPSHOT.jar))

# vim: set noet sw=4 ts=4:
