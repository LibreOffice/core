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



$(eval $(call gb_Library_Library,tvhlp1))

$(eval $(call gb_Library_add_precompiled_header,tvhlp1,$(SRCDIR)/xmlhelp/inc/pch/precompiled_xmlhelp))

$(eval $(call gb_Library_set_componentfile,tvhlp1,xmlhelp/source/treeview/tvhlp1))

$(eval $(call gb_Library_set_include,tvhlp1,\
	$$(INCLUDE) \
	-I$(SRCDIR)/xmlhelp/inc \
	-I$(SRCDIR)/xmlhelp/inc/pch \
	-I$(SRCDIR)/solenv/inc \
	-I$(OUTDIR)/inc/stl \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_api,tvhlp1,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,tvhlp1,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    stl \
    $(gb_STDLIBS) \
))

$(call gb_Library_use_externals,tvhlp1,\
	expat_utf8 \
)

$(eval $(call gb_Library_add_exception_objects,tvhlp1,\
    xmlhelp/source/treeview/tvfactory \
    xmlhelp/source/treeview/tvread \
))

# vim: set noet sw=4 ts=4:
