# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_JunitTest_JunitTest,jurt_urp))

$(eval $(call gb_JunitTest_use_customtargets,jurt_urp,\
    jurt/test/com/sun/star/lib/uno/protocols/urp \
))

$(eval $(call gb_JunitTest_use_jars,jurt_urp,\
    ridl \
))

$(eval $(call gb_JunitTest_use_jar_classset,jurt_urp,jurt))

$(eval $(call gb_JunitTest_add_sourcefiles,jurt_urp,\
    jurt/test/com/sun/star/lib/uno/protocols/urp/Cache_Test \
    jurt/test/com/sun/star/lib/uno/protocols/urp/Marshaling_Test \
    jurt/test/com/sun/star/lib/uno/protocols/urp/Protocol_Test \
    jurt/test/com/sun/star/lib/uno/protocols/urp/TestBridge \
    jurt/test/com/sun/star/lib/uno/protocols/urp/TestObject \
))

$(eval $(call gb_JunitTest_add_classes,jurt_urp,\
    com.sun.star.lib.uno.protocols.urp.Cache_Test \
    com.sun.star.lib.uno.protocols.urp.Marshaling_Test \
    com.sun.star.lib.uno.protocols.urp.Protocol_Test \
))

# vim:set noet sw=4 ts=4:
