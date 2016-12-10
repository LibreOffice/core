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



$(eval $(call gb_Library_Library,embobj))

$(eval $(call gb_Library_add_precompiled_header,embobj,$(SRCDIR)/embeddedobj/inc/pch/precompiled_embeddedobj))

$(eval $(call gb_Library_set_componentfile,embobj,embeddedobj/util/embobj))

$(eval $(call gb_Library_set_include,embobj,\
        $$(INCLUDE) \
	-I$(SRCDIR)/embeddedobj/inc/pch \
	-I$(SRCDIR)/embeddedobj/source/inc \
))

$(eval $(call gb_Library_add_api,embobj,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,embobj,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	stl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,embobj,\
	embeddedobj/source/commonembedding/miscobj \
	embeddedobj/source/commonembedding/specialobject \
	embeddedobj/source/commonembedding/persistence \
	embeddedobj/source/commonembedding/embedobj \
	embeddedobj/source/commonembedding/inplaceobj \
	embeddedobj/source/commonembedding/visobj \
	embeddedobj/source/general/dummyobject \
	embeddedobj/source/general/xcreator \
	embeddedobj/source/commonembedding/xfactory \
	embeddedobj/source/commonembedding/register \
	embeddedobj/source/general/docholder \
	embeddedobj/source/general/intercept \
))

# vim: set noet sw=4 ts=4:
