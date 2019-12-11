# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_JunitTest_JunitTest,juh))

$(eval $(call gb_JunitTest_set_defs,juh,\
    $$(DEFS) \
    -XX:MaxGCPauseMillis=50 \
))

$(eval $(call gb_JunitTest_use_jars,juh,\
    OOoRunner \
    ridl \
))

$(eval $(call gb_JunitTest_use_jar_classset,juh,ridljar))

$(eval $(call gb_JunitTest_add_sourcefiles,juh,\
    javaunohelper/com/sun/star/comp/helper/Bootstrap \
    javaunohelper/com/sun/star/comp/helper/BootstrapException \
    javaunohelper/com/sun/star/comp/helper/ComponentContext \
    javaunohelper/com/sun/star/comp/helper/ComponentContextEntry \
    javaunohelper/com/sun/star/lib/uno/helper/ComponentBase \
    javaunohelper/com/sun/star/lib/uno/helper/InterfaceContainer \
    javaunohelper/com/sun/star/lib/uno/helper/MultiTypeInterfaceContainer \
    javaunohelper/com/sun/star/lib/uno/helper/PropertySet \
    javaunohelper/com/sun/star/lib/uno/helper/WeakAdapter \
    javaunohelper/com/sun/star/lib/uno/helper/WeakBase \
    javaunohelper/test/com/sun/star/comp/helper/ComponentContext_Test \
    javaunohelper/test/com/sun/star/lib/uno/helper/AWeakBase \
    javaunohelper/test/com/sun/star/lib/uno/helper/ComponentBase_Test \
    javaunohelper/test/com/sun/star/lib/uno/helper/InterfaceContainer_Test \
    javaunohelper/test/com/sun/star/lib/uno/helper/MultiTypeInterfaceContainer_Test \
    javaunohelper/test/com/sun/star/lib/uno/helper/PropertySet_Test \
    javaunohelper/test/com/sun/star/lib/uno/helper/ProxyProvider \
    javaunohelper/test/com/sun/star/lib/uno/helper/WeakBase_Test \
))

$(eval $(call gb_JunitTest_add_classes,juh,\
    com.sun.star.comp.helper.ComponentContext_Test \
    com.sun.star.lib.uno.helper.ComponentBase_Test \
    com.sun.star.lib.uno.helper.InterfaceContainer_Test \
    com.sun.star.lib.uno.helper.MultiTypeInterfaceContainer_Test \
    com.sun.star.lib.uno.helper.PropertySet_Test \
    com.sun.star.lib.uno.helper.WeakBase_Test \
))

# vim:set noet sw=4 ts=4:
