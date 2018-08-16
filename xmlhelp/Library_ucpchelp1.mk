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



$(eval $(call gb_Library_Library,ucpchelp1))

$(eval $(call gb_Library_add_precompiled_header,ucpchelp1,$(SRCDIR)/xmlhelp/inc/pch/precompiled_xmlhelp))

$(eval $(call gb_Library_set_componentfile,ucpchelp1,xmlhelp/util/ucpchelp1))

$(eval $(call gb_Library_set_include,ucpchelp1,\
	$$(INCLUDE) \
	-I$(SRCDIR)/xmlhelp/inc \
	-I$(SRCDIR)/xmlhelp/inc/pch \
	-I$(SRCDIR)/xmlhelp/source/cxxhelp/inc \
	-I$(SRCDIR)/solenv/inc \
	-I$(OUTDIR)/inc/stl \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_api,ucpchelp1,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_defs,ucpchelp1,\
	-DHAVE_EXPAT_H \
))

$(eval $(call gb_Library_add_linked_libs,ucpchelp1,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    stl \
    ucbhelper \
    $(gb_STDLIBS) \
))

$(call gb_Library_use_externals,ucpchelp1,\
	expat_utf8 \
	libxml2 \
	libxslt \
)

$(eval $(call gb_Library_add_exception_objects,ucpchelp1,\
    xmlhelp/source/cxxhelp/provider/bufferedinputstream \
    xmlhelp/source/cxxhelp/provider/content \
    xmlhelp/source/cxxhelp/provider/contentcaps \
    xmlhelp/source/cxxhelp/provider/databases \
    xmlhelp/source/cxxhelp/provider/db \
    xmlhelp/source/cxxhelp/provider/inputstream \
    xmlhelp/source/cxxhelp/provider/provider \
    xmlhelp/source/cxxhelp/provider/resultset \
    xmlhelp/source/cxxhelp/provider/resultsetbase \
    xmlhelp/source/cxxhelp/provider/resultsetforroot \
    xmlhelp/source/cxxhelp/provider/resultsetforquery \
    xmlhelp/source/cxxhelp/provider/services \
    xmlhelp/source/cxxhelp/provider/urlparameter \
    xmlhelp/source/cxxhelp/qe/DocGenerator \
    xmlhelp/source/cxxhelp/util/Decompressor \
))

# vim: set noet sw=4 ts=4:
