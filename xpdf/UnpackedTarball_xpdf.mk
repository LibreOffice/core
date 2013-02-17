# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,xpdf))

$(eval $(call gb_UnpackedTarball_set_tarball,xpdf,$(XPDF_TARBALL)))

$(eval $(call gb_UnpackedTarball_fix_end_of_line,xpdf,\
	ms_make.bat \
))

$(eval $(call gb_UnpackedTarball_add_patches,xpdf,\
	xpdf/xpdf-3.02.patch \
	xpdf/xpdf-3.02-ooopwd.patch \
	xpdf/xpdf-3.02-gfxColorMaxComps.patch \
	xpdf/xpdf-no-writable-literals.patch \
))

# vim: set noet sw=4 ts=4:
