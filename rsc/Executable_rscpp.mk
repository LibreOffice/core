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



$(eval $(call gb_Executable_Executable,rscpp))

$(eval $(call gb_Executable_add_precompiled_header,rscpp,$(SRCDIR)/rsc/inc/pch/precompiled_rsc))

$(eval $(call gb_Executable_set_include,rscpp,\
	$$(INCLUDE) \
	-I$(SRCDIR)/rsc/inc \
	-I$(SRCDIR)/rsc/inc/pch \
))

$(eval $(call gb_Executable_add_defs,rscpp,\
	-DSOLAR \
))

ifeq ($(GUI),UNX)
$(eval $(call gb_Executable_add_defs,rscpp,\
	-Dunix \
))
endif

$(eval $(call gb_Executable_add_linked_libs,rscpp,\
	i18nisolang1 \
	sal \
	tl \
	vos3 \
	stl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Executable_add_cobjects,rscpp,\
	rsc/source/rscpp/cpp1 \
	rsc/source/rscpp/cpp2 \
	rsc/source/rscpp/cpp3 \
	rsc/source/rscpp/cpp4 \
	rsc/source/rscpp/cpp5 \
	rsc/source/rscpp/cpp6 \
))

# cc: Sun C 5.5 Patch 112761-10 2004/08/10
# Solaris x86 compiler ICE
# "cpp6.c", [get]:ube: internal error
# remove after compiler upgrade
ifeq ($(OS)$(CPU),SOLARISI)
$(eval $(call gb_LinkTarget_set_cxx_optimization, \
	rsc/source/rscpp/cpp6, $(gb_COMPILERNOOPTFLAGS) \
))
endif

# vim: set noet sw=4 ts=4:
