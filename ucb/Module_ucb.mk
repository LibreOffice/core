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



$(eval $(call gb_Module_Module,ucb))

$(eval $(call gb_Module_add_targets,ucb,\
	Library_cached1 \
	Library_srtrs1 \
	Library_ucb1 \
	Library_ucpdav1 \
	Library_ucpexpand1 \
	Library_ucpext \
	Library_ucpfile1 \
	Library_ucpftp1 \
	Library_ucphier1 \
	Library_ucppkg1 \
	Library_ucptdoc1 \
	Package_xml \
))

ifeq ($(ENABLE_GNOMEVFS),TRUE)
$(eval $(call gb_Module_add_targets,ucb,\
	Library_ucpgvfs1 \
))
endif

ifeq ($(ENABLE_GIO),TRUE)
$(eval $(call gb_Module_add_targets,ucb,\
	Library_ucpgio1 \
))
endif

ifneq ($(OOO_JUNIT_JAR),)
$(eval $(call gb_Module_add_subsequentcheck_targets,ucb,\
	JunitTest_ucb_complex \
	JunitTest_ucb_unoapi \
))
endif

# vim: set noet sw=4 ts=4:
