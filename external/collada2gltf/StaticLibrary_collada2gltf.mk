# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,collada2gltf))

$(eval $(call gb_StaticLibrary_use_unpacked,collada2gltf,collada2gltf))

$(eval $(call gb_StaticLibrary_use_externals,collada2gltf, \
	opencollada_parser \
	png \
))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,collada2gltf))

$(eval $(call gb_StaticLibrary_add_defs,collada2gltf,\
	-DUSE_OPEN3DGC \
))

# collada2gltf has a lot of warnings, suppress them.
$(eval $(call gb_StaticLibrary_add_cxxflags,collada2gltf,\
	-w \
))
ifeq ($(COM),MSC)
$(eval $(call gb_StaticLibrary_add_defs,collada2gltf,\
	-D_CRT_SECURE_NO_WARNINGS \
	-EHsc \
))

endif

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,collada2gltf,cpp))

$(eval $(call gb_StaticLibrary_set_include,collada2gltf,\
	-I$(call gb_UnpackedTarball_get_dir,collada2gltf) \
	-I$(call gb_UnpackedTarball_get_dir,collada2gltf)/assetModifiers \
	-I$(call gb_UnpackedTarball_get_dir,collada2gltf)/convert \
	-I$(call gb_UnpackedTarball_get_dir,collada2gltf)/dependencies/json/include/rapidjson \
	-I$(call gb_UnpackedTarball_get_dir,collada2gltf)/extensions/o3dgc-compression \
	-I$(call gb_UnpackedTarball_get_dir,collada2gltf)/GLTF \
	-I$(call gb_UnpackedTarball_get_dir,collada2gltf)/helpers \
	-I$(call gb_UnpackedTarball_get_dir,collada2gltf)/JSON \
	-I$(call gb_UnpackedTarball_get_dir,collada2gltf)/profiles/webgl-1.0 \
	-I$(call gb_UnpackedTarball_get_dir,collada2gltf)/shaders \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,collada2gltf,\
	UnpackedTarball/collada2gltf/COLLADA2GLTFWriter \
	UnpackedTarball/collada2gltf/GitSHA1 \
	UnpackedTarball/collada2gltf/GLTFOpenCOLLADAUtils \
	UnpackedTarball/collada2gltf/assetModifiers/GLTFFlipUVModifier \
	UnpackedTarball/collada2gltf/convert/meshConverter \
	UnpackedTarball/collada2gltf/convert/animationConverter \
	UnpackedTarball/collada2gltf/extensions/o3dgc-compression/GLTF-Open3DGC \
	UnpackedTarball/collada2gltf/GLTF/GLTFAccessor \
	UnpackedTarball/collada2gltf/GLTF/GLTFAccessorCache \
	UnpackedTarball/collada2gltf/GLTF/GLTFAsset \
	UnpackedTarball/collada2gltf/GLTF/GLTFAnimation \
	UnpackedTarball/collada2gltf/GLTF/GLTFBuffer \
	UnpackedTarball/collada2gltf/GLTF/GLTFConfig \
	UnpackedTarball/collada2gltf/GLTF/GLTFEffect \
	UnpackedTarball/collada2gltf/GLTF/GLTFExtraDataHandler \
	UnpackedTarball/collada2gltf/GLTF/GLTFInputStream \
	UnpackedTarball/collada2gltf/GLTF/GLTFMesh \
	UnpackedTarball/collada2gltf/GLTF/GLTFOutputStream \
	UnpackedTarball/collada2gltf/GLTF/GLTFPrimitive \
	UnpackedTarball/collada2gltf/GLTF/GLTFProfile \
	UnpackedTarball/collada2gltf/GLTF/GLTFSkin \
	UnpackedTarball/collada2gltf/GLTF/GLTFUtils \
	UnpackedTarball/collada2gltf/GLTF/GLTFWriter \
	UnpackedTarball/collada2gltf/helpers/geometryHelpers \
	UnpackedTarball/collada2gltf/helpers/mathHelpers \
	UnpackedTarball/collada2gltf/JSON/JSONArray \
	UnpackedTarball/collada2gltf/JSON/JSONNumber \
	UnpackedTarball/collada2gltf/JSON/JSONObject \
	UnpackedTarball/collada2gltf/JSON/JSONString \
	UnpackedTarball/collada2gltf/JSON/JSONValue \
	UnpackedTarball/collada2gltf/profiles/webgl-1.0/GLTFWebGL_1_0_Profile \
	UnpackedTarball/collada2gltf/shaders/commonProfileShaders \
))

# o3dgc external library compiled from collada2gltf
$(eval $(call gb_StaticLibrary_set_include,collada2gltf,\
	-I$(call gb_UnpackedTarball_get_dir,collada2gltf)/dependencies/o3dgc/src/o3dgc_common_lib/inc \
	-I$(call gb_UnpackedTarball_get_dir,collada2gltf)/dependencies/o3dgc/src/o3dgc_decode_lib/inc \
	-I$(call gb_UnpackedTarball_get_dir,collada2gltf)/dependencies/o3dgc/src/o3dgc_encode_lib/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,collada2gltf,\
	UnpackedTarball/collada2gltf/dependencies/o3dgc/src/o3dgc_common_lib/src/o3dgcArithmeticCodec \
	UnpackedTarball/collada2gltf/dependencies/o3dgc/src/o3dgc_common_lib/src/o3dgcTools \
	UnpackedTarball/collada2gltf/dependencies/o3dgc/src/o3dgc_common_lib/src/o3dgcTriangleFans \
	UnpackedTarball/collada2gltf/dependencies/o3dgc/src/o3dgc_decode_lib/src/o3dgcDynamicVectorDecoder \
	UnpackedTarball/collada2gltf/dependencies/o3dgc/src/o3dgc_encode_lib/src/o3dgcDynamicVectorEncoder \
))

# vim: set noet sw=4 ts=4:
