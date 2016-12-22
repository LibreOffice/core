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



$(eval $(call gb_Library_Library,fps_gnome))

$(eval $(call gb_Library_set_componentfile,fps_gnome,fpicker/source/unx/gnome/fps_gnome))

$(eval $(call gb_Library_set_include,fps_gnome,\
        $$(INCLUDE) \
	$(filter -I%,$(GTK_CFLAGS)) \
	-I$(SRCDIR)/fpicker/inc/pch \
))

$(eval $(call gb_Library_add_cflags,gps_gnome,\
	$(filter-out -I%,$(GTK_CFLAGS)) \
))

$(eval $(call gb_Library_add_api,fps_gnome,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,fps_gnome,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	tl \
	vcl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_libs,fps_gnome,\
	$(GTK_LIBS) \
))

$(eval $(call gb_Library_add_exception_objects,fps_gnome,\
	fpicker/source/unx/gnome/SalGtkPicker \
	fpicker/source/unx/gnome/SalGtkFilePicker \
	fpicker/source/unx/gnome/SalGtkFolderPicker \
	fpicker/source/unx/gnome/resourceprovider \
	fpicker/source/unx/gnome/FPentry \
))

# vim: set noet sw=4 ts=4:
