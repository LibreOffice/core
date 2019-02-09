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



$(eval $(call gb_Module_Module,pyuno))

$(eval $(call gb_Module_add_targets,pyuno,\
	Library_pyuno \
	Library_pythonloader \
	Package_py \
	Package_pyuno_ini \
))

ifeq ($(GUI),UNX)
$(eval $(call gb_Module_add_targets,pyuno,\
	Library_pyuno_loader \
))
endif

ifneq ($(SYSTEM_PYTHON),YES)
ifneq ($(GUI),OS2)

$(eval $(call gb_Module_add_targets,pyuno,\
	Package_zipcore \
))

ifeq ($(GUI),WNT)
$(eval $(call gb_Module_add_targets,pyuno,\
	Executable_pyuno_python_exe \
))
else ifeq ($(GUI),UNX)
$(eval $(call gb_Module_add_targets,pyuno,\
	Package_python_bin \
	Package_pyuno_python_sh \
))
endif

endif
endif

# vim: set noet sw=4 ts=4:
