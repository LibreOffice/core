###############################################################
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
###############################################################



$(eval $(call gb_Module_Module,dbaccess))

$(eval $(call gb_Module_add_targets,dbaccess,\
	AllLangResTarget_adabasui \
	AllLangResTarget_dba\
	AllLangResTarget_dbmm\
	AllLangResTarget_dbui\
	AllLangResTarget_sdbt\
	Library_adabasui\
	Library_dba\
	Library_dbaxml\
	Library_dbmm\
	Library_dbui\
	Library_sdbt\
	Package_inc \
	Package_uiconfig \
	Package_xml \
))

ifeq ($(GUI),WNT)
$(eval $(call gb_Module_add_targets,dbaccess,\
	Executable_odbcconfig \
))
endif

$(eval $(call gb_Module_add_subsequentcheck_targets,dbaccess,\
	JunitTest_dbaccess_complex \
	JunitTest_dbaccess_unoapi \
))

# vim: set noet ts=4 sw=4:
