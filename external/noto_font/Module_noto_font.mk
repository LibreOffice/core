# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,noto_font))

$(eval $(call gb_Module_add_targets,noto_font,\
	ExternalPackage_noto \
	ExternalPackage_notosansjp \
	ExternalPackage_notosanskr \
	ExternalPackage_notosanssc \
	ExternalPackage_notosanstc \
	ExternalPackage_notoserifjp \
	ExternalPackage_notoserifkr \
	ExternalPackage_notoserifsc \
	ExternalPackage_notoseriftc \
	UnpackedTarball_noto \
	UnpackedTarball_notosansjp \
	UnpackedTarball_notosanskr \
	UnpackedTarball_notosanssc \
	UnpackedTarball_notosanstc \
	UnpackedTarball_notoserifjp \
	UnpackedTarball_notoserifkr \
	UnpackedTarball_notoserifsc \
	UnpackedTarball_notoseriftc \
))

# vim: set noet sw=4 ts=4:
