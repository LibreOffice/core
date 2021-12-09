# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,stocservices))

$(eval $(call gb_Library_use_udk_api,stocservices))

$(eval $(call gb_Library_use_libraries,stocservices,\
    cppu \
    cppuhelper \
    sal \
))

$(eval $(call gb_Library_use_external,stocservices,boost_headers))

$(eval $(call gb_Library_set_componentfile,stocservices,stoc/util/stocservices,ure/services))

$(eval $(call gb_Library_add_exception_objects,stocservices,\
    stoc/source/typeconv/convert \
    stoc/source/uriproc/ExternalUriReferenceTranslator \
    stoc/source/uriproc/UriReference \
    stoc/source/uriproc/UriReferenceFactory \
    stoc/source/uriproc/UriSchemeParser_vndDOTsunDOTstarDOTexpand \
    stoc/source/uriproc/UriSchemeParser_vndDOTsunDOTstarDOTscript \
    stoc/source/uriproc/VndSunStarPkgUrlReferenceFactory \
))

# vim:set noet sw=4 ts=4:
