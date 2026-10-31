# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,configmgr_unit))

$(eval $(call gb_CppunitTest_add_exception_objects,configmgr_unit, \
    configmgr/qa/unit/test \
))

$(eval $(call gb_CppunitTest_set_componentfile,configmgr_unit,configmgr/source/configmgr))

$(eval $(call gb_CppunitTest_use_library_objects,configmgr_unit,configmgr))

$(eval $(call gb_CppunitTest_use_sdk_api,configmgr_unit,))

$(eval $(call gb_CppunitTest_use_custom_headers,configmgr_unit,\
        officecfg/registry \
))

$(eval $(call gb_CppunitTest_set_include,configmgr_unit,\
    $$(INCLUDE) \
    -I$(SRCDIR)/configmgr/source \
))

$(eval $(call gb_CppunitTest_use_libraries,configmgr_unit, \
	    comphelper \
	    cppu \
	    cppuhelper \
	    sal \
	    salhelper \
	    test \
	    tl \
	    unotest \
	    xmlreader \
		i18nlangtag \
))

$(eval $(call gb_CppunitTest_use_ure,configmgr_unit))

$(eval $(call gb_CppunitTest_use_configuration,configmgr_unit))

# Collabora Online inserts the shared AutoText, Dictionary, Template and DocumentTheme paths
# into this layer at run time. Declare it here, with an ini file that does not exist, the way
# the product does, so testSharedOrganizationLayer can check the mechanism still works.
$(eval $(call gb_CppunitTest__use_configuration,configmgr_unit,sharedext,$(WORKDIR)/configmgr-no-such-layer.ini))

ifeq ($(OS),MACOSX)
# The managed preferences layer, read from the preferences of the test process (normally empty).
$(call gb_CppunitTest_get_target,configmgr_unit) : CONFIGURATION_LAYERS += macosprefs:

$(eval $(call gb_CppunitTest_use_system_darwin_frameworks,configmgr_unit,\
    CoreFoundation \
    Foundation \
))
endif

# A read-only user layer that finalizes a property, which is what "Final" in the winreg and
# macosprefs layers relies on.
$(call gb_CppunitTest_get_target,configmgr_unit) : CONFIGURATION_LAYERS += user:*$(call gb_Helper_make_url,$(SRCDIR)/configmgr/qa/unit/data/finalized.xcu)

$(eval $(call gb_CppunitTest_use_components,configmgr_unit,\
    i18npool/util/i18npool \
    i18npool/source/search/i18nsearch \
    sax/source/expatwrap/expwrap \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
))

$(eval $(call gb_CppunitTest_use_externals,configmgr_unit,\
    boost_headers \
    icu_headers \
))
