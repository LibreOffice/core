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



$(eval $(call gb_Module_Module,fpicker))

$(eval $(call gb_Module_add_targets,fpicker,\
	AllLangResTarget_fps_office \
	Library_fpicker \
	Library_fps_office \
	Package_xml \
))

ifeq ($(GUIBASE),unx)
ifeq ($(ENABLE_GTK),TRUE)
GTK_TWO_FOUR:=$(shell pkg-config --exists 'gtk+-2.0 >= 2.4.0' && echo ok)
ifeq ($(GTK_TWO_FOUR),ok)
$(eval $(call gb_Module_add_targets,fpicker,\
	Library_fps_gnome \
))
else
	_GTK_VERSION_ERR := $(shell pkg-config --print-errors --exists 'gtk+-2.0 >= 2.4.0')
endif
endif
endif

ifeq ($(GUIBASE),unx)
ifeq ($(ENABLE_KDE4),TRUE)
$(eval $(call gb_Module_add_targets,fpicker,\
	Library_fps_kde4 \
))
endif
endif

ifeq ($(GUI),WNT)
$(eval $(call gb_Module_add_targets,fpicker,\
	Library_fps \
	Library_fop \
	WinResTarget_fpicker \
))
endif

ifeq ($(GUIBASE),aqua)
$(eval $(call gb_Module_add_targets,fpicker,\
	Library_fps_aqua \
))
endif
# vim: set noet sw=4 ts=4:
