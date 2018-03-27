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



$(eval $(call gb_Executable_Executable,uno-skeletonmaker))

$(eval $(call gb_Executable_add_api,uno-skeletonmaker,\
	udkapi \
))

$(eval $(call gb_Executable_set_include,uno-skeletonmaker,\
	-I$(SRCDIR)/unodevtools/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Executable_add_linked_libs,uno-skeletonmaker,\
	cppu \
	cppuhelper \
	reg \
	sal \
	salhelper \
	stl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Executable_add_linked_static_libs,uno-skeletonmaker,\
	codemaker \
	commoncpp \
	commonjava \
))

$(eval $(call gb_Executable_add_exception_objects,uno-skeletonmaker,\
	unodevtools/source/unodevtools/options \
	unodevtools/source/unodevtools/typeblob \
	unodevtools/source/unodevtools/typemanager \
	unodevtools/source/skeletonmaker/cppcompskeleton \
	unodevtools/source/skeletonmaker/cpptypemaker \
	unodevtools/source/skeletonmaker/javacompskeleton \
	unodevtools/source/skeletonmaker/javatypemaker \
	unodevtools/source/skeletonmaker/skeletoncommon \
	unodevtools/source/skeletonmaker/skeletonmaker \
))

# vim: set noet sw=4 ts=4:
