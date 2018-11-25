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



$(eval $(call gb_Library_Library,officebean))

$(eval $(call gb_Library_set_include,officebean,\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc \
))

#$(eval $(call gb_Library_add_api,officebean, \
#        udkapi \
#	offapi \
#))

#$(eval $(call gb_Library_add_defs,officebean,\
#	-DFORMULA_DLLIMPLEMENTATION \
#))

$(eval $(call gb_Library_add_linked_libs,officebean,\
	stl \
	$(gb_STDLIBS) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_linked_libs,officebean,\
	sal \
	user32 \
))
endif

$(eval $(call gb_Library_add_libs,officebean,\
	$(AWTLIB) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_cobjects,officebean,\
	bean/native/win32/com_sun_star_comp_beans_LocalOfficeWindow \
	bean/native/win32/com_sun_star_beans_LocalOfficeWindow \
))
else
$(eval $(call gb_Library_add_cobjects,officebean,\
	bean/native/unix/com_sun_star_comp_beans_LocalOfficeWindow \
	bean/native/unix/com_sun_star_beans_LocalOfficeWindow \
))
endif

# vim: set noet sw=4 ts=4:

