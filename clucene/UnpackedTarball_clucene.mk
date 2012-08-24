# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,clucene))

$(eval $(call gb_UnpackedTarball_set_tarball,clucene,$(CLUCENE_TARBALL)))

$(eval $(call gb_UnpackedTarball_fix_cxx_suffix,clucene,cpp))

ifneq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_set_post_action,clucene,\
	mkdir -p inc/internal/CLucene/util && \
	mv src/shared/CLucene/util/dirent.h inc/internal/CLucene/util \
))
endif

$(eval $(call gb_UnpackedTarball_set_patchlevel,clucene,0))

# clucene-multimap-put.patch was proposed upstream, see
# http://sourceforge.net/mailarchive/message.php?msg_id=29143260
$(eval $(call gb_UnpackedTarball_add_patches,clucene,\
	clucene/patches/clucene-debug.patch \
	clucene/patches/clucene-gcc-atomics.patch \
	clucene/patches/clucene-internal-zlib.patch \
	clucene/patches/clucene-multimap-put.patch \
	clucene/patches/clucene-narrowing-conversions.patch \
	clucene/patches/clucene-nullptr.patch \
	clucene/patches/clucene-warnings.patch \
))

ifeq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_file,clucene,src/shared/CLucene/_clucene-config.h,clucene/configs/_clucene-config-MSVC.h))
ifeq ($(COM),MSC)
$(eval $(call gb_UnpackedTarball_add_file,clucene,src/shared/CLucene/clucene-config.h,clucene/configs/clucene-config-MSVC.h))
else
$(eval $(call gb_UnpackedTarball_add_file,clucene,src/shared/CLucene/clucene-config.h,clucene/configs/clucene-config-MINGW.h))
endif
else # ! $(OS),WNT
$(eval $(call gb_UnpackedTarball_add_file,clucene,src/shared/CLucene/clucene-config.h,clucene/configs/clucene-config-generic.h))
ifeq ($(OS),LINUX)
$(eval $(call gb_UnpackedTarball_add_file,clucene,src/shared/CLucene/_clucene-config.h,clucene/configs/_clucene-config-LINUX.h))
else
$(eval $(call gb_UnpackedTarball_add_file,clucene,src/shared/CLucene/_clucene-config.h,clucene/configs/_clucene-config-generic.h))
endif
endif # $(OS),WNT

# vim: set noet sw=4 ts=4:
