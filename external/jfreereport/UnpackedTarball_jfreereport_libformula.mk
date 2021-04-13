# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,jfreereport_libformula))

$(eval $(call gb_UnpackedTarball_set_tarball,jfreereport_libformula,$(JFREEREPORT_LIBFORMULA_TARBALL),0))

$(eval $(call gb_UnpackedTarball_fix_end_of_line,jfreereport_libformula,\
	common_build.xml \
))

$(eval $(call gb_UnpackedTarball_add_patches,jfreereport_libformula,\
	external/jfreereport/patches/common_build.patch \
	external/jfreereport/patches/libformula-time-notz.patch \
	external/jfreereport/patches/libformula-seconds_rounding.patch.1 \
	external/jfreereport/patches/libformula-minutes_truncation.patch.1 \
	external/jfreereport/patches/libformula-datevalue_truncation.patch.1 \
	external/jfreereport/patches/libformula-1.1.3-remove-commons-logging.patch.1 \
))

# vim: set noet sw=4 ts=4:
