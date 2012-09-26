# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,java_uno))

ifeq ($(GUI)$(COM),WNTGCC)
ifeq ($(EXCEPTIONS),sjlj)
$(eval $(call gb_Library_add_defs,java_uno,\
	-DBROKEN_ALLOCA \
))
endif
endif

$(eval $(call gb_Library_use_internal_comprehensive_api,java_uno,\
	udkapi \
))

$(eval $(call gb_Library_set_include,java_uno,\
	-I$(SRCDIR)/bridges/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_libraries,java_uno,\
	cppu \
	jvmaccess \
	sal \
	salhelper \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,java_uno))

$(eval $(call gb_Library_add_exception_objects,java_uno,\
	bridges/source/jni_uno/jni_bridge \
	bridges/source/jni_uno/jni_data \
	bridges/source/jni_uno/jni_info \
	bridges/source/jni_uno/jni_java2uno \
	bridges/source/jni_uno/jni_uno2java \
	bridges/source/jni_uno/nativethreadpool \
))

# vim: set noet sw=4 ts=4:
