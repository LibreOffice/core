# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ucpfile1))

$(eval $(call gb_Library_set_componentfile,ucpfile1,ucb/source/ucp/file/ucpfile1))

$(eval $(call gb_Library_use_external,ucpfile1,boost_headers))

$(eval $(call gb_Library_use_sdk_api,ucpfile1))

$(eval $(call gb_Library_use_libraries,ucpfile1,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	tl \
	ucbhelper \
))

$(eval $(call gb_Library_add_exception_objects,ucpfile1,\
	ucb/source/ucp/file/bc \
	ucb/source/ucp/file/filcmd \
	ucb/source/ucp/file/filglob \
	ucb/source/ucp/file/filid \
	ucb/source/ucp/file/filinpstr \
	ucb/source/ucp/file/filinsreq \
	ucb/source/ucp/file/filnot \
	ucb/source/ucp/file/filprp \
	ucb/source/ucp/file/filrec \
	ucb/source/ucp/file/filrow \
	ucb/source/ucp/file/filrset \
	ucb/source/ucp/file/filstr \
	ucb/source/ucp/file/filtask \
	ucb/source/ucp/file/prov \
))

# vim: set noet sw=4 ts=4:
