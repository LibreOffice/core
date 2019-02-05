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



$(eval $(call gb_Executable_Executable,saxparser))

$(eval $(call gb_Executable_add_precompiled_header,saxparser,$(SRCDIR)/i18npool/inc/pch/precompiled_i18npool))

$(eval $(call gb_Executable_set_include,saxparser,\
	$$(INCLUDE) \
	-I$(SRCDIR)/i18npool/inc \
	-I$(SRCDIR)/i18npool/inc/pch \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Executable_add_api,saxparser,\
	offapi \
	udkapi \
))

# the xmlparser build breaks in this header file
# 
#      /opt/SUNWspro/WS6U1/include/CC/Cstd/./limits
#
# which defines a class with member functions called min() and max().
# the build breaks because in solar.h, there is something like this
# 
#      #define max(x,y) ((x) < (y) ? (y) : (x))
#      #define min(x,y) ((x) < (y) ? (x) : (y))
#
# so the only choice is to prevent "CC/Cstd/./limits" from being 
# included:
ifeq ($(OS),SOLARIS)
$(eval $(call gb_Executable_add_defs,saxparser,\
	-D__STD_LIMITS \
))
endif


$(eval $(call gb_Executable_add_linked_libs,saxparser,\
	cppu \
	cppuhelper \
	sal \
	stl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Executable_add_exception_objects,saxparser,\
	i18npool/source/localedata/saxparser \
	i18npool/source/localedata/LocaleNode \
	i18npool/source/localedata/filewriter \
))

# vim: set noet sw=4 ts=4:
