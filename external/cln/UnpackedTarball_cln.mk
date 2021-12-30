# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,cln))

$(eval $(call gb_UnpackedTarball_set_tarball,cln,$(CLN_TARBALL)))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,cln))

ifeq ($(COM),MSC)
$(eval $(call gb_UnpackedTarball_set_post_action,cln,\
	touch include/cln/config.h.in \
))
endif

cln_patches += malloc_static_initialization_order.patch

$(eval $(call gb_UnpackedTarball_set_patchlevel,cln,0))

$(eval $(call gb_UnpackedTarball_add_patches,cln, \
        $(foreach patch,$(cln_patches),external/cln/$(patch)) \
))

# vim: set noet sw=4 ts=4:
