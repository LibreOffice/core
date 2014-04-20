# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libgltf))

$(eval $(call gb_UnpackedTarball_set_tarball,libgltf,$(LIBGLTF_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchflags,libgltf,--binary))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libgltf,1))

$(eval $(call gb_UnpackedTarball_add_patches,libgltf,\
	external/libgltf/patches/include_path_glew.patch \
	external/libgltf/patches/include_path_freetype.patch \
	external/libgltf/patches/missing_include_cstring.patch \
	external/libgltf/patches/win_only_variables.patch \
	external/libgltf/patches/unneeded_context_handling.patch \
	external/libgltf/patches/include_typo_texture.patch \
	external/libgltf/patches/adress_of_temporary.patch \
	external/libgltf/patches/avoid_c++11.patch \
	external/libgltf/patches/json_charbuffer_used_as_cstring.patch \
	external/libgltf/patches/shader_charbuffer_used_as_cstring.patch \
))

# vim: set noet sw=4 ts=4:
