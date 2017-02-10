# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,langtag))

$(eval $(call gb_UnpackedTarball_set_tarball,langtag,$(LIBLANGTAG_TARBALL),,liblangtag))

$(eval $(call gb_UnpackedTarball_set_pre_action,langtag,\
	$(GNUTAR) -x -j -f $(gb_UnpackedTarget_TARFILE_LOCATION)/$(LANGTAGREG_TARBALL) \
))

ifneq ($(OS),MACOSX)
ifneq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_patches,langtag,\
	external/liblangtag/liblangtag-bundled-soname.patch.0 \
))
endif
endif

$(eval $(call gb_UnpackedTarball_add_patches,langtag, \
	$(if $(SYSTEM_LIBXML),,external/liblangtag/langtag-libtool-rpath.patch.0) \
    external/liblangtag/clang-cl.patch.0 \
))

# vim: set noet sw=4 ts=4:
