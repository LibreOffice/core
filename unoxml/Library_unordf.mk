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



$(eval $(call gb_Library_Library,unordf))

$(eval $(call gb_Library_set_componentfile,unordf,unoxml/source/rdf/unordf))

$(eval $(call gb_Library_set_include,unordf,\
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_set_defs,unordf,\
    $$(DEFS) \
))

$(eval $(call gb_Library_add_linked_libs,unordf,\
    cppuhelper \
    cppu \
    sal \
    stl \
    rdf \
    xslt \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,unordf,\
    unoxml/source/rdf/CBlankNode \
    unoxml/source/rdf/CURI \
    unoxml/source/rdf/CLiteral \
    unoxml/source/rdf/librdf_repository \
    unoxml/source/rdf/librdf_services \
))

ifeq ($(SYSTEM_REDLAND),YES)
$(eval $(call gb_Library_set_cxxflags,unordf,\
    $$(CXXFLAGS) \
    -DSYSTEM_REDLAND $$(REDLAND_CFLAGS) \
))
endif

ifeq ($(SYSTEM_LIBXSLT),YES)
$(eval $(call gb_Library_set_cxxflags,unordf,\
    $$(CXXFLAGS) \
    $$(LIBXSLT_CFLAGS) \
))
endif

# vim: set noet sw=4 ts=4:

