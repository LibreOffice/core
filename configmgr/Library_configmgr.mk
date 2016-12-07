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



$(eval $(call gb_Library_Library,configmgr))

$(eval $(call gb_Library_add_precompiled_header,configmgr,$(SRCDIR)/configmgr/inc/pch/precompiled_configmgr))

$(eval $(call gb_Library_set_componentfile,configmgr,configmgr/source/configmgr))

$(eval $(call gb_Library_set_include,configmgr,\
        $$(INCLUDE) \
	-I$(SRCDIR)/configmgr/inc/pch \
))

$(eval $(call gb_Library_add_api,configmgr,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,configmgr,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	stl \
	xmlreader \
	$(gb_STDLIBS) \
))


$(eval $(call gb_Library_add_exception_objects,configmgr,\
	configmgr/source/access \
	configmgr/source/broadcaster \
	configmgr/source/childaccess \
	configmgr/source/components \
	configmgr/source/configurationprovider \
	configmgr/source/configurationregistry \
	configmgr/source/data \
	configmgr/source/defaultprovider \
	configmgr/source/groupnode \
	configmgr/source/localizedpropertynode \
	configmgr/source/localizedvaluenode \
	configmgr/source/lock \
	configmgr/source/modifications \
	configmgr/source/node \
	configmgr/source/nodemap \
	configmgr/source/parsemanager \
	configmgr/source/partial \
	configmgr/source/propertynode \
	configmgr/source/rootaccess \
	configmgr/source/services \
	configmgr/source/setnode \
	configmgr/source/type \
	configmgr/source/update \
	configmgr/source/valueparser \
	configmgr/source/writemodfile \
	configmgr/source/xcdparser \
	configmgr/source/xcsparser \
	configmgr/source/xcuparser \
	configmgr/source/xmldata \
))

# vim: set noet sw=4 ts=4:
