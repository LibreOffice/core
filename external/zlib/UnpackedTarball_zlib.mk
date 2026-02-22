# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,zlib))

$(eval $(call gb_UnpackedTarball_set_tarball,zlib,$(ZLIB_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_post_action,zlib,\
	mkdir -p x64 && \
	cp $(addsuffix .c,adler32 compress crc32 deflate inffast inflate inftrees trees zutil) x64 \
))

$(eval $(call gb_UnpackedTarball_set_patchlevel,zlib,0))

$(eval $(call gb_UnpackedTarball_add_patches,zlib, \
    external/zlib/missinginclude.patch \
    external/zlib/werror-undef.patch \
))

# cannot use post_action since $(file ..) would be run when the recipe is parsed, i.e. would always
# happen before the tarball is unpacked
$(gb_UnpackedTarball_workdir)/zlib/zlib-uninstalled.pc: $(call gb_UnpackedTarball_get_target,zlib)
	$(file >$@,$(call gb_pkgconfig_file,zlib,1.3.1,$(ZLIB_CFLAGS),$(ZLIB_LIBS)))

# vim: set noet sw=4 ts=4:
