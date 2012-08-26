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
#https://svn.boost.org/trac/boost/ticket/4127
boost_patches += boost.4127.warnings.patch
#https://svn.boost.org/trac/boost/ticket/4713
boost_patches += boost.4713.warnings.patch
#https://svn.boost.org/trac/boost/ticket/5119
boost_patches += boost.5119.unordered_map-cp-ctor.patch
#http://gcc.gnu.org/bugzilla/show_bug.cgi?id=47679
boost_patches += boost.gcc47679.patch
#https://svn.boost.org/trac/boost/ticket/6369
boost_patches += boost.6369.warnings.patch
#https://svn.boost.org/trac/boost/ticket/6397
boost_patches += boost.6397.warnings.patch
#backport from boost 1.48.0 fix for "opcode not supported on this processor"
boost_patches += boost.mipsbackport.patch

boost_patches += boost.windows.patch
boost_patches += boost.vc2012.patch

# Help static analysis tools (see SAL_UNUSED_PARAMETER in sal/types.h):
ifeq (GCC,$(COM))
boost_patches += boost_1_44_0-unused-parameters.patch
boost_patches += boost_1_44_0-logical-op-parentheses.patch
endif

# Backporting fixes for the GCC 4.7 -std=c++11 mode from Boost 1.48.0:
boost_patches += boost_1_44_0-gcc4.7.patch

# Clang warnings:
boost_patches += boost_1_44_0-clang-warnings.patch

# Backport http://svn.boost.org/svn/boost/trunk r76133 "Fix threading detection
# in GCC-4.7 experimental":
boost_patches += boost_1_44_0-gthreads.patch

$(eval $(call gb_UnpackedTarball_UnpackedTarball,boost))

$(eval $(call gb_UnpackedTarball_set_tarball,boost,$(BOOST_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,boost,\
	$(foreach patch,$(boost_patches),boost/$(patch)) \
))

$(eval $(call gb_UnpackedTarball_copy_header_files,boost,\
	boost \
))

# vim: set noet sw=4 ts=4:
