# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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

# vim:set shiftwidth=4 softtabstop=4 expandtab:
