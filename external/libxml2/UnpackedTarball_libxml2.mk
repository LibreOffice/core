# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libxml2))

$(eval $(call gb_UnpackedTarball_set_tarball,libxml2,$(LIBXML_TARBALL),,libxml2))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,libxml2))

$(eval $(call gb_UnpackedTarball_add_patches,libxml2,\
	$(if $(filter SOLARIS,$(OS)),external/libxml2/libxml2-global-symbols.patch) \
	external/libxml2/libxml2-vc10.patch \
	external/libxml2/libxml2-XMLCALL-redefine.patch.0 \
	$(if $(filter ANDROID,$(OS)),external/libxml2/libxml2-android.patch) \
	external/libxml2/libxml2-icu.patch.0 \
))

$(eval $(call gb_UnpackedTarball_add_file,libxml2,xml2-config.in,external/libxml2/xml2-config.in))

# vim: set noet sw=4 ts=4:
