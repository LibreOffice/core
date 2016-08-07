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



$(eval $(call gb_Library_Library,comphelper))

$(eval $(call gb_Library_add_package_headers,comphelper,comphelper_inc))

$(eval $(call gb_Library_add_precompiled_header,comphelper,$(SRCDIR)/comphelper/inc/pch/precompiled_comphelper))

$(eval $(call gb_Library_set_componentfile,comphelper,comphelper/util/comphelp))

$(eval $(call gb_Library_set_include,comphelper,\
    -I$(SRCDIR)/comphelper/inc/pch \
    -I$(SRCDIR)/comphelper/source/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,comphelper,\
       -DCOMPHELPER_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,comphelper,\
    sal \
    cppuhelper \
    cppu \
    ucbhelper \
    vos3 \
    stl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_api,comphelper,\
    udkapi \
    offapi \
))

$(eval $(call gb_Library_add_exception_objects,comphelper,\
    comphelper/source/compare/AnyCompareFactory \
    comphelper/source/container/IndexedPropertyValuesContainer \
    comphelper/source/container/NamedPropertyValuesContainer \
    comphelper/source/container/container \
    comphelper/source/container/containermultiplexer \
    comphelper/source/container/embeddedobjectcontainer \
    comphelper/source/container/enumerablemap \
    comphelper/source/container/enumhelper \
    comphelper/source/container/namecontainer \
    comphelper/source/eventattachermgr/eventattachermgr \
    comphelper/source/misc/accessiblecomponenthelper \
    comphelper/source/misc/accessiblecontexthelper \
    comphelper/source/misc/accessibleeventbuffer \
    comphelper/source/misc/accessibleeventnotifier \
    comphelper/source/misc/accessiblekeybindinghelper \
    comphelper/source/misc/accessibleselectionhelper \
    comphelper/source/misc/accessibletexthelper \
    comphelper/source/misc/accessiblewrapper \
    comphelper/source/misc/accimplaccess \
    comphelper/source/misc/anytostring \
    comphelper/source/misc/asyncnotification \
    comphelper/source/misc/comphelper_module \
    comphelper/source/misc/comphelper_services \
    comphelper/source/misc/componentbase \
    comphelper/source/misc/componentcontext \
    comphelper/source/misc/componentmodule \
    comphelper/source/misc/configurationhelper \
    comphelper/source/misc/docpasswordhelper \
    comphelper/source/misc/docpasswordrequest \
    comphelper/source/misc/documentinfo \
    comphelper/source/misc/documentiologring \
    comphelper/source/misc/evtlistenerhlp \
    comphelper/source/misc/evtmethodhelper \
    comphelper/source/misc/ihwrapnofilter \
    comphelper/source/misc/instancelocker \
    comphelper/source/misc/interaction \
    comphelper/source/misc/legacysingletonfactory \
    comphelper/source/misc/listenernotification \
    comphelper/source/misc/locale \
    comphelper/source/misc/logging \
    comphelper/source/misc/mediadescriptor \
    comphelper/source/misc/mimeconfighelper \
    comphelper/source/misc/namedvaluecollection \
    comphelper/source/misc/numberedcollection \
    comphelper/source/misc/numbers \
    comphelper/source/misc/officeresourcebundle \
    comphelper/source/misc/officerestartmanager \
    comphelper/source/misc/proxyaggregation \
    comphelper/source/misc/regpathhelper \
    comphelper/source/misc/scopeguard \
    comphelper/source/misc/SelectionMultiplex \
    comphelper/source/misc/sequenceashashmap \
    comphelper/source/misc/sequence \
    comphelper/source/misc/servicedecl \
    comphelper/source/misc/serviceinfohelper \
    comphelper/source/misc/sharedmutex \
    comphelper/source/misc/stillreadwriteinteraction \
    comphelper/source/misc/anycompare \
    comphelper/source/misc/storagehelper \
    comphelper/source/misc/string \
    comphelper/source/misc/synchronousdispatch \
    comphelper/source/misc/types \
    comphelper/source/misc/uieventslogger \
    comphelper/source/misc/weak \
    comphelper/source/misc/weakeventlistener \
    comphelper/source/officeinstdir/officeinstallationdirectories \
    comphelper/source/processfactory/componentfactory \
    comphelper/source/processfactory/processfactory \
    comphelper/source/property/ChainablePropertySet \
    comphelper/source/property/ChainablePropertySetInfo \
    comphelper/source/property/composedprops \
    comphelper/source/property/genericpropertyset \
    comphelper/source/property/MasterPropertySet \
    comphelper/source/property/MasterPropertySetInfo \
    comphelper/source/property/opropertybag \
    comphelper/source/property/propagg \
    comphelper/source/property/propertybag \
    comphelper/source/property/propertycontainer \
    comphelper/source/property/propertycontainerhelper \
    comphelper/source/property/property \
    comphelper/source/property/propertysethelper \
    comphelper/source/property/propertysetinfo \
    comphelper/source/property/propertystatecontainer \
    comphelper/source/property/propmultiplex \
    comphelper/source/property/propstate \
    comphelper/source/property/TypeGeneration \
    comphelper/source/streaming/basicio \
    comphelper/source/streaming/memorystream \
    comphelper/source/streaming/oslfile2streamwrap \
    comphelper/source/streaming/otransactedfilestream \
    comphelper/source/streaming/seekableinput \
    comphelper/source/streaming/seqinputstreamserv \
    comphelper/source/streaming/seqoutputstreamserv \
    comphelper/source/streaming/seqstream \
    comphelper/source/streaming/streamsection \
    comphelper/source/xml/attributelist \
    comphelper/source/xml/ofopxmlhelper \
))

# vim: set noet sw=4 ts=4:
