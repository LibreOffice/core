# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,ucpdav1))

$(eval $(call gb_Library_set_componentfile,ucpdav1,ucb/source/ucp/webdav-neon/ucpdav1))

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
	libxml2 \
	neon \
	openssl \
))

$(eval $(call gb_Library_add_exception_objects,ucpdav1,\
	ucb/source/ucp/webdav-neon/ContentProperties \
	ucb/source/ucp/webdav-neon/DateTimeHelper \
	ucb/source/ucp/webdav-neon/DAVProperties \
	ucb/source/ucp/webdav-neon/DAVResourceAccess \
	ucb/source/ucp/webdav-neon/DAVSessionFactory \
	ucb/source/ucp/webdav-neon/LinkSequence \
	ucb/source/ucp/webdav-neon/LockEntrySequence \
	ucb/source/ucp/webdav-neon/LockSequence \
	ucb/source/ucp/webdav-neon/NeonHeadRequest \
	ucb/source/ucp/webdav-neon/NeonInputStream \
	ucb/source/ucp/webdav-neon/NeonLockStore \
	ucb/source/ucp/webdav-neon/NeonPropFindRequest \
	ucb/source/ucp/webdav-neon/NeonSession \
	ucb/source/ucp/webdav-neon/NeonUri \
	ucb/source/ucp/webdav-neon/UCBDeadPropertyValue \
	ucb/source/ucp/webdav-neon/webdavcontentcaps \
	ucb/source/ucp/webdav-neon/webdavcontent \
	ucb/source/ucp/webdav-neon/webdavdatasupplier \
	ucb/source/ucp/webdav-neon/webdavprovider \
	ucb/source/ucp/webdav-neon/webdavresultset \
	ucb/source/ucp/webdav-neon/webdavservices \
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
