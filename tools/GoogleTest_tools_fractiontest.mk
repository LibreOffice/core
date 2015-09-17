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


$(eval $(call gb_GoogleTest_GoogleTest,tools_fractiontest))
 
$(eval $(call gb_GoogleTest_add_exception_objects,tools_fractiontest, \
	tools/test/tests \
))
 
$(eval $(call gb_GoogleTest_add_linked_libs,tools_fractiontest, \
    tl \
    sal \
    sfx \
    stl \
    $(gb_STDLIBS) \
))
 
$(eval $(call gb_GoogleTest_set_include,tools_fractiontest,\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/offuh \
	-I$(OUTDIR)/inc \
	-I$(SRCDIR)/tools/inc/pch \
))
 
$(eval $(call gb_GoogleTest_set_ldflags,tools_fractiontest,\
    $$(LDFLAGS) \
))

# vim: set noet sw=4 ts=4:
