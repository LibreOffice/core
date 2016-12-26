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



$(eval $(call gb_Module_Module,slideshow))

$(eval $(call gb_Module_add_targets,slideshow,\
	Library_slideshow \
	StaticLibrary_sldshw_s \
))

ifeq ($(ENABLE_OPENGL),TRUE)
$(eval $(call gb_Module_add_targets,slideshow,\
	Library_OGLTrans \
))
endif

ifeq ($(ENABLE_UNIT_TESTS),YES)
$(eval $(call gb_Module_add_check_targets,slideshow,\
        GoogleTest_slideshow_tests \
))
endif

# crashes on startup, did so before port to gbuild:
#        GoogleTest_slideshow_demoshow \



# vim: set noet sw=4 ts=4:
