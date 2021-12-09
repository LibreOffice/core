# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,namingservice))

$(eval $(call gb_Library_use_external,namingservice,boost_headers))

$(eval $(call gb_Library_use_udk_api,namingservice))

$(eval $(call gb_Library_use_libraries,namingservice,\
    cppu \
    cppuhelper \
    sal \
))

$(eval $(call gb_Library_set_componentfile,namingservice,stoc/source/namingservice/namingservice,ure/services))

$(eval $(call gb_Library_add_exception_objects,namingservice,\
    stoc/source/namingservice/namingservice \
))

# vim:set noet sw=4 ts=4:
