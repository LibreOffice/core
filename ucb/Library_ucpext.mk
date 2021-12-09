# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ucpext))

$(eval $(call gb_Library_set_componentfile,ucpext,ucb/source/ucp/ext/ucpext,services))

$(eval $(call gb_Library_use_external,ucpext,boost_headers))

$(eval $(call gb_Library_use_sdk_api,ucpext))

$(eval $(call gb_Library_use_libraries,ucpext,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	tl \
	ucbhelper \
))

$(eval $(call gb_Library_add_exception_objects,ucpext,\
	ucb/source/ucp/ext/ucpext_content \
	ucb/source/ucp/ext/ucpext_datasupplier \
	ucb/source/ucp/ext/ucpext_provider \
	ucb/source/ucp/ext/ucpext_resultset \
))

# vim: set noet sw=4 ts=4:
