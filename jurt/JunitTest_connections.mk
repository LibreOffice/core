# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_JunitTest_JunitTest,jurt_connections))

$(eval $(call gb_JunitTest_use_jars,jurt_connections,\
    ridl \
))

$(eval $(call gb_JunitTest_use_jar_classset,jurt_connections,jurt))

$(eval $(call gb_JunitTest_add_sourcefiles,jurt_connections,\
    jurt/test/com/sun/star/comp/connections/PipedConnection_Test \
))

$(eval $(call gb_JunitTest_add_classes,jurt_connections,\
    com.sun.star.comp.connections.PipedConnection_Test \
))

# vim:set noet sw=4 ts=4:
