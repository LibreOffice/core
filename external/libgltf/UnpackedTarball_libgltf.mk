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
	external/libgltf/patches/libgltf_configure.patch \
	external/libgltf/patches/remove_config_h_in_dependencies.patch \
	external/libgltf/patches/init_scene_set_handle.patch \
	external/libgltf/patches/extend_get_bitmap_for_rgba_ones.patch \
	external/libgltf/patches/wrong_duration_caused_by_uninitialized_member.patch \
	external/libgltf/patches/wrong_media_time_caused_by_missing_fmod.patch \
	external/libgltf/patches/wrong_animation_state_when_set_time_to_null.patch \
	external/libgltf/patches/no_effect_of_set_looping.patch \
	external/libgltf/patches/wrong_media_time_at_first_run_caused_by_uninitilized_member.patch \
	external/libgltf/patches/no_animation_in_orbit_mode.patch \
	external/libgltf/patches/walkthrough_mode_affects_orbit_mode_unexpectedly.patch \
	external/libgltf/patches/move_fps_closer_to_the_corner.patch \
))

# vim: set noet sw=4 ts=4:
