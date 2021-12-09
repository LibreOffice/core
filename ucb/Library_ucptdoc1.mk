# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ucptdoc1))

$(eval $(call gb_Library_set_componentfile,ucptdoc1,ucb/source/ucp/tdoc/ucptdoc1,services))

$(eval $(call gb_Library_use_sdk_api,ucptdoc1))

$(eval $(call gb_Library_use_libraries,ucptdoc1,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	tl \
	ucbhelper \
))

$(eval $(call gb_Library_add_exception_objects,ucptdoc1,\
	ucb/source/ucp/tdoc/tdoc_contentcaps \
	ucb/source/ucp/tdoc/tdoc_content \
	ucb/source/ucp/tdoc/tdoc_datasupplier \
	ucb/source/ucp/tdoc/tdoc_docmgr \
	ucb/source/ucp/tdoc/tdoc_documentcontentfactory \
	ucb/source/ucp/tdoc/tdoc_passwordrequest \
	ucb/source/ucp/tdoc/tdoc_provider \
	ucb/source/ucp/tdoc/tdoc_resultset \
	ucb/source/ucp/tdoc/tdoc_stgelems \
	ucb/source/ucp/tdoc/tdoc_storage \
	ucb/source/ucp/tdoc/tdoc_uri \
))

# vim: set noet sw=4 ts=4:
