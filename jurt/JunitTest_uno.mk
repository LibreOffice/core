# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_JunitTest_JunitTest,jurt_uno))

$(eval $(call gb_JunitTest_use_jars,jurt_uno,\
    OOoRunnerLight \
    ridl \
))

$(eval $(call gb_JunitTest_use_jar_classset,jurt_uno,jurt))

$(eval $(call gb_JunitTest_add_sourcefiles,jurt_uno,\
    jurt/test/com/sun/star/uno/AnyConverter_Test \
    jurt/test/com/sun/star/uno/UnoRuntime_EnvironmentTest \
    jurt/test/com/sun/star/uno/WeakReference_Test \
))

$(eval $(call gb_JunitTest_add_classes,jurt_uno,\
    com.sun.star.uno.AnyConverter_Test \
    com.sun.star.uno.UnoRuntime_EnvironmentTest \
    com.sun.star.uno.WeakReference_Test \
))

# vim:set noet sw=4 ts=4:
