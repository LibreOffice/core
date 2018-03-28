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



$(eval $(call gb_Library_Library,vos3))

$(eval $(call gb_Library_add_package_headers,vos3,vos_inc))

$(eval $(call gb_Library_set_include,vos3,\
	$$(INCLUDE) \
	-I$(SRCDIR)/vos/inc \
	-I$(OUTDIR)/inc \
))

#$(eval $(call gb_Library_add_api,vos3, \
#        udkapi \
#	offapi \
#))

$(eval $(call gb_Library_add_defs,vos3,\
	-DVOS_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,vos3,\
	sal \
	stl \
	$(gb_STDLIBS) \
))

ifeq ($(GUI),WNT)
$(eval $(call gb_Library_add_linked_libs,vos3,\
	ws2_32 \
))
endif

$(eval $(call gb_Library_add_exception_objects,vos3,\
	vos/source/conditn \
	vos/source/module \
	vos/source/mutex \
	vos/source/object \
	vos/source/pipe \
	vos/source/process \
	vos/source/refernce \
	vos/source/security \
	vos/source/signal \
	vos/source/socket \
	vos/source/stream \
	vos/source/thread \
	vos/source/timer \
	vos/source/xception \
))

# vim: set noet sw=4 ts=4:

