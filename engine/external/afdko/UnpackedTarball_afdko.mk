# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,afdko))

$(eval $(call gb_UnpackedTarball_set_tarball,afdko,$(AFDKO_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_pre_action,afdko,\
    unzip -q -d a4 -o $(gb_UnpackedTarget_TARFILE_LOCATION)/$(ANTLR4CPPRUNTIME_TARBALL) \
))

$(eval $(call gb_UnpackedTarball_set_patchlevel,afdko,1))

# lookuplist-deref.patch upstream attempt as:
# https://github.com/adobe-type-tools/afdko/pull/1840
# glyphmetrics-instance.patch upstream attempt as:
# https://github.com/adobe-type-tools/afdko/pull/1841

$(eval $(call gb_UnpackedTarball_add_patches,afdko, \
    external/afdko/extern_mergefonts.patch \
    external/afdko/extern_addfeatures.patch \
    external/afdko/fatal-callback.patch \
    external/afdko/glyphmetrics-instance.patch \
    external/afdko/lookuplist-deref.patch \
    external/afdko/pastend-iterators.patch \
    external/afdko/itemvariationstore-tag.patch \
    external/afdko/sfnt-named-structs.patch \
    external/afdko/antlr4-chrono.patch \
    external/afdko/ubsan.patch.0 \
    external/afdko/classrec-fallback-ref.patch.0 \
))

# static replacement for the cmake-generated version files
$(eval $(call gb_UnpackedTarball_add_file,afdko,c/shared/include/afdko_version.h,external/afdko/afdko_version.h))
$(eval $(call gb_UnpackedTarball_add_file,afdko,c/shared/afdko_version.cpp,external/afdko/afdko_version.cpp))

# vim: set noet sw=4 ts=4:
