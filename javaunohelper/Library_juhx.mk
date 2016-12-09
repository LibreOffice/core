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



$(eval $(call gb_Library_Library,juhx))

$(eval $(call gb_Library_add_precompiled_header,juhx,$(SRCDIR)/javaunohelper/inc/pch/precompiled_javaunohelper))

$(eval $(call gb_Library_set_include,juhx,\
        $$(INCLUDE) \
	-I$(SRCDIR)/javaunohelper/inc/pch \
))

$(eval $(call gb_Library_add_api,juhx,\
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,juhx,\
	cppu \
	cppuhelper \
	jvmaccess \
	sal \
	salhelper \
	$(gb_STDLIBS) \
))


$(eval $(call gb_Library_add_exception_objects,juhx,\
	javaunohelper/source/javaunohelper \
	javaunohelper/source/bootstrap \
	javaunohelper/source/vm \
))

# vim: set noet sw=4 ts=4:
