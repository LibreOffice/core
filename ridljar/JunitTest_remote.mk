# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_JunitTest_JunitTest,ridl_remote))

$(eval $(call gb_Jar_use_customtargets,ridl_remote, \
    ridljar/javamaker \
))

$(eval $(call gb_JunitTest_use_jar_classset,ridl_remote,libreoffice))

$(eval $(call gb_JunitTest_add_classpath,ridl_remote,$(gb_CustomTarget_workdir)/ridljar/javamaker))

$(eval $(call gb_JunitTest_add_sourcefiles,ridl_remote,\
    ridljar/test/com/sun/star/lib/uno/environments/remote/JavaThreadPoolFactory_Test \
    ridljar/test/com/sun/star/lib/uno/environments/remote/JobQueue_Test \
    ridljar/test/com/sun/star/lib/uno/environments/remote/TestIWorkAt \
    ridljar/test/com/sun/star/lib/uno/environments/remote/TestReceiver \
    ridljar/test/com/sun/star/lib/uno/environments/remote/TestWorkAt \
    ridljar/test/com/sun/star/lib/uno/environments/remote/ThreadId_Test \
    ridljar/test/com/sun/star/lib/uno/environments/remote/ThreadPool_Test \
))

$(eval $(call gb_JunitTest_add_classes,ridl_remote,\
    com.sun.star.lib.uno.environments.remote.JavaThreadPoolFactory_Test \
    com.sun.star.lib.uno.environments.remote.JobQueue_Test \
    com.sun.star.lib.uno.environments.remote.ThreadId_Test \
    com.sun.star.lib.uno.environments.remote.ThreadPool_Test \
))

# vim:set noet sw=4 ts=4:
