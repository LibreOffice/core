# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,libgltf))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,libgltf))

$(eval $(call gb_StaticLibrary_use_unpacked,libgltf,libgltf))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,libgltf,cpp))

# Avoid warnings
$(eval $(call gb_StaticLibrary_add_cxxflags,libgltf,-w))

$(eval $(call gb_StaticLibrary_use_externals,libgltf,\
    boost_headers \
    glm_headers \
    mesa_headers \
    glew \
    zlib \
    freetype \
))

$(eval $(call gb_StaticLibrary_set_include,libgltf,\
    -I$(call gb_UnpackedTarball_get_dir,libgltf)/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,libgltf,\
	UnpackedTarball/libgltf/src/Camera \
	UnpackedTarball/libgltf/src/Common \
	UnpackedTarball/libgltf/src/Font \
	UnpackedTarball/libgltf/src/FPSCounter \
	UnpackedTarball/libgltf/src/libgltf \
	UnpackedTarball/libgltf/src/LoadScene \
	UnpackedTarball/libgltf/src/RenderScene \
	UnpackedTarball/libgltf/src/Shaders \
	UnpackedTarball/libgltf/src/Texture \
))

# vim: set noet sw=4 ts=4:
