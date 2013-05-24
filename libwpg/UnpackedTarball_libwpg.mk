# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libwpg))

$(eval $(call gb_UnpackedTarball_set_tarball,libwpg,$(WPG_TARBALL)))

# HACK to avoid the need to deliver the headers to $(OUTDIR). This
# should be removed when libwpg is updated.
$(eval $(call gb_UnpackedTarball_set_post_action,libwpg,\
	mkdir -p inc/libwpg && \
	cp -f src/lib/libwpg.h src/lib/WPGPaintInterface.h src/lib/WPGraphics.h inc/libwpg \
))

# vim: set noet sw=4 ts=4:
