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



$(eval $(call gb_Library_Library,i18nutil))

$(eval $(call gb_Library_set_include,i18nutil,\
        $$(INCLUDE) \
	-I$(SRCDIR)/i18nutil/inc \
))

$(eval $(call gb_Library_add_defs,i18nutil,\
	-DI18NUTIL_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_api,i18nutil,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,i18nutil,\
	cppu \
	sal \
	stl \
	$(gb_STDLIBS) \
))


$(eval $(call gb_Library_add_exception_objects,i18nutil,\
	i18nutil/source/utility/casefolding \
))

$(eval $(call gb_Library_add_noexception_objects,i18nutil,\
	i18nutil/source/utility/unicode \
	i18nutil/source/utility/widthfolding \
	i18nutil/source/utility/oneToOneMapping \
))



# vim: set noet sw=4 ts=4:
