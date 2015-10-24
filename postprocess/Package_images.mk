# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,postprocess_images,$(call gb_CustomTarget_get_workdir,postprocess/images)))

$(eval $(call gb_Package_add_files,postprocess_images,$(LIBO_SHARE_FOLDER)/config,\
	$(foreach theme,$(WITH_THEMES),images_$(theme).zip) \
))

# images.list in XML format, for online help
ifneq ($(filter HELP,$(BUILD_TYPE)),)
$(eval $(call gb_Package_add_files,postprocess_images,$(LIBO_SHARE_HELP_FOLDER),\
	$(foreach theme,$(WITH_THEMES),links_$(theme).xml) \
))
endif

# vim: set noet sw=4 ts=4:
