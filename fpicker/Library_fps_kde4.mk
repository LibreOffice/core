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



$(eval $(call gb_Library_Library,fps_kde4))

$(eval $(call gb_Library_set_componentfile,fps_kde4,fpicker/source/unx/kde4/fps_kde4))

$(eval $(call gb_Library_set_include,fps_kde4,\
        $$(INCLUDE) \
	$(filter -I%,$(KDE4_CFLAGS)) \
	-I$(SRCDIR)/fpicker/inc/pch \
))

$(eval $(call gb_Library_add_cflags,fps_kde4,\
        $(filter-out -I%,$(KDE4_CFLAGS)) \
))

$(eval $(call gb_Library_add_api,fps_kde4,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,fps_kde4,\
	cppu \
	cppuhelper \
	sal \
	tl \
	vcl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_libs,fps_kde4,\
	$(KDE4_LIBS) \
	-lkio \
	-lkfile \
))

$(eval $(call gb_Library_add_exception_objects,fps_kde4,\
	fpicker/source/unx/kde4/KDE4FilePicker \
	fpicker/source/unx/kde4/KDE4FPEntry \
))

$(WORKDIR)/CustomTarget/fpicker/source/unx/kde4/KDE4FilePicker.moc.cxx : fpicker/source/unx/kde4/KDE4FilePicker.hxx
	mkdir -p $(dir $@) && \
	$(MOC4) $< -o $@

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,fps_kde4,\
	CustomTarget/fpicker/source/unx/kde4/KDE4FilePicker.moc.cxx \
))

# vim: set noet sw=4 ts=4:
