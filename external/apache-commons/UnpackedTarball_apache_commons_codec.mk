# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,apache_commons_codec))

$(eval $(call gb_UnpackedTarball_set_tarball,apache_commons_codec,$(APACHE_COMMONS_CODEC_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,apache_commons_codec,3))

$(eval $(call gb_UnpackedTarball_fix_end_of_line,apache_commons_codec,\
        build.xml \
))

$(eval $(call gb_UnpackedTarball_add_patches,apache_commons_codec,\
	external/apache-commons/patches/codec.patch \
))

# vim: set noet sw=4 ts=4:
