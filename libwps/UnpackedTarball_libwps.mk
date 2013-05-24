# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libwps))

$(eval $(call gb_UnpackedTarball_set_tarball,libwps,$(WPS_TARBALL)))

# HACK to avoid the need to deliver the headers to $(OUTDIR). This
# should be removed when libwps is updated.
$(eval $(call gb_UnpackedTarball_set_post_action,libwps,\
	mkdir -p inc/libwps && \
	cp -f src/lib/libwps.h src/lib/WPSDocument.h inc/libwps \
))

# vim: set noet sw=4 ts=4:
