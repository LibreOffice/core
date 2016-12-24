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



$(eval $(call gb_Library_Library,fps))

$(eval $(call gb_Library_set_componentfile,fps,fpicker/util/fps))

$(eval $(call gb_Library_set_include,fps,\
        $$(INCLUDE) \
	-I$(SRCDIR)/fpicker/inc/pch \
))

$(eval $(call gb_Library_add_api,fps,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,fps,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	stl \
	tl \
	vcl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_linked_libs,fps,\
	advapi32 \
	comdlg32 \
	gdi32 \
	kernel32 \
	ole32 \
	oleaut32 \
	shell32 \
	user32\
	uuid \
	uwinapi \
))

$(eval $(call gb_Library_add_cxxobjects,fps,\
	fpicker/source/win32/filepicker/FileOpenDlg \
	fpicker/source/win32/filepicker/FPentry \
	fpicker/source/win32/filepicker/FilePicker \
	fpicker/source/win32/filepicker/WinFileOpenImpl \
	fpicker/source/win32/filepicker/FilterContainer \
	fpicker/source/win32/filepicker/controlaccess \
	fpicker/source/win32/filepicker/dibpreview \
	fpicker/source/win32/filepicker/helppopupwindow \
	fpicker/source/win32/filepicker/controlcommand \
	fpicker/source/win32/filepicker/filepickerstate \
	fpicker/source/win32/filepicker/getfilenamewrapper \
	fpicker/source/win32/filepicker/asynceventnotifier \
	fpicker/source/win32/filepicker/previewadapter \
	fpicker/source/win32/filepicker/previewbase \
	fpicker/source/win32/filepicker/filepickereventnotification \
	fpicker/source/win32/filepicker/customcontrol \
	fpicker/source/win32/filepicker/customcontrolcontainer \
	fpicker/source/win32/filepicker/customcontrolfactory \
	fpicker/source/win32/filepicker/dialogcustomcontrols \
	fpicker/source/win32/filepicker/asyncrequests \
	fpicker/source/win32/filepicker/VistaFilePickerEventHandler \
	fpicker/source/win32/filepicker/VistaFilePickerImpl \
	fpicker/source/win32/filepicker/VistaFilePicker \
	fpicker/source/win32/filepicker/SolarMutex \
	, -D_UNICODE -DUNICODE -U_WIN32_WINNT -D_WIN32_WINNT=0x0600 $(gb_LinkTarget_EXCEPTIONFLAGS) \
))

ifeq ($(COM),GCC)
$(eval $(call gb_Library_add_cxxobjects,fps,\
	fpicker/source/win32/misc/WinImplHelper \
	fpicker/source/win32/misc/AutoBuffer \
	fpicker/source/win32/misc/resourceprovider \
	, -fexceptions -fno-enforce-eh-specs -DUNICODE -D_UNICODE $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
else
$(eval $(call gb_Library_add_cxxobjects,fps,\
	fpicker/source/win32/misc/WinImplHelper \
	fpicker/source/win32/misc/AutoBuffer \
	fpicker/source/win32/misc/resourceprovider \
	, -DUNICODE -D_UNICODE $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
endif

$(eval $(call gb_Library_add_nativeres,fps,src))

# vim: set noet sw=4 ts=4:
