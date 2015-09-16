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



$(eval $(call gb_Module_Module,sfx2))

$(eval $(call gb_Module_add_targets,sfx2,\
    AllLangResTarget_sfx2 \
    Library_sfx \
    Package_inc \
    Package_sdi \
))

ifeq ($(ENABLE_UNIT_TESTS),YES)
$(eval $(call gb_Module_add_check_targets,sfx2,\
    GoogleTest_sfx2_metadatable \
))
endif

ifneq ($(OOO_JUNIT_JAR),)
$(eval $(call gb_Module_add_subsequentcheck_targets,sfx2,\
    JunitTest_sfx2_complex \
    JunitTest_sfx2_unoapi \
))
endif

ifeq ($(ENABLE_SYSTRAY_GTK),TRUE)
$(eval $(call gb_Module_add_targets,sfx2,\
    Library_qstart \
))
endif

#todo: source/dialog BUILD_VER_STRING
#todo: source/doc SYSTEM_LIBXML2
#todo: ENABLE_LAYOUT
#todo: clean up quickstarter stuff in both libraries
#todo: move standard pool to svl

# vim: set noet sw=4 ts=4:
