# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_tpllayoutimpr,$(SRCDIR)/extras/source/templates/layout))

$(eval $(call gb_Package_set_outdir,extras_tpllayoutimpr,$(INSTDIR)))

$(eval $(call gb_Package_add_files,extras_tpllayoutimpr,share/template/common/layout,\
	AbstractGreen.otp \
	AbstractRed.otp \
	AbstractYellow.otp \
	BrightBlue.otp \
	DNA.otp \
	Inspiration.otp \
	LushGreen.otp \
	Metropolis.otp \
	Sunset.otp \
	Vintage.otp \
))

# vim: set noet sw=4 ts=4:
