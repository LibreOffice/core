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



$(eval $(call gb_Library_Library,xstor))

$(eval $(call gb_Library_add_precompiled_header,xstor,$(SRCDIR)/package/inc/pch/precompiled_package))

$(eval $(call gb_Library_set_componentfile,xstor,package/source/xstor/xstor))

$(eval $(call gb_Library_set_include,xstor,\
        $$(INCLUDE) \
	-I$(SRCDIR)/package/inc \
	-I$(SRCDIR)/package/inc/pch \
))

$(eval $(call gb_Library_add_defs,xstor,\
	-DPACKAGE_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_api,xstor,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,xstor,\
	comphelper \
	cppuhelper \
	cppu \
	sal \
	$(gb_STDLIBS) \
))


$(eval $(call gb_Library_add_exception_objects,xstor,\
	package/source/xstor/ohierarchyholder \
	package/source/xstor/ocompinstream \
	package/source/xstor/oseekinstream \
	package/source/xstor/owriteablestream \
	package/source/xstor/xstorage \
	package/source/xstor/xfactory \
	package/source/xstor/disposelistener \
	package/source/xstor/selfterminatefilestream \
	package/source/xstor/switchpersistencestream \
	package/source/xstor/register \
))

# vim: set noet sw=4 ts=4:
