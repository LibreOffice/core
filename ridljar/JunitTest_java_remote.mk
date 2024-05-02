# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_JunitTest_JunitTest,ridl_java_remote))

$(eval $(call gb_JunitTest_use_jars,ridl_java_remote,\
    OOoRunner \
))

$(eval $(call gb_Jar_use_customtargets,ridl_java_remote, \
    ridljar/javamaker \
))

$(eval $(call gb_JunitTest_use_jar_classset,ridl_java_remote,libreoffice))

$(eval $(call gb_JunitTest_add_classpath,ridl_java_remote,$(gb_CustomTarget_workdir)/ridljar/javamaker))

$(eval $(call gb_JunitTest_add_sourcefiles,ridl_java_remote,\
    ridljar/test/com/sun/star/lib/uno/bridges/java_remote/BridgedObject_Test \
    ridljar/test/com/sun/star/lib/uno/bridges/java_remote/ProxyFactory_Test \
    ridljar/test/com/sun/star/lib/uno/bridges/java_remote/java_remote_bridge_Test \
))

$(eval $(call gb_JunitTest_add_classes,ridl_java_remote,\
    com.sun.star.lib.uno.bridges.java_remote.BridgedObject_Test \
    com.sun.star.lib.uno.bridges.java_remote.ProxyFactory_Test \
    com.sun.star.lib.uno.bridges.java_remote.java_remote_bridge_Test \
))

# vim:set noet sw=4 ts=4:
