# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,postprocess_images,$(gb_CustomTarget_workdir)/postprocess/images))

$(eval $(call gb_Package_add_files,postprocess_images,$(LIBO_SHARE_FOLDER)/config,\
	$(foreach theme,$(WITH_THEMES),images_$(theme).zip) \
))

# vim: set noet sw=4 ts=4:
