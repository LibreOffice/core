# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ucpdropbox))

$(eval $(call gb_Library_set_componentfile,ucpdropbox,ucb/source/ucp/dropbox/ucpdropbox,services))

$(eval $(call gb_Library_use_sdk_api,ucpdropbox))

$(eval $(call gb_Library_use_custom_headers,ucpdropbox,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_libraries,ucpdropbox,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	ucbhelper \
	tl \
))

$(eval $(call gb_Library_use_externals,ucpdropbox,\
	boost_headers \
	curl \
))

$(eval $(call gb_Library_add_exception_objects,ucpdropbox,\
	ucb/source/ucp/dropbox/DropboxApiClient \
	ucb/source/ucp/dropbox/dropbox_provider \
	ucb/source/ucp/dropbox/dropbox_content \
	ucb/source/ucp/dropbox/dropbox_datasupplier \
	ucb/source/ucp/dropbox/dropbox_resultset \
	ucb/source/ucp/dropbox/dropbox_json \
	ucb/source/ucp/dropbox/oauth2_http_server \
))

# vim: set noet sw=4 ts=4:
