# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ucpdav1))

$(eval $(call gb_Library_set_componentfile,ucpdav1,ucb/source/ucp/webdav/ucpdav1))

$(eval $(call gb_Library_use_sdk_api,ucpdav1))

$(eval $(call gb_Library_use_libraries,ucpdav1,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	ucbhelper \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_externals,ucpdav1,\
	boost_headers \
	apr \
	openssl \
	serf \
	zlib \
))

$(eval $(call gb_Library_add_exception_objects,ucpdav1,\
	ucb/source/ucp/webdav/AprEnv \
	ucb/source/ucp/webdav/ContentProperties \
	ucb/source/ucp/webdav/DAVProperties \
	ucb/source/ucp/webdav/DAVResourceAccess \
	ucb/source/ucp/webdav/DAVSessionFactory \
	ucb/source/ucp/webdav/DateTimeHelper \
	ucb/source/ucp/webdav/SerfCallbacks \
	ucb/source/ucp/webdav/SerfCopyReqProcImpl \
	ucb/source/ucp/webdav/SerfDeleteReqProcImpl \
	ucb/source/ucp/webdav/SerfGetReqProcImpl \
	ucb/source/ucp/webdav/SerfHeadReqProcImpl \
	ucb/source/ucp/webdav/SerfInputStream \
	ucb/source/ucp/webdav/SerfLockReqProcImpl \
	ucb/source/ucp/webdav/SerfLockStore \
	ucb/source/ucp/webdav/SerfMkColReqProcImpl \
	ucb/source/ucp/webdav/SerfMoveReqProcImpl \
	ucb/source/ucp/webdav/SerfPostReqProcImpl \
	ucb/source/ucp/webdav/SerfPropFindReqProcImpl \
	ucb/source/ucp/webdav/SerfPropPatchReqProcImpl \
	ucb/source/ucp/webdav/SerfPutReqProcImpl \
	ucb/source/ucp/webdav/SerfRequestProcessor \
	ucb/source/ucp/webdav/SerfRequestProcessorImpl \
	ucb/source/ucp/webdav/SerfSession \
	ucb/source/ucp/webdav/SerfUnlockReqProcImpl \
	ucb/source/ucp/webdav/SerfUri \
	ucb/source/ucp/webdav/UCBDeadPropertyValue \
	ucb/source/ucp/webdav/webdavcontent \
	ucb/source/ucp/webdav/webdavcontentcaps \
	ucb/source/ucp/webdav/webdavdatasupplier \
	ucb/source/ucp/webdav/webdavprovider \
	ucb/source/ucp/webdav/webdavresponseparser \
	ucb/source/ucp/webdav/webdavresultset \
	ucb/source/ucp/webdav/webdavservices \
 ))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,ucpdav1,\
	ws2_32 \
))
endif

ifeq ($(OS),SOLARIS)
$(eval $(call gb_Library_add_libs,ucpdav1,\
	-ldl \
	-lnsl \
	-lsocket \
))
endif

# vim: set noet sw=4 ts=4:
