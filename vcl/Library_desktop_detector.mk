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



$(eval $(call gb_Library_Library,desktop_detector))

#$(eval $(call gb_Library_add_package_headers,vcl,vcl_inc))

$(eval $(call gb_Library_add_precompiled_header,desktop_detector,$(SRCDIR)/vcl/inc/pch/precompiled_vcl))

$(eval $(call gb_Library_add_api,desktop_detector,\
    udkapi \
    offapi \
))

$(eval $(call gb_Library_set_include,desktop_detector,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/vcl/inc/pch \
    -I$(SRCDIR)/solenv/inc \
    -I$(OUTDIR)/inc/stl \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_defs,desktop_detector,\
    -DDESKTOP_DETECTOR_IMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,desktop_detector,\
    vcl \
    tl \
    utl \
    sot \
    ucbhelper \
    basegfx \
    comphelper \
    cppuhelper \
    i18nisolang1 \
    i18npaper \
    i18nutil \
    jvmaccess \
    stl \
    cppu \
    sal \
    vos3 \
    X11 \
    Xext \
    SM \
    ICE \
    $(gb_STDLIBS) \
))

$(call gb_Library_use_externals,desktop_detector,\
    icule \
    icuuc \
)

$(eval $(call gb_Library_add_exception_objects,desktop_detector,\
    vcl/unx/generic/desktopdetect/desktopdetector \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_linked_libs,desktop_detector,\
    dl \
    m \
    pthread \
))
endif

# vim: set noet sw=4 ts=4:
