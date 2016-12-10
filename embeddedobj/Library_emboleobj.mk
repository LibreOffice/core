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



$(eval $(call gb_Library_Library,emboleobj))

$(eval $(call gb_Library_add_precompiled_header,emboleobj,$(SRCDIR)/embeddedobj/inc/pch/precompiled_embeddedobj))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_set_componentfile,emboleobj,embeddedobj/source/msole/emboleobj.windows))
else
$(eval $(call gb_Library_set_componentfile,emboleobj,embeddedobj/source/msole/emboleobj))
endif

$(eval $(call gb_Library_set_include,emboleobj,\
        $$(INCLUDE) \
	-I$(SRCDIR)/embeddedobj/inc/pch \
	-I$(SRCDIR)/embeddedobj/source/inc \
))

$(eval $(call gb_Library_add_api,emboleobj,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,emboleobj,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	stl \
	tl \
	utl \
	vos3 \
	$(gb_STDLIBS) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_linked_libs,emboleobj,\
	gdi32 \
	ole32 \
	oleaut32 \
	uuid \
))
else ifeq ($(OS),OS2)
$(eval $(call gb_Library_add_linked_libs,emboleobj,\
        gdi32 \
        ole32 \
        oleaut32 \
        uuid \
))
endif

$(eval $(call gb_Library_add_exception_objects,emboleobj,\
	embeddedobj/source/msole/closepreventer \
	embeddedobj/source/msole/oleregister \
	embeddedobj/source/msole/xolefactory \
	embeddedobj/source/msole/olepersist \
	embeddedobj/source/msole/oleembed \
	embeddedobj/source/msole/olevisual \
	embeddedobj/source/msole/olemisc \
	embeddedobj/source/msole/ownview \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_exception_objects,emboleobj,\
	embeddedobj/source/msole/graphconvert \
	embeddedobj/source/msole/olecomponent \
	embeddedobj/source/msole/olewrapclient \
	embeddedobj/source/msole/xdialogcreator \
	embeddedobj/source/msole/advisesink \
))
endif

# vim: set noet sw=4 ts=4:
