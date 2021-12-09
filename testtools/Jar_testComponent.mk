# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Jar_Jar,testComponent))

$(eval $(call gb_Jar_use_jars,testComponent,\
    libreoffice \
))

$(eval $(call gb_Jar_use_api,testComponent,\
    udkapi \
))

$(eval $(call gb_Jar_use_customtargets,testComponent,\
    testtools/bridgetest_javamaker \
))

$(eval $(call gb_Jar_set_packageroot,testComponent,com))

$(eval $(call gb_Jar_set_manifest,testComponent,$(SRCDIR)/testtools/com/sun/star/comp/bridge/manifest))

$(eval $(call gb_Jar_set_componentfile,testComponent,testtools/source/bridgetest/testComponent,NONE,uno_services))

$(eval $(call gb_Jar_add_packagedirs,testComponent,\
    $(call gb_CustomTarget_get_workdir,testtools/bridgetest_javamaker)/test \
))

$(eval $(call gb_Jar_add_sourcefiles,testComponent,\
	testtools/com/sun/star/comp/bridge/TestComponent \
	testtools/com/sun/star/comp/bridge/CurrentContextChecker \
	testtools/com/sun/star/comp/bridge/TestComponentMain \
))
