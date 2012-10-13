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

$(eval $(call gb_UnpackedTarball_UnpackedTarball,jfreereport_libloader))

$(eval $(call gb_UnpackedTarball_set_tarball,jfreereport_libloader,$(JFREEREPORT_LIBLOADER_TARBALL),0))

$(eval $(call gb_UnpackedTarball_fix_end_of_line,jfreereport_libloader,\
	common_build.xml \
))

$(eval $(call gb_UnpackedTarball_add_patches,jfreereport_libloader,\
	jfreereport/patches/common_build.patch \
	jfreereport/patches/libloader-$(LIBLOADER_VERSION)-deprecated.patch \
))

# vim: set noet sw=4 ts=4:
