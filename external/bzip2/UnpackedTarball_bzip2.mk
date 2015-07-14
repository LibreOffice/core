# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,bzip2))

$(eval $(call gb_UnpackedTarball_set_tarball,bzip2,$(BZIP2_TARBALL)))

$(eval $(call gb_UnpackedTarball_fix_end_of_line,bzip2,\
	blocksort.c \
	bzlib.c \
	compress.c \
	crctable.c \
	decompress.c \
	huffman.c \
	randtable.c \
))

# vim: set noet sw=4 ts=4:
