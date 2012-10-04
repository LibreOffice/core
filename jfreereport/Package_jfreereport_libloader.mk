# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# for VERSION
include $(SRCDIR)/jfreereport/version.mk

$(eval $(call gb_Package_Package,jfreereport_libloader,$(call gb_UnpackedTarball_get_dir,jfreereport_libloader)))

$(eval $(call gb_Package_use_external_project,jfreereport_libloader,jfreereport_libloader))

$(eval $(call gb_Package_add_file,jfreereport_libloader,bin/libloader-$(LIBLOADER_VERSION).jar,dist/libloader-$(LIBLOADER_VERSION).jar))

# vim: set noet sw=4 ts=4:
