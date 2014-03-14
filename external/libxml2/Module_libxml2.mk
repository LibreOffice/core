# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,libxml2))

$(eval $(call gb_Module_add_targets,libxml2,\
	ExternalPackage_xml2 \
	ExternalPackage_xml2_win32 \
	ExternalProject_xml2 \
	UnpackedTarball_xml2 \
))

# vim: set noet sw=4 ts=4:
