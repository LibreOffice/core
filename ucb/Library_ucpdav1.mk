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

$(eval $(call gb_Library_add_standard_system_libs,ucpdav1))

$(eval $(call gb_Library_use_externals,ucpdav1,\
	libxml2 \
	neon \
	openssl \
))

$(eval $(call gb_Library_add_exception_objects,ucpdav1,\
	ucb/source/ucp/webdav/ContentProperties \
	ucb/source/ucp/webdav/DateTimeHelper \
	ucb/source/ucp/webdav/DAVProperties \
	ucb/source/ucp/webdav/DAVResourceAccess \
	ucb/source/ucp/webdav/DAVSessionFactory \
	ucb/source/ucp/webdav/LinkSequence \
	ucb/source/ucp/webdav/LockEntrySequence \
	ucb/source/ucp/webdav/LockSequence \
	ucb/source/ucp/webdav/NeonHeadRequest \
	ucb/source/ucp/webdav/NeonInputStream \
	ucb/source/ucp/webdav/NeonLockStore \
	ucb/source/ucp/webdav/NeonPropFindRequest \
	ucb/source/ucp/webdav/NeonSession \
	ucb/source/ucp/webdav/NeonUri \
	ucb/source/ucp/webdav/UCBDeadPropertyValue \
	ucb/source/ucp/webdav/webdavcontentcaps \
	ucb/source/ucp/webdav/webdavcontent \
	ucb/source/ucp/webdav/webdavdatasupplier \
	ucb/source/ucp/webdav/webdavprovider \
	ucb/source/ucp/webdav/webdavresultset \
	ucb/source/ucp/webdav/webdavservices \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_libraries,ucpdav1,\
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
