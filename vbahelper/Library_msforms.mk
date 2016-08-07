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



# MODULE is the name of the module the makefile is located in

# declare a library
# LIB is the name of the library as it is found in Repository.mk
$(eval $(call gb_Library_Library,msforms))

# for platforms supporting PCH: declare the location of the pch file
# this is the name of the cxx file (without extension)
#$(eval $(call gb_Library_add_precompiled_header,msforms,$(SRCDIR)/vbahelper/PCH_FILE))

# in case UNO services are exported: declare location of component file
$(eval $(call gb_Library_set_componentfile,msforms,vbahelper/util/msforms))

# add any additional include paths for this library here
$(eval $(call gb_Library_set_include,msforms,\
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_api,msforms,\
    udkapi \
    offapi \
    oovbaapi \
))

# add libraries to be linked to LIB; again these names need to be given as
# specified in Repository.mk
$(eval $(call gb_Library_add_linked_libs,msforms,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sb \
    sfx \
    svl \
    svt \
    svx \
    stl \
    ootk \
    tl \
    vbahelper \
    vcl \
    $(gb_STDLIBS) \
))

# add all source files that shall be compiled with exceptions enabled
# the name is relative to $(SRCROOT) and must not contain an extension
$(eval $(call gb_Library_add_exception_objects,msforms,\
    vbahelper/source/msforms/service \
    vbahelper/source/msforms/vbabutton \
    vbahelper/source/msforms/vbacheckbox \
    vbahelper/source/msforms/vbacombobox \
    vbahelper/source/msforms/vbacontrol \
    vbahelper/source/msforms/vbacontrols \
    vbahelper/source/msforms/vbaframe \
    vbahelper/source/msforms/vbaimage \
    vbahelper/source/msforms/vbalabel \
    vbahelper/source/msforms/vbalistbox \
    vbahelper/source/msforms/vbalistcontrolhelper \
    vbahelper/source/msforms/vbamultipage \
    vbahelper/source/msforms/vbanewfont \
    vbahelper/source/msforms/vbapages \
    vbahelper/source/msforms/vbaprogressbar \
    vbahelper/source/msforms/vbaradiobutton \
    vbahelper/source/msforms/vbascrollbar \
    vbahelper/source/msforms/vbaspinbutton \
    vbahelper/source/msforms/vbasystemaxcontrol \
    vbahelper/source/msforms/vbatextbox \
    vbahelper/source/msforms/vbatogglebutton \
    vbahelper/source/msforms/vbauserform \
))

# vim: set noet sw=4 ts=4:

