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
	Package_dejavu \
	Package_gentium \
	Package_liberation \
	Package_liberation_narrow \
	Package_libertineg \
	UnpackedTarball_dejavu \
	UnpackedTarball_gentium \
	UnpackedTarball_liberation \
	UnpackedTarball_liberation_narrow \
	UnpackedTarball_libertineg \
))

# vim: set noet sw=4 ts=4:
