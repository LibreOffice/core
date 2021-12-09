# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ucpftp1))

$(eval $(call gb_Library_set_componentfile,ucpftp1,ucb/source/ucp/ftp/ucpftp1,services))

$(eval $(call gb_Library_use_external,ucpftp1,boost_headers))

$(eval $(call gb_Library_use_sdk_api,ucpftp1))

$(eval $(call gb_Library_use_libraries,ucpftp1,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	ucbhelper \
))

$(eval $(call gb_Library_use_externals,ucpftp1,\
	curl \
	zlib \
))

$(eval $(call gb_Library_add_exception_objects,ucpftp1,\
	ucb/source/ucp/ftp/ftpcfunc \
	ucb/source/ucp/ftp/ftpcontentcaps \
	ucb/source/ucp/ftp/ftpcontent \
	ucb/source/ucp/ftp/ftpcontentidentifier \
	ucb/source/ucp/ftp/ftpcontentprovider \
	ucb/source/ucp/ftp/ftpdirp \
	ucb/source/ucp/ftp/ftpdynresultset \
	ucb/source/ucp/ftp/ftpintreq \
	ucb/source/ucp/ftp/ftploaderthread \
	ucb/source/ucp/ftp/ftpresultsetbase \
	ucb/source/ucp/ftp/ftpresultsetI \
	ucb/source/ucp/ftp/ftpurl \
))

# vim: set noet sw=4 ts=4:
