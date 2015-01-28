# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,udunits2))

ifeq ($(SYSTEM_UDUNITS2),)

$(eval $(call gb_Module_add_targets,udunits2,\
	ExternalPackage_udunits2 \
	ExternalProject_udunits2 \
	UnpackedTarball_udunits2 \
))

endif

# vim: set noet sw=4 ts=4:
