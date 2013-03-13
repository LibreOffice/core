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
	liblangtag/liblangtag-0.4.0-xmlDocCopyNode.patch \
))
    # liblangtag-0.4.0-xmlDocCopyNode.patch addressed upstream as
    # <https://github.com/tagoh/liblangtag/pull/8> "Use xmlDocCopyNode to merge
    #  nodes into different docs"

ifeq ($(OS),MACOSX)
$(eval $(call gb_UnpackedTarball_add_patches,langtag,\
	liblangtag/liblangtag-0.4.0-mac.patch \
	liblangtag/liblangtag-0.4.0-configure-atomic-cflag-pollution.patch \
))
else
$(eval $(call gb_UnpackedTarball_add_patches,langtag,\
	liblangtag/liblangtag-0.4.0-msvc-warning.patch \
	liblangtag/liblangtag-0.4.0-configure-atomic-cflag-pollution.patch \
	liblangtag/liblangtag-0.4.0-windows.patch \
	liblangtag/liblangtag-0.4.0-reg2xml-encoding-problem.patch \
	liblangtag/liblangtag-0.4.0-windows2.patch \
	liblangtag/liblangtag-0.4.0-msvcprojects.patch \
	liblangtag/liblangtag-0.4.0-ppc.patch \
	liblangtag/liblangtag-0.4.0-vsnprintf.patch \
))
endif

ifeq ($(CROSS_COMPILING),YES)
$(eval $(call gb_UnpackedTarball_add_patches,langtag,\
	liblangtag/liblangtag-0.4.0-cross.patch \
))
endif

ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
$(eval $(call gb_UnpackedTarball_add_patches,langtag,\
	liblangtag/liblangtag-0.4.0-mingw.patch \
))
else

# liblangtag-0.4.0-ar-lib.patch and ar-lib are addressed upstream as
# <https://bitbucket.org/tagoh/liblangtag/issue/1>

$(eval $(call gb_UnpackedTarball_add_patches,langtag,\
	liblangtag/liblangtag-0.4.0-msc-configure.patch \
	liblangtag/liblangtag-0.4.0-cross.patch \
	liblangtag/liblangtag-0.4.0-ar-lib.patch \
))
$(eval $(call gb_UnpackedTarball_add_files,langtag,,\
	liblangtag/ar-lib \
))
endif
endif
# vim: set noet sw=4 ts=4:
