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



$(eval $(call gb_Library_Library,binaryurp))

$(eval $(call gb_Library_set_componentfile,binaryurp,binaryurp/source/binaryurp))

$(eval $(call gb_Library_set_include,binaryurp,\
        $$(INCLUDE) \
))

$(eval $(call gb_Library_add_api,binaryurp,\
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,binaryurp,\
	cppu \
	cppuhelper \
	sal \
	salhelper \
	$(gb_STDLIBS) \
))


$(eval $(call gb_Library_add_exception_objects,binaryurp,\
	binaryurp/source/binaryany \
	binaryurp/source/bridge \
	binaryurp/source/bridgefactory \
	binaryurp/source/currentcontext \
	binaryurp/source/incomingrequest \
	binaryurp/source/lessoperators \
	binaryurp/source/marshal \
	binaryurp/source/outgoingrequests \
	binaryurp/source/proxy \
	binaryurp/source/reader \
	binaryurp/source/unmarshal \
	binaryurp/source/writer \
))

# vim: set noet sw=4 ts=4:
