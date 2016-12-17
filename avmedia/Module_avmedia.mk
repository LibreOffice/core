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



$(eval $(call gb_Module_Module,avmedia))

$(eval $(call gb_Module_add_targets,avmedia,\
	AllLangResTarget_avmedia \
	Library_avmedia \
	Package_inc \
	Package_avmedia_jar \
))

ifeq ($(ENABLE_GSTREAMER),TRUE)
ifeq ($(GUI),UNX)
ifneq ($(GUIBASE),aqua)
$(eval $(call gb_Module_add_targets,avmedia,\
        Library_avmediagst \
))
endif
endif
endif

# Seems obsolete, "javamedia" is not found anywhere else in the project,
# and avmedia.jar is delivered from the pre-existing avmedia.jar in
# main/avmedia/source/java. This is not tested:
ifeq ($(GUIBASE),javamedia)
$(eval $(call gb_Module_add_targets,avmedia,\
	Jar_avmedia \
))
endif

ifeq ($(GUI),WNT)
ifneq ($(strip $(ENABLE_DIRECTX)),)
$(eval $(call gb_Module_add_targets,avmedia,\
	Library_avmediawin \
))
endif
endif

ifeq ($(GUIBASE),aqua)
$(eval $(call gb_Module_add_targets,avmedia,\
	Library_avmediaMacAVF \
	Library_avmediaQuickTime \
))
endif


# vim: set noet sw=4 ts=4:
