###############################################################
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
###############################################################



$(eval $(call gb_Library_Library,ucbhelper))

$(eval $(call gb_Library_add_package_headers,ucbhelper,ucbhelper_inc))

$(eval $(call gb_Library_add_precompiled_header,ucbhelper,$(SRCDIR)/ucbhelper/inc/pch/precompiled_ucbhelper))

$(eval $(call gb_Library_add_api,ucbhelper,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,ucbhelper,\
	$$(INCLUDE) \
	-I$(SRCDIR)/ucbhelper/inc \
	-I$(SRCDIR)/ucbhelper/inc/pch \
))

$(eval $(call gb_Library_add_defs,ucbhelper,\
	-DUCBHELPER_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,ucbhelper,\
	stl \
	sal \
	cppu \
	cppuhelper \
	salhelper \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,ucbhelper,\
	ucbhelper/source/provider/simpleauthenticationrequest \
	ucbhelper/source/provider/simpleioerrorrequest \
	ucbhelper/source/provider/cancelcommandexecution \
	ucbhelper/source/provider/contenthelper \
	ucbhelper/source/provider/propertyvalueset \
	ucbhelper/source/provider/resultset \
	ucbhelper/source/provider/resultsetmetadata \
	ucbhelper/source/provider/simplecertificatevalidationrequest \
	ucbhelper/source/provider/contentidentifier \
	ucbhelper/source/provider/resultsethelper \
	ucbhelper/source/provider/simplenameclashresolverequest \
	ucbhelper/source/provider/simpleinteractionrequest \
	ucbhelper/source/provider/handleinteractionrequest \
	ucbhelper/source/provider/registerucb \
	ucbhelper/source/provider/contentinfo \
	ucbhelper/source/provider/interactionrequest \
	ucbhelper/source/provider/commandenvironmentproxy \
	ucbhelper/source/provider/providerhelper \
	ucbhelper/source/client/proxydecider \
	ucbhelper/source/client/content \
	ucbhelper/source/client/contentbroker \
	ucbhelper/source/client/commandenvironment \
	ucbhelper/source/client/activedatastreamer \
	ucbhelper/source/client/fileidentifierconverter \
	ucbhelper/source/client/interceptedinteraction \
	ucbhelper/source/client/activedatasink \
))

# vim: set noet sw=4 ts=4:
