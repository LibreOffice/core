# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,langtag))

$(eval $(call gb_UnpackedTarball_set_tarball,langtag,$(LIBLANGTAG_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,langtag,\
	liblangtag/liblangtag-0.5.0-msvc-warning.patch \
	liblangtag/liblangtag-0.5.0-reg2xml-encoding-problem.patch \
	liblangtag/liblangtag-0.5.0-vsnprintf.patch \
))

ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
$(eval $(call gb_UnpackedTarball_add_patches,langtag,\
	liblangtag/liblangtag-0.5.0-mingw.patch \
))
endif
endif
# vim: set noet sw=4 ts=4:
