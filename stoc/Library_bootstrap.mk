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

$(eval $(call gb_Library_set_include,bootstrap,\
    -I$(SRCDIR)/stoc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_external,bootstrap,boost_headers))

$(eval $(call gb_Library_use_internal_bootstrap_api,bootstrap,\
    udkapi \
))

$(eval $(call gb_Library_use_libraries,bootstrap,\
    cppu \
    cppuhelper \
    reg \
    sal \
    salhelper \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_set_componentfile,bootstrap,stoc/util/bootstrap))

$(eval $(call gb_Library_add_exception_objects,bootstrap,\
    stoc/source/bootstrap/services \
    stoc/source/defaultregistry/defaultregistry \
    stoc/source/implementationregistration/implreg \
    stoc/source/implementationregistration/mergekeys \
    stoc/source/loader/dllcomponentloader \
    stoc/source/registry_tdprovider/functiondescription \
    stoc/source/registry_tdprovider/methoddescription \
    stoc/source/registry_tdprovider/rdbtdp_tdenumeration \
    stoc/source/registry_tdprovider/structtypedescription \
    stoc/source/registry_tdprovider/td \
    stoc/source/registry_tdprovider/tdcomp \
    stoc/source/registry_tdprovider/tdconsts \
    stoc/source/registry_tdprovider/tdef \
    stoc/source/registry_tdprovider/tdenum \
    stoc/source/registry_tdprovider/tdiface \
    stoc/source/registry_tdprovider/tdmodule \
    stoc/source/registry_tdprovider/tdprop \
    stoc/source/registry_tdprovider/tdprovider \
    stoc/source/registry_tdprovider/tdservice \
    stoc/source/registry_tdprovider/tdsingleton \
    stoc/source/security/access_controller \
    stoc/source/security/file_policy \
    stoc/source/security/permissions \
    stoc/source/servicemanager/servicemanager \
    stoc/source/simpleregistry/simpleregistry \
    stoc/source/tdmanager/tdmgr \
    stoc/source/tdmanager/tdmgr_check \
    stoc/source/tdmanager/tdmgr_tdenumeration \
))

# vim:set noet sw=4 ts=4:
