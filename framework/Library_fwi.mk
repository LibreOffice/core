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



$(eval $(call gb_Library_Library,fwi))

$(eval $(call gb_Library_add_package_headers,fwi,framework_inc))

$(eval $(call gb_Library_add_defs,fwi,\
    -DFWI_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_api,fwi,\
    udkapi \
    offapi \
))

$(eval $(call gb_Library_set_include,fwi,\
    -I$(SRCDIR)/framework/inc/pch \
    -I$(SRCDIR)/framework/source/inc \
    -I$(SRCDIR)/framework/inc \
    -I$(WORKDIR)/inc/framework/ \
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/framework \
))

$(eval $(call gb_Library_add_linked_libs,fwi,\
    comphelper \
    cppu \
    cppuhelper \
    i18nisolang1 \
    sal \
    stl \
    svl \
    svt \
    ootk \
    tl \
    utl \
    vcl \
    vos3 \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,fwi,\
    framework/source/fwi/classes/converter \
    framework/source/fwi/classes/propertysethelper \
    framework/source/fwi/classes/protocolhandlercache \
    framework/source/fwi/helper/mischelper \
    framework/source/fwi/helper/networkdomain \
    framework/source/fwi/helper/shareablemutex \
    framework/source/fwi/jobs/configaccess \
    framework/source/fwi/jobs/jobconst \
    framework/source/fwi/threadhelp/lockhelper \
    framework/source/fwi/threadhelp/transactionmanager \
    framework/source/fwi/uielement/constitemcontainer \
    framework/source/fwi/uielement/itemcontainer \
    framework/source/fwi/uielement/rootitemcontainer \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_linked_libs,fwi,\
    advapi32 \
))
endif

# vim: set noet sw=4 ts=4:
