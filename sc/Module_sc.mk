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



$(eval $(call gb_Module_Module,sc))

$(eval $(call gb_Module_add_targets,sc,\
	AllLangResTarget_sc \
	Library_sc \
	Library_scd \
	Library_scfilt \
	Library_scui \
	Library_vbaobj \
	Package_uiconfig \
	Package_xml \
))

ifeq ($(ENABLE_UNIT_TESTS),YES)
$(eval $(call gb_Module_add_targets,sc,\
	GoogleTest_sc \
))
endif

ifneq ($(OOO_JUNIT_JAR),)
$(eval $(call gb_Module_add_subsequentcheck_targets,sc,\
	JunitTest_sc_complex \
	JunitTest_sc_complex_cellRanges \
))
endif


# vim: set noet sw=4 ts=4:
