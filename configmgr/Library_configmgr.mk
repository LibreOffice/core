#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,configmgr))

$(eval $(call gb_Library_set_precompiled_header,configmgr,configmgr/inc/pch/precompiled_configmgr))

$(eval $(call gb_Library_add_exception_objects,configmgr, \
    configmgr/source/access \
    configmgr/source/broadcaster \
    configmgr/source/childaccess \
    configmgr/source/components \
    configmgr/source/configurationprovider \
    configmgr/source/configurationregistry \
    configmgr/source/data \
    configmgr/source/defaultprovider \
    configmgr/source/groupnode \
    configmgr/source/localizedpropertynode \
    configmgr/source/localizedvaluenode \
    configmgr/source/lock \
    configmgr/source/modifications \
    configmgr/source/node \
    configmgr/source/nodemap \
    configmgr/source/parsemanager \
    configmgr/source/partial \
    configmgr/source/propertynode \
    configmgr/source/readonlyaccess \
    configmgr/source/readwriteaccess \
    configmgr/source/rootaccess \
    configmgr/source/rootnode \
    configmgr/source/setnode \
    configmgr/source/type \
    configmgr/source/update \
    configmgr/source/valueparser \
    configmgr/source/writemodfile \
    configmgr/source/xcdparser \
    configmgr/source/xcsparser \
    configmgr/source/xcuparser \
    configmgr/source/xmldata \
    $(if $(ENABLE_DCONF),configmgr/source/dconf) \
    $(if $(filter $(OS),WNT),configmgr/source/winreg) \
))

$(eval $(call gb_Library_use_externals,configmgr, \
    boost_headers \
    dconf \
))

$(eval $(call gb_Library_use_sdk_api,configmgr))

$(eval $(call gb_Library_use_libraries,configmgr, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    salhelper \
    tl \
    xmlreader \
    i18nlangtag \
))

$(eval $(call gb_Library_set_componentfile,configmgr,configmgr/source/configmgr,services))
