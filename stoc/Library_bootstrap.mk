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
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
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

$(eval $(call gb_Library_use_internal_bootstrap_api,bootstrap,\
    bootstrap \
))

$(eval $(call gb_Library_use_libraries,bootstrap,\
    cppu \
    cppuhelper \
    reg \
    sal \
    salhelper \
    xmlreader \
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
    stoc/source/simpleregistry/textualservices \
    stoc/source/tdmanager/tdmgr \
    stoc/source/tdmanager/tdmgr_check \
    stoc/source/tdmanager/tdmgr_tdenumeration \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
