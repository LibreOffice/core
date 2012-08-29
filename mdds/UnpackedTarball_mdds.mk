# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,mdds))

$(eval $(call gb_UnpackedTarball_set_tarball,mdds,$(MDDS_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,mdds,\
	mdds/mdds_0.6.0.patch \
	mdds/0001-help-compiler-select-the-right-overload-of-vector-in.patch \
	mdds/mdds_0.6.0-unused-parameter.patch \
	mdds/mdds_0.6.0-unreachable-code.patch \
	mdds/0001-fix-linking-error-with-boost-1.50.patch \
	mdds/0001-Workaround-for-gcc-bug.patch \
))

# vim: set noet sw=4 ts=4:
