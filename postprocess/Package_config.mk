# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,postprocess_config,$(call gb_CustomTarget_get_workdir,postprocess/config)))

$(eval $(call gb_Package_add_file,postprocess_config,bin/uiconfig.zip,uiconfig.zip))

$(eval $(call gb_Package_add_files,postprocess_config,bin,\
	$(foreach lang,$(filter-out en-US,$(gb_WITH_LANG)),uiconfig_$(lang).zip) \
))

# vim: set noet sw=4 ts=4:
