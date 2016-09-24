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
	external/collada2gltf/patches/wrong_uri_usage.patch.1 \
	external/collada2gltf/patches/collada2gltf.new-delete-mismatch.patch.1 \
	external/collada2gltf/patches/collada2gltf.fix-memory-leak.patch.1 \
	external/collada2gltf/patches/collada2gltf.fix-memory-leak2.patch.1 \
	external/collada2gltf/patches/shader_compatibility.patch.1 \
	external/collada2gltf/patches/drop_tr1_support.patch.1 \
	external/collada2gltf/patches/collada2gltf.add-glsl-version.patch.1 \
))

# vim: set noet sw=4 ts=4:
