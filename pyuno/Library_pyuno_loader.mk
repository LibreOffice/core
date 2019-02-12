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


# Name is changd to pyuno.so on all *nix systems in main/RepositoryFixes.mk
# python expects modules without the lib prefix 

$(eval $(call gb_Library_Library,pyuno_loader))

$(eval $(call gb_Library_set_include,pyuno_loader,\
	$$(INCLUDE) \
	-I$(SRCDIR)/pyuno/inc \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,pyuno_loader,-ldl))
else ifeq ($(OS),SOLARIS)
$(eval $(call gb_Library_add_libs,pyuno_loader,-ldl))
else ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_add_libs,pyuno_loader,-ldl))
endif

$(eval $(call gb_Library_add_cobjects,pyuno_loader,\
	pyuno/source/module/pyuno_dlopenwrapper \
))


# vim: set noet sw=4 ts=4:

