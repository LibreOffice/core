# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libjxl))

$(eval $(call gb_UnpackedTarball_set_tarball,libjxl,$(LIBJXL_TARBALL)))

# Exclude tools/benchmark/metrics subdirectory containing symlinks that tar
# in the Windows build cannot handle
$(eval $(call gb_UnpackedTarball_set_exclude_pattern,libjxl,\
    */tools/benchmark/metrics \
))

$(eval $(call gb_UnpackedTarball_add_files,libjxl,lib/include/jxl,\
    external/libjxl/jxl_cms_export.h \
    external/libjxl/jxl_export.h \
    external/libjxl/jxl_threads_export.h \
    external/libjxl/version.h \
))

ifeq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_patches,libjxl,\
	external/libjxl/windows-compile-directives.patch.0 \
))
endif

# vim: set noet sw=4 ts=4:
