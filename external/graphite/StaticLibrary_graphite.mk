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

$(eval $(call gb_StaticLibrary_set_warnings_disabled,graphite))

$(eval $(call gb_StaticLibrary_set_include,graphite,\
	-I$(gb_UnpackedTarball_workdir)/graphite/include \
	-I$(gb_UnpackedTarball_workdir)/graphite/src \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,graphite,\
	-DGRAPHITE2_NTRACING \
	-DGRAPHITE2_STATIC \
))

ifeq ($(COM),GCC)
ifneq ($(COM_IS_CLANG),TRUE)
$(eval $(call gb_StaticLibrary_add_cxxflags,graphite,\
	 -fpermissive \
))
endif
endif

ifeq ($(OS),ANDROID)
# Force optimizations on Android, because otherwise Pass.cpp triggers an
# infinite loop in clang, at least in this version:
# Android (5058415 based on r339409) clang version 8.0.2 (https://android.googlesource.com/toolchain/clang 40173bab62ec746213857d083c0e8b0abb568790) (https://android.googlesource.com/toolchain/llvm 7a6618d69e7e8111e1d49dc9e7813767c5ca756a) (based on LLVM 8.0.2svn)
$(eval $(call gb_StaticLibrary_add_cxxflags,graphite,\
     -Os \
))
endif

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,graphite,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,graphite,\
	UnpackedTarball/graphite/src/$(if $(filter GCC,$(COM)),direct,call)_machine \
	UnpackedTarball/graphite/src/gr_char_info \
	UnpackedTarball/graphite/src/gr_face \
	UnpackedTarball/graphite/src/gr_features \
	UnpackedTarball/graphite/src/gr_font \
	UnpackedTarball/graphite/src/gr_logging \
	UnpackedTarball/graphite/src/gr_segment \
	UnpackedTarball/graphite/src/gr_slot \
	UnpackedTarball/graphite/src/json \
	UnpackedTarball/graphite/src/CmapCache \
	UnpackedTarball/graphite/src/Code \
	UnpackedTarball/graphite/src/Collider \
	UnpackedTarball/graphite/src/Decompressor \
	UnpackedTarball/graphite/src/Face \
	UnpackedTarball/graphite/src/FeatureMap \
	UnpackedTarball/graphite/src/FileFace \
	UnpackedTarball/graphite/src/Font \
	UnpackedTarball/graphite/src/GlyphCache \
	UnpackedTarball/graphite/src/GlyphFace \
	UnpackedTarball/graphite/src/Intervals \
	UnpackedTarball/graphite/src/Justifier \
	UnpackedTarball/graphite/src/NameTable \
	UnpackedTarball/graphite/src/Pass \
	UnpackedTarball/graphite/src/Position \
	UnpackedTarball/graphite/src/Segment \
	UnpackedTarball/graphite/src/Silf \
	UnpackedTarball/graphite/src/Slot \
	UnpackedTarball/graphite/src/Sparse \
	UnpackedTarball/graphite/src/TtfUtil \
	UnpackedTarball/graphite/src/UtfCodec \
))

# vim: set noet sw=4 ts=4:
