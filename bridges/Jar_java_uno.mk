# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Jar_Jar,java_uno))

$(eval $(call gb_Jar_use_jars,java_uno,\
	jurt \
	ridl \
))

$(eval $(call gb_Jar_set_manifest,java_uno,$(SRCDIR)/bridges/source/jni_uno/java/com/sun/star/bridges/jni_uno/manifest))

$(eval $(call gb_Jar_set_packageroot,java_uno,com))

$(eval $(call gb_Jar_add_manifest_classpath,java_uno,\
	ridl.jar \
   	jurt.jar \
   	../../lib/ \
   	../bin/ \
))

$(eval $(call gb_Jar_add_sourcefiles,java_uno,\
	bridges/source/jni_uno/java/com/sun/star/bridges/jni_uno/JNI_info_holder \
	bridges/source/jni_uno/java/com/sun/star/bridges/jni_uno/JNI_proxy \
))

# vim: set noet sw=4 ts=4:
