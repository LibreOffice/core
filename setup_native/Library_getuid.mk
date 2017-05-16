# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,getuid))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_defs,getuid,\
    -D_GNU_SOURCE \
))
endif

$(call gb_LinkTarget_get_target,$(call gb_Library_get_linktarget,getuid)) : gb_CC := $(filter-out -fsanitize%,$(gb_CC))

# the library is used by LD_PRELOAD; make sure that we see the symbols ;-)
ifeq ($(COM),GCC)
$(eval $(call gb_Library_add_cflags,getuid,\
    -fvisibility=default \
))
endif

$(eval $(call gb_Library_add_cobjects,getuid,\
    setup_native/scripts/source/getuid \
))

$(eval $(call gb_Library_add_libs,getuid,\
	-ldl \
))

# vim: set noet sw=4 ts=4:
