# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

epubgen_patches :=
# Backport of <https://sourceforge.net/p/libepubgen/code/ci/96e9786f5aba1078251f36e58feefc8d953cdea0/>.
epubgen_patches += libepubgen-validation1.patch.1
# Backport of <https://sourceforge.net/p/libepubgen/code/ci/2e51fb9163bbc6b9a27fa524382c5aad9700dd0d/>.
epubgen_patches += libepubgen-validation2.patch.1
# Backport of <https://sourceforge.net/p/libepubgen/code/ci/9041ef42f9e0a5c4bc3b0a912d36683c4e10ca84/>.
epubgen_patches += libepubgen-validation3.patch.1
# Backport of <https://sourceforge.net/p/libepubgen/code/ci/49f6461d4751d3b16e32ab8f9c93a3856b33be49/>.
epubgen_patches += libepubgen-vc.patch.1

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libepubgen))

$(eval $(call gb_UnpackedTarball_set_tarball,libepubgen,$(EPUBGEN_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,libepubgen,\
	$(foreach patch,$(epubgen_patches),external/libepubgen/$(patch)) \
))

# vim: set noet sw=4 ts=4:
