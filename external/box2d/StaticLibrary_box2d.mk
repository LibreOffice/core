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
    -I$(gb_UnpackedTarball_workdir)/box2d/extern/simde/\
    -I$(gb_UnpackedTarball_workdir)/box2d/src/\
    $$(INCLUDE)\
))

ifeq ($(OS),WNT)
$(eval $(call gb_StaticLibrary_add_cflags,box2d,\
    /std:c17 /experimental:c11atomics \
))
endif

$(eval $(call gb_StaticLibrary_add_defs,box2d,\
    -DBOX2D_DISABLE_SIMD \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,box2d,\
	UnpackedTarball/box2d/src/aabb \
	UnpackedTarball/box2d/src/arena_allocator \
	UnpackedTarball/box2d/src/body \
	UnpackedTarball/box2d/src/dynamic_tree \
	UnpackedTarball/box2d/src/island \
	UnpackedTarball/box2d/src/motor_joint \
	UnpackedTarball/box2d/src/mover_joint \
	UnpackedTarball/box2d/src/table \
	UnpackedTarball/box2d/src/wheel_joint \
	UnpackedTarball/box2d/src/broad_phase \
	UnpackedTarball/box2d/src/core \
	UnpackedTarball/box2d/src/geometry \
	UnpackedTarball/box2d/src/joint \
	UnpackedTarball/box2d/src/shape \
	UnpackedTarball/box2d/src/timer \
	UnpackedTarball/box2d/src/constraint_graph \
	UnpackedTarball/box2d/src/distance \
	UnpackedTarball/box2d/src/hull \
	UnpackedTarball/box2d/src/manifold \
	UnpackedTarball/box2d/src/parallel_for \
	UnpackedTarball/box2d/src/physics_world \
	UnpackedTarball/box2d/src/pogo_joint \
	UnpackedTarball/box2d/src/prismatic_joint \
	UnpackedTarball/box2d/src/types \
	UnpackedTarball/box2d/src/bitset \
	UnpackedTarball/box2d/src/contact \
	UnpackedTarball/box2d/src/contact_solver \
	UnpackedTarball/box2d/src/contact_solver_w4 \
	UnpackedTarball/box2d/src/distance_joint \
	UnpackedTarball/box2d/src/id_pool \
	UnpackedTarball/box2d/src/math_functions \
	UnpackedTarball/box2d/src/revolute_joint \
	UnpackedTarball/box2d/src/recording \
	UnpackedTarball/box2d/src/snapshot \
	UnpackedTarball/box2d/src/scheduler \
	UnpackedTarball/box2d/src/sensor \
	UnpackedTarball/box2d/src/simd \
	UnpackedTarball/box2d/src/solver \
	UnpackedTarball/box2d/src/solver_set \
	UnpackedTarball/box2d/src/weld_joint \
))

# vim: set noet sw=4 ts=4:
