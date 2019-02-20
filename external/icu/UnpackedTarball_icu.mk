# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,icu))

$(eval $(call gb_UnpackedTarball_set_tarball,icu,$(ICU_TARBALL)))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,icu,source))

# Data zip contains data/... and needs to end up in icu/source/data/...
# Only data/misc/icudata.rc is needed for a Cygwin/MSVC build.
$(eval $(call gb_UnpackedTarball_set_pre_action,icu,\
	unzip -q -d source -o $(gb_UnpackedTarget_TARFILE_LOCATION)/$(ICU_DATA_TARBALL) data/misc/icudata.rc \
))

$(eval $(call gb_UnpackedTarball_set_patchlevel,icu,0))

$(eval $(call gb_UnpackedTarball_add_patches,icu,\
	external/icu/icu4c-build.patch.1 \
	external/icu/icu4c-aix.patch.1 \
	external/icu/icu4c-warnings.patch.1 \
	external/icu/icu4c-macosx.patch.1 \
	external/icu/icu4c-solarisgcc.patch.1 \
	external/icu/icu4c-mkdir.patch.1 \
	external/icu/icu4c-$(if $(filter ANDROID,$(OS)),android,rpath).patch.1 \
	external/icu/icu4c-ubsan.patch.1 \
	external/icu/icu4c-scriptrun.patch.1 \
	external/icu/icu4c-rtti.patch.1 \
	external/icu/icu4c-clang-cl.patch.1 \
	$(if $(filter-out ANDROID,$(OS)),external/icu/icu4c-icudata-stdlibs.patch.1) \
	external/icu/icu4c-khmerbreakengine.patch.1 \
	external/icu/icu4c-61-werror-shadow.patch.1 \
	external/icu/gcc9.patch \
	external/icu/char8_t.patch \
	external/icu/CVE-2018-18928.patch.2 \
))

$(eval $(call gb_UnpackedTarball_add_file,icu,source/data/brkitr/khmerdict.dict,external/icu/khmerdict.dict))

# vim: set noet sw=4 ts=4:
