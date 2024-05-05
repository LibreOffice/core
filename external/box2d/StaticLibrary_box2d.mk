# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,box2d))

$(eval $(call gb_StaticLibrary_use_unpacked,box2d,box2d))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,box2d))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,box2d,cpp))

$(eval $(call gb_StaticLibrary_set_include,box2d,\
    -I$(gb_UnpackedTarball_workdir)/box2d/include/\
    -I$(gb_UnpackedTarball_workdir)/box2d/src/\
    $$(INCLUDE)\
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,box2d,\
	UnpackedTarball/box2d/src/common/b2_timer \
	UnpackedTarball/box2d/src/common/b2_stack_allocator \
	UnpackedTarball/box2d/src/common/b2_draw \
	UnpackedTarball/box2d/src/common/b2_math \
	UnpackedTarball/box2d/src/common/b2_block_allocator \
	UnpackedTarball/box2d/src/common/b2_settings \
	UnpackedTarball/box2d/src/dynamics/b2_body \
	UnpackedTarball/box2d/src/dynamics/b2_polygon_circle_contact \
	UnpackedTarball/box2d/src/dynamics/b2_circle_contact \
	UnpackedTarball/box2d/src/dynamics/b2_contact_solver \
	UnpackedTarball/box2d/src/dynamics/b2_polygon_contact \
	UnpackedTarball/box2d/src/dynamics/b2_chain_polygon_contact \
	UnpackedTarball/box2d/src/dynamics/b2_chain_circle_contact \
	UnpackedTarball/box2d/src/dynamics/b2_contact \
	UnpackedTarball/box2d/src/dynamics/b2_edge_polygon_contact \
	UnpackedTarball/box2d/src/dynamics/b2_edge_circle_contact \
	UnpackedTarball/box2d/src/dynamics/b2_wheel_joint \
	UnpackedTarball/box2d/src/dynamics/b2_friction_joint \
	UnpackedTarball/box2d/src/dynamics/b2_prismatic_joint \
	UnpackedTarball/box2d/src/dynamics/b2_weld_joint \
	UnpackedTarball/box2d/src/dynamics/b2_pulley_joint \
	UnpackedTarball/box2d/src/dynamics/b2_mouse_joint \
	UnpackedTarball/box2d/src/dynamics/b2_motor_joint \
	UnpackedTarball/box2d/src/dynamics/b2_distance_joint \
	UnpackedTarball/box2d/src/dynamics/b2_gear_joint \
	UnpackedTarball/box2d/src/dynamics/b2_joint \
	UnpackedTarball/box2d/src/dynamics/b2_revolute_joint \
	UnpackedTarball/box2d/src/dynamics/b2_world_callbacks \
	UnpackedTarball/box2d/src/dynamics/b2_fixture \
	UnpackedTarball/box2d/src/dynamics/b2_contact_manager \
	UnpackedTarball/box2d/src/dynamics/b2_island \
	UnpackedTarball/box2d/src/dynamics/b2_world \
	UnpackedTarball/box2d/src/rope/b2_rope \
	UnpackedTarball/box2d/src/collision/b2_time_of_impact \
	UnpackedTarball/box2d/src/collision/b2_collide_polygon \
	UnpackedTarball/box2d/src/collision/b2_distance \
	UnpackedTarball/box2d/src/collision/b2_collision \
	UnpackedTarball/box2d/src/collision/b2_collide_edge \
	UnpackedTarball/box2d/src/collision/b2_collide_circle \
	UnpackedTarball/box2d/src/collision/b2_broad_phase \
	UnpackedTarball/box2d/src/collision/b2_edge_shape \
	UnpackedTarball/box2d/src/collision/b2_circle_shape \
	UnpackedTarball/box2d/src/collision/b2_chain_shape \
	UnpackedTarball/box2d/src/collision/b2_polygon_shape \
	UnpackedTarball/box2d/src/collision/b2_dynamic_tree \
))

# vim: set noet sw=4 ts=4:
