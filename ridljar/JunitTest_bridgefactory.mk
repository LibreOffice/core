# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_JunitTest_JunitTest,ridl_bridgefactory))

$(eval $(call gb_Jar_use_customtargets,ridl_bridgefactory, \
    ridljar/javamaker \
))

$(eval $(call gb_JunitTest_use_jar_classset,ridl_bridgefactory,libreoffice))

$(eval $(call gb_JunitTest_add_classpath,ridl_bridgefactory,$(gb_CustomTarget_workdir)/ridljar/javamaker))

$(eval $(call gb_JunitTest_add_sourcefiles,ridl_bridgefactory,\
    ridljar/test/com/sun/star/comp/bridgefactory/BridgeFactory_Test \
))

$(eval $(call gb_JunitTest_add_classes,ridl_bridgefactory,\
    com.sun.star.comp.bridgefactory.BridgeFactory_Test \
))

# vim:set noet sw=4 ts=4:
