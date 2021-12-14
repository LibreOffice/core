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

$(eval $(call gb_Library_use_sdk_api,ucpdav1))

$(eval $(call gb_Library_set_include,ucpdav1,\
    -I$(SRCDIR)/ucb/source/ucp/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_libraries,ucpdav1,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	ucbhelper \
	tl \
))

ifeq ($(WITH_WEBDAV),curl)

$(eval $(call gb_Library_set_componentfile,ucpdav1,ucb/source/ucp/webdav-curl/ucpdav1,services))

$(eval $(call gb_Library_use_externals,ucpdav1,\
	curl \
))

$(eval $(call gb_Library_use_custom_headers,ucpdav1,\
	officecfg/registry \
))

$(eval $(call gb_Library_add_exception_objects,ucpdav1,\
	ucb/source/ucp/webdav-curl/ContentProperties \
	ucb/source/ucp/webdav-curl/CurlSession \
	ucb/source/ucp/webdav-curl/CurlUri \
	ucb/source/ucp/webdav-curl/DAVProperties \
	ucb/source/ucp/webdav-curl/DAVResourceAccess \
	ucb/source/ucp/webdav-curl/DAVSessionFactory \
	ucb/source/ucp/webdav-curl/DAVTypes \
	ucb/source/ucp/webdav-curl/DateTimeHelper \
	ucb/source/ucp/webdav-curl/PropfindCache \
	ucb/source/ucp/webdav-curl/SerfLockStore \
	ucb/source/ucp/webdav-curl/UCBDeadPropertyValue \
	ucb/source/ucp/webdav-curl/webdavcontent \
	ucb/source/ucp/webdav-curl/webdavcontentcaps \
	ucb/source/ucp/webdav-curl/webdavdatasupplier \
	ucb/source/ucp/webdav-curl/webdavprovider \
	ucb/source/ucp/webdav-curl/webdavresponseparser \
	ucb/source/ucp/webdav-curl/webdavresultset \
))

else
ifeq ($(WITH_WEBDAV),neon)

$(eval $(call gb_Library_set_componentfile,ucpdav1,ucb/source/ucp/webdav-neon/ucpdav1,services))

$(eval $(call gb_Library_use_externals,ucpdav1,\
	boost_headers \
	libxml2 \
	neon \
	openssl \
))

$(eval $(call gb_Library_use_custom_headers,ucpdav1,\
	officecfg/registry \
))

$(eval $(call gb_Library_add_exception_objects,ucpdav1,\
	ucb/source/ucp/webdav-neon/ContentProperties \
	ucb/source/ucp/webdav-neon/DateTimeHelper \
	ucb/source/ucp/webdav-neon/DAVProperties \
	ucb/source/ucp/webdav-neon/DAVResourceAccess \
	ucb/source/ucp/webdav-neon/DAVSessionFactory \
	ucb/source/ucp/webdav-neon/DAVTypes \
	ucb/source/ucp/webdav-neon/LinkSequence \
	ucb/source/ucp/webdav-neon/LockEntrySequence \
	ucb/source/ucp/webdav-neon/LockSequence \
	ucb/source/ucp/webdav-neon/NeonHeadRequest \
	ucb/source/ucp/webdav-neon/NeonInputStream \
	ucb/source/ucp/webdav-neon/NeonLockStore \
	ucb/source/ucp/webdav-neon/NeonPropFindRequest \
	ucb/source/ucp/webdav-neon/NeonSession \
	ucb/source/ucp/webdav-neon/NeonUri \
	ucb/source/ucp/webdav-neon/PropfindCache \
	ucb/source/ucp/webdav-neon/UCBDeadPropertyValue \
	ucb/source/ucp/webdav-neon/webdavcontentcaps \
	ucb/source/ucp/webdav-neon/webdavcontent \
	ucb/source/ucp/webdav-neon/webdavdatasupplier \
	ucb/source/ucp/webdav-neon/webdavprovider \
	ucb/source/ucp/webdav-neon/webdavresultset \
))

endif # WITH_WEBDAV
endif # WITH_WEBDAV

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
