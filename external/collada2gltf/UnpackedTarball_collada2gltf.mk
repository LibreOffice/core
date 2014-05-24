# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,collada2gltf))

$(eval $(call gb_UnpackedTarball_set_tarball,collada2gltf,$(COLLADA2GLTF_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_file,collada2gltf,.,external/collada2gltf/GitSHA1.cpp))

$(eval $(call gb_UnpackedTarball_add_patches,collada2gltf,\
	external/collada2gltf/patches/add-unzip-kmz-files-to-collada-files-module.patch.3 \
	external/collada2gltf/patches/wrong_uri_usage.patch.1 \
))

ifneq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_patches,collada2gltf,\
	external/collada2gltf/patches/write-only-.dae-file-which-we-need.patch.0 \
))

endif



# vim: set noet sw=4 ts=4:
