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



$(eval $(call gb_Library_Library,animcore))

$(eval $(call gb_Library_set_componentfile,animcore,animations/source/animcore/animcore))

$(eval $(call gb_Library_set_include,animcore,\
        $$(INCLUDE) \
        -I$(SRCDIR)/animations/inc \
))

$(eval $(call gb_Library_add_api,animcore,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,animcore,\
	cppuhelper \
	cppu \
	sal \
	$(gb_STDLIBS) \
))


$(eval $(call gb_Library_add_exception_objects,animcore,\
	animations/source/animcore/animcore \
	animations/source/animcore/factreg \
	animations/source/animcore/targetpropertiescreator \
))

# vim: set noet sw=4 ts=4:
