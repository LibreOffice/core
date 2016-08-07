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



$(eval $(call gb_Library_Library,fsstorage))

$(eval $(call gb_Library_set_componentfile,fsstorage,svl/source/fsstor/fsstorage))

$(eval $(call gb_Library_set_include,fsstorage,\
    $$(SOLARINC) \
    -I$(WORKDIR)/inc/svl \
    -I$(WORKDIR)/inc/ \
    -I$(SRCDIR)/svl/inc \
    -I$(SRCDIR)/svl/inc/svl \
    -I$(SRCDIR)/svl/source/inc \
    -I$(SRCDIR)/svl/inc/pch \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_api,fsstorage,\
    udkapi \
    offapi \
))

$(eval $(call gb_Library_add_linked_libs,fsstorage,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    stl \
    tl \
    ucbhelper \
    utl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,fsstorage,\
    svl/source/fsstor/fsfactory \
    svl/source/fsstor/fsstorage \
    svl/source/fsstor/oinputstreamcontainer \
    svl/source/fsstor/ostreamcontainer \
))

# vim: set noet sw=4 ts=4:
