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



$(eval $(call gb_Library_Library,java_uno))

$(eval $(call gb_Library_add_precompiled_header,java_uno,$(SRCDIR)/bridges/inc/pch/precompiled_bridges_java_uno))

$(eval $(call gb_Library_set_include,java_uno,\
	$$(INCLUDE) \
	-I$(SRCDIR)/bridges/inc \
	-I$(SRCDIR)/bridges/inc/pch \
	-I$(SRCDIR)/bridges/source/jni_uno \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_set_private_extract_of_public_api,java_uno,$(OUTDIR)/bin/udkapi.rdb,\
	com.sun.star.uno.XInterface \
	com.sun.star.uno.TypeClass \
))

#$(eval $(call gb_Library_add_api,java_uno, \
#        udkapi \
#	offapi \
#))

$(eval $(call gb_Library_set_versionmap,java_uno,$(SRCDIR)/bridges/source/jni_uno/java_uno.map))

$(eval $(call gb_Library_add_linked_libs,java_uno,\
	cppu \
	jvmaccess \
	sal \
	salhelper \
	stl \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,java_uno,\
	bridges/source/jni_uno/jni_bridge \
	bridges/source/jni_uno/jni_data \
	bridges/source/jni_uno/jni_info \
	bridges/source/jni_uno/jni_java2uno \
	bridges/source/jni_uno/jni_uno2java \
	bridges/source/jni_uno/nativethreadpool \
))

# vim: set noet sw=4 ts=4:

