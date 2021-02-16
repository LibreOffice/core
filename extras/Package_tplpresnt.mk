# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_tplpresnt,$(call gb_CustomTarget_get_workdir,extras/source/templates/presnt)))

$(eval $(call gb_Package_add_files,extras_tplpresnt,$(LIBO_SHARE_FOLDER)/template/common/presnt,\
	Candy.otp \
	DNA.otp \
	Inspiration.otp \
	Metropolis.otp \
	Vintage.otp \
	Focus.otp \
	Midnightblue.otp \
	Vivid.otp \
	Pencil.otp \
	Blueprint_Plans.otp \
	Portfolio.otp \
	Progress.otp \
	Beehive.otp \
	Blue_Curve.otp \
	Forestbird.otp \
	Lights.otp \
	Nature_Illustration.otp \
	Piano.otp \
	Sunset.otp \
))

# vim: set noet sw=4 ts=4:
