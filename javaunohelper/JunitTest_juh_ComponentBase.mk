# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_JunitTest_JunitTest,juh_ComponentBase))

$(eval $(call gb_JunitTest_set_defs,juh_ComponentBase,\
    $$(DEFS) \
    -XX:MaxGCPauseMillis=5 \
))


$(eval $(call gb_JunitTest_use_jars,juh_ComponentBase,\
    ridl \
    jurt \
))

$(eval $(call gb_JunitTest_use_jar_classset,juh_ComponentBase,ridljar))

$(eval $(call gb_JunitTest_add_sourcefiles,juh_ComponentBase,\
    javaunohelper/com/sun/star/lib/uno/helper/ComponentBase \
    javaunohelper/com/sun/star/lib/uno/helper/InterfaceContainer \
    javaunohelper/com/sun/star/lib/uno/helper/MultiTypeInterfaceContainer \
    javaunohelper/com/sun/star/lib/uno/helper/WeakAdapter \
    javaunohelper/com/sun/star/lib/uno/helper/WeakBase \
    javaunohelper/test/com/sun/star/lib/uno/helper/AWeakBase \
    javaunohelper/test/com/sun/star/lib/uno/helper/ComponentBase_Test \
    javaunohelper/test/com/sun/star/lib/uno/helper/ProxyProvider \
))

$(eval $(call gb_JunitTest_add_classes,juh_ComponentBase,\
    com.sun.star.lib.uno.helper.ComponentBase_Test \
))

# vim:set noet sw=4 ts=4:
