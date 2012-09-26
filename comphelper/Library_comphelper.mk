# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Library_Library,comphelper))

$(eval $(call gb_Library_use_packages,comphelper,\
	comphelper_inc \
	officecfg_cppheader \
))

$(eval $(call gb_Library_set_componentfile,comphelper,comphelper/util/comphelp))

$(eval $(call gb_Library_set_include,comphelper,\
    -I$(SRCDIR)/comphelper/source/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,comphelper,\
       -DCOMPHELPER_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_libraries,comphelper,\
    cppu \
    cppuhelper \
    sal \
    salhelper \
    ucbhelper \
	$(gb_UWINAPI) \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_use_sdk_api,comphelper))

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
    comphelper/source/misc/configuration \
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
    comphelper/source/misc/weak \
    comphelper/source/misc/weakeventlistener \
    comphelper/source/officeinstdir/officeinstallationdirectories \
    comphelper/source/processfactory/processfactory \
    comphelper/source/property/ChainablePropertySet \
    comphelper/source/property/ChainablePropertySetInfo \
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
    comphelper/source/streaming/seekableinput \
    comphelper/source/streaming/seqinputstreamserv \
    comphelper/source/streaming/seqoutputstreamserv \
    comphelper/source/streaming/seqstream \
    comphelper/source/streaming/streamsection \
    comphelper/source/xml/attributelist \
    comphelper/source/xml/ofopxmlhelper \
    comphelper/source/xml/xmltools \
))

# vim: set noet sw=4 ts=4:
