# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,qjs))

$(eval $(call gb_Library_add_defs,qjs,-DBUILDING_QJS_SHARED))

$(eval $(call gb_Library_add_generated_cobjects,qjs, \
    UnpackedTarball/quickjs/cutils \
    UnpackedTarball/quickjs/dtoa \
    UnpackedTarball/quickjs/libregexp \
    UnpackedTarball/quickjs/libunicode \
    UnpackedTarball/quickjs/quickjs \
))

$(eval $(call gb_Library_set_warnings_disabled,qjs))

$(eval $(call gb_Library_use_unpacked,qjs,quickjs))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,qjs, \
    -lm \
))
endif

# At least VS 2022 defaults to a pre-C11 that doesn't support <stdatomic.h>:
ifeq ($(COM),MSC)
$(eval $(call gb_Library_add_cflags,qjs,/std:c11))
endif

# vim: set noet sw=4 ts=4:
