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

$(eval $(call gb_UnpackedTarball_add_patches,langtag,\
	liblangtag/liblangtag-0.5.1-msvc-warning.patch \
	liblangtag/liblangtag-0.5.1-vsnprintf.patch \
	liblangtag/liblangtag-0.5.1-msvc-ssize_t.patch \
	liblangtag/liblangtag-0.5.1-msvc-snprintf.patch \
	liblangtag/liblangtag-0.5.1-msvc-strtoull.patch \
	liblangtag/liblangtag-0.5.1-scope-declaration.patch \
	liblangtag/liblangtag-0.5.1-redefinition-of-typedef.patch \
	liblangtag/liblangtag-0.5.1-undefined-have-sys-param-h.patch \
	liblangtag/liblangtag-0.5.1-windows-do-not-prepend-dir-separator.patch \
	liblangtag/liblangtag-0.5.1-unistd.patch \
))

ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
$(eval $(call gb_UnpackedTarball_add_patches,langtag,\
	liblangtag/liblangtag-0.5.1-mingw.patch \
))
endif
endif
# vim: set noet sw=4 ts=4:
