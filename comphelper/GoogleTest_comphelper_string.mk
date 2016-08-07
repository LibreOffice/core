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



$(eval $(call gb_GoogleTest_GoogleTest,comphelper_string))

$(eval $(call gb_GoogleTest_add_exception_objects,comphelper_string, \
	comphelper/qa/test_string \
))

$(eval $(call gb_GoogleTest_add_linked_libs,comphelper_string, \
    comphelper \
    cppuhelper \
    cppu \
    sal \
    stl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_GoogleTest_set_include,comphelper_string,\
	$$(INCLUDE) \
	-I$(SRCDIR)/formula/inc \
	-I$(SRCDIR)/comphelper/inc/pch \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_GoogleTest_add_api,comphelper_string,\
	udkapi \
))

# vim: set noet sw=4 ts=4:
