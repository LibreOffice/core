# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ucppkg1))

$(eval $(call gb_Library_set_componentfile,ucppkg1,ucb/source/ucp/package/ucppkg1,services))

$(eval $(call gb_Library_use_external,ucppkg1,boost_headers))

$(eval $(call gb_Library_use_sdk_api,ucppkg1))

$(eval $(call gb_Library_use_libraries,ucppkg1,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	ucbhelper \
))

$(eval $(call gb_Library_add_exception_objects,ucppkg1,\
	ucb/source/ucp/package/pkgcontentcaps \
	ucb/source/ucp/package/pkgcontent \
	ucb/source/ucp/package/pkgdatasupplier \
	ucb/source/ucp/package/pkgprovider \
	ucb/source/ucp/package/pkgresultset \
	ucb/source/ucp/package/pkguri \
))

# vim: set noet sw=4 ts=4:
