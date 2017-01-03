#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



$(eval $(call gb_Library_Library,ucpdav1))

$(eval $(call gb_Library_add_precompiled_header,ucpdav1,$(SRCDIR)/ucb/inc/pch/precompiled_webdav))

$(eval $(call gb_Library_set_componentfile,ucpdav1,ucb/source/ucp/webdav/ucpdav1))

$(eval $(call gb_Library_set_include,ucpdav1,\
        $$(INCLUDE) \
	-I$(SRCDIR)/ucb/inc/pch \
	-I$(SRCDIR)/ucb/source/inc \
))

$(eval $(call gb_Library_add_api,ucpdav1,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,ucpdav1,\
	comphelper \
	cppuhelper \
	cppu \
	sal \
	salhelper \
	stl \
	tl \
	ucbhelper \
	$(gb_STDLIBS) \
))

ifeq ($(GUI),WNT)
$(eval $(call gb_Library_add_linked_libs,ucpdav1,wsock32))
ifneq ($(WINDOWS_VISTA_PSDK),)
$(eval $(call gb_Library_add_linked_libs,ucpdav1,ws2_32))
endif
endif

ifeq ($(OS),SOLARIS)
$(eval $(call gb_Library_add_linked_libs,ucpdav1,\
	dl \
	nsl \
	socket \
))
endif

$(call gb_Library_use_external,ucpdav1,apr)
$(call gb_Library_use_external,ucpdav1,apr_util)
$(call gb_Library_use_external,ucpdav1,libxml2)
$(call gb_Library_use_external,ucpdav1,openssl)
$(call gb_Library_use_external,ucpdav1,serf)

$(eval $(call gb_Library_add_exception_objects,ucpdav1,\
	ucb/source/ucp/webdav/webdavservices \
	ucb/source/ucp/webdav/webdavprovider \
	ucb/source/ucp/webdav/webdavcontent \
	ucb/source/ucp/webdav/webdavcontentcaps \
	ucb/source/ucp/webdav/webdavresultset \
	ucb/source/ucp/webdav/webdavdatasupplier \
	ucb/source/ucp/webdav/ContentProperties \
	ucb/source/ucp/webdav/DAVProperties \
	ucb/source/ucp/webdav/DAVSessionFactory \
	ucb/source/ucp/webdav/DAVResourceAccess \
	ucb/source/ucp/webdav/AprEnv \
	ucb/source/ucp/webdav/webdavresponseparser \
	ucb/source/ucp/webdav/SerfUri \
	ucb/source/ucp/webdav/SerfRequestProcessor \
	ucb/source/ucp/webdav/SerfRequestProcessorImpl \
	ucb/source/ucp/webdav/SerfRequestProcessorImplFac \
	ucb/source/ucp/webdav/SerfPropFindReqProcImpl \
	ucb/source/ucp/webdav/SerfPropPatchReqProcImpl \
	ucb/source/ucp/webdav/SerfGetReqProcImpl \
	ucb/source/ucp/webdav/SerfHeadReqProcImpl \
	ucb/source/ucp/webdav/SerfPutReqProcImpl \
	ucb/source/ucp/webdav/SerfPostReqProcImpl \
	ucb/source/ucp/webdav/SerfDeleteReqProcImpl \
	ucb/source/ucp/webdav/SerfMkColReqProcImpl \
	ucb/source/ucp/webdav/SerfCopyReqProcImpl \
	ucb/source/ucp/webdav/SerfMoveReqProcImpl \
	ucb/source/ucp/webdav/SerfLockReqProcImpl \
	ucb/source/ucp/webdav/SerfLockRefreshProcImpl \
	ucb/source/ucp/webdav/SerfUnlockProcImpl \
	ucb/source/ucp/webdav/SerfLockStore \
	ucb/source/ucp/webdav/SerfSession \
	ucb/source/ucp/webdav/SerfCallbacks \
	ucb/source/ucp/webdav/SerfInputStream \
	ucb/source/ucp/webdav/DateTimeHelper \
	ucb/source/ucp/webdav/UCBDeadPropertyValue \
))

# vim: set noet sw=4 ts=4:
