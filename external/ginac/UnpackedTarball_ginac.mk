# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,ginac))

$(eval $(call gb_UnpackedTarball_set_tarball,ginac,$(GINAC_TARBALL)))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,ginac))

ifeq ($(COM),MSC)
$(eval $(call gb_UnpackedTarball_set_post_action,ginac,\
	touch ginac/config.h \
))
endif

ginac_patches += digits_static_initialization_order.patch container_missing_copy_dtr.patch print_missing_copy_dtr.patch numeric_unused_parameter.patch

$(eval $(call gb_UnpackedTarball_set_patchlevel,ginac,0))

$(eval $(call gb_UnpackedTarball_add_patches,ginac, \
	$(foreach patch,$(ginac_patches),external/ginac/$(patch)) \
))

# vim: set noet sw=4 ts=4:
