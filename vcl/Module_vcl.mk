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



$(eval $(call gb_Module_Module,vcl))

$(eval $(call gb_Module_add_targets,vcl,\
    Library_vcl \
    StaticLibrary_vclmain \
    Package_inc \
    AllLangResTarget_vcl \
))

ifeq ($(GUIBASE),unx)
$(eval $(call gb_Module_add_targets,vcl,\
    Library_vclplug_gen \
    Library_vclplug_svp \
    Library_desktop_detector \
))


ifeq ($(ENABLE_GTK),TRUE)
$(eval $(call gb_Module_add_targets,vcl,\
    Library_vclplug_gtk \
))
endif

ifeq ($(ENABLE_KDE),TRUE)
$(eval $(call gb_Module_add_targets,vcl,\
    Library_vclplug_kde \
))
endif
ifeq ($(ENABLE_KDE4),TRUE)
$(eval $(call gb_Module_add_targets,vcl,\
    Library_vclplug_kde4 \
))
endif
endif

ifeq ($(GUIBASE),aqua)
$(eval $(call gb_Module_add_targets,vcl,\
    Package_osx \
))
endif

ifeq ($(GUIBASE),WIN)
$(eval $(call gb_Module_add_targets,vcl,\
    WinResTarget_vcl \
))
endif

ifeq ($(GUIBASE),os2)
$(eval $(call gb_Module_add_targets,vcl,\
    WinResTarget_vcl \
))
endif

# vim: set noet sw=4 ts=4:
