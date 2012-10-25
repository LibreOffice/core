# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,graphite))

$(eval $(call gb_StaticLibrary_use_unpacked,graphite,graphite))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,graphite))

$(eval $(call gb_StaticLibrary_set_include,graphite,\
	-I$(call gb_UnpackedTarball_get_dir,graphite/include) \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,graphite,\
	-DDISABLE_TRACING \
	-DGR2_STATIC \
))

ifeq ($(COM),GCC)
$(eval $(call gb_StaticLibrary_add_cxxflags,graphite,\
	 -fpermissive \
))
endif

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,graphite,cpp))

$(eval $(call gb_StaticLibrary_add_generated_cxxobjects,graphite,\
	UnpackedTarball/graphite/src/$(if $(filter GCC,$(COM)),direct,call)_machine \
	UnpackedTarball/graphite/src/gr_char_info \
	UnpackedTarball/graphite/src/gr_face \
	UnpackedTarball/graphite/src/gr_features \
	UnpackedTarball/graphite/src/gr_font \
	UnpackedTarball/graphite/src/gr_logging \
	UnpackedTarball/graphite/src/gr_segment \
	UnpackedTarball/graphite/src/gr_slot \
	UnpackedTarball/graphite/src/json \
	UnpackedTarball/graphite/src/Bidi \
	UnpackedTarball/graphite/src/CachedFace \
	UnpackedTarball/graphite/src/CmapCache \
	UnpackedTarball/graphite/src/Code \
	UnpackedTarball/graphite/src/Face \
	UnpackedTarball/graphite/src/FeatureMap \
	UnpackedTarball/graphite/src/FileFace \
	UnpackedTarball/graphite/src/Font \
	UnpackedTarball/graphite/src/GlyphCache \
	UnpackedTarball/graphite/src/GlyphFace \
	UnpackedTarball/graphite/src/Justifier \
	UnpackedTarball/graphite/src/NameTable \
	UnpackedTarball/graphite/src/Pass \
	UnpackedTarball/graphite/src/SegCache \
	UnpackedTarball/graphite/src/SegCacheEntry \
	UnpackedTarball/graphite/src/SegCacheStore \
	UnpackedTarball/graphite/src/Segment \
	UnpackedTarball/graphite/src/Silf \
	UnpackedTarball/graphite/src/Slot \
	UnpackedTarball/graphite/src/Sparse \
	UnpackedTarball/graphite/src/TtfUtil \
	UnpackedTarball/graphite/src/UtfCodec \
))

# vim: set noet sw=4 ts=4:
