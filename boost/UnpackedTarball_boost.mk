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
#https://svn.boost.org/trac/boost/ticket/4713
boost_patches += boost.4713.warnings.patch
#http://gcc.gnu.org/bugzilla/show_bug.cgi?id=47679
boost_patches += boost.gcc47679.patch
#https://svn.boost.org/trac/boost/ticket/6369
boost_patches += boost.6369.warnings.patch
#https://svn.boost.org/trac/boost/ticket/6397
boost_patches += boost.6397.warnings.patch
#https://svn.boost.org/trac/boost/ticket/7551
boost_patches += boost.7551.unusedvars.patch
boost_patches += boost.4100.warnings.patch
boost_patches += boost.4510.warnings.patch
#https://svn.boost.org/trac/boost/ticket/6142
boost_patches += boost.6142.warnings.patch.1
boost_patches += boost.libcdr.warnings.patch.1

# Help static analysis tools (see SAL_UNUSED_PARAMETER in sal/types.h):
ifeq (GCC,$(COM))
boost_patches += boost_1_44_0-unused-parameters.patch
endif

# Clang warnings:
boost_patches += boost_1_44_0-clang-warnings.patch

boost_patches += boost_1_44_0-gcc4.8.patch

boost_patches += boost.auto_link.patch
boost_patches += boost.endian.patch
boost_patches += boost.loplugin.patch
boost_patches += boost.std.move.patch
boost_patches += boost.transform_width.patch
boost_patches += boost.wundef.patch
boost_patches += boost.wunused.patch
boost_patches += boost.wshadow.patch
boost_patches += boost.wdeprecated-register.patch.0
boost_patches += boost.wuninitialized.patch

$(eval $(call gb_UnpackedTarball_UnpackedTarball,boost))

$(eval $(call gb_UnpackedTarball_set_tarball,boost,$(BOOST_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,boost,3))

$(eval $(call gb_UnpackedTarball_add_patches,boost,\
	$(foreach patch,$(boost_patches),boost/$(patch)) \
))

# vim: set noet sw=4 ts=4:
