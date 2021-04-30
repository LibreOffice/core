# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,deployment))

$(eval $(call gb_Library_set_include,deployment,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc \
    -I$(SRCDIR)/desktop/source/deployment/inc \
    -I$(SRCDIR)/desktop/source/deployment/registry/inc \
))

$(eval $(call gb_Library_use_external,deployment,boost_headers))

$(eval $(call gb_Library_set_precompiled_header,deployment,desktop/inc/pch/precompiled_deployment))

$(eval $(call gb_Library_use_sdk_api,deployment))

$(eval $(call gb_Library_use_libraries,deployment,\
    comphelper \
    cppu \
    cppuhelper \
    deploymentmisc \
    sal \
    svl \
    tl \
    ucbhelper \
    utl \
    vcl \
    xmlscript \
    i18nlangtag \
))

$(eval $(call gb_Library_set_componentfile,deployment,desktop/source/deployment/deployment))

ifneq ($(ENABLE_WASM_STRIP_CLUCENE),TRUE)
$(eval $(call gb_Library_set_componentfile,deployment,desktop/source/deployment/deployment.extended))

$(eval $(call gb_Library_add_exception_objects,deployment,\
    desktop/source/deployment/registry/help/dp_help \
    desktop/source/deployment/registry/component/dp_component \
))
endif

$(eval $(call gb_Library_add_exception_objects,deployment,\
    desktop/source/deployment/dp_log \
    desktop/source/deployment/dp_persmap \
    desktop/source/deployment/dp_xml \
    desktop/source/deployment/manager/dp_activepackages \
    desktop/source/deployment/manager/dp_commandenvironments \
    desktop/source/deployment/manager/dp_extensionmanager \
    desktop/source/deployment/manager/dp_informationprovider \
    desktop/source/deployment/manager/dp_manager \
    desktop/source/deployment/manager/dp_managerfac \
    desktop/source/deployment/manager/dp_properties \
    desktop/source/deployment/registry/component/dp_compbackenddb \
    desktop/source/deployment/registry/configuration/dp_configuration \
    desktop/source/deployment/registry/configuration/dp_configurationbackenddb \
    desktop/source/deployment/registry/dp_backend \
    desktop/source/deployment/registry/dp_backenddb \
    desktop/source/deployment/registry/dp_registry \
    desktop/source/deployment/registry/executable/dp_executable \
    desktop/source/deployment/registry/executable/dp_executablebackenddb \
    desktop/source/deployment/registry/help/dp_helpbackenddb \
    desktop/source/deployment/registry/package/dp_extbackenddb \
    desktop/source/deployment/registry/package/dp_package \
    desktop/source/deployment/registry/script/dp_lib_container \
    desktop/source/deployment/registry/script/dp_script \
    desktop/source/deployment/registry/script/dp_scriptbackenddb \
    desktop/source/deployment/registry/sfwk/dp_parceldesc \
    desktop/source/deployment/registry/sfwk/dp_sfwk \
))

ifneq (,$(filter XMLHELP,$(BUILD_TYPE)))
$(eval $(call gb_Library_use_libraries,deployment,\
    helplinker \
))
endif

# vim: set ts=4 sw=4 et:
