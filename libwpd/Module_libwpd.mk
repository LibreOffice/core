# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,libwpd))

ifeq ($(SYSTEM_LIBWPD),NO)

$(eval $(call gb_Module_add_targets,libwpd,\
	Package_libwpd \
	StaticLibrary_wpd \
	UnpackedTarball_wpd \
))

endif

# vim: set noet sw=4 ts=4:
