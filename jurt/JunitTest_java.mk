# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_JunitTest_JunitTest,jurt_java))

$(eval $(call gb_Jar_use_customtargets,jurt_java, \
    ridljar/javamaker \
))

$(eval $(call gb_JunitTest_use_jar_classset,jurt_java,ridl))

$(eval $(call gb_JunitTest_add_classpath,jurt_java,$(call gb_CustomTarget_get_workdir,ridljar/javamaker)))

$(eval $(call gb_JunitTest_add_sourcefiles,jurt_java,\
    jurt/test/com/sun/star/lib/uno/environments/java/java_environment_Test \
))

$(eval $(call gb_JunitTest_add_classes,jurt_java,\
    com.sun.star.lib.uno.environments.java.java_environment_Test \
))

# vim:set noet sw=4 ts=4:
