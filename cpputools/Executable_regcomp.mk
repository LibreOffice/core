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



$(eval $(call gb_Executable_Executable,regcomp))

$(eval $(call gb_Executable_add_linked_libs,regcomp,\
	cppu \
	cppuhelper \
	sal \
	stl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Executable_set_private_extract_of_public_api,regcomp,$(OUTDIR)/bin/udkapi.rdb,\
	com.sun.star.uno.TypeClass \
	com.sun.star.lang.XMultiServiceFactory \
	com.sun.star.lang.XSingleServiceFactory \
	com.sun.star.lang.XMultiComponentFactory \
	com.sun.star.lang.XSingleComponentFactory \
	com.sun.star.lang.XComponent \
	com.sun.star.container.XContentEnumerationAccess \
	com.sun.star.container.XSet \
	com.sun.star.loader.CannotActivateFactoryException \
	com.sun.star.registry.XImplementationRegistration2 \
))

$(eval $(call gb_Executable_add_exception_objects,regcomp,\
	cpputools/source/registercomponent/registercomponent \
))

# vim: set noet sw=4 ts=4:
