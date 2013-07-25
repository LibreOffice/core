# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libvisio))

$(eval $(call gb_UnpackedTarball_set_tarball,libvisio,$(VISIO_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,libvisio,\
	libvisio/libvisio-0.0.30-msvc.patch.1 \
	libvisio/libvisio-wpx_seek_end.patch.1 \
))

# vim: set noet sw=4 ts=4:
