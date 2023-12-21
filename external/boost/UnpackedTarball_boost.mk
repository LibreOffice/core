# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

boost_patches :=

#https://svn.boost.org/trac/boost/ticket/6142
boost_patches += boost.6142.warnings.patch.1

boost_patches += boost.noiconv.patch
boost_patches += boost.between.warning.patch
boost_patches += boost.fallback.encoding.patch

boost_patches += rtti.patch.0

# https://svn.boost.org/trac/boost/ticket/11505
boost_patches += boost_1_59_0.mpl.config.wundef.patch
# https://svn.boost.org/trac/boost/ticket/11501
boost_patches += boost_1_59_0.property_tree.wreturn-type.patch

boost_patches += clang-cl.patch.0

boost_patches += boost_1_63_0.undef.warning.patch.1
boost_patches += windows-no-utf8-locales.patch.0

boost_patches += msvc2017.patch.0

boost_patches += boost-ios.patch.0

boost_patches += boost.file_iterator.sharing_win.patch

boost_patches += Wundef.patch.0

$(eval $(call gb_UnpackedTarball_UnpackedTarball,boost))

$(eval $(call gb_UnpackedTarball_set_tarball,boost,$(BOOST_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,boost,3))

$(eval $(call gb_UnpackedTarball_add_patches,boost,\
	$(foreach patch,$(boost_patches),external/boost/$(patch)) \
    external/boost/boost-emscripten-noshm.patch.0 \
    external/boost/boost-emscripten-nowasm.patch.0 \
))

# vim: set noet sw=4 ts=4:
