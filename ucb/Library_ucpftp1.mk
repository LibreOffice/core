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



$(eval $(call gb_Library_Library,ucpftp1))

$(eval $(call gb_Library_add_precompiled_header,ucpftp1,$(SRCDIR)/ucb/inc/pch/precompiled_ftp))

$(eval $(call gb_Library_set_componentfile,ucpftp1,ucb/source/ucp/ftp/ucpftp1))

$(eval $(call gb_Library_set_include,ucpftp1,\
        $$(INCLUDE) \
	-I$(SRCDIR)/ucb/inc/pch \
	-I$(SRCDIR)/ucb/source/inc \
))

$(eval $(call gb_Library_add_api,ucpftp1,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,ucpftp1,\
	cppuhelper \
	cppu \
	sal \
	stl \
	ucbhelper \
	$(gb_STDLIBS) \
))

$(call gb_Library_use_external,ucpftp1,curl)

$(eval $(call gb_Library_add_exception_objects,ucpftp1,\
	ucb/source/ucp/ftp/ftpservices \
	ucb/source/ucp/ftp/ftpcontentprovider \
	ucb/source/ucp/ftp/ftpcontent \
	ucb/source/ucp/ftp/ftpcontentidentifier \
	ucb/source/ucp/ftp/ftpcontentcaps \
	ucb/source/ucp/ftp/ftpdynresultset \
	ucb/source/ucp/ftp/ftpresultsetbase \
	ucb/source/ucp/ftp/ftpresultsetI \
	ucb/source/ucp/ftp/ftploaderthread \
	ucb/source/ucp/ftp/ftpinpstr \
	ucb/source/ucp/ftp/ftpdirp \
	ucb/source/ucp/ftp/ftpcfunc \
	ucb/source/ucp/ftp/ftpurl \
	ucb/source/ucp/ftp/ftpintreq \
))

# vim: set noet sw=4 ts=4:
