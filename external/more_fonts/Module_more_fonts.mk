# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,more_fonts))

$(eval $(call gb_Module_add_targets,more_fonts,\
	Package_conf \
	ExternalPackage_caladea \
	ExternalPackage_carlito \
	ExternalPackage_dejavu \
	ExternalPackage_gentium \
	ExternalPackage_karla \
	ExternalPackage_liberation \
	ExternalPackage_liberation_narrow \
	ExternalPackage_libertineg \
	ExternalPackage_opensans \
	ExternalPackage_ptserif \
	ExternalPackage_sourcecode \
	ExternalPackage_sourcesans \
	UnpackedTarball_caladea \
	UnpackedTarball_carlito \
	UnpackedTarball_dejavu \
	UnpackedTarball_gentium \
	UnpackedTarball_karla \
	UnpackedTarball_liberation \
	UnpackedTarball_liberation_narrow \
	UnpackedTarball_libertineg \
	UnpackedTarball_opensans \
	UnpackedTarball_ptserif \
	UnpackedTarball_sourcecode \
	UnpackedTarball_sourcesans \
))

# vim: set noet sw=4 ts=4:
