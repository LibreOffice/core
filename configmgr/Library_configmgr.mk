##
## Version: MPL 1.1 / GPLv3+ / LGPLv3+
##
## The contents of this file are subject to the Mozilla Public License Version
## 1.1 (the "License"); you may not use this file except in compliance with
## the License or as specified alternatively below. You may obtain a copy of
## the License at http://www.mozilla.org/MPL/
##
## Software distributed under the License is distributed on an "AS IS" basis,
## WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
## for the specific language governing rights and limitations under the
## License.
##
## Major Contributor(s):
## Copyright (C) 2011 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
## (initial developer)
##
## All Rights Reserved.
##
## For minor contributions see the git repository.
##
## Alternatively, the contents of this file may be used under the terms of
## either the GNU General Public License Version 3 or later (the "GPLv3+"), or
## the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
## in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
## instead of those above.
##

$(eval $(call gb_Library_Library,configmgr))

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
    configmgr/source/services \
    configmgr/source/setnode \
    configmgr/source/type \
    configmgr/source/update \
    configmgr/source/valueparser \
    configmgr/source/writemodfile \
    configmgr/source/xcdparser \
    configmgr/source/xcsparser \
    configmgr/source/xcuparser \
    configmgr/source/xmldata \
))

$(eval $(call gb_Library_use_sdk_api,configmgr))

$(eval $(call gb_Library_use_libraries,configmgr, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    salhelper \
    xmlreader \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_set_componentfile,configmgr,configmgr/source/configmgr))
