# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

boost_patches :=

#http://gcc.gnu.org/bugzilla/show_bug.cgi?id=47679
boost_patches += boost.gcc47679.patch

#https://svn.boost.org/trac/boost/ticket/6142
boost_patches += boost.6142.warnings.patch.1

# https://svn.boost.org/trac/boost/ticket/9885
boost_patches += boost.multi_array.C4510.warnings.patch
# https://svn.boost.org/trac/boost/ticket/9903
boost_patches += boost.utility.Wundef.warnings.patch

boost_patches += boost.type_index.Wundef.warnings.patch

boost_patches += boost.auto_link.patch

boost_patches += ubsan.patch.0
boost_patches += rtti.patch.0

# https://svn.boost.org/trac/boost/ticket/11505
boost_patches += boost_1_59_0.mpl.config.wundef.patch
# https://svn.boost.org/trac/boost/ticket/11501
boost_patches += boost_1_59_0.property_tree.wreturn-type.patch

boost_patches += clang-cl.patch.0

boost_patches += boost_1_60_0.undef.warning.patch
boost_patches += boost_1_63_0.undef.warning.patch.1

$(eval $(call gb_UnpackedTarball_UnpackedTarball,boost))

$(eval $(call gb_UnpackedTarball_set_tarball,boost,$(BOOST_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,boost,3))

$(eval $(call gb_UnpackedTarball_add_patches,boost,\
	$(foreach patch,$(boost_patches),external/boost/$(patch)) \
))

# vim: set noet sw=4 ts=4:
