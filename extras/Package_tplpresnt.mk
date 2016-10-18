# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_tplpresnt,$(SRCDIR)/extras/source/templates/presnt))

$(eval $(call gb_Package_add_files,extras_tplpresnt,$(LIBO_SHARE_FOLDER)/template/common/presnt,\
	BrightBlue.otp \
	DNA.otp \
	Inspiration.otp \
	LushGreen.otp \
	Metropolis.otp \
	Sunset.otp \
	Vintage.otp \
	Alizarin.otp \
	Focus.otp \
	Impress.otp \
	Midnightblue.otp \
	Vivid.otp \
	Pencil.otp \
))

# vim: set noet sw=4 ts=4:
