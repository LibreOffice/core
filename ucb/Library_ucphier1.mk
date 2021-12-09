# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ucphier1))

$(eval $(call gb_Library_set_componentfile,ucphier1,ucb/source/ucp/hierarchy/ucphier1,services))

$(eval $(call gb_Library_use_external,ucphier1,boost_headers))

$(eval $(call gb_Library_use_sdk_api,ucphier1))

$(eval $(call gb_Library_use_libraries,ucphier1,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	tl \
	ucbhelper \
))

$(eval $(call gb_Library_add_exception_objects,ucphier1,\
	ucb/source/ucp/hierarchy/dynamicresultset \
	ucb/source/ucp/hierarchy/hierarchycontentcaps \
	ucb/source/ucp/hierarchy/hierarchycontent \
	ucb/source/ucp/hierarchy/hierarchydata \
	ucb/source/ucp/hierarchy/hierarchydatasource \
	ucb/source/ucp/hierarchy/hierarchydatasupplier \
	ucb/source/ucp/hierarchy/hierarchyprovider \
	ucb/source/ucp/hierarchy/hierarchyuri \
))

# vim: set noet sw=4 ts=4:
