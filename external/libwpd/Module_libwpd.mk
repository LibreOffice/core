# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,libwpd))

$(eval $(call gb_Module_add_targets,libwpd,\
	UnpackedTarball_libwpd \
))

ifeq ($(COM),MSC)

$(eval $(call gb_Module_add_targets,libwpd,\
	Library_wpd \
))

else

$(eval $(call gb_Module_add_targets,libwpd,\
	ExternalPackage_libwpd \
	ExternalProject_libwpd \
))

endif

# vim: set noet sw=4 ts=4:
