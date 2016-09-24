# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_JunitTest_JunitTest,jurt_remote))

$(eval $(call gb_JunitTest_use_jars,jurt_remote,\
    ridl \
))

$(eval $(call gb_JunitTest_use_jar_classset,jurt_remote,jurt))

$(eval $(call gb_JunitTest_add_sourcefiles,jurt_remote,\
    jurt/test/com/sun/star/lib/uno/environments/remote/JavaThreadPoolFactory_Test \
    jurt/test/com/sun/star/lib/uno/environments/remote/JobQueue_Test \
    jurt/test/com/sun/star/lib/uno/environments/remote/TestIWorkAt \
    jurt/test/com/sun/star/lib/uno/environments/remote/TestReceiver \
    jurt/test/com/sun/star/lib/uno/environments/remote/TestWorkAt \
    jurt/test/com/sun/star/lib/uno/environments/remote/ThreadId_Test \
    jurt/test/com/sun/star/lib/uno/environments/remote/ThreadPool_Test \
))

$(eval $(call gb_JunitTest_add_classes,jurt_remote,\
    com.sun.star.lib.uno.environments.remote.JavaThreadPoolFactory_Test \
    com.sun.star.lib.uno.environments.remote.JobQueue_Test \
    com.sun.star.lib.uno.environments.remote.ThreadId_Test \
    com.sun.star.lib.uno.environments.remote.ThreadPool_Test \
))

# vim:set noet sw=4 ts=4:
