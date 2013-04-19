# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_JunitTest_JunitTest,jurt_java_remote))

$(eval $(call gb_JunitTest_use_jars,jurt_java_remote,\
	OOoRunnerLight \
    ridl \
))

$(eval $(call gb_JunitTest_use_jar_classset,jurt_java_remote,jurt))

$(eval $(call gb_JunitTest_add_sourcefiles,jurt_java_remote,\
    jurt/test/com/sun/star/lib/uno/bridges/java_remote/BridgedObject_Test \
    jurt/test/com/sun/star/lib/uno/bridges/java_remote/ProxyFactory_Test \
    jurt/test/com/sun/star/lib/uno/bridges/java_remote/java_remote_bridge_Test \
))

$(eval $(call gb_JunitTest_add_classes,jurt_java_remote,\
    com.sun.star.lib.uno.bridges.java_remote.BridgedObject_Test \
    com.sun.star.lib.uno.bridges.java_remote.ProxyFactory_Test \
    com.sun.star.lib.uno.bridges.java_remote.java_remote_bridge_Test \
))

# vim:set noet sw=4 ts=4:
