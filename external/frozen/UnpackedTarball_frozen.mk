# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,frozen))

$(eval $(call gb_UnpackedTarball_set_tarball,frozen,$(FROZEN_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,frozen,\
	external/frozen/cid1532449_use_move_ctor.0 \
	external/frozen/cid1586676_use_move_ctor.0 \
	external/frozen/cid1538304_reference_ctor.0 \
	external/frozen/cid1618765_big_param.0 \
))

# vim: set noet sw=4 ts=4:
