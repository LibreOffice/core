# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,curl))

$(eval $(call gb_UnpackedTarball_set_tarball,curl,$(CURL_TARBALL),,curl))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,curl))

# 0001-const-up-readonly-H2_NON_FIELD.patch
# upstream effort as:
# https://github.com/curl/curl/pull/17996

$(eval $(call gb_UnpackedTarball_add_patches,curl,\
	external/curl/zlib.patch.0 \
))

ifeq ($(ENABLE_EMSCRIPTEN_PROXY_POSIX_SOCKETS),TRUE)
$(eval $(call gb_UnpackedTarball_add_patches,curl, \
    external/curl/emscripten-proxy-poll.patch.0 \
))
endif

# vim: set noet sw=4 ts=4:
