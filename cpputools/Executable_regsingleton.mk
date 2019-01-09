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



$(eval $(call gb_Executable_Executable,regsingleton))

$(eval $(call gb_Executable_add_linked_libs,regsingleton,\
	cppu \
	cppuhelper \
	sal \
	stl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Executable_set_private_extract_of_public_api,regsingleton,$(OUTDIR)/bin/udkapi.rdb,\
 	com.sun.star.uno.TypeClass \
 	com.sun.star.uno.XAggregation \
 	com.sun.star.uno.XWeak \
 	com.sun.star.uno.XComponentContext \
 	com.sun.star.lang.XTypeProvider \
 	com.sun.star.lang.XComponent \
 	com.sun.star.lang.XSingleServiceFactory \
 	com.sun.star.lang.XSingleComponentFactory \
 	com.sun.star.lang.XMultiServiceFactory \
 	com.sun.star.lang.XMultiComponentFactory \
 	com.sun.star.container.XHierarchicalNameAccess \
	com.sun.star.registry.XSimpleRegistry \
	com.sun.star.registry.XRegistryKey \
))

$(eval $(call gb_Executable_add_exception_objects,regsingleton,\
	cpputools/source/regsingleton/regsingleton \
))

# vim: set noet sw=4 ts=4:
