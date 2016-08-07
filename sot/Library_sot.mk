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



$(eval $(call gb_Library_Library,sot))

$(eval $(call gb_Library_add_package_headers,sot,sot_inc))

$(eval $(call gb_Library_add_precompiled_header,sot,$(SRCDIR)/sot/inc/pch/precompiled_sot))

$(eval $(call gb_Library_set_componentfile,sot,sot/util/sot))

$(eval $(call gb_Library_set_include,sot,\
    -I$(SRCDIR)/sot/inc/pch \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_api,sot,\
    udkapi \
    offapi \
))

$(eval $(call gb_Library_add_defs,sot,\
    -DSOT_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,sot,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    tl \
    ucbhelper \
    stl \
    utl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,sot,\
    sot/source/unoolestorage/xolesimplestorage \
    sot/source/unoolestorage/register \
    sot/source/base/formats \
    sot/source/base/object \
    sot/source/base/exchange \
    sot/source/base/filelist \
    sot/source/base/factory \
    sot/source/sdstor/stg \
    sot/source/sdstor/stgavl \
    sot/source/sdstor/stgcache \
    sot/source/sdstor/stgdir \
    sot/source/sdstor/stgelem \
    sot/source/sdstor/stgio \
    sot/source/sdstor/stgole \
    sot/source/sdstor/stgstrms \
    sot/source/sdstor/storage \
    sot/source/sdstor/storinfo \
    sot/source/sdstor/ucbstorage \
    sot/source/sdstor/unostorageholder \
))

# vim: set noet sw=4 ts=4:
