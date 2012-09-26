# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,deployment))

$(eval $(call gb_Library_set_include,deployment,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc \
    -I$(SRCDIR)/desktop/source/deployment/inc \
    -I$(SRCDIR)/desktop/source/deployment/registry/inc \
))

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
    xmlscript \
	$(gb_UWINAPI) \
    $(gb_STDLIBS) \
))

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Library_use_externals,deployment,\
    berkeleydb \
))
endif

$(eval $(call gb_Library_set_componentfile,deployment,desktop/source/deployment/deployment))

$(eval $(call gb_Library_add_exception_objects,deployment,\
    desktop/source/deployment/dp_log \
    desktop/source/deployment/dp_services \
    desktop/source/deployment/dp_xml \
    desktop/source/deployment/manager/dp_activepackages \
    desktop/source/deployment/manager/dp_commandenvironments \
    desktop/source/deployment/manager/dp_extensionmanager \
    desktop/source/deployment/manager/dp_informationprovider \
    desktop/source/deployment/manager/dp_manager \
    desktop/source/deployment/manager/dp_managerfac \
    desktop/source/deployment/manager/dp_properties \
    desktop/source/deployment/registry/component/dp_compbackenddb \
    desktop/source/deployment/registry/component/dp_component \
    desktop/source/deployment/registry/configuration/dp_configuration \
    desktop/source/deployment/registry/configuration/dp_configurationbackenddb \
    desktop/source/deployment/registry/dp_backend \
    desktop/source/deployment/registry/dp_backenddb \
    desktop/source/deployment/registry/dp_registry \
    desktop/source/deployment/registry/executable/dp_executable \
    desktop/source/deployment/registry/executable/dp_executablebackenddb \
    desktop/source/deployment/registry/help/dp_help \
    desktop/source/deployment/registry/help/dp_helpbackenddb \
    desktop/source/deployment/registry/package/dp_extbackenddb \
    desktop/source/deployment/registry/package/dp_package \
    desktop/source/deployment/registry/script/dp_lib_container \
    desktop/source/deployment/registry/script/dp_script \
    desktop/source/deployment/registry/script/dp_scriptbackenddb \
    desktop/source/deployment/registry/sfwk/dp_parceldesc \
    desktop/source/deployment/registry/sfwk/dp_sfwk \
))

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))

$(eval $(call gb_Library_use_libraries,deployment,\
    helplinker \
))

$(eval $(call gb_Library_add_exception_objects,deployment,\
    desktop/source/deployment/dp_persmap \
))

endif

# vim: set ts=4 sw=4 et:
