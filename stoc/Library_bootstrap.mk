# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,bootstrap))

ifneq ($(debug),)

$(eval $(call gb_Library_add_defs,bootstrap,\
    $(if $(diag),\
        $(if $(filter $(diag),cache full),-D__CACHE_DIAGNOSE) \
        $(if $(filter-out $(diag),cache),-D__DIAGNOSE)) \
))

endif

$(eval $(call gb_Library_use_external,bootstrap,boost_headers))

$(eval $(call gb_Library_use_internal_bootstrap_api,bootstrap,\
    udkapi \
))

$(eval $(call gb_Library_use_libraries,bootstrap,\
    comphelper \
    cppu \
    cppuhelper \
    reg \
    sal \
    salhelper \
    tl \
))

$(eval $(call gb_Library_set_componentfile,bootstrap,stoc/util/bootstrap,ure/services))

$(eval $(call gb_Library_add_exception_objects,bootstrap,\
    stoc/source/defaultregistry/defaultregistry \
    stoc/source/implementationregistration/implreg \
    stoc/source/implementationregistration/mergekeys \
    stoc/source/loader/dllcomponentloader \
    stoc/source/security/access_controller \
    stoc/source/security/file_policy \
    stoc/source/security/permissions \
    stoc/source/servicemanager/servicemanager \
    stoc/source/simpleregistry/simpleregistry \
))

# vim:set noet sw=4 ts=4:
