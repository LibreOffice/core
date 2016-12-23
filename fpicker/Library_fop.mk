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



$(eval $(call gb_Library_Library,fop))

$(eval $(call gb_Library_set_componentfile,fop,fpicker/util/fop))

$(eval $(call gb_Library_set_include,fop,\
        $$(INCLUDE) \
	-I$(SRCDIR)/fpicker/inc/pch \
))

$(eval $(call gb_Library_add_api,fop,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,fop,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	stl \
	tl \
	vcl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_linked_libs,fop,\
	advapi32 \
	gdi32 \
	ole32 \
	oleaut32 \
	shell32 \
	user32 \
	uwinapi \
))

$(eval $(call gb_Library_add_exception_objects,fop,\
	fpicker/source/win32/folderpicker/Fopentry \
	fpicker/source/win32/folderpicker/FolderPicker \
	fpicker/source/win32/folderpicker/WinFOPImpl \
	fpicker/source/win32/folderpicker/MtaFop \
))

ifeq ($(COM),GCC)
$(eval $(call gb_Library_add_cxxobjects,fop,\
	fpicker/source/win32/misc/WinImplHelper \
	fpicker/source/win32/misc/AutoBuffer \
	fpicker/source/win32/misc/resourceprovider \
	, -fexceptions -fno-enforce-eh-specs -DUNICODE -D_UNICODE $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
else
$(eval $(call gb_Library_add_cxxobjects,fop,\
	fpicker/source/win32/misc/WinImplHelper \
	fpicker/source/win32/misc/AutoBuffer \
	fpicker/source/win32/misc/resourceprovider \
	, -DUNICODE -D_UNICODE $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
endif

# vim: set noet sw=4 ts=4:
