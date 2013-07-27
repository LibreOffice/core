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




$(eval $(call gb_Library_Library,qstart_gtk))

$(eval $(call gb_Library_set_include,qstart_gtk,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sfx2/inc \
    -I$(SRCDIR)/sfx2/inc/sfx2 \
    -I$(SRCDIR)/sfx2/inc/pch \
    -I$(OUTDIR)/inc/offuh \
    -I$(OUTDIR)/inc \
    $(filter -I%,$(GTK_CFLAGS)) \
))

$(eval $(call gb_Library_set_defs,qstart_gtk,\
    $$(DEFS) \
    -DDLL_NAME=$(notdir $(call gb_Library_get_target,sfx2)) \
    -DENABLE_QUICKSTART_APPLET \
))

$(eval $(call gb_Library_set_cflags,qstart_gtk,\
    $$(CFLAGS) \
    $(filter-out -I%,$(GTK_CFLAGS)) \
))

$(eval $(call gb_Library_set_ldflags,qstart_gtk,\
    $$(LDFLAGS) \
))

$(eval $(call gb_Library_add_external_libs,qstart_gtk,	\
    $(patsubst -l%,%, $(filter -l%,	$(GTK_LIBS)))	\
))



$(eval $(call gb_Library_add_linked_libs,qstart_gtk,\
    comphelper \
    cppu \
    cppuhelper \
    fwe \
    i18nisolang1 \
    sal \
    sax \
    sb \
    sot \
    stl \
    svl \
    svt \
    ootk \
    tl \
    ucbhelper \
    utl \
    vcl \
    vos3 \
    xml2 \
    sfx \
))

$(eval $(call gb_Library_add_exception_objects,qstart_gtk,\
    sfx2/source/appl/shutdowniconunx \
))

# vim: set noet sw=4 ts=4:
