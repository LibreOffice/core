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



$(eval $(call gb_Library_Library,fps_aqua))

$(eval $(call gb_Library_set_componentfile,fps_aqua,fpicker/source/aqua/fps_aqua))

$(eval $(call gb_Library_set_include,fps_aqua,\
        $$(INCLUDE) \
	-I$(SRCDIR)/fpicker/inc/pch \
))

$(eval $(call gb_Library_add_api,fps_aqua,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,fps_aqua,\
	cppu \
	cppuhelper \
	sal \
	tl \
	vcl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,fps_aqua,\
	fpicker/source/aqua/resourceprovider \
	fpicker/source/aqua/FPentry \
	fpicker/source/aqua/SalAquaPicker \
	fpicker/source/aqua/SalAquaFilePicker \
	fpicker/source/aqua/SalAquaFolderPicker \
	fpicker/source/aqua/CFStringUtilities \
	fpicker/source/aqua/FilterHelper \
	fpicker/source/aqua/ControlHelper \
	fpicker/source/aqua/NSString_OOoAdditions \
	fpicker/source/aqua/NSURL_OOoAdditions \
	fpicker/source/aqua/AquaFilePickerDelegate \
))

$(eval $(call gb_Library_add_cxxflags,fps_aqua,\
    -x objective-c++ \
))

# vim: set noet sw=4 ts=4:
