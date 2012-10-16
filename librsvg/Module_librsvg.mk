# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,librsvg))

ifeq ($(SYSTEM_LIBRSVG),NO)
$(eval $(call gb_Module_add_targets,librsvg,\
	UnpackedTarball_rsvg \
))
ifeq ($(OS),MACOSX)
$(eval $(call gb_Module_add_targets,librsvg,\
	ExternalPackage_librsvg \
	ExternalProject_librsvg \
))
else ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,librsvg,\
	Library_librsvg-2-2 \
))
endif
endif

# vim: set noet sw=4 ts=4:
