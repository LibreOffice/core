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



$(eval $(call gb_Library_Library,spa))

$(eval $(call gb_Library_set_include,spa,\
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/stl \
))

$(eval $(call gb_Library_add_defs,spa,\
    -DSPA_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_api,spa,\
    udkapi \
    offapi \
))

$(eval $(call gb_Library_add_linked_libs,spa,\
    svt \
    vcl \
    utl \
    tl \
    basegfx \
    comphelper \
    i18nisolang1 \
    stl \
    cppu \
    sal \
    vos3 \
    $(gb_STDLIBS) \
))


$(eval $(call gb_Library_add_exception_objects,spa,\
    padmin/source/adddlg \
    padmin/source/cmddlg \
    padmin/source/fontentry \
    padmin/source/helper \
    padmin/source/newppdlg \
    padmin/source/padialog \
    padmin/source/progress \
    padmin/source/prtsetup \
    padmin/source/titlectrl \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_linked_libs,spa,\
    dl \
    m \
    pthread \
))
endif

# vim: set noet sw=4 ts=4:
