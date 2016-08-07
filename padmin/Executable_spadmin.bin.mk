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



$(eval $(call gb_Executable_Executable,spadmin.bin))

$(eval $(call gb_Executable_set_include,spadmin.bin,\
    $$(INCLUDE) \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_Executable_add_linked_static_libs,spadmin.bin,\
    vclmain \
))

$(eval $(call gb_Executable_add_api,spadmin.bin,\
    udkapi \
    offapi \
))

$(eval $(call gb_Executable_add_linked_libs,spadmin.bin,\
    spa \
    svt \
    vcl \
    utl \
    ucbhelper \
    comphelper \
    tl \
    cppuhelper \
    cppu \
    sal \
    stl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Executable_add_exception_objects,spadmin.bin,\
    padmin/source/desktopcontext \
    padmin/source/pamain \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Executable_add_linked_libs,spadmin.bin,\
    dl \
    pthread \
))
endif

# vim: set noet sw=4 ts=4:
