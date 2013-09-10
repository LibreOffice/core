# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,afms_fontunxafm,afms))

$(eval $(call gb_ExternalPackage_set_outdir,afms_fontunxafm,$(gb_INSTROOT)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,afms_fontunxafm,$(LIBO_SHARE_FOLDER)/psprint/fontmetric,\
	Courier-Bold.afm \
	Courier-BoldOblique.afm \
	Courier-Oblique.afm \
	Courier.afm \
	Helvetica-Bold.afm \
	Helvetica-BoldOblique.afm \
	Helvetica-Oblique.afm \
	Helvetica.afm \
	ITCAvantGarde-Book.afm \
	ITCAvantGarde-BookOblique.afm \
	ITCAvantGarde-Demi.afm \
	ITCAvantGarde-DemiOblique.afm \
	ITCBookman-Demi.afm \
	ITCBookman-DemiItalic.afm \
	ITCBookman-Light.afm \
	ITCBookman-LightItalic.afm \
	ITCZapfChancery-MediumItalic.afm \
	NewCenturySchlbk-Bold.afm \
	NewCenturySchlbk-BoldItalic.afm \
	NewCenturySchlbk-Italic.afm \
	NewCenturySchlbk-Roman.afm \
	Palatino-Bold.afm \
	Palatino-BoldItalic.afm \
	Palatino-Italic.afm \
	Palatino-Roman.afm \
	Symbol.afm \
	Times-Bold.afm \
	Times-BoldItalic.afm \
	Times-Italic.afm \
	Times-Roman.afm \
	ZapfDingbats.afm \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
