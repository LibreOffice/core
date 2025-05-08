# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,libeot))

$(eval $(call gb_StaticLibrary_use_unpacked,libeot,libeot))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,libeot))

$(eval $(call gb_StaticLibrary_set_include,libeot, \
    -I$(gb_UnpackedTarball_workdir)/libeot/inc \
    -I$(gb_UnpackedTarball_workdir)/libeot/src \
    -I$(gb_UnpackedTarball_workdir)/libeot/src/ctf \
    -I$(gb_UnpackedTarball_workdir)/libeot/src/lzcomp \
    -I$(gb_UnpackedTarball_workdir)/libeot/src/util \
    $$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,libeot, \
	-DDECOMPRESS_ON \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,libeot,\
	UnpackedTarball/libeot/src/libeot \
	UnpackedTarball/libeot/src/EOT \
	UnpackedTarball/libeot/src/writeFontFile \
	UnpackedTarball/libeot/src/triplet_encodings \
	UnpackedTarball/libeot/src/ctf/parseCTF \
	UnpackedTarball/libeot/src/ctf/parseTTF \
	UnpackedTarball/libeot/src/ctf/SFNTContainer \
	UnpackedTarball/libeot/src/util/stream \
	UnpackedTarball/libeot/src/lzcomp/ahuff \
	UnpackedTarball/libeot/src/lzcomp/bitio \
	UnpackedTarball/libeot/src/lzcomp/liblzcomp \
	UnpackedTarball/libeot/src/lzcomp/lzcomp \
	UnpackedTarball/libeot/src/lzcomp/mtxmem \
))



# vim: set noet sw=4 ts=4:
