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
	external/curl/0001-const-up-readonly-H2_NON_FIELD.patch.1 \
	external/curl/0001-cookie-don-t-treat-the-leading-slash-as-trailing.patch \
	external/curl/0001-ws-get-a-new-mask-for-each-new-outgoing-frame.patch \
	external/curl/0001-url-fix-reuse-of-connections-using-HTTP-Negotiate.patch \
	external/curl/0001-url-fix-copy-and-paste-url_match_auth_nego-mistake.patch \
	external/curl/0001-http-only-send-bearer-if-auth-is-allowed.patch \
	external/curl/0001-proxy-auth-additional-tests.patch \
))

ifeq ($(ENABLE_EMSCRIPTEN_PROXY_POSIX_SOCKETS),TRUE)
$(eval $(call gb_UnpackedTarball_add_patches,curl, \
    external/curl/emscripten-proxy-poll.patch.0 \
))
endif

# vim: set noet sw=4 ts=4:
