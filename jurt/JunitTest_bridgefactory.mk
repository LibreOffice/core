# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_JunitTest_JunitTest,jurt_bridgefactory))

$(eval $(call gb_JunitTest_use_jars,jurt_bridgefactory,\
    ridl \
))

$(eval $(call gb_JunitTest_use_jar_classset,jurt_bridgefactory,jurt))

$(eval $(call gb_JunitTest_add_sourcefiles,jurt_bridgefactory,\
    jurt/test/com/sun/star/comp/bridgefactory/BridgeFactory_Test \
))

$(eval $(call gb_JunitTest_add_classes,jurt_bridgefactory,\
    com.sun.star.comp.bridgefactory.BridgeFactory_Test \
))

# vim:set noet sw=4 ts=4:
