# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ucpgio1))

$(eval $(call gb_Library_set_componentfile,ucpgio1,ucb/source/ucp/gio/ucpgio,services))

$(eval $(call gb_Library_use_external,ucpgio1,boost_headers))

$(eval $(call gb_Library_use_sdk_api,ucpgio1))

$(eval $(call gb_Library_use_libraries,ucpgio1,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	ucbhelper \
	vcl \
))

$(eval $(call gb_Library_use_externals,ucpgio1,\
	gio \
))

$(eval $(call gb_Library_add_exception_objects,ucpgio1,\
	ucb/source/ucp/gio/gio_content \
	ucb/source/ucp/gio/gio_datasupplier \
	ucb/source/ucp/gio/gio_inputstream \
	ucb/source/ucp/gio/gio_mount \
	ucb/source/ucp/gio/gio_outputstream \
	ucb/source/ucp/gio/gio_provider \
	ucb/source/ucp/gio/gio_resultset \
	ucb/source/ucp/gio/gio_seekable \
))

# vim: set noet sw=4 ts=4:
