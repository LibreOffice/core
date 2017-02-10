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

ifneq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_set_post_action,clucene,\
	mkdir -p inc/internal/CLucene/util && \
	mv src/shared/CLucene/util/dirent.h inc/internal/CLucene/util \
))
endif

$(eval $(call gb_UnpackedTarball_set_patchlevel,clucene,0))

# clucene-multimap-put.patch was proposed upstream, see
#  http://sourceforge.net/mailarchive/message.php?msg_id=29143260
# clucene-mutex.patch was proposed upstream, see
#  http://sourceforge.net/mailarchive/message.php?msg_id=32314782
# clucene-asan.patch was proposed upstream, see
#  http://sourceforge.net/mailarchive/message.php?msg_id=32367781
# see msvc14.0 error report on:
# https://connect.microsoft.com/VisualStudio/feedback/details/1424082/vs2015-rc-c-compiler-mixes-up-template-parameter-with-identical-named-inherited-typedef
$(eval $(call gb_UnpackedTarball_add_patches,clucene,\
	external/clucene/patches/clucene-debug.patch \
	external/clucene/patches/clucene-multimap-put.patch \
	external/clucene/patches/clucene-narrowing-conversions.patch \
	external/clucene/patches/clucene-nullptr.patch \
	external/clucene/patches/clucene-warnings.patch \
	external/clucene/patches/clucene-aix.patch \
	external/clucene/patches/clucene-git1-win64.patch \
	external/clucene/patches/clucene-ub.patch \
	external/clucene/patches/clucene-mutex.patch \
	external/clucene/patches/clucene-asan.patch \
	external/clucene/patches/clucene-mixes-uptemplate-parameter-msvc-14.patch \
))

ifneq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_patches,clucene,\
	external/clucene/patches/clucene-libcpp.patch \
))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_file,clucene,src/shared/CLucene/_clucene-config.h,external/clucene/configs/_clucene-config-MSVC.h))
$(eval $(call gb_UnpackedTarball_add_file,clucene,src/shared/CLucene/clucene-config.h,external/clucene/configs/clucene-config-MSVC.h))
else # ! $(OS),WNT
ifeq ($(HAVE_GCC_BUILTIN_ATOMIC),TRUE)
$(eval $(call gb_UnpackedTarball_add_file,clucene,src/shared/CLucene/clucene-config.h,external/clucene/configs/clucene-config-GCC-atomic.h))
else
$(eval $(call gb_UnpackedTarball_add_file,clucene,src/shared/CLucene/clucene-config.h,external/clucene/configs/clucene-config-generic.h))
endif
ifeq ($(OS),LINUX)
$(eval $(call gb_UnpackedTarball_add_file,clucene,src/shared/CLucene/_clucene-config.h,external/clucene/configs/_clucene-config-LINUX.h))
else
$(eval $(call gb_UnpackedTarball_add_file,clucene,src/shared/CLucene/_clucene-config.h,external/clucene/configs/_clucene-config-generic.h))
endif
endif # $(OS),WNT

# vim: set noet sw=4 ts=4:
