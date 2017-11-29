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



$(eval $(call gb_Executable_Executable,rdbmaker))

$(eval $(call gb_Executable_add_api,rdbmaker,\
	udkapi \
))

$(eval $(call gb_Executable_set_include,rdbmaker,\
	-I$(SRCDIR)/rdbmaker/inc/pch \
	-I$(SRCDIR)/rdbmaker/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Executable_add_linked_libs,rdbmaker,\
	cppu \
	cppuhelper \
	reg \
	sal \
	salhelper \
	stl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Executable_add_exception_objects,rdbmaker,\
	rdbmaker/source/codemaker/typemanager \
	rdbmaker/source/codemaker/global \
	rdbmaker/source/codemaker/dependency \
	rdbmaker/source/codemaker/options \
	rdbmaker/source/rdbmaker/rdbmaker \
	rdbmaker/source/rdbmaker/specialtypemanager \
	rdbmaker/source/rdbmaker/typeblop \
	rdbmaker/source/rdbmaker/rdbtype \
	rdbmaker/source/rdbmaker/rdboptions \
))

# vim: set noet sw=4 ts=4:
