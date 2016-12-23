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



$(eval $(call gb_Library_Library,fps_office))

$(eval $(call gb_Library_set_componentfile,fps_office,fpicker/source/office/fps_office))

$(eval $(call gb_Library_set_include,fps_office,\
        $$(INCLUDE) \
	-I$(SRCDIR)/fpicker/inc/pch \
))

$(eval $(call gb_Library_add_api,fps_office,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,fps_office,\
	comphelper \
	cppu \
	cppuhelper \
	ootk \
	sal \
	stl \
	svl \
	svt \
	tl \
	ucbhelper \
	utl \
	vcl \
	$(gb_STDLIBS) \
))


$(eval $(call gb_Library_add_exception_objects,fps_office,\
	fpicker/source/office/asyncfilepicker \
	fpicker/source/office/commonpicker \
	fpicker/source/office/OfficeControlAccess \
	fpicker/source/office/OfficeFilePicker \
	fpicker/source/office/OfficeFolderPicker \
	fpicker/source/office/fpinteraction \
	fpicker/source/office/fpsmartcontent \
	fpicker/source/office/fps_office \
	fpicker/source/office/iodlg \
	fpicker/source/office/iodlgimp \
))

# vim: set noet sw=4 ts=4:
