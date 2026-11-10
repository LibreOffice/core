# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,coinmp))

$(eval $(call gb_UnpackedTarball_set_tarball,coinmp,$(COINOR_CBC_TARBALL)))

# Cbc's top-level configure builds each COIN-OR project whose directory sits beside Cbc's. The
# Visual Studio projects expect each project one level deeper, as in CoinUtils/CoinUtils/src.
$(eval $(call gb_UnpackedTarball_set_pre_action,coinmp,\
	$(foreach tarball,$(COINOR_COINUTILS_TARBALL) $(COINOR_OSI_TARBALL) $(COINOR_CLP_TARBALL) $(COINOR_CGL_TARBALL),\
		$(GNUTAR) -x -z --no-same-owner --strip-components=1 \
			-f $(call gb_Helper_cyg_path,$(gb_UnpackedTarget_TARFILE_LOCATION)/$(tarball)) \
			$(patsubst %.tar.gz,%,$(tarball))/$(firstword $(subst -, ,$(tarball))) && ) \
	$(if $(filter MSC,$(COM)),\
		find . -path '*/MSVisualStudio/v17/*' \
			\( -name '*.sln' -o -name '*.vcxproj' -o -name '*.vcxproj.filters' \) \
			-exec sed -i -E -f $(call gb_Helper_cyg_path,$(SRCDIR)/external/coinmp/vs-project-paths.sed) {} + &&) \
	true \
))

$(eval $(call gb_UnpackedTarball_set_patchlevel,coinmp,0))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,coinmp))
$(eval $(call gb_UnpackedTarball_update_autoconf_configs,coinmp,\
	Cbc \
	Cgl \
	Clp \
	CoinUtils \
	Osi \
))

ifneq ($(MSYSTEM),)
$(eval $(call gb_UnpackedTarball_set_patchflags,coinmp,--binary))
endif

$(eval $(call gb_UnpackedTarball_add_patches,coinmp,\
	external/coinmp/no-binaries.patch.1 \
	external/coinmp/werror-undef.patch.0 \
	external/coinmp/deprecated-copy.patch.0 \
	external/coinmp/implicit-fallthrough.patch.0 \
	external/coinmp/ubsan.patch.0 \
	external/coinmp/rpath.patch \
	external/coinmp/libtool.patch \
	external/coinmp/Wnon-c-typedef-for-linkage.patch \
	external/coinmp/odr.patch \
	external/coinmp/const.patch.1 \
	external/coinmp/const2.patch.1 \
	external/coinmp/const3.patch.1 \
))

# vim: set noet sw=4 ts=4:
