# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ucpslack))

$(eval $(call gb_Library_use_sdk_api,ucpslack))

$(eval $(call gb_Library_use_custom_headers,ucpslack,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_libraries,ucpslack,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	ucbhelper \
	tl \
))

$(eval $(call gb_Library_use_externals,ucpslack,\
	boost_headers \
	curl \
	openssl \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_use_system_darwin_frameworks,ucpslack,\
	Security \
	CoreFoundation \
))
# Add system OpenSSL for HTTPS server
$(eval $(call gb_Library_add_libs,ucpslack,\
	-L/opt/homebrew/lib \
	-lssl \
	-lcrypto \
))
$(eval $(call gb_Library_add_cxxflags,ucpslack,\
	-I/opt/homebrew/include \
))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,ucpslack,\
	secur32 \
	crypt32 \
	ws2_32 \
))
endif

$(eval $(call gb_Library_add_exception_objects,ucpslack,\
	ucb/source/ucp/slack/SlackApiClient \
	ucb/source/ucp/slack/slack_json \
	ucb/source/ucp/slack/slack_oauth2_server \
	ucb/source/ucp/slack/native_https/NativeHTTPSServer \
	ucb/source/ucp/slack/native_https/ssl_securetransport \
	ucb/source/ucp/slack/native_https/ssl_schannel \
	ucb/source/ucp/slack/native_https/ssl_openssl \
))

# vim: set noet sw=4 ts=4:
