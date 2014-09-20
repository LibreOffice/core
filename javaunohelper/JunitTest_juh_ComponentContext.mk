# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_JunitTest_JunitTest,juh_ComponentContext))

$(eval $(call gb_JunitTest_use_jars,juh_ComponentContext,\
    ridl \
    jurt \
))

$(eval $(call gb_JunitTest_use_jar_classset,juh_ComponentContext,ridljar))

$(eval $(call gb_JunitTest_add_sourcefiles,juh_ComponentContext,\
    javaunohelper/com/sun/star/comp/helper/Bootstrap \
    javaunohelper/com/sun/star/comp/helper/BootstrapException \
    javaunohelper/com/sun/star/comp/helper/ComponentContext \
    javaunohelper/com/sun/star/comp/helper/ComponentContextEntry \
    javaunohelper/test/com/sun/star/comp/helper/ComponentContext_Test \
))

$(eval $(call gb_JunitTest_add_classes,juh_ComponentContext,\
    com.sun.star.comp.helper.ComponentContext_Test \
))

# vim:set noet sw=4 ts=4:
