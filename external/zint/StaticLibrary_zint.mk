# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,zint))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,zint))

$(eval $(call gb_StaticLibrary_use_unpacked,zint,zint))

# in sections: common, onedim, postal, twodim
$(eval $(call gb_StaticLibrary_add_generated_cobjects,zint,\
	\
	UnpackedTarball/zint/backend/common \
	UnpackedTarball/zint/backend/library \
	UnpackedTarball/zint/backend/render \
	UnpackedTarball/zint/backend/large \
	UnpackedTarball/zint/backend/reedsol \
	UnpackedTarball/zint/backend/gs1 \
	\
	UnpackedTarball/zint/backend/code \
	UnpackedTarball/zint/backend/code128 \
	UnpackedTarball/zint/backend/2of5 \
	UnpackedTarball/zint/backend/upcean \
	UnpackedTarball/zint/backend/telepen \
	UnpackedTarball/zint/backend/medical \
	UnpackedTarball/zint/backend/plessey \
	UnpackedTarball/zint/backend/rss \
	\
	UnpackedTarball/zint/backend/postal \
	UnpackedTarball/zint/backend/auspost \
	UnpackedTarball/zint/backend/imail \
	\
	UnpackedTarball/zint/backend/code16k \
	UnpackedTarball/zint/backend/dmatrix \
	UnpackedTarball/zint/backend/pdf417 \
	UnpackedTarball/zint/backend/qr \
	UnpackedTarball/zint/backend/maxicode \
	UnpackedTarball/zint/backend/composite \
	UnpackedTarball/zint/backend/aztec \
	UnpackedTarball/zint/backend/code49 \
	UnpackedTarball/zint/backend/code1 \
	UnpackedTarball/zint/backend/gridmtx \
))

# vim: set noet sw=4 ts=4:
