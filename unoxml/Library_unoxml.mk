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



$(eval $(call gb_Library_Library,unoxml))

$(eval $(call gb_Library_set_componentfile,unoxml,unoxml/source/service/unoxml))

$(eval $(call gb_Library_set_include,unoxml,\
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_api,unoxml,\
    udkapi \
    offapi \
))

$(eval $(call gb_Library_add_linked_libs,unoxml,\
    ucbhelper \
    sax \
    comphelper \
    cppuhelper \
    cppu \
    sal \
    stl \
    $(gb_STDLIBS) \
))

$(call gb_Library_use_external,unoxml,libxml2)

$(eval $(call gb_Library_add_exception_objects,unoxml,\
    unoxml/source/dom/node \
    unoxml/source/dom/document \
    unoxml/source/dom/element \
    unoxml/source/dom/attr \
    unoxml/source/dom/cdatasection \
    unoxml/source/dom/characterdata \
    unoxml/source/dom/comment \
    unoxml/source/dom/documentbuilder \
    unoxml/source/dom/documentfragment \
    unoxml/source/dom/documenttype \
    unoxml/source/dom/entity \
    unoxml/source/dom/entityreference \
    unoxml/source/dom/notation \
    unoxml/source/dom/processinginstruction \
    unoxml/source/dom/text \
    unoxml/source/dom/domimplementation \
    unoxml/source/dom/elementlist \
    unoxml/source/dom/childlist \
    unoxml/source/dom/notationsmap \
    unoxml/source/dom/entitiesmap \
    unoxml/source/dom/attributesmap \
    unoxml/source/dom/saxbuilder \
    unoxml/source/xpath/xpathobject \
    unoxml/source/xpath/nodelist \
    unoxml/source/xpath/xpathapi \
    unoxml/source/events/event \
    unoxml/source/events/eventdispatcher \
    unoxml/source/events/mutationevent \
    unoxml/source/events/uievent \
    unoxml/source/events/mouseevent \
    unoxml/source/events/testlistener \
    unoxml/source/service/services \
))

# vim: set noet sw=4 ts=4:

