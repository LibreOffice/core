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



$(eval $(call gb_Library_Library,fwm))

$(eval $(call gb_Library_set_componentfile,fwm,framework/util/fwm))

$(eval $(call gb_Library_set_include,fwm,\
    -I$(SRCDIR)/framework/inc/pch \
    -I$(SRCDIR)/framework/inc \
    -I$(SRCDIR)/framework/source/inc \
    -I$(WORKDIR)/inc/framework/ \
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/framework \
))

$(eval $(call gb_Library_add_api,fwm,\
    udkapi \
    offapi \
))

$(eval $(call gb_Library_add_linked_libs,fwm,\
    comphelper \
    cppu \
    cppuhelper \
    fwi \
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

$(eval $(call gb_Library_add_exception_objects,fwm,\
    framework/source/dispatch/systemexec \
    framework/source/jobs/helponstartup \
    framework/source/jobs/shelljob \
    framework/source/register/register3rdcomponents \
    framework/source/tabwin/tabwindow \
    framework/source/tabwin/tabwinfactory \
))

# vim: set noet sw=4 ts=4:
