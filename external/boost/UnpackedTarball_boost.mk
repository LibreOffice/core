# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

boost_patches :=
#https://svn.boost.org/trac/boost/ticket/3780
boost_patches += boost.3780.aliasing.patch
#https://svn.boost.org/trac/boost/ticket/9397
boost_patches += boost.9397.warnings.patch.0
#http://gcc.gnu.org/bugzilla/show_bug.cgi?id=47679
boost_patches += boost.gcc47679.patch
#https://svn.boost.org/trac/boost/ticket/6369
boost_patches += boost.6369.warnings.patch
#https://svn.boost.org/trac/boost/ticket/7551
boost_patches += boost.7551.unusedvars.patch
#https://svn.boost.org/trac/boost/ticket/6142
boost_patches += boost.6142.warnings.patch.1

boost_patches += boost.bind.Wunused-local-typedefs.warnings.patch
boost_patches += boost.concept_check.C4100.warnings.patch
boost_patches += boost.concept_check.Wunused-local-typedefs.warnings.patch
boost_patches += boost.crc.Wshadow.warnings.patch
boost_patches += boost.date_time.Wshadow.warnings.patch
boost_patches += boost.date_time.Wunused-local-typedefs.warnings.patch
boost_patches += boost.math.Wdeprecated-register.patch.0
boost_patches += boost.math.Wunused-local-typedefs.warnings.patch
boost_patches += boost.multi_array.C4510.warnings.patch
boost_patches += boost.multi_array.Wshadow.warnings.patch
boost_patches += boost.multi_array.Wundef.warnings.patch
boost_patches += boost.multi_array.Wunused-local-typedefs.warnings.patch
boost_patches += boost.multi_index.Wshadow.warnings.patch.1
boost_patches += boost.predef.Wundef.warnings.patch
boost_patches += boost.preprocessor.Wundef.warnings.patch
boost_patches += boost.property_tree.Wshadow.warnings.patch.1
boost_patches += boost.property_tree.Wtautological-constant-out-of-range-compare.warnings.patch.0
boost_patches += boost.ptr_container.Wshadow.warnings.patch
boost_patches += boost.ptr_container.Wignored-qualifiers.warnings.patch
boost_patches += boost.ptr_container.Wextra.warnings.patch
boost_patches += boost.ptr_container.Wunused-parameter.warnings.patch
boost_patches += boost.random.Wshadow.warnings.patch
boost_patches += boost.random.Wuninitialized.patch
boost_patches += boost.random.Wunused-local-typedefs.warnings.patch
boost_patches += boost.spirit.Wshadow.warnings.patch
boost_patches += boost.spirit.Wunused-local-typedefs.warnings.patch
boost_patches += boost.unordered.Wshadow.warnings.patch
boost_patches += boost.tuple.Wunused-local-typedefs.warnings.patch
boost_patches += boost.utility.Wundef.warnings.patch
boost_patches += boost.uuid.Wshadow.warnings.patch

# Help static analysis tools (see SAL_UNUSED_PARAMETER in sal/types.h):
ifeq (GCC,$(COM))
boost_patches += boost_1_44_0-unused-parameters.patch
endif

# Clang warnings:
boost_patches += boost_1_44_0-clang-warnings.patch

boost_patches += boost.auto_link.patch
boost_patches += boost.endian.patch
boost_patches += boost.loplugin.patch
boost_patches += boost.std.move.patch
boost_patches += boost.transform_width.patch
boost_patches += boost.wunused.patch
boost_patches += boost.wdeprecated-auto_ptr.patch.0

$(eval $(call gb_UnpackedTarball_UnpackedTarball,boost))

$(eval $(call gb_UnpackedTarball_set_tarball,boost,$(BOOST_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,boost,3))

$(eval $(call gb_UnpackedTarball_add_patches,boost,\
	$(foreach patch,$(boost_patches),external/boost/$(patch)) \
))

# vim: set noet sw=4 ts=4:
