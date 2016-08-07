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



$(eval $(call gb_Executable_Executable,bmp))

$(eval $(call gb_Executable_set_include,bmp,\
    $$(INCLUDE) \
    -I$(WORKDIR)/inc/svtools \
    -I$(WORKDIR)/inc/ \
    -I$(OUTDIR)/inc/ \
    -I$(SRCDIR)/svtools/inc \
    -I$(SRCDIR)/svtools/inc/svtools \
    -I$(SRCDIR)/svtools/source/inc \
    -I$(SRCDIR)/svtools/inc/pch \
))

$(eval $(call gb_Executable_add_api,bmp,\
    udkapi \
    offapi \
))

$(eval $(call gb_Executable_add_linked_libs,bmp,\
    sal \
    stl \
    tl \
    vcl \
    vos3 \
    $(gb_STDLIBS) \
))

ifneq ($(USE_SYSTEM_STL),YES)
ifeq ($(PRODUCT),)
 $(eval $(call gb_Executable_set_cxxflags,bmp,\
        $$(CXXFLAGS) \
        -D_DEBUG \
))
endif
endif

$(eval $(call gb_Executable_add_exception_objects,bmp,\
    svtools/bmpmaker/bmp \
    svtools/bmpmaker/bmpcore \
))

# vim: set noet sw=4 ts=4:
