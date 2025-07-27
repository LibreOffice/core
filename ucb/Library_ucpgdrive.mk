# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ucpgdrive))

$(eval $(call gb_Library_set_componentfile,ucpgdrive,ucb/source/ucp/gdrive/ucpgdrive,services))

$(eval $(call gb_Library_use_sdk_api,ucpgdrive))

$(eval $(call gb_Library_use_custom_headers,ucpgdrive,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_libraries,ucpgdrive,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	ucbhelper \
	tl \
))

$(eval $(call gb_Library_use_externals,ucpgdrive,\
	boost_headers \
	curl \
))

$(eval $(call gb_Library_add_exception_objects,ucpgdrive,\
	ucb/source/ucp/gdrive/GoogleDriveApiClient \
	ucb/source/ucp/gdrive/gdrive_provider \
	ucb/source/ucp/gdrive/gdrive_content \
	ucb/source/ucp/gdrive/gdrive_datasupplier \
	ucb/source/ucp/gdrive/gdrive_resultset \
	ucb/source/ucp/gdrive/gdrive_json \
	ucb/source/ucp/gdrive/oauth2_http_server \
))

# vim: set noet sw=4 ts=4:
